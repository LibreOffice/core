/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <jni.h>

#include <android/log.h>
#include <android/looper.h>
#include <android/bitmap.h>

#include <android/androidinst.hxx>
#include <headless/svpdummies.hxx>
#include <generic/gendata.hxx>
#include <osl/detail/android-bootstrap.h>
#include <rtl/strbuf.hxx>
#include <basebmp/scanlineformats.hxx>
#include <touch/touch.h>

#define LOGTAG "LibreOffice/androidinst"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOGTAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOGTAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOGTAG, __VA_ARGS__))

static jclass appClass = 0;

// Horrible hack
static int viewWidth = 1, viewHeight = 1;

class AndroidSalData : public SalGenericData
{
public:
    AndroidSalData( SalInstance *pInstance ) : SalGenericData( SAL_DATA_ANDROID, pInstance ) {}
    virtual void ErrorTrapPush() {}
    virtual bool ErrorTrapPop( bool ) { return false; }
};

static void BlitFrameRegionToWindow(ANativeWindow_Buffer *pOutBuffer,
                                    const basebmp::BitmapDeviceSharedPtr& aDev,
                                    const ARect &rSrcRect,
                                    int nDestX, int nDestY)
{
    basebmp::RawMemorySharedArray aSrcData = aDev->getBuffer();
    basegfx::B2IVector aDevSize = aDev->getSize();
    sal_Int32 nStride = aDev->getScanlineStride();
    unsigned char *pSrc = aSrcData.get();

    // FIXME: do some cropping goodness on aSrcRect to ensure no overflows etc.
    ARect aSrcRect = rSrcRect;

    for (unsigned int y = 0; y < (unsigned int)(aSrcRect.bottom - aSrcRect.top); y++)
    {
        unsigned char *sp = ( pSrc + nStride * (aSrcRect.top + y) +
                              aSrcRect.left * 4 /* src pixel size */ );

        switch (pOutBuffer->format) {
        case WINDOW_FORMAT_RGBA_8888:
        case WINDOW_FORMAT_RGBX_8888:
        {
            unsigned char *dp = ( (unsigned char *)pOutBuffer->bits +
                                  pOutBuffer->stride * 4 * (y + nDestY) +
                                  nDestX * 4 /* dest pixel size */ );
            for (unsigned int x = 0; x < (unsigned int)(aSrcRect.right - aSrcRect.left); x++)
            {
                dp[x*4 + 0] = sp[x*4 + 0]; // R
                dp[x*4 + 1] = sp[x*4 + 1]; // G
                dp[x*4 + 2] = sp[x*4 + 2]; // B
                dp[x*4 + 3] = 255; // A
            }
            break;
        }
        case WINDOW_FORMAT_RGB_565:
        {
            unsigned char *dp = ( (unsigned char *)pOutBuffer->bits +
                                  pOutBuffer->stride * 2 * (y + nDestY) +
                                  nDestX * 2 /* dest pixel size */ );
            for (unsigned int x = 0; x < (unsigned int)(aSrcRect.right - aSrcRect.left); x++)
            {
                unsigned char b = sp[x*3 + 0]; // B
                unsigned char g = sp[x*3 + 1]; // G
                unsigned char r = sp[x*3 + 2]; // R
                dp[x*2 + 0] = (r & 0xf8) | (g >> 5);
                dp[x*2 + 1] = ((g & 0x1c) << 5) | ((b & 0xf8) >> 3);
            }
            break;
        }
        default:
            LOGI("unknown pixel format %d !", pOutBuffer->format);
            break;
        }
    }
}

void AndroidSalInstance::BlitFrameToWindow(ANativeWindow_Buffer *pOutBuffer,
                                           const basebmp::BitmapDeviceSharedPtr& aDev)
{
    basegfx::B2IVector aDevSize = aDev->getSize();
    ARect aWhole = { 0, 0, aDevSize.getX(), aDevSize.getY() };
    BlitFrameRegionToWindow(pOutBuffer, aDev, aWhole, 0, 0);
}

