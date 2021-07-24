/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <sal/log.hxx>
#include <unotools/saveopt.hxx>
#include <rtl/instance.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <officecfg/Office/Recovery.hxx>
#include <officecfg/Office/Common.hxx>

using namespace utl;
using namespace com::sun::star::uno;

namespace {

class SvtSaveOptions_Impl;
class SvtLoadOptions_Impl;

}

#define CFG_READONLY_DEFAULT    false

struct SvtLoadSaveOptions_Impl
{
    std::unique_ptr<SvtSaveOptions_Impl> pSaveOpt;
    std::unique_ptr<SvtLoadOptions_Impl> pLoadOpt;
};

static std::unique_ptr<SvtLoadSaveOptions_Impl> pOptions;
static sal_Int32           nRefCount = 0;

namespace {

class SvtSaveOptions_Impl : public utl::ConfigItem
{
    sal_Int32                           nAutoSaveTime;
    bool                            bAutoSave;

    SvtSaveOptions::ODFDefaultVersion   eODFDefaultVersion;

    bool                            bROODFDefaultVersion;

    virtual void            ImplCommit() override;

public:
                            SvtSaveOptions_Impl();

    virtual void            Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;

    SvtSaveOptions::ODFDefaultVersion
                            GetODFDefaultVersion() const        { return eODFDefaultVersion; }

    void                    SetODFDefaultVersion( SvtSaveOptions::ODFDefaultVersion eNew );

    bool                IsReadOnly( SvtSaveOptions::EOption eOption ) const;
};

}

void SvtSaveOptions_Impl::SetODFDefaultVersion( SvtSaveOptions::ODFDefaultVersion eNew )
{
    if ( !bROODFDefaultVersion && eODFDefaultVersion != eNew )
    {
        eODFDefaultVersion = eNew;
        SetModified();
    }
}

bool SvtSaveOptions_Impl::IsReadOnly( SvtSaveOptions::EOption eOption ) const
{
    bool bReadOnly = CFG_READONLY_DEFAULT;
    switch(eOption)
    {
        case SvtSaveOptions::EOption::OdfDefaultVersion :
            bReadOnly = bROODFDefaultVersion;
            break;
    }
    return bReadOnly;
}

#define FORMAT              0
#define ODFDEFAULTVERSION   1

static Sequence< OUString > GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Graphic/Format",
        "ODF/DefaultVersion"
    };

    const int nCount = SAL_N_ELEMENTS( aPropNames );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

SvtSaveOptions_Impl::SvtSaveOptions_Impl()
    : ConfigItem( "Office.Common/Save" )
    , nAutoSaveTime( 0 )
    , eODFDefaultVersion( SvtSaveOptions::ODFVER_LATEST )
    , bROODFDefaultVersion( CFG_READONLY_DEFAULT )
{
    Sequence< OUString > aNames = GetPropertyNames();
    Sequence< Any > aValues = GetProperties( aNames );
    Sequence< sal_Bool > aROStates = GetReadOnlyStates( aNames );
    EnableNotification( aNames );
    const Any* pValues = aValues.getConstArray();
    const sal_Bool* pROStates = aROStates.getConstArray();
    DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
    DBG_ASSERT( aROStates.getLength() == aNames.getLength(), "GetReadOnlyStates failed" );
    if ( aValues.getLength() == aNames.getLength() && aROStates.getLength() == aNames.getLength() )
    {
        for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
        {
            if ( pValues[nProp].hasValue() )
            {
                switch ( nProp )
                {
                    case FORMAT:
                        // not supported anymore
                        break;

                    case ODFDEFAULTVERSION :
                    {
                        sal_Int16 nTmp = 0;
                        if ( pValues[nProp] >>= nTmp )
                        {
                            if( nTmp == 3 )
                                eODFDefaultVersion = SvtSaveOptions::ODFVER_LATEST;
                            else
                                eODFDefaultVersion = SvtSaveOptions::ODFDefaultVersion( nTmp );
                        }
                        else {
                            SAL_WARN( "unotools.config", "SvtSaveOptions_Impl::SvtSaveOptions_Impl(): Wrong Type!" );
                        };
                        bROODFDefaultVersion = pROStates[nProp];
                        break;
                    }

                    default:
                        OSL_FAIL( "Wrong Type!" );
                }
            }
        }
    }

    if (!utl::ConfigManager::IsFuzzing())
    {
        bAutoSave = officecfg::Office::Recovery::AutoSave::Enabled::get();
        nAutoSaveTime = officecfg::Office::Recovery::AutoSave::TimeIntervall::get();
    }
    else
    {
        bAutoSave = false;
        nAutoSaveTime = 0;
    }
}

