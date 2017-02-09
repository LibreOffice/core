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
#include <tools/solar.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include "itemholder1.hxx"
#include <officecfg/Office/Recovery.hxx>

using namespace utl;
using namespace com::sun::star::uno;

class SvtSaveOptions_Impl;
class SvtLoadOptions_Impl;

#define CFG_READONLY_DEFAULT    false

struct SvtLoadSaveOptions_Impl
{
    SvtSaveOptions_Impl* pSaveOpt;
    SvtLoadOptions_Impl* pLoadOpt;
};

static SvtLoadSaveOptions_Impl* pOptions = nullptr;
static sal_Int32           nRefCount = 0;

class SvtSaveOptions_Impl : public utl::ConfigItem
{
    sal_Int32                           nAutoSaveTime;
    bool                            bUseUserData,
                                        bBackup,
                                        bAutoSave,
                                        bAutoSavePrompt,
                                        bUserAutoSave,
                                        bDocInfSave,
                                        bSaveWorkingSet,
                                        bSaveDocView,
                                        bSaveRelINet,
                                        bSaveRelFSys,
                                        bSaveUnpacked,
                                        bDoPrettyPrinting,
                                        bWarnAlienFormat,
                                        bLoadDocPrinter,
                                        bUseSHA1InODF12,
                                        bUseBlowfishInODF12;

    SvtSaveOptions::ODFDefaultVersion   eODFDefaultVersion;

    bool                            bROAutoSaveTime,
                                        bROUseUserData,
                                        bROBackup,
                                        bROAutoSave,
                                        bROAutoSavePrompt,
                                        bROUserAutoSave,
                                        bRODocInfSave,
                                        bROSaveWorkingSet,
                                        bROSaveDocView,
                                        bROSaveRelINet,
                                        bROSaveRelFSys,
                                        bROSaveUnpacked,
                                        bROWarnAlienFormat,
                                        bRODoPrettyPrinting,
                                        bROLoadDocPrinter,
                                        bROODFDefaultVersion,
                                        bROUseSHA1InODF12,
                                        bROUseBlowfishInODF12;

    virtual void            ImplCommit() override;

public:
                            SvtSaveOptions_Impl();

    virtual void            Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;

    sal_Int32               GetAutoSaveTime() const             { return nAutoSaveTime; }
    bool                    IsUseUserData() const               { return bUseUserData; }
    bool                    IsBackup() const                    { return bBackup; }
    bool                    IsAutoSave() const                  { return bAutoSave; }
    bool                    IsAutoSavePrompt() const            { return bAutoSavePrompt; }
    bool                    IsUserAutoSave() const              { return bUserAutoSave; }
    bool                    IsDocInfoSave() const               { return bDocInfSave; }
    bool                    IsSaveWorkingSet() const            { return bSaveWorkingSet;         }
    bool                    IsSaveDocView() const               { return bSaveDocView; }
    bool                    IsSaveRelINet() const               { return bSaveRelINet; }
    bool                    IsSaveRelFSys() const               { return bSaveRelFSys; }
    bool                    IsSaveUnpacked() const              { return bSaveUnpacked; }
    bool                IsPrettyPrintingEnabled( ) const    { return bDoPrettyPrinting; }
    bool                IsWarnAlienFormat() const           { return bWarnAlienFormat; }
    bool                IsLoadDocPrinter() const            { return bLoadDocPrinter; }
    bool                IsUseSHA1InODF12() const            { return bUseSHA1InODF12; }
    bool                IsUseBlowfishInODF12() const        { return bUseBlowfishInODF12; }

    SvtSaveOptions::ODFDefaultVersion
                            GetODFDefaultVersion() const        { return eODFDefaultVersion; }

    void                    SetAutoSaveTime( sal_Int32 n );
    void                    SetUseUserData( bool b );
    void                    SetBackup( bool b );
    void                    SetAutoSave( bool b );
    void                    SetAutoSavePrompt( bool b );
    void                    SetUserAutoSave( bool b );
    void                    SetDocInfoSave( bool b );
    void                    SetSaveWorkingSet( bool b );
    void                    SetSaveDocView( bool b );
    void                    SetSaveRelINet( bool b );
    void                    SetSaveRelFSys( bool b );
    void                    EnablePrettyPrinting( bool _bDoPP );
    void                    SetWarnAlienFormat( bool _bDoPP );
    void                    SetLoadDocPrinter( bool bNew );
    void                    SetODFDefaultVersion( SvtSaveOptions::ODFDefaultVersion eNew );