void AndroidSalInstance::RedrawWindows(ANativeWindow_Buffer *pBuffer)
{
    if (pBuffer->bits != NULL)
    {
        int i = 0;
        std::list< SalFrame* >::const_iterator it;
        for ( it = getFrames().begin(); it != getFrames().end(); i++, it++ )
        {
            SvpSalFrame *pFrame = static_cast<SvpSalFrame *>(*it);

            if (pFrame->IsVisible())
            {
                BlitFrameToWindow (pBuffer, pFrame->getDevice());
            }
        }
    }
    else
        LOGI("no buffer for locked window");
}

void AndroidSalInstance::damaged(AndroidSalFrame */* frame */)
{
    static bool beenHere = false;
    static jmethodID nCallbackDamaged = 0;

    // Check if we are running in an app that has registered for damage callbacks
    // static public void callbackDamaged();
    // Call the Java layer to post an invalidate if necessary

    if (appClass != 0 && !beenHere) {
        nCallbackDamaged = m_pJNIEnv->GetStaticMethodID(appClass, "callbackDamaged", "()V");
        if (nCallbackDamaged == 0)
            LOGE("Could not find the callbackDamaged method");
        beenHere = true;
    }

    if (appClass != 0 && nCallbackDamaged != 0)
        m_pJNIEnv->CallStaticVoidMethod(appClass, nCallbackDamaged);
}

void AndroidSalInstance::GetWorkArea( Rectangle& rRect )
{
    rRect = Rectangle( Point( 0, 0 ),
                       Size( viewWidth, viewHeight ) );
}

/*
 * Try too hard to get a frame, in the absence of anything better to do
 */
SalFrame *AndroidSalInstance::getFocusFrame() const
{
    SalFrame *pFocus = SvpSalFrame::GetFocusFrame();
    if (!pFocus) {
        LOGI("no focus frame, re-focusing first visible frame");
        const std::list< SalFrame* >& rFrames( getFrames() );
        for( std::list< SalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end(); ++it )
        {
            SvpSalFrame *pFrame = const_cast<SvpSalFrame*>(static_cast<const SvpSalFrame*>(*it));
            if( pFrame->IsVisible() )
            {
                pFrame->GetFocus();
                pFocus = pFrame;
                break;
            }
        }
    }
    return pFocus;
}

AndroidSalInstance *AndroidSalInstance::getInstance()
{
    if (!ImplGetSVData())
        return NULL;
    AndroidSalData *pData = static_cast<AndroidSalData *>(ImplGetSVData()->mpSalData);
    if (!pData)
        return NULL;
    return static_cast<AndroidSalInstance *>(pData->m_pInstance);
}

AndroidSalInstance::AndroidSalInstance( SalYieldMutex *pMutex )
    : SvpSalInstance( pMutex )
{
    int res = (lo_get_javavm())->AttachCurrentThread(&m_pJNIEnv, NULL);
    LOGI("AttachCurrentThread res=%d env=%p", res, m_pJNIEnv);
}

AndroidSalInstance::~AndroidSalInstance()
{
    LOGI("destroyed Android Sal Instance");
}

bool AndroidSalInstance::AnyInput( sal_uInt16 nType )
{
    if( (nType & VCL_INPUT_TIMER) != 0 )
        return CheckTimeout( false );

    // Unfortunately there is no way to check for a specific type of
    // input being queued. That information is too hidden, sigh.
    return SvpSalInstance::s_pDefaultInstance->PostedEventsInQueue();
}

class AndroidSalSystem : public SvpSalSystem {
public:
    AndroidSalSystem() : SvpSalSystem() {}
    virtual ~AndroidSalSystem() {}
    virtual int ShowNativeDialog( const OUString& rTitle,
                                  const OUString& rMessage,
                                  const std::list< OUString >& rButtons,
                                  int nDefButton );
};

SalSystem *AndroidSalInstance::CreateSalSystem()
{
    return new AndroidSalSystem();
}