void SvtSaveOptions_Impl::ImplCommit()
{
    Sequence< OUString > aOrgNames = GetPropertyNames();
    OUString* pOrgNames = aOrgNames.getArray();
    sal_Int32 nOrgCount = aOrgNames.getLength();

    Sequence< OUString > aNames( nOrgCount );
    Sequence< Any > aValues( nOrgCount );
    OUString* pNames = aNames.getArray();
    Any* pValues = aValues.getArray();
    sal_Int32 nRealCount = 0;

    for (sal_Int32 i=0; i<nOrgCount; ++i)
    {
        switch (i)
        {
            case FORMAT:
                // not supported anymore
                break;
            case ODFDEFAULTVERSION:
                if (!bROODFDefaultVersion)
                {
                    pValues[nRealCount] <<= (eODFDefaultVersion == SvtSaveOptions::ODFVER_LATEST) ? sal_Int16( 3 ) : sal_Int16( eODFDefaultVersion );
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;

            default:
                SAL_WARN( "unotools.config", "invalid index to save a path" );
        }
    }

    aNames.realloc(nRealCount);
    aValues.realloc(nRealCount);
    PutProperties( aNames, aValues );

    std::shared_ptr< comphelper::ConfigurationChanges > batch(
    comphelper::ConfigurationChanges::create());
    officecfg::Office::Recovery::AutoSave::TimeIntervall::set(nAutoSaveTime, batch);
    officecfg::Office::Recovery::AutoSave::Enabled::set(bAutoSave, batch);
    batch->commit();
}

void SvtSaveOptions_Impl::Notify( const Sequence<OUString>& )
{
}

namespace {

class SvtLoadOptions_Impl : public utl::ConfigItem
{
private:
    bool                            bLoadUserDefinedSettings;

    virtual void            ImplCommit() override;

public:
                            SvtLoadOptions_Impl();

    virtual void            Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;

    void                    SetLoadUserSettings(bool b){bLoadUserDefinedSettings = b; SetModified();}
    bool                IsLoadUserSettings() const {return bLoadUserDefinedSettings;}
};

}

constexpr OUStringLiteral cUserDefinedSettings = u"UserDefinedSettings";

SvtLoadOptions_Impl::SvtLoadOptions_Impl()
    : ConfigItem( "Office.Common/Load" )
    , bLoadUserDefinedSettings( false )
{
    Sequence< OUString > aNames { cUserDefinedSettings };
    Sequence< Any > aValues = GetProperties( aNames );
    EnableNotification( aNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
    pValues[0] >>= bLoadUserDefinedSettings;
}

void SvtLoadOptions_Impl::ImplCommit()
{
    PutProperties(
        {cUserDefinedSettings}, {css::uno::Any(bLoadUserDefinedSettings)});
}

void SvtLoadOptions_Impl::Notify( const Sequence<OUString>& )
{
    SAL_WARN( "unotools.config", "properties have been changed" );
}

namespace
{
    class LocalSingleton : public rtl::Static< osl::Mutex, LocalSingleton >
    {
    };
}

SvtSaveOptions::SvtSaveOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( LocalSingleton::get() );
    if ( !pOptions )
    {
        pOptions.reset(new SvtLoadSaveOptions_Impl);
        pOptions->pSaveOpt.reset(new SvtSaveOptions_Impl);
        pOptions->pLoadOpt.reset( new SvtLoadOptions_Impl);
    }
    ++nRefCount;
    pImp = pOptions.get();
}

SvtSaveOptions::~SvtSaveOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( LocalSingleton::get() );
    if ( !--nRefCount )
    {
        if ( pOptions->pSaveOpt->IsModified() )
            pOptions->pSaveOpt->Commit();
        if ( pOptions->pLoadOpt->IsModified() )
            pOptions->pLoadOpt->Commit();

        pOptions.reset();
    }
}

void SvtSaveOptions::SetLoadUserSettings(bool b)
{
    pImp->pLoadOpt->SetLoadUserSettings(b);
}

bool   SvtSaveOptions::IsLoadUserSettings() const
{
    return pImp->pLoadOpt->IsLoadUserSettings();
}

void SvtSaveOptions::SetODFDefaultVersion( SvtSaveOptions::ODFDefaultVersion eVersion )
{
    pImp->pSaveOpt->SetODFDefaultVersion( eVersion );
}

SvtSaveOptions::ODFDefaultVersion SvtSaveOptions::GetODFDefaultVersion() const
{
    auto const nRet = pImp->pSaveOpt->GetODFDefaultVersion();
    SAL_WARN_IF(nRet == ODFVER_UNKNOWN, "unotools.config", "DefaultVersion is ODFVER_UNKNOWN?");
    return (nRet == ODFVER_UNKNOWN) ? ODFVER_LATEST : nRet;
}

SvtSaveOptions::ODFSaneDefaultVersion GetODFSaneDefaultVersion()
{
    sal_Int16 nTmp = officecfg::Office::Common::Save::ODF::DefaultVersion::get();
    SvtSaveOptions::ODFDefaultVersion eODFDefaultVersion;
    if( nTmp == 3 )
        eODFDefaultVersion = SvtSaveOptions::ODFVER_LATEST;
    else
        eODFDefaultVersion = SvtSaveOptions::ODFDefaultVersion( nTmp );
    return SvtSaveOptions::GetODFSaneDefaultVersion(eODFDefaultVersion);
}

SvtSaveOptions::ODFSaneDefaultVersion SvtSaveOptions::GetODFSaneDefaultVersion() const
{
    return GetODFSaneDefaultVersion(pImp->pSaveOpt->GetODFDefaultVersion());
}

SvtSaveOptions::ODFSaneDefaultVersion SvtSaveOptions::GetODFSaneDefaultVersion(ODFDefaultVersion eODFDefaultVersion)
{
    switch (eODFDefaultVersion)
    {
        default:
            assert(!"map new ODFDefaultVersion to ODFSaneDefaultVersion");
            break;
        case ODFVER_UNKNOWN:
        case ODFVER_LATEST:
            return ODFSVER_LATEST_EXTENDED;
        case ODFVER_010:
            return ODFSVER_010;
        case ODFVER_011:
            return ODFSVER_011;
        case ODFVER_012:
            return ODFSVER_012;
        case ODFVER_012_EXT_COMPAT:
            return ODFSVER_012_EXT_COMPAT;
        case ODFVER_012_EXTENDED:
            return ODFSVER_012_EXTENDED;
        case ODFVER_013:
            return ODFSVER_013;
    }
    return ODFSVER_LATEST_EXTENDED;
}

bool SvtSaveOptions::IsReadOnly( SvtSaveOptions::EOption eOption ) const
{
    return pImp->pSaveOpt->IsReadOnly(eOption);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