    bool                IsReadOnly( SvtSaveOptions::EOption eOption ) const;
};

void SvtSaveOptions_Impl::SetAutoSaveTime( sal_Int32 n )
{
    if (!bROAutoSaveTime && nAutoSaveTime!=n)
    {
        nAutoSaveTime = n;
        SetModified();
        Commit();
    }
}

void SvtSaveOptions_Impl::SetUseUserData( bool b )
{
    if (!bROUseUserData && bUseUserData!=b)
    {
        bUseUserData = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetBackup( bool b )
{
    if (!bROBackup && bBackup!=b)
    {
        bBackup = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetAutoSave( bool b )
{
    if (!bROAutoSave && bAutoSave!=b)
    {
        bAutoSave = b;
        SetModified();
        Commit();
    }
}

void SvtSaveOptions_Impl::SetAutoSavePrompt( bool b )
{
    if (!bROAutoSavePrompt && bAutoSavePrompt!=b)
    {
        bAutoSavePrompt = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetUserAutoSave( bool b )
{
    if (!bROUserAutoSave && bUserAutoSave!=b)
    {
        bUserAutoSave = b;
        SetModified();
        Commit();
    }
}

void SvtSaveOptions_Impl::SetDocInfoSave(bool b)
{
    if (!bRODocInfSave && bDocInfSave!=b)
    {
        bDocInfSave = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetSaveWorkingSet( bool b )
{
    if (!bROSaveWorkingSet && bSaveWorkingSet!=b)
    {
        bSaveWorkingSet = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetSaveDocView( bool b )
{
    if (!bROSaveDocView && bSaveDocView!=b)
    {
        bSaveDocView = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetSaveRelINet( bool b )
{
    if (!bROSaveRelINet && bSaveRelINet!=b)
    {
        bSaveRelINet = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetSaveRelFSys( bool b )
{
    if (!bROSaveRelFSys && bSaveRelFSys!=b)
    {
        bSaveRelFSys = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::EnablePrettyPrinting( bool _bDoPP )
{
    if (!bRODoPrettyPrinting && bDoPrettyPrinting!=_bDoPP)
    {
        bDoPrettyPrinting = _bDoPP;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetWarnAlienFormat( bool _bDoPP )
{
    if (!bROWarnAlienFormat && bWarnAlienFormat!=_bDoPP)
    {
        bWarnAlienFormat = _bDoPP;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetLoadDocPrinter( bool bNew )
{
    if ( !bROLoadDocPrinter && bLoadDocPrinter != bNew )
    {
        bLoadDocPrinter = bNew;
        SetModified();
    }
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
        case SvtSaveOptions::EOption::AutoSaveTime :
            bReadOnly = bROAutoSaveTime;
            break;
        case SvtSaveOptions::EOption::UseUserData :
            bReadOnly = bROUseUserData;
            break;
        case SvtSaveOptions::EOption::Backup :
            bReadOnly = bROBackup;
            break;
        case SvtSaveOptions::EOption::AutoSave :
            bReadOnly = bROAutoSave;
            break;
        case SvtSaveOptions::EOption::AutoSavePrompt :
            bReadOnly = bROAutoSavePrompt;
            break;
        case SvtSaveOptions::EOption::UserAutoSave :
            bReadOnly = bROUserAutoSave;
            break;
        case SvtSaveOptions::EOption::DocInfSave :
            bReadOnly = bRODocInfSave;
            break;
        case SvtSaveOptions::EOption::SaveWorkingSet :
            bReadOnly = bROSaveWorkingSet;
            break;
        case SvtSaveOptions::EOption::SaveDocView :
            bReadOnly = bROSaveDocView;
            break;
        case SvtSaveOptions::EOption::SaveRelInet :
            bReadOnly = bROSaveRelINet;
            break;
        case SvtSaveOptions::EOption::SaveRelFsys :
            bReadOnly = bROSaveRelFSys;
            break;
        case SvtSaveOptions::EOption::DoPrettyPrinting :
            bReadOnly = bRODoPrettyPrinting;
            break;
        case SvtSaveOptions::EOption::WarnAlienFormat :
            bReadOnly = bROWarnAlienFormat;
            break;
        case SvtSaveOptions::EOption::LoadDocPrinter :
            bReadOnly = bROLoadDocPrinter;
            break;
        case SvtSaveOptions::EOption::OdfDefaultVersion :
            bReadOnly = bROLoadDocPrinter;
            break;
    }
    return bReadOnly;
}

#define FORMAT              0
#define TIMEINTERVALL       1
#define USEUSERDATA         2
#define CREATEBACKUP        3
#define AUTOSAVE            4
#define PROMPT              5
#define EDITPROPERTY        6
#define SAVEVIEWINFO        7
#define UNPACKED            8
#define PRETTYPRINTING      9
#define WARNALIENFORMAT     10
#define LOADDOCPRINTER      11
#define FILESYSTEM          12
#define INTERNET            13
#define SAVEWORKINGSET      14
#define ODFDEFAULTVERSION   15
#define USESHA1INODF12      16
#define USEBLOWFISHINODF12  17

Sequence< OUString > GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Graphic/Format",
        "Document/AutoSaveTimeIntervall",
        "Document/UseUserData",
        "Document/CreateBackup",
        "Document/AutoSave",
        "Document/AutoSavePrompt",
        "Document/EditProperty",
        "Document/ViewInfo",
        "Document/Unpacked",
        "Document/PrettyPrinting",
        "Document/WarnAlienFormat",
        "Document/LoadPrinter",
        "URL/FileSystem",
        "URL/Internet",
        "WorkingSet",
        "ODF/DefaultVersion",
        "ODF/UseSHA1InODF12",
        "ODF/UseBlowfishInODF12"
    };

    const int nCount = sizeof( aPropNames ) / sizeof( const char* );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

SvtSaveOptions_Impl::SvtSaveOptions_Impl()
    : ConfigItem( "Office.Common/Save" )
    , nAutoSaveTime( 0 )
    , bUseUserData( false )
    , bBackup( false )
    , bAutoSave( false )
    , bAutoSavePrompt( false )
    , bUserAutoSave( false )
    , bDocInfSave( false )
    , bSaveWorkingSet( false )
    , bSaveDocView( false )
    , bSaveRelINet( false )
    , bSaveRelFSys( false )
    , bSaveUnpacked( false )
    , bDoPrettyPrinting( false )
    , bWarnAlienFormat( true )
    , bLoadDocPrinter( true )
    , bUseSHA1InODF12( false )
    , bUseBlowfishInODF12( false )
    , eODFDefaultVersion( SvtSaveOptions::ODFVER_LATEST )
    , bROAutoSaveTime( CFG_READONLY_DEFAULT )
    , bROUseUserData( CFG_READONLY_DEFAULT )
    , bROBackup( CFG_READONLY_DEFAULT )
    , bROAutoSave( CFG_READONLY_DEFAULT )
    , bROAutoSavePrompt( CFG_READONLY_DEFAULT )
    , bROUserAutoSave( CFG_READONLY_DEFAULT )
    , bRODocInfSave( CFG_READONLY_DEFAULT )
    , bROSaveWorkingSet( CFG_READONLY_DEFAULT )
    , bROSaveDocView( CFG_READONLY_DEFAULT )
    , bROSaveRelINet( CFG_READONLY_DEFAULT )
    , bROSaveRelFSys( CFG_READONLY_DEFAULT )
    , bROSaveUnpacked( CFG_READONLY_DEFAULT )
    , bROWarnAlienFormat( CFG_READONLY_DEFAULT )
    , bRODoPrettyPrinting( CFG_READONLY_DEFAULT )
    , bROLoadDocPrinter( CFG_READONLY_DEFAULT )
    , bROODFDefaultVersion( CFG_READONLY_DEFAULT )
    , bROUseSHA1InODF12( CFG_READONLY_DEFAULT )
    , bROUseBlowfishInODF12( CFG_READONLY_DEFAULT )
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
                sal_Int32 nTemp = 0;
                switch ( nProp )
                {
                    case FORMAT:
                        // not supported anymore
                        break;

                    case TIMEINTERVALL :
                        if ( pValues[nProp] >>= nTemp )
                            nAutoSaveTime = nTemp;
                        else {
                            OSL_FAIL( "Wrong Type!" );
                        };
                        bROAutoSaveTime = pROStates[nProp];
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
                    {
                        bool bTemp = bool();
                        if ( pValues[nProp] >>= bTemp )
                        {
                            switch ( nProp )
                            {
                                case USEUSERDATA :
                                    bUseUserData = bTemp;
                                    bROUseUserData = pROStates[nProp];
                                    break;
                                case CREATEBACKUP :
                                    bBackup = bTemp;
                                    bROBackup = pROStates[nProp];
                                    break;
                                case AUTOSAVE :
                                    bAutoSave = bTemp;
                                    bROAutoSave = pROStates[nProp];
                                    break;
                                case PROMPT :
                                    bAutoSavePrompt = bTemp;
                                    bROAutoSavePrompt = pROStates[nProp];
                                    break;
                                case EDITPROPERTY :
                                    bDocInfSave = bTemp;
                                    bRODocInfSave = pROStates[nProp];
                                    break;
                                case SAVEWORKINGSET :
                                    bSaveWorkingSet = bTemp;
                                    bROSaveWorkingSet = pROStates[nProp];
                                    break;
                                case SAVEVIEWINFO :
                                    bSaveDocView = bTemp;
                                    bROSaveDocView = pROStates[nProp];
                                    break;
                                case FILESYSTEM :
                                    bSaveRelFSys = bTemp;
                                    bROSaveRelFSys = pROStates[nProp];
                                    break;
                                case INTERNET :
                                    bSaveRelINet = bTemp;
                                    bROSaveRelINet = pROStates[nProp];
                                    break;
                                case UNPACKED :
                                    bSaveUnpacked = bTemp;
                                    bROSaveUnpacked = pROStates[nProp];
                                    break;

                                case PRETTYPRINTING:
                                    bDoPrettyPrinting = bTemp;
                                    bRODoPrettyPrinting = pROStates[nProp];
                                    break;

                                case WARNALIENFORMAT:
                                    bWarnAlienFormat = bTemp;
                                    bROWarnAlienFormat = pROStates[nProp];
                                    break;

                                case LOADDOCPRINTER:
                                    bLoadDocPrinter = bTemp;
                                    bROLoadDocPrinter = pROStates[nProp];
                                    break;

                                case USESHA1INODF12:
                                    bUseSHA1InODF12 = bTemp;
                                    bROUseSHA1InODF12 = pROStates[nProp];
                                    break;

                                case USEBLOWFISHINODF12:
                                    bUseBlowfishInODF12 = bTemp;
                                    bROUseBlowfishInODF12 = pROStates[nProp];
                                    break;

                                default :
                                    SAL_WARN( "unotools.config", "invalid index to load a path" );
                            }
                        }
                        else
                        {
                            OSL_FAIL( "Wrong Type!" );
                        }
                    }
                }
            }
        }
    }

    if (!utl::ConfigManager::IsAvoidConfig())
    {
        bAutoSave = officecfg::Office::Recovery::AutoSave::Enabled::get();
        nAutoSaveTime = officecfg::Office::Recovery::AutoSave::TimeIntervall::get();
        bUserAutoSave = officecfg::Office::Recovery::AutoSave::UserAutoSaveEnabled::get();
    }
    else
    {
        bAutoSave = false;
        nAutoSaveTime = 0;
        bUserAutoSave = false;
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
            case TIMEINTERVALL :
                if (!bROAutoSaveTime)
                {
                    pValues[nRealCount] <<= nAutoSaveTime;
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case USEUSERDATA :
                if (!bROUseUserData)
                {
                    pValues[nRealCount] <<= bUseUserData;
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case CREATEBACKUP :
                if (!bROBackup)
                {
                    pValues[nRealCount] <<= bBackup;
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case AUTOSAVE :
                if (!bROAutoSave)
                {
                    pValues[nRealCount] <<= bAutoSave;
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case PROMPT :
                if (!bROAutoSavePrompt)
                {
                    pValues[nRealCount] <<= bAutoSavePrompt;
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case EDITPROPERTY :
                if (!bRODocInfSave)
                {
                    pValues[nRealCount] <<= bDocInfSave;
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case SAVEWORKINGSET :
                if (!bROSaveWorkingSet)
                {
                    pValues[nRealCount] <<= bSaveWorkingSet;
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case SAVEVIEWINFO :
                if (!bROSaveDocView)
                {
                    pValues[nRealCount] <<= bSaveDocView;
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case FILESYSTEM :
                if (!bROSaveRelFSys)
                {
                    pValues[nRealCount] <<= bSaveRelFSys;
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case INTERNET :
                if (!bROSaveRelINet)
                {
                    pValues[nRealCount] <<= bSaveRelINet;
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case UNPACKED :
                if (!bROSaveUnpacked)
                {
                    pValues[nRealCount] <<= bSaveUnpacked;
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case PRETTYPRINTING:
                if (!bRODoPrettyPrinting)
                {
                    pValues[nRealCount] <<= bDoPrettyPrinting;
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case WARNALIENFORMAT:
                if (!bROWarnAlienFormat)
                {
                    pValues[nRealCount] <<= bWarnAlienFormat;
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case LOADDOCPRINTER:
                if (!bROLoadDocPrinter)
                {
                    pValues[nRealCount] <<= bLoadDocPrinter;
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case ODFDEFAULTVERSION:
                if (!bROODFDefaultVersion)
                {
                    pValues[nRealCount] <<= (eODFDefaultVersion == SvtSaveOptions::ODFVER_LATEST) ? sal_Int16( 3 ) : sal_Int16( eODFDefaultVersion );
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case USESHA1INODF12:
                if (!bROUseSHA1InODF12)
                {
                    pValues[nRealCount] <<= bUseSHA1InODF12;
                    pNames[nRealCount] = pOrgNames[i];
                    ++nRealCount;
                }
                break;
            case USEBLOWFISHINODF12:
                if (!bROUseBlowfishInODF12)
                {
                    pValues[nRealCount] <<= bUseBlowfishInODF12;
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
    officecfg::Office::Recovery::AutoSave::UserAutoSaveEnabled::set(bUserAutoSave, batch);
    batch->commit();
}

void SvtSaveOptions_Impl::Notify( const Sequence<OUString>& )
{
}

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

const sal_Char cUserDefinedSettings[] = "UserDefinedSettings";

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
        pOptions = new SvtLoadSaveOptions_Impl;
        pOptions->pSaveOpt = new SvtSaveOptions_Impl;
        pOptions->pLoadOpt = new SvtLoadOptions_Impl;

        ItemHolder1::holdConfigItem(EItem::SaveOptions);
   }
   ++nRefCount;
    pImp = pOptions;
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

        DELETEZ( pOptions->pLoadOpt );
        DELETEZ( pOptions->pSaveOpt );
        DELETEZ( pOptions );
    }
}

void SvtSaveOptions::SetAutoSaveTime( sal_Int32 n )
{
    pImp->pSaveOpt->SetAutoSaveTime( n );
}

sal_Int32 SvtSaveOptions::GetAutoSaveTime() const
{
    return pImp->pSaveOpt->GetAutoSaveTime();
}

void SvtSaveOptions::SetUseUserData( bool b )
{
    pImp->pSaveOpt->SetUseUserData( b );
}

bool SvtSaveOptions::IsUseUserData() const
{
    return pImp->pSaveOpt->IsUseUserData();
}

void SvtSaveOptions::SetBackup( bool b )
{
    pImp->pSaveOpt->SetBackup( b );
}

bool SvtSaveOptions::IsBackup() const
{
    return pImp->pSaveOpt->IsBackup();
}

void SvtSaveOptions::SetAutoSave( bool b )
{
    pImp->pSaveOpt->SetAutoSave( b );
}

bool SvtSaveOptions::IsAutoSave() const
{
    return pImp->pSaveOpt->IsAutoSave();
}

void SvtSaveOptions::SetAutoSavePrompt( bool b )
{
    pImp->pSaveOpt->SetAutoSavePrompt( b );
}

bool SvtSaveOptions::IsAutoSavePrompt() const
{
    return pImp->pSaveOpt->IsAutoSavePrompt();
}

void SvtSaveOptions::SetUserAutoSave( bool b )
{
    pImp->pSaveOpt->SetUserAutoSave( b );
}

bool SvtSaveOptions::IsUserAutoSave() const
{
    return pImp->pSaveOpt->IsUserAutoSave();
}

void SvtSaveOptions::SetDocInfoSave(bool b)
{
    pImp->pSaveOpt->SetDocInfoSave( b );
}

bool SvtSaveOptions::IsDocInfoSave() const
{
    return pImp->pSaveOpt->IsDocInfoSave();
}

void SvtSaveOptions::SetSaveWorkingSet( bool b )
{
    pImp->pSaveOpt->SetSaveWorkingSet( b );
}

bool SvtSaveOptions::IsSaveWorkingSet() const
{
    return pImp->pSaveOpt->IsSaveWorkingSet();
}

void SvtSaveOptions::SetSaveDocView( bool b )
{
    pImp->pSaveOpt->SetSaveDocView( b );
}

bool SvtSaveOptions::IsSaveDocView() const
{
    return pImp->pSaveOpt->IsSaveDocView();
}

void SvtSaveOptions::SetSaveRelINet( bool b )
{
    pImp->pSaveOpt->SetSaveRelINet( b );
}

bool SvtSaveOptions::IsSaveRelINet() const
{
    return pImp->pSaveOpt->IsSaveRelINet();
}

void SvtSaveOptions::SetSaveRelFSys( bool b )
{
    pImp->pSaveOpt->SetSaveRelFSys( b );
}

bool SvtSaveOptions::IsSaveRelFSys() const
{
    return pImp->pSaveOpt->IsSaveRelFSys();
}

bool SvtSaveOptions::IsSaveUnpacked() const
{
    return pImp->pSaveOpt->IsSaveUnpacked();
}

void SvtSaveOptions::SetLoadUserSettings(bool b)
{
    pImp->pLoadOpt->SetLoadUserSettings(b);
}

bool   SvtSaveOptions::IsLoadUserSettings() const
{
    return pImp->pLoadOpt->IsLoadUserSettings();
}

void SvtSaveOptions::SetPrettyPrinting( bool _bEnable )
{
    pImp->pSaveOpt->EnablePrettyPrinting( _bEnable );
}

bool SvtSaveOptions::IsPrettyPrinting() const
{
    return pImp->pSaveOpt->IsPrettyPrintingEnabled();
}

void SvtSaveOptions::SetWarnAlienFormat( bool _bEnable )
{
    pImp->pSaveOpt->SetWarnAlienFormat( _bEnable );
}

bool SvtSaveOptions::IsWarnAlienFormat() const
{
    return pImp->pSaveOpt->IsWarnAlienFormat();
}

void SvtSaveOptions::SetLoadDocumentPrinter( bool _bEnable )
{
    pImp->pSaveOpt->SetLoadDocPrinter( _bEnable );
}

bool SvtSaveOptions::IsLoadDocumentPrinter() const
{
    return pImp->pSaveOpt->IsLoadDocPrinter();
}

void SvtSaveOptions::SetODFDefaultVersion( SvtSaveOptions::ODFDefaultVersion eVersion )
{
    pImp->pSaveOpt->SetODFDefaultVersion( eVersion );
}

SvtSaveOptions::ODFDefaultVersion SvtSaveOptions::GetODFDefaultVersion() const
{
    return pImp->pSaveOpt->GetODFDefaultVersion();
}

SvtSaveOptions::ODFSaneDefaultVersion SvtSaveOptions::GetODFSaneDefaultVersion() const
{
    switch (pImp->pSaveOpt->GetODFDefaultVersion())
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
    }
    return ODFSVER_LATEST_EXTENDED;
}

bool SvtSaveOptions::IsUseSHA1InODF12() const
{
    return pImp->pSaveOpt->IsUseSHA1InODF12();
}

bool SvtSaveOptions::IsUseBlowfishInODF12() const
{
    return pImp->pSaveOpt->IsUseBlowfishInODF12();
}

bool SvtSaveOptions::IsReadOnly( SvtSaveOptions::EOption eOption ) const
{
    return pImp->pSaveOpt->IsReadOnly(eOption);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
