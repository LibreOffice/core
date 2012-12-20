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
#include <headless/svpinst.hxx>
#include <headless/svpdummies.hxx>
#include <generic/gendata.hxx>

class HeadlessSalInstance : public SvpSalInstance
{
public:
    HeadlessSalInstance( SalYieldMutex *pMutex );
    virtual ~HeadlessSalInstance();

    virtual SalSystem* CreateSalSystem();
};

HeadlessSalInstance::HeadlessSalInstance( SalYieldMutex *pMutex ) :
    SvpSalInstance( pMutex)
{
}

HeadlessSalInstance::~HeadlessSalInstance()
{
}

class HeadlessSalSystem : public SvpSalSystem {
public:
    HeadlessSalSystem() : SvpSalSystem() {}
    virtual ~HeadlessSalSystem() {}
    virtual int ShowNativeDialog( const OUString& rTitle,
                                  const OUString& rMessage,
                                  const std::list< OUString >& rButtons,
                                  int nDefButton )
    {
        (void)rButtons; (void)nDefButton;
        ::fprintf(stdout, "LibreOffice - dialog '%s': '%s'",
                            OUStringToOString(rTitle, RTL_TEXTENCODING_ASCII_US).getStr(),
                            OUStringToOString(rMessage, RTL_TEXTENCODING_ASCII_US).getStr());
        return 0;
    }
};

SalSystem *HeadlessSalInstance::CreateSalSystem()
{
    return new HeadlessSalSystem();
}

class HeadlessSalData : public SalGenericData
{
public:
    HeadlessSalData( SalInstance *pInstance ) : SalGenericData( SAL_DATA_HEADLESS, pInstance ) {}
    virtual void ErrorTrapPush() {}
    virtual bool ErrorTrapPop( bool ) { return false; }
};

// All the interesting stuff is slaved from the AndroidSalInstance
void InitSalData()   {}
void DeInitSalData() {}
void InitSalMain()   {}
void DeInitSalMain() {}

void SalAbort( const OUString& rErrorText, bool bDumpCore )
{
    OUString aError( rErrorText );
    if( aError.isEmpty() )
        aError = "Unknown application error";
    ::fprintf( stderr, "%s\n", OUStringToOString(rErrorText, osl_getThreadTextEncoding()).getStr() );

    ::fprintf( stderr, "SalAbort: '%s'",
                        OUStringToOString(aError, RTL_TEXTENCODING_ASCII_US).getStr());
    if( bDumpCore )
        abort();
    else
        _exit(1);
}

const OUString& SalGetDesktopEnvironment()
{
    static OUString aEnv( "headless" );
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
    HeadlessSalInstance* pInstance = new HeadlessSalInstance( new SalYieldMutex() );
    new HeadlessSalData( pInstance );
    pInstance->AcquireYieldMutex(1);
    return pInstance;
}

void DestroySalInstance( SalInstance *pInst )
{
    pInst->ReleaseYieldMutex();
    delete pInst;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
