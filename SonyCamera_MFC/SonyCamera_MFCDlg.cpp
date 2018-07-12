
// SonyCamera_MFCDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SonyCamera_MFC.h"
#include "SonyCamera_MFCDlg.h"
#include "afxdialogex.h"

#include <iostream>
#include <XCCamAPI.h>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// #define _IMAGECALLBACK_

#ifdef _IMAGECALLBACK_
extern "C" VOID CALLBACK ImageDataRcv(HCAMERA, XCCAM_IMAGE*, pXCCAM_IMAGEDATAINFO, PVOID);
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSonyCamera_MFCDlg 对话框



CSonyCamera_MFCDlg::CSonyCamera_MFCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSonyCamera_MFCDlg::IDD, pParent),
	isShowingImage(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSonyCamera_MFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSonyCamera_MFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPENCAM, &CSonyCamera_MFCDlg::OnBnClickedButtonOpencam)
	ON_BN_CLICKED(IDC_BUTTON_SHOWIMG, &CSonyCamera_MFCDlg::OnBnClickedButtonShowimg)
	ON_BN_CLICKED(IDC_BUTTON_STOPSHOWIMG, &CSonyCamera_MFCDlg::OnBnClickedButtonStopshowimg)
	ON_BN_CLICKED(IDC_BUTTON_CLOSECAM, &CSonyCamera_MFCDlg::OnBnClickedButtonClosecam)
	ON_BN_CLICKED(IDC_BUTTON_TRIGGER, &CSonyCamera_MFCDlg::OnBnClickedButtonTrigger)
END_MESSAGE_MAP()


// CSonyCamera_MFCDlg 消息处理程序

BOOL CSonyCamera_MFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSonyCamera_MFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSonyCamera_MFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSonyCamera_MFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




//---------------------------------------------------------------------------
// System Call back
// When bus reset occurred, when the numbers of camera that I can open changed
// on a bus, when system abnormality occurred, it is called.
extern "C" VOID CALLBACK SystemFunc(STATUS_SYSTEMCODE Status, PVOID Countext)
{
	switch (Status)
	{
	case STATUSXCCAM_DEVICECHANGE:				// Processing of Device Change
		AfxMessageBox(_T("Device Change Event"));
		break;

	case STATUSXCCAM_POWERUP:					// Processing of PowerUP
		AfxMessageBox(_T("PowerUP Event"));
		break;
	}
}