class AndroidSalFrame : public SvpSalFrame
{
public:
    AndroidSalFrame( AndroidSalInstance *pInstance,
                     SalFrame           *pParent,
                     sal_uLong           nSalFrameStyle,
                     SystemParentData   *pSysParent )
        : SvpSalFrame( pInstance, pParent, nSalFrameStyle,
                       true, basebmp::FORMAT_THIRTYTWO_BIT_TC_MASK_RGBA,
                       pSysParent )
    {
        enableDamageTracker();
        if (pParent == NULL && viewWidth > 1 && viewHeight > 1)
            SetPosSize(0, 0, viewWidth, viewHeight, SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT);
    }

    virtual void GetWorkArea( Rectangle& rRect )
    {
        AndroidSalInstance::getInstance()->GetWorkArea( rRect );
    }

    virtual void damaged( const basegfx::B2IBox& rDamageRect)
    {
        if (rDamageRect.getWidth() <= 0 ||
            rDamageRect.getHeight() <= 0)
        {
            return;
        }
        AndroidSalInstance::getInstance()->damaged( this );
    }

    virtual void UpdateSettings( AllSettings &rSettings )
    {
        // Clobber the UI fonts
#if 0
        psp::FastPrintFontInfo aInfo;
        aInfo.m_aFamilyName = OUString( "Roboto" );
        aInfo.m_eItalic = ITALIC_NORMAL;
        aInfo.m_eWeight = WEIGHT_NORMAL;
        aInfo.m_eWidth = WIDTH_NORMAL;
        psp::PrintFontManager::get().matchFont( aInfo, rSettings.GetUILocale() );
#endif

        // FIXME: is 14 point enough ?
        Font aFont( OUString( "Roboto" ), Size( 0, 14 ) );

        StyleSettings aStyleSet = rSettings.GetStyleSettings();
        aStyleSet.SetAppFont( aFont );
        aStyleSet.SetHelpFont( aFont );
        aStyleSet.SetMenuFont( aFont );
        aStyleSet.SetToolFont( aFont );
        aStyleSet.SetLabelFont( aFont );
        aStyleSet.SetInfoFont( aFont );
        aStyleSet.SetRadioCheckFont( aFont );
        aStyleSet.SetPushButtonFont( aFont );
        aStyleSet.SetFieldFont( aFont );
        aStyleSet.SetIconFont( aFont );
        aStyleSet.SetGroupFont( aFont );

        rSettings.SetStyleSettings( aStyleSet );
    }
};

SalFrame *AndroidSalInstance::CreateChildFrame( SystemParentData* pParent, sal_uLong nStyle )
{
    return new AndroidSalFrame( this, NULL, nStyle, pParent );
}

SalFrame *AndroidSalInstance::CreateFrame( SalFrame* pParent, sal_uLong nStyle )
{
    return new AndroidSalFrame( this, pParent, nStyle, NULL );
}


// All the interesting stuff is slaved from the AndroidSalInstance
void InitSalData()   {}
void DeInitSalData() {}
void InitSalMain()   {}

void SalAbort( const OUString& rErrorText, bool bDumpCore )
{
    OUString aError( rErrorText );
    if( aError.isEmpty() )
        aError = "Unknown application error";
    LOGI("%s", OUStringToOString(rErrorText, osl_getThreadTextEncoding()).getStr() );

    LOGI("SalAbort: '%s'",
         OUStringToOString(aError, RTL_TEXTENCODING_ASCII_US).getStr());
    if( bDumpCore )
        abort();
    else
        _exit(1);
}

const OUString& SalGetDesktopEnvironment()
{
    static OUString aEnv( "android" );
    return aEnv;
}

SalData::SalData() :
    m_pInstance( 0 ),
    m_pPlugin( 0 ),
    m_pPIManager(0 )
{
}

SalData::~SalData()
{
}

