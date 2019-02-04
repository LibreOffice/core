/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <headless/svpinst.hxx>
#include <headless/svpdummies.hxx>
#include <unx/gendata.hxx>

class HeadlessSalInstance : public SvpSalInstance
{
public:
    explicit HeadlessSalInstance(std::unique_ptr<SalYieldMutex> pMutex);

    virtual SalSystem* CreateSalSystem() override;
};

HeadlessSalInstance::HeadlessSalInstance(std::unique_ptr<SalYieldMutex> pMutex)
    : SvpSalInstance(std::move(pMutex))
{
}

class HeadlessSalSystem : public SvpSalSystem {
public:
    HeadlessSalSystem() : SvpSalSystem() {}
    virtual int ShowNativeDialog( const OUString& rTitle,
                                  const OUString& rMessage,
                                  const std::vector< OUString >& rButtons ) override
    {
        (void)rButtons;
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

class HeadlessSalData : public GenericUnixSalData
{
public:
    explicit HeadlessSalData( SalInstance *pInstance ) : GenericUnixSalData( SAL_DATA_HEADLESS, pInstance ) {}
    virtual void ErrorTrapPush() override {}
    virtual bool ErrorTrapPop( bool ) override { return false; }
};

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
    m_pInstance( nullptr ),
    m_pPIManager( nullptr )
{
}

SalData::~SalData()
{
}

// This is our main entry point:
SalInstance *CreateSalInstance()
{
    HeadlessSalInstance* pInstance = new HeadlessSalInstance(std::make_unique<SvpSalYieldMutex>());
    new HeadlessSalData( pInstance );
    pInstance->AcquireYieldMutex();
    return pInstance;
}

void DestroySalInstance( SalInstance *pInst )
{
    pInst->ReleaseYieldMutexAll();
    delete pInst;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