void CSonyCamera_MFCDlg::OnBnClickedButtonOpencam()
{
	// TODO:  在此添加控件通知处理程序代码
	m_EndEvent = CreateEvent(NULL, true, false, NULL);
	m_RcvTermEvent = CreateEvent(NULL, true, false, NULL);

	XCCAM_SetStructVersion(XCCAM_LIBRARY_STRUCT_VERSION);
	XCCAM_SetCallBack(NULL, SystemFunc);

	// 打开相机
	if (!XCCAM_Open(NULL, &hCamera))
	{
		MessageBox(_T("Camera Not found"));
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		return;
	}

	// 获取相机信息
	// Get the camera model name
	XCCAM_DEVINFO		cinfo;
	if (!XCCAM_CameraInfo(hCamera, &cinfo))
	{
		MessageBox(_T("Get Camera Info Error"));
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		XCCAM_Close(hCamera);
		hCamera = 0;
		return;
	}
	// CString		string;
	switch (cinfo.DeviceType)
	{
	case XCCAM_GIGECAMERA:
		cout << "Free Run Sample   Model=" << cinfo.u.GigEDev.ModelName << cinfo.UID << endl;
		// string.Format(_T("Free Run Sample   Model=%S 0x%016I64X"), cinfo.u.GigEDev.ModelName, cinfo.UID);
		break;

	case XCCAM_USBCAMERA:
		cout << "Free Run Sample   Model=" << cinfo.u.UsbDev.ModelName << cinfo.UID << endl;
		// string.Format(_T("Free Run Sample   Model=%S 0x%016I64X"), cinfo.u.UsbDev.ModelName, cinfo.UID);
		break;
	}

	// 获取相机特性句柄
	
	if (!XCCAM_GetFeatureHandle(hCamera, (HNodeMap*)&m_hFeature))
	{
		MessageBox(_T("Get FeatureHandle Error"));
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		XCCAM_Close(hCamera);
		hCamera = 0;
		return;
	}

	// 设置相机采集模式
	if (!XCCAM_SetFeatureEnumeration(m_hFeature, "AcquisitionMode", "Continuous"))
	{
		MessageBox(_T("Set Feature Enumeration Error"));
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		XCCAM_Close(hCamera);
		hCamera = 0;
		return;
	}

	BOOL ret = FALSE;
	//char buffer[100];

	//XCCAM_FEATUREINFO TriggerModeInfo, TriggerSourceInfo, TriggerSoftwareInfo, \
	//	TriggerDelayInfo, GainAutoInfo, ExposureAutoInfo, ExposureTimeInfo, GainRawInfo;

	//ret = XCCAM_FeatureInfo(m_hFeature, "TriggerMode", &TriggerModeInfo);
	//ret = XCCAM_FeatureInfo(m_hFeature, "TriggerSource", &TriggerSourceInfo);
	//ret = XCCAM_FeatureInfo(m_hFeature, "TriggerSoftware", &TriggerSoftwareInfo);
	//ret = XCCAM_FeatureInfo(m_hFeature, "TriggerDelay", &TriggerDelayInfo);
	//ret = XCCAM_FeatureInfo(m_hFeature, "GainAuto", &GainAutoInfo);
	//ret = XCCAM_FeatureInfo(m_hFeature, "ExposureAuto", &ExposureAutoInfo);
	//ret = XCCAM_FeatureInfo(m_hFeature, "ExposureTime", &ExposureTimeInfo);
	//ret = XCCAM_FeatureInfo(m_hFeature, "GainRaw", &GainRawInfo);


	//// 设置相机的触发模式和触发延迟
	//ret = XCCAM_SetFeatureEnumeration(m_hFeature, "TriggerMode", "On");
	//ret = XCCAM_SetFeatureEnumeration(m_hFeature, "TriggerSource", "Software");
	//ret = XCCAM_SetFeatureFloat(m_hFeature, "TriggerDelay", 0);	// 2s

	//// 设置增益和曝光
	//ret = XCCAM_SetFeatureEnumeration(m_hFeature, "GainAuto", "Off");
	//ret = XCCAM_SetFeatureInteger(m_hFeature, "GainRaw", 100);	// -38 ~ 442
	//ret = XCCAM_SetFeatureEnumeration(m_hFeature, "ExposureAuto", "Off");
	//ret = XCCAM_SetFeatureFloat(m_hFeature, "ExposureTime", 60000.0);

	//// 设置相机的其他参数
	//INT64 WidthSize, HeightSize, OffSetX, OffSetY;

	//WidthSize = 640;
	//HeightSize = 480;
	//OffSetX = 0;
	//OffSetY = 0;

	//XCCAM_SetFeatureInteger(m_hFeature, "OffsetX", OffSetX);
	//XCCAM_SetFeatureInteger(m_hFeature, "OffsetY", OffSetY);

	//XCCAM_FEATUREINFO WInfo, HInfo, OXInfo, OYInfo;

	//XCCAM_FeatureInfo(m_hFeature, "Width", &WInfo);
	//XCCAM_FeatureInfo(m_hFeature, "Height", &HInfo);
	//XCCAM_FeatureInfo(m_hFeature, "OffsetX", &OXInfo);
	//XCCAM_FeatureInfo(m_hFeature, "OffsetY", &OYInfo);

	//if (WInfo.u.IntReg.MaxValue < WidthSize)
	//	WidthSize = WInfo.u.IntReg.MaxValue;
	//OffSetX = (WInfo.u.IntReg.MaxValue - WidthSize) / 2;
	//if (HInfo.u.IntReg.MaxValue < HeightSize)
	//	HeightSize = HInfo.u.IntReg.MaxValue;
	//OffSetY = (HInfo.u.IntReg.MaxValue - HeightSize) / 2;

	//XCCAM_SetFeatureInteger(m_hFeature, "Width", WidthSize);
	//XCCAM_SetFeatureInteger(m_hFeature, "Height", HeightSize);
	//XCCAM_SetFeatureInteger(m_hFeature, "OffsetX", OffSetX);
	//XCCAM_SetFeatureInteger(m_hFeature, "OffsetY", OffSetY);

	// 设置颜色转换模式?
	XCCAM_COLORCONVMODE Mode = {};
	Mode.StoreMode = XCCAM_MEMmode;
	Mode.DIBMode = XCCAM_DIB32;
	Mode.ShiftID = XCCAM_SFTAUTO;
	Mode.Parallel_Thread = 4;
	Mode.BayerRevision_G = FALSE;
	if (!XCCAM_SetConvMode(hCamera, &Mode, NULL))
	{
		MessageBox(_T("Conver Mode Set Error"));
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		XCCAM_Close(hCamera);
		hCamera = 0;
		return;
	}

	// 申请相机的资源？
	if (!XCCAM_ResourceAlloc(hCamera))
	{
		MessageBox(_T("Resource Alloc Error"));
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		::XCCAM_Close(hCamera);
		hCamera = 0;
		return;
	}

	// 为获取的图像申请内存空间
	XCCAM_ImageAlloc(hCamera, &pImage);

	// 获取BMP图像信息
	ULONG Len = 0;
	if (!XCCAM_GetBMPINFO(hCamera, NULL, &Len, false))
	{
		MessageBox(_T("Get BMPINFO Error"));
		XCCAM_ResourceRelease(hCamera);
		XCCAM_ImageFreeAll(hCamera);
		XCCAM_Close(hCamera);
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		hCamera = 0;
		return;
	}
	m_pBitInfo = (PBITMAPINFO)new BYTE[Len];
	if (!XCCAM_GetBMPINFO(hCamera, m_pBitInfo, &Len, false))
	{
		MessageBox(_T("Get BMPINFO Error"));
		XCCAM_ResourceRelease(hCamera);
		XCCAM_ImageFreeAll(hCamera);
		XCCAM_Close(hCamera);
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		hCamera = 0;
		return;
	}

	// 为转换后的图像申请内存空间
	int height = m_pBitInfo->bmiHeader.biHeight;
	int width = m_pBitInfo->bmiHeader.biWidth;
	int channels = m_pBitInfo->bmiHeader.biBitCount / 8;
	cv_image.create(height, width, CV_8UC4);
	m_pImage = NULL;
	if (cv_image.isContinuous()) {
		m_pImage = cv_image.data;
	}
	else {
		MessageBox(_T("opencv mat is not continuous!"));
		XCCAM_ResourceRelease(hCamera);
		XCCAM_ImageFreeAll(hCamera);
		delete[] m_pBitInfo;
		XCCAM_Close(hCamera);
		CloseHandle(m_EndEvent);
		CloseHandle(m_RcvTermEvent);
		hCamera = 0;
		return;
	}

	MessageBox(_T("Camera Open Successfully!"));

	return;
}


