/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <jni.h>

#include <android/looper.h>
#include <android/bitmap.h>

#include <android/androidinst.hxx>
#include <headless/svpdummies.hxx>
#include <headless/svpdata.hxx>
#include <osl/detail/android-bootstrap.h>
#include <rtl/strbuf.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <memory>
#include <unistd.h>

// Horrible hack
static int viewWidth = 1, viewHeight = 1;

void AndroidSalInstance::GetWorkArea(tools::Rectangle& rRect)
{
    rRect = tools::Rectangle( Point( 0, 0 ),
                       Size( viewWidth, viewHeight ) );
}

AndroidSalInstance *AndroidSalInstance::getInstance()
{
    if (!ImplGetSVData())
        return NULL;
    return static_cast<AndroidSalInstance *>(GetSalInstance());
}

AndroidSalInstance::AndroidSalInstance( std::unique_ptr<SalYieldMutex> pMutex )
    : SvpSalInstance( std::move(pMutex) )
{
    // FIXME: remove when uniPoll & runLoop is the only Android entry point.
    int res = (lo_get_javavm())->AttachCurrentThread(&m_pJNIEnv, NULL);
    LOGI("AttachCurrentThread res=%d env=%p", res, m_pJNIEnv);
}

// This is never called on Android until app exit.
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
    return SvpSalInstance::s_pDefaultInstance->HasUserEvents();
}

void AndroidSalInstance::updateMainThread()
{
    int res = (lo_get_javavm())->AttachCurrentThread(&m_pJNIEnv, NULL);
    LOGI("updateMainThread AttachCurrentThread res=%d env=%p", res, m_pJNIEnv);
    SvpSalInstance::updateMainThread();
}

void AndroidSalInstance::releaseMainThread()
{
    int res = (lo_get_javavm())->DetachCurrentThread();
    LOGI("releaseMainThread DetachCurrentThread res=%d", res);

    SvpSalInstance::releaseMainThread();
}

class AndroidSalSystem : public SvpSalSystem {
public:
    AndroidSalSystem() : SvpSalSystem() {}
    virtual ~AndroidSalSystem() {}
    virtual int ShowNativeDialog( const OUString& rTitle,
                                  const OUString& rMessage,
                                  const std::vector< OUString >& rButtons );
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
                     SalFrameStyleFlags  nSalFrameStyle )
        : SvpSalFrame(pInstance, pParent, nSalFrameStyle)
    {
        if (pParent == NULL && viewWidth > 1 && viewHeight > 1)
            SetPosSize(0, 0, viewWidth, viewHeight, SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT);
    }

    virtual void GetWorkArea(tools::Rectangle& rRect)
    {
        AndroidSalInstance::getInstance()->GetWorkArea( rRect );
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
        aStyleSet.SetRadioCheckFont( aFont );
        aStyleSet.SetPushButtonFont( aFont );
        aStyleSet.SetFieldFont( aFont );
        aStyleSet.SetIconFont( aFont );
        aStyleSet.SetTabFont( aFont );
        aStyleSet.SetGroupFont( aFont );

        rSettings.SetStyleSettings( aStyleSet );
    }
};

SalFrame *AndroidSalInstance::CreateChildFrame( SystemParentData* /*pParent*/, SalFrameStyleFlags nStyle )
{
    return new AndroidSalFrame( this, NULL, nStyle );
}

SalFrame *AndroidSalInstance::CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle )
{
    return new AndroidSalFrame( this, pParent, nStyle );
}

// This is our main entry point:
extern "C" SalInstance *create_SalInstance()
{
    LOGI("Android: create_SalInstance!");
    AndroidSalInstance* pInstance = new AndroidSalInstance( std::make_unique<SvpSalYieldMutex>() );
    new SvpSalData();
    return pInstance;
}

int AndroidSalSystem::ShowNativeDialog( const OUString& rTitle,
                                        const OUString& rMessage,
                                        const std::vector< OUString >& rButtons )
{
    (void)rButtons;
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
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(nullptr,
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  rMessage));
        xBox->set_title(rTitle);
        xBox->run();
    }
    else
        LOGE("VCL not initialized");
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
