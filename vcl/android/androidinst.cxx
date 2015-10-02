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
#include <vcl/settings.hxx>

#define LOGTAG "LibreOffice/androidinst"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOGTAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOGTAG, __VA_ARGS__))

static jclass appClass = 0;

// Horrible hack
static int viewWidth = 1, viewHeight = 1;

class AndroidSalData : public SalGenericData
{
public:
    explicit AndroidSalData( SalInstance *pInstance ) : SalGenericData( SAL_DATA_ANDROID, pInstance ) {}
    virtual void ErrorTrapPush() {}
    virtual bool ErrorTrapPop( bool ) { return false; }
};

void AndroidSalInstance::damaged(AndroidSalFrame* /* frame */)
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
    int res = (lo_get_javavm())->DetachCurrentThread();
    LOGI("DetachCurrentThread res=%d", res);
    LOGI("destroyed Android Sal Instance");
}

bool AndroidSalInstance::AnyInput( VclInputFlags nType )
{
    if( nType & VclInputFlags::TIMER )
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
                     SalFrameStyleFlags  nSalFrameStyle,
                     SystemParentData   *pSysParent )
        : SvpSalFrame( pInstance, pParent, nSalFrameStyle,
                       true, basebmp::Format::ThirtyTwoBitTcMaskRGBA,
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
        aInfo.m_aFamilyName = "Roboto";
        aInfo.m_eItalic = ITALIC_NORMAL;
        aInfo.m_eWeight = WEIGHT_NORMAL;
        aInfo.m_eWidth = WIDTH_NORMAL;
        psp::PrintFontManager::get().matchFont( aInfo, rSettings.GetUILocale() );
#endif

        // FIXME: is 14 point enough ?
        vcl::Font aFont( OUString( "Roboto" ), Size( 0, 14 ) );

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
        aStyleSet.SetTabFont( aFont );
        aStyleSet.SetGroupFont( aFont );

        rSettings.SetStyleSettings( aStyleSet );
    }
};

SalFrame *AndroidSalInstance::CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle )
{
    return new AndroidSalFrame( this, NULL, nStyle, pParent );
}

SalFrame *AndroidSalInstance::CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle )
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

#include <vcl/layout.hxx>

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

        MessageDialog aVclErrBox(NULL, rMessage);
        aVclErrBox.SetText(rTitle);
        aVclErrBox.Execute();
    }
    else
        LOGE("VCL not initialized");
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