// 定义图像回调函数
#ifdef _IMAGECALLBACK_
extern "C"
VOID  CALLBACK ImageDataRcv(HCAMERA hCamera, 
							XCCAM_IMAGE* pImage, 
							pXCCAM_IMAGEDATAINFO pImageInfo,
							PVOID Context )
{
	CSonyCamera_MFCDlg* pMp = (CSonyCamera_MFCDlg*)Context;
	/* Color-converts the acquired image data with the color conversion API 
	and convert it to data for display. */
	XCCAM_ConvExec(hCamera, pImage, pMp->m_pImage);
	cv::imshow("figure", pMp->cv_image);
	cv::waitKey(20);
}
#endif

// 定义获取图像的线程的函数
UINT AFX_CDECL ImageAcquisition(LPVOID Countext)
{
	CSonyCamera_MFCDlg *pMp = (CSonyCamera_MFCDlg *)Countext;

	// 创建用于显示图像的窗口
	cv::namedWindow("figure");

	LARGE_INTEGER li;
	LONGLONG start, end, freq;

	QueryPerformanceFrequency(&li);
	freq = li.QuadPart;

	while (1){
		// 请求退出采集图像事件句柄，若请求到，则退出采集过程
		if (WaitForSingleObject(pMp->m_EndEvent, 0) == WAIT_OBJECT_0){
			cv::destroyWindow("figure");
			ResetEvent(pMp->m_EndEvent);
			break;
		}

		QueryPerformanceCounter(&li);
		start = li.QuadPart;

		XCCAM_ImageReq(pMp->hCamera, pMp->pImage);						// 获取图像
		XCCAM_ImageComplete(pMp->hCamera, pMp->pImage, -1, NULL);		// 等待图像获取完成，超时参数设置为-1，无限等待
		
		QueryPerformanceCounter(&li);
		end = li.QuadPart;
		int useTime = (int)((end - start) * 1000 / freq);
		std::cout << "acqu time: " << useTime << "ms" << std::endl;

		XCCAM_ConvExec(pMp->hCamera, pMp->pImage, pMp->m_pImage);		// 将获取到的图像转换成用于显示的格式
		cv::imshow("figure", pMp->cv_image);
		cv::waitKey(20);
	}

	// 设置采集终止事件，表示采集进程终止
	SetEvent(pMp->m_RcvTermEvent);
	return 0;
}