// This is our main entry point:
SalInstance *CreateSalInstance()
{
    LOGI("Android: CreateSalInstance!");
    AndroidSalInstance* pInstance = new AndroidSalInstance( new SalYieldMutex() );
    new AndroidSalData( pInstance );
    pInstance->AcquireYieldMutex(1);
    return pInstance;
}

void DestroySalInstance( SalInstance *pInst )
{
    pInst->ReleaseYieldMutex();
    delete pInst;
}

#include <vcl/msgbox.hxx>

int AndroidSalSystem::ShowNativeDialog( const OUString& rTitle,
                                        const OUString& rMessage,
                                        const std::list< OUString >& rButtons,
                                        int nDefButton )
{
    (void)rButtons; (void)nDefButton;
    LOGI("LibreOffice native dialog '%s': '%s'",
         OUStringToOString(rTitle, RTL_TEXTENCODING_ASCII_US).getStr(),
         OUStringToOString(rMessage, RTL_TEXTENCODING_ASCII_US).getStr());
    LOGI("Dialog '%s': '%s'",
         OUStringToOString(rTitle, RTL_TEXTENCODING_ASCII_US).getStr(),
         OUStringToOString(rMessage, RTL_TEXTENCODING_ASCII_US).getStr());

    if (AndroidSalInstance::getInstance() != NULL)
    {
        // Does Android have a native dialog ? if not,. we have to do this ...

        // Of course it has. android.app.AlertDialog seems like a good
        // choice, it even has one, two or three buttons. Naturally,
        // it intended to be used from Java, so some verbose JNI
        // horror would be needed to use it directly here. Probably we
        // want some easier to use magic wrapper, hmm.

        ErrorBox aVclErrBox( NULL, WB_OK, rTitle );
        aVclErrBox.SetText( rMessage );
        aVclErrBox.Execute();
    }
    else
        LOGE("VCL not initialized");
    return 0;
}

// public static native void renderVCL(Bitmap bitmap);
extern "C" SAL_JNI_EXPORT void JNICALL
Java_org_libreoffice_android_AppSupport_renderVCL(JNIEnv *env,
                                                  jobject /* clazz */,
                                                  jobject bitmap)
{
    AndroidBitmapInfo  info;
    void*              pixels;
    int                ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGI("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGI("AndroidBitmap_lockPixels() failed ! error=%d", ret);
    }

/*
typedef struct ANativeWindow_Buffer {
    // The number of pixels that are show horizontally.
    int32_t width;

    // The number of pixels that are shown vertically.
    int32_t height;

    // The number of *pixels* that a line in the buffer takes in
    // memory.  This may be >= width.
    int32_t stride;

    // The format of the buffer.  One of WINDOW_FORMAT_*
    int32_t format;

    // The actual bits.
    void* bits;

    // Do not touch.
    uint32_t reserved[6];
} ANativeWindow_Buffer;
*/

    ANativeWindow_Buffer dummyOut; // look like a window for now ...
    dummyOut.width = info.width;
    dummyOut.height = info.height;
    dummyOut.stride = info.stride / 4; // sigh !
    dummyOut.format = info.format;
    dummyOut.bits = pixels;
    AndroidSalInstance::getInstance()->RedrawWindows (&dummyOut);

    AndroidBitmap_unlockPixels(env, bitmap);
}

// public static native void registerForDamageCallback(Class destinationClass);
extern "C" SAL_JNI_EXPORT void JNICALL
Java_org_libreoffice_android_AppSupport_registerForDamageCallback(JNIEnv * env,
                                                                  jobject /* clazz */,
                                                                  jclass destinationClass)
{
    appClass = (jclass) env->NewGlobalRef(destinationClass);
}

// public static native void setViewSize(int width, int height);
extern "C" SAL_JNI_EXPORT void JNICALL
Java_org_libreoffice_android_AppSupport_setViewSize(JNIEnv * /* env */,
                                                    jobject /* clazz */,
                                                    jint width,
                                                    jint height)
{
    // Horrible
    viewWidth = width;
    viewHeight = height;
}

