/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Novell, Inc.
 *   Michael Meeks <michael.meeks@suse.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include <android/androidinst.hxx>
#include <headless/svpdummies.hxx>
#include <generic/gendata.hxx>
#include <jni.h>
#include <android/log.h>
#include <osl/detail/android-bootstrap.h>
#include <rtl/strbuf.hxx>
#include <basebmp/scanlineformats.hxx>

class AndroidSalData : public SalGenericData
{
public:
    AndroidSalData( SalInstance *pInstance ) : SalGenericData( SAL_DATA_ANDROID, pInstance ) {}
    virtual void ErrorTrapPush() {}
    virtual bool ErrorTrapPop( bool ) { return false; }
};

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
    fprintf (stderr, "created Android Sal Instance (thread: %d)\n",
             (int)pthread_self());
}

AndroidSalInstance::~AndroidSalInstance()
{
    fprintf (stderr, "destroyed Android Sal Instance\n");
}

void AndroidSalInstance::Wakeup()
{
    fprintf (stderr, "Wakeup alooper\n");
    fprintf (stderr, "busted - no global looper\n");
}

void AndroidSalInstance::DoReleaseYield (int /* nTimeoutMS */)
{
    // Presumably this should never be called at all as we don't do
    // NativeActivity-based apps any more?
    static bool beenhere = false;
    if (!beenhere)
    {
        fprintf (stderr, "**** Huh, %s called in non-NativeActivity app\n", __FUNCTION__);
        beenhere = true;
    }
}

bool AndroidSalInstance::AnyInput( sal_uInt16 nType )
{
    if( (nType & VCL_INPUT_TIMER) != 0 )
        return CheckTimeout( false );
    // FIXME: ideally we should check our input queue here ...
    fprintf (stderr, "FIXME: AnyInput returns false\n");
    return false;
}

class AndroidSalSystem : public SvpSalSystem {
public:
    AndroidSalSystem() : SvpSalSystem() {}
    virtual ~AndroidSalSystem() {}
    virtual int ShowNativeDialog( const rtl::OUString& rTitle,
                                  const rtl::OUString& rMessage,
                                  const std::list< rtl::OUString >& rButtons,
                                  int nDefButton );
};

SalSystem *AndroidSalInstance::CreateSalSystem()
{
    return new AndroidSalSystem();
}

// All the interesting stuff is slaved from the AndroidSalInstance
void InitSalData()   {}
void DeInitSalData() {}
void InitSalMain()   {}
void DeInitSalMain() {}

void SalAbort( const rtl::OUString& rErrorText, bool bDumpCore )
{
    rtl::OUString aError( rErrorText );
    if( aError.isEmpty() )
        aError = rtl::OUString::createFromAscii("Unknown application error");
    ::fprintf( stderr, "%s\n", rtl::OUStringToOString(rErrorText, osl_getThreadTextEncoding()).getStr() );

    __android_log_print(ANDROID_LOG_INFO, "LibreOffice", "SalAbort: '%s'",
                        rtl::OUStringToOString(aError, RTL_TEXTENCODING_ASCII_US).getStr());
    if( bDumpCore )
        abort();
    else
        _exit(1);
}

const OUString& SalGetDesktopEnvironment()
{
    static rtl::OUString aEnv( RTL_CONSTASCII_USTRINGPARAM( "android" ) );
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
    fprintf (stderr, "Android: CreateSalInstance!\n");
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

int AndroidSalSystem::ShowNativeDialog( const rtl::OUString& rTitle,
                                        const rtl::OUString& rMessage,
                                        const std::list< rtl::OUString >& rButtons,
                                        int nDefButton )
{
    (void)rButtons;
    (void)nDefButton;

    fprintf (stderr, "LibreOffice native dialog '%s': '%s'\n",
             rtl::OUStringToOString(rTitle, RTL_TEXTENCODING_ASCII_US).getStr(),
             rtl::OUStringToOString(rMessage, RTL_TEXTENCODING_ASCII_US).getStr());

    __android_log_print(ANDROID_LOG_INFO, "LibreOffice", "Dialog '%s': '%s'",
                        rtl::OUStringToOString(rTitle, RTL_TEXTENCODING_ASCII_US).getStr(),
                        rtl::OUStringToOString(rMessage, RTL_TEXTENCODING_ASCII_US).getStr());

    if (AndroidSalInstance::getInstance() != NULL)
    {
        // Does Android have a native dialog ? if not,. we have to do this ...

        // Of course it has. android.app.AlertDialog seems like a good
        // choice, it even has one, two or three buttons. Naturally,
        // it intended to be used from Java, so some verbose JNI
        // horror would be needed to use it directly here. Probably we
        // want some easier to use magic wrapper, hmm.

#if 0 // Let's not drag in this stuff, we are not going to display anythinf
      // anyway using a VCL ErrorBox

        ErrorBox aVclErrBox( NULL, WB_OK, rTitle );
        aVclErrBox.SetText( rMessage );
        aVclErrBox.Execute();
#endif
    }
    else
        fprintf (stderr, "VCL not initialized\n");
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