void CSonyCamera_MFCDlg::OnBnClickedButtonShowimg()
{
	// TODO:  在此添加控件通知处理程序代码
	if (isShowingImage == TRUE){
		MessageBox(_T("Image is being showed!"));
		return;
	}

#ifndef _IMAGECALLBACK_
	// 开始图像采集
	XCCAM_ImageStart(hCamera);

	// 创建采集显示线程
	AfxBeginThread(ImageAcquisition, this);

#else
	cv::namedWindow("figure");
	XCCAM_SetImageCallBack(hCamera, this, ImageDataRcv, 1, FALSE);
	XCCAM_ImageStart(hCamera);
#endif

	isShowingImage = TRUE;
	return;
}


void CSonyCamera_MFCDlg::OnBnClickedButtonStopshowimg()
{
	// TODO:  在此添加控件通知处理程序代码
	if (isShowingImage == FALSE){
		MessageBox(_T("Image is not being showed!"));
		return;
	}

#ifdef _IMAGECALLBACK_
	XCCAM_ImageStop(hCamera);
	cv::destroyWindow("figure");
	XCCAM_SetImageCallBack(hCamera, NULL, NULL, 0, FALSE);
#else
	SetEvent(m_EndEvent);
	XCCAM_ImageReqAbortAll(hCamera);
	WaitForSingleObject(m_RcvTermEvent, INFINITE);
	ResetEvent(m_RcvTermEvent);
	XCCAM_ImageStop(hCamera);
#endif

	isShowingImage = FALSE;
	return;
}


void CSonyCamera_MFCDlg::OnBnClickedButtonClosecam()
{
	// TODO:  在此添加控件通知处理程序代码
	if (isShowingImage == TRUE){
		// 如果正在显示图像，先停止显示
#ifdef _IMAGECALLBACK_
		XCCAM_ImageStop(hCamera);
		cv::destroyWindow("figure");
		XCCAM_SetImageCallBack(hCamera, NULL, NULL, 0, FALSE);
#else
		SetEvent(m_EndEvent);
		XCCAM_ImageReqAbortAll(hCamera);
		WaitForSingleObject(m_RcvTermEvent, INFINITE);
		ResetEvent(m_RcvTermEvent);
		XCCAM_ImageStop(hCamera);
#endif
	}

	// 释放相关资源
	XCCAM_ResourceRelease(hCamera);
	XCCAM_ImageFreeAll(hCamera);
	delete[] m_pBitInfo;
	XCCAM_Close(hCamera);
	CloseHandle(m_EndEvent);
	CloseHandle(m_RcvTermEvent);

	isShowingImage = FALSE;
	return;
}


void CSonyCamera_MFCDlg::OnBnClickedButtonTrigger()
{
	// TODO:  在此添加控件通知处理程序代码

	XCCAM_FeatureCommand(m_hFeature, "TriggerSoftware");

	return;
}