// public static native void key(char c);
extern "C" SAL_JNI_EXPORT void JNICALL
Java_org_libreoffice_android_AppSupport_key(JNIEnv * /* env */,
                                            jobject /* clazz */,
                                            jchar c)
{
    SalFrame *pFocus = AndroidSalInstance::getInstance()->getFocusFrame();
    if (pFocus) {
        KeyEvent aEvent(c, c, 0);
        Application::PostKeyEvent(VCLEVENT_WINDOW_KEYINPUT, pFocus->GetWindow(), &aEvent);
        Application::PostKeyEvent(VCLEVENT_WINDOW_KEYUP, pFocus->GetWindow(), &aEvent);
    }
    else
        LOGW("No focused frame to emit event on");
}

// public static native void touch(int action, int x, int y);
extern "C" SAL_JNI_EXPORT void JNICALL
Java_org_libreoffice_android_AppSupport_touch(JNIEnv * /* env */,
                                              jobject /* clazz */,
                                              jint action,
                                              jint x,
                                              jint y)
{
    SalFrame *pFocus = AndroidSalInstance::getInstance()->getFocusFrame();
    if (pFocus) {
        MouseEvent aEvent;
        sal_uLong nEvent;

        switch (action) {
        case AMOTION_EVENT_ACTION_DOWN:
            aEvent = MouseEvent(Point(x, y), 1, MOUSE_SIMPLECLICK, MOUSE_LEFT);
            nEvent = VCLEVENT_WINDOW_MOUSEBUTTONDOWN;
            break;
        case AMOTION_EVENT_ACTION_UP:
            aEvent = MouseEvent(Point(x, y), 1, MOUSE_SIMPLECLICK, MOUSE_LEFT);
            nEvent = VCLEVENT_WINDOW_MOUSEBUTTONUP;
            break;
        case AMOTION_EVENT_ACTION_MOVE:
            aEvent = MouseEvent(Point(x, y), 1, MOUSE_SIMPLEMOVE, MOUSE_LEFT);
            nEvent = VCLEVENT_WINDOW_MOUSEMOVE;
            break;
        default:
            LOGE("AppSupport.touch: Invalid action %d", action);
            return;
        }
        Application::PostMouseEvent(nEvent, pFocus->GetWindow(), &aEvent);
    }
    else
        LOGW("No focused frame to emit event on");
}

// public static native void zoom(float scale, int x, int y);
extern "C" SAL_JNI_EXPORT void JNICALL
Java_org_libreoffice_android_AppSupport_zoom(JNIEnv * /* env */,
                                             jobject /* clazz */,
                                             jfloat scale,
                                             jint x,
                                             jint y)
{
    SalFrame *pFocus = AndroidSalInstance::getInstance()->getFocusFrame();
    if (pFocus) {
        SAL_INFO( "vcl.androidinst", "zoom: " << scale << "@(" << x << "," << y << ")" );
        ZoomEvent aEvent( Point( x, y ), scale);
        Application::PostZoomEvent(VCLEVENT_WINDOW_ZOOM, pFocus->GetWindow(), &aEvent);
    }
    else
        LOGW("No focused frame to emit event on");
}

// public static native void scroll(int x, int y);
extern "C" SAL_JNI_EXPORT void JNICALL
Java_org_libreoffice_android_AppSupport_scroll(JNIEnv * /* env */,
                                               jobject /* clazz */,
                                               jint x,
                                               jint y)
{
    SalFrame *pFocus = AndroidSalInstance::getInstance()->getFocusFrame();
    if (pFocus) {
        SAL_INFO( "vcl.androidinst", "scroll: " << "(" << x << "," << y << ")" );
        ScrollEvent aEvent( x, y );
        Application::PostScrollEvent(VCLEVENT_WINDOW_SCROLL, pFocus->GetWindow(), &aEvent);
    }
    else
        LOGW("No focused frame to emit event on");
}

extern "C" void
lo_show_keyboard()
{
}

extern "C" void
lo_hide_keyboard()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
