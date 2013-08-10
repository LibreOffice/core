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

#include <unotools/saveopt.hxx>
#include "rtl/instance.hxx"
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include "itemholder1.hxx"
#include <officecfg/Office/Recovery.hxx>

using namespace utl;
using namespace com::sun::star::uno;


class SvtSaveOptions_Impl;
class SvtLoadOptions_Impl;

#define CFG_READONLY_DEFAULT    sal_False

struct SvtLoadSaveOptions_Impl
{
    SvtSaveOptions_Impl* pSaveOpt;
    SvtLoadOptions_Impl* pLoadOpt;
};

static SvtLoadSaveOptions_Impl* pOptions = NULL;
static sal_Int32           nRefCount = 0;

class SvtSaveOptions_Impl : public utl::ConfigItem
{
    sal_Int32                           nAutoSaveTime;
    sal_Bool                            bUseUserData,
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

    sal_Bool                            bROAutoSaveTime,
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

public:
                            SvtSaveOptions_Impl();
                            ~SvtSaveOptions_Impl();

    virtual void            Notify( const com::sun::star::uno::Sequence< OUString >& aPropertyNames );
    virtual void            Commit();

    sal_Int32               GetAutoSaveTime() const             { return nAutoSaveTime; }
    sal_Bool                    IsUseUserData() const               { return bUseUserData; }
    sal_Bool                    IsBackup() const                    { return bBackup; }
    sal_Bool                    IsAutoSave() const                  { return bAutoSave; }
    sal_Bool                    IsAutoSavePrompt() const            { return bAutoSavePrompt; }
    sal_Bool                    IsUserAutoSave() const              { return bUserAutoSave; }
    sal_Bool                    IsDocInfoSave() const               { return bDocInfSave; }
    sal_Bool                    IsSaveWorkingSet() const            { return bSaveWorkingSet;         }
    sal_Bool                    IsSaveDocView() const               { return bSaveDocView; }
    sal_Bool                    IsSaveRelINet() const               { return bSaveRelINet; }
    sal_Bool                    IsSaveRelFSys() const               { return bSaveRelFSys; }
    sal_Bool                    IsSaveUnpacked() const              { return bSaveUnpacked; }
    sal_Bool                IsPrettyPrintingEnabled( ) const    { return bDoPrettyPrinting; }
    sal_Bool                IsWarnAlienFormat() const           { return bWarnAlienFormat; }
    sal_Bool                IsLoadDocPrinter() const            { return bLoadDocPrinter; }
    sal_Bool                IsUseSHA1InODF12() const            { return bUseSHA1InODF12; }
    sal_Bool                IsUseBlowfishInODF12() const        { return bUseBlowfishInODF12; }

    SvtSaveOptions::ODFDefaultVersion
                            GetODFDefaultVersion() const        { return eODFDefaultVersion; }

    void                    SetAutoSaveTime( sal_Int32 n );
    void                    SetUseUserData( sal_Bool b );
    void                    SetBackup( sal_Bool b );
    void                    SetAutoSave( sal_Bool b );
    void                    SetAutoSavePrompt( sal_Bool b );
    void                    SetUserAutoSave( sal_Bool b );
    void                    SetDocInfoSave( sal_Bool b );
    void                    SetSaveWorkingSet( sal_Bool b );
    void                    SetSaveDocView( sal_Bool b );
    void                    SetSaveRelINet( sal_Bool b );
    void                    SetSaveRelFSys( sal_Bool b );
    void                    EnablePrettyPrinting( sal_Bool _bDoPP );
    void                    SetWarnAlienFormat( sal_Bool _bDoPP );
    void                    SetLoadDocPrinter( sal_Bool bNew );
    void                    SetODFDefaultVersion( SvtSaveOptions::ODFDefaultVersion eNew );

    sal_Bool                IsReadOnly( SvtSaveOptions::EOption eOption ) const;
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

void SvtSaveOptions_Impl::SetUseUserData( sal_Bool b )
{
    if (!bROUseUserData && bUseUserData!=b)
    {
        bUseUserData = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetBackup( sal_Bool b )
{
    if (!bROBackup && bBackup!=b)
    {
        bBackup = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetAutoSave( sal_Bool b )
{
    if (!bROAutoSave && bAutoSave!=b)
    {
        bAutoSave = b;
        SetModified();
        Commit();
    }
}

void SvtSaveOptions_Impl::SetAutoSavePrompt( sal_Bool b )
{
    if (!bROAutoSavePrompt && bAutoSavePrompt!=b)
    {
        bAutoSavePrompt = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetUserAutoSave( sal_Bool b )
{
    if (!bROUserAutoSave && bUserAutoSave!=b)
    {
        bUserAutoSave = b;
        SetModified();
        Commit();
    }
}

void SvtSaveOptions_Impl::SetDocInfoSave(sal_Bool b)
{
    if (!bRODocInfSave && bDocInfSave!=b)
    {
        bDocInfSave = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetSaveWorkingSet( sal_Bool b )
{
    if (!bROSaveWorkingSet && bSaveWorkingSet!=b)
    {
        bSaveWorkingSet = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetSaveDocView( sal_Bool b )
{
    if (!bROSaveDocView && bSaveDocView!=b)
    {
        bSaveDocView = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetSaveRelINet( sal_Bool b )
{
    if (!bROSaveRelINet && bSaveRelINet!=b)
    {
        bSaveRelINet = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetSaveRelFSys( sal_Bool b )
{
    if (!bROSaveRelFSys && bSaveRelFSys!=b)
    {
        bSaveRelFSys = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::EnablePrettyPrinting( sal_Bool _bDoPP )
{
    if (!bRODoPrettyPrinting && bDoPrettyPrinting!=_bDoPP)
    {
        bDoPrettyPrinting = _bDoPP;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetWarnAlienFormat( sal_Bool _bDoPP )
{
    if (!bROWarnAlienFormat && bWarnAlienFormat!=_bDoPP)
    {
        bWarnAlienFormat = _bDoPP;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetLoadDocPrinter( sal_Bool bNew )
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

sal_Bool SvtSaveOptions_Impl::IsReadOnly( SvtSaveOptions::EOption eOption ) const
{
    sal_Bool bReadOnly = CFG_READONLY_DEFAULT;
    switch(eOption)
    {
        case SvtSaveOptions::E_AUTOSAVETIME :
            bReadOnly = bROAutoSaveTime;
            break;
        case SvtSaveOptions::E_USEUSERDATA :
            bReadOnly = bROUseUserData;
            break;
        case SvtSaveOptions::E_BACKUP :
            bReadOnly = bROBackup;
            break;
        case SvtSaveOptions::E_AUTOSAVE :
            bReadOnly = bROAutoSave;
            break;
        case SvtSaveOptions::E_AUTOSAVEPROMPT :
            bReadOnly = bROAutoSavePrompt;
            break;
        case SvtSaveOptions::E_USERAUTOSAVE :
            bReadOnly = bROUserAutoSave;
            break;
        case SvtSaveOptions::E_DOCINFSAVE :
            bReadOnly = bRODocInfSave;
            break;
        case SvtSaveOptions::E_SAVEWORKINGSET :
            bReadOnly = bROSaveWorkingSet;
            break;
        case SvtSaveOptions::E_SAVEDOCVIEW :
            bReadOnly = bROSaveDocView;
            break;
        case SvtSaveOptions::E_SAVERELINET :
            bReadOnly = bROSaveRelINet;
            break;
        case SvtSaveOptions::E_SAVERELFSYS :
            bReadOnly = bROSaveRelFSys;
            break;
        case SvtSaveOptions::E_SAVEUNPACKED :
            bReadOnly = bROSaveUnpacked;
            break;
        case SvtSaveOptions::E_DOPRETTYPRINTING :
            bReadOnly = bRODoPrettyPrinting;
            break;
        case SvtSaveOptions::E_WARNALIENFORMAT :
            bReadOnly = bROWarnAlienFormat;
            break;
        case SvtSaveOptions::E_LOADDOCPRINTER :
            bReadOnly = bROLoadDocPrinter;
            break;
        case SvtSaveOptions::E_ODFDEFAULTVERSION :
            bReadOnly = bROLoadDocPrinter;
            break;
        case SvtSaveOptions::E_USESHA1INODF12:
            bReadOnly = bROUseSHA1InODF12;
            break;
        case SvtSaveOptions::E_USEBLOWFISHINODF12:
            bReadOnly = bROUseBlowfishInODF12;
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

// -----------------------------------------------------------------------

SvtSaveOptions_Impl::SvtSaveOptions_Impl()
    : ConfigItem( OUString("Office.Common/Save") )
    , nAutoSaveTime( 0 )
    , bUseUserData( sal_False )
    , bBackup( sal_False )
    , bAutoSave( sal_False )
    , bAutoSavePrompt( sal_False )
    , bUserAutoSave( sal_False )
    , bDocInfSave( sal_False )
    , bSaveWorkingSet( sal_False )
    , bSaveDocView( sal_False )
    , bSaveRelINet( sal_False )
    , bSaveRelFSys( sal_False )
    , bSaveUnpacked( sal_False )
    , bDoPrettyPrinting( sal_False )
    , bWarnAlienFormat( sal_True )
    , bLoadDocPrinter( sal_True )
    , bUseSHA1InODF12( sal_False )
    , bUseBlowfishInODF12( sal_False )
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
                        sal_Bool bTemp = sal_Bool();
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

    bAutoSave = officecfg::Office::Recovery::AutoSave::Enabled::get();
    nAutoSaveTime = officecfg::Office::Recovery::AutoSave::TimeIntervall::get();
    bUserAutoSave = officecfg::Office::Recovery::AutoSave::UserAutoSaveEnabled::get();
}

SvtSaveOptions_Impl::~SvtSaveOptions_Impl()
{}

void SvtSaveOptions_Impl::Commit()
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

    boost::shared_ptr< comphelper::ConfigurationChanges > batch(
    comphelper::ConfigurationChanges::create());
    officecfg::Office::Recovery::AutoSave::TimeIntervall::set(nAutoSaveTime, batch);
    officecfg::Office::Recovery::AutoSave::Enabled::set(bAutoSave, batch);
    officecfg::Office::Recovery::AutoSave::UserAutoSaveEnabled::set(bUserAutoSave, batch);
    batch->commit();
}

// -----------------------------------------------------------------------

void SvtSaveOptions_Impl::Notify( const Sequence<OUString>& )
{
}


class SvtLoadOptions_Impl : public utl::ConfigItem
{

    sal_Bool                            bLoadUserDefinedSettings;

public:
                            SvtLoadOptions_Impl();
                            ~SvtLoadOptions_Impl();

    virtual void            Notify( const com::sun::star::uno::Sequence< OUString >& aPropertyNames );
    virtual void            Commit();

    void                    SetLoadUserSettings(sal_Bool b){bLoadUserDefinedSettings = b; SetModified();}
    sal_Bool                IsLoadUserSettings() const {return bLoadUserDefinedSettings;}
};
// -----------------------------------------------------------------------
const sal_Char cUserDefinedSettings[] = "UserDefinedSettings";

SvtLoadOptions_Impl::SvtLoadOptions_Impl()
    : ConfigItem( OUString("Office.Common/Load") )
    , bLoadUserDefinedSettings( sal_False )
{
    Sequence< OUString > aNames(1);
    aNames[0] = OUString::createFromAscii(cUserDefinedSettings);
    Sequence< Any > aValues = GetProperties( aNames );
    EnableNotification( aNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
    if (pValues[0].getValueTypeClass() == ::com::sun::star::uno::TypeClass_BOOLEAN)
         bLoadUserDefinedSettings = *(sal_Bool *)pValues[0].getValue();
}
// -----------------------------------------------------------------------
SvtLoadOptions_Impl::~SvtLoadOptions_Impl()
{
}
// -----------------------------------------------------------------------
void SvtLoadOptions_Impl::Commit()
{
    Sequence< OUString > aNames(1);
    aNames[0] = OUString::createFromAscii(cUserDefinedSettings);
    Sequence< Any > aValues( 1 );
    aValues[0].setValue(&bLoadUserDefinedSettings, ::getBooleanCppuType());
    PutProperties( aNames, aValues );
}
// -----------------------------------------------------------------------
void SvtLoadOptions_Impl::Notify( const Sequence<OUString>& )
{
    SAL_WARN( "unotools.config", "properties have been changed" );
}
// -----------------------------------------------------------------------

namespace
{
    class LocalSingleton : public rtl::Static< osl::Mutex, LocalSingleton >
    {
    };
}

// -----------------------------------------------------------------------
SvtSaveOptions::SvtSaveOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( LocalSingleton::get() );
    if ( !pOptions )
    {
        pOptions = new SvtLoadSaveOptions_Impl;
        pOptions->pSaveOpt = new SvtSaveOptions_Impl;
        pOptions->pLoadOpt = new SvtLoadOptions_Impl;

        ItemHolder1::holdConfigItem(E_SAVEOPTIONS);
   }
   ++nRefCount;
    pImp = pOptions;
}

// -----------------------------------------------------------------------

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

void SvtSaveOptions::SetUseUserData( sal_Bool b )
{
    pImp->pSaveOpt->SetUseUserData( b );
}

sal_Bool SvtSaveOptions::IsUseUserData() const
{
    return pImp->pSaveOpt->IsUseUserData();
}

void SvtSaveOptions::SetBackup( sal_Bool b )
{
    pImp->pSaveOpt->SetBackup( b );
}

sal_Bool SvtSaveOptions::IsBackup() const
{
    return pImp->pSaveOpt->IsBackup();
}

void SvtSaveOptions::SetAutoSave( sal_Bool b )
{
    pImp->pSaveOpt->SetAutoSave( b );
}

sal_Bool SvtSaveOptions::IsAutoSave() const
{
    return pImp->pSaveOpt->IsAutoSave();
}

void SvtSaveOptions::SetAutoSavePrompt( sal_Bool b )
{
    pImp->pSaveOpt->SetAutoSavePrompt( b );
}

sal_Bool SvtSaveOptions::IsAutoSavePrompt() const
{
    return pImp->pSaveOpt->IsAutoSavePrompt();
}

void SvtSaveOptions::SetUserAutoSave( sal_Bool b )
{
    pImp->pSaveOpt->SetUserAutoSave( b );
}

sal_Bool SvtSaveOptions::IsUserAutoSave() const
{
    return pImp->pSaveOpt->IsUserAutoSave();
}

void SvtSaveOptions::SetDocInfoSave(sal_Bool b)
{
    pImp->pSaveOpt->SetDocInfoSave( b );
}

sal_Bool SvtSaveOptions::IsDocInfoSave() const
{
    return pImp->pSaveOpt->IsDocInfoSave();
}

void SvtSaveOptions::SetSaveWorkingSet( sal_Bool b )
{
    pImp->pSaveOpt->SetSaveWorkingSet( b );
}

sal_Bool SvtSaveOptions::IsSaveWorkingSet() const
{
    return pImp->pSaveOpt->IsSaveWorkingSet();
}

void SvtSaveOptions::SetSaveDocView( sal_Bool b )
{
    pImp->pSaveOpt->SetSaveDocView( b );
}

sal_Bool SvtSaveOptions::IsSaveDocView() const
{
    return pImp->pSaveOpt->IsSaveDocView();
}

void SvtSaveOptions::SetSaveRelINet( sal_Bool b )
{
    pImp->pSaveOpt->SetSaveRelINet( b );
}

sal_Bool SvtSaveOptions::IsSaveRelINet() const
{
    return pImp->pSaveOpt->IsSaveRelINet();
}

void SvtSaveOptions::SetSaveRelFSys( sal_Bool b )
{
    pImp->pSaveOpt->SetSaveRelFSys( b );
}

sal_Bool SvtSaveOptions::IsSaveRelFSys() const
{
    return pImp->pSaveOpt->IsSaveRelFSys();
}

sal_Bool SvtSaveOptions::IsSaveUnpacked() const
{
    return pImp->pSaveOpt->IsSaveUnpacked();
}

void SvtSaveOptions::SetLoadUserSettings(sal_Bool b)
{
    pImp->pLoadOpt->SetLoadUserSettings(b);
}

sal_Bool   SvtSaveOptions::IsLoadUserSettings() const
{
    return pImp->pLoadOpt->IsLoadUserSettings();
}

void SvtSaveOptions::SetPrettyPrinting( sal_Bool _bEnable )
{
    pImp->pSaveOpt->EnablePrettyPrinting( _bEnable );
}

sal_Bool SvtSaveOptions::IsPrettyPrinting() const
{
    return pImp->pSaveOpt->IsPrettyPrintingEnabled();
}

void SvtSaveOptions::SetWarnAlienFormat( sal_Bool _bEnable )
{
    pImp->pSaveOpt->SetWarnAlienFormat( _bEnable );
}

sal_Bool SvtSaveOptions::IsWarnAlienFormat() const
{
    return pImp->pSaveOpt->IsWarnAlienFormat();
}

void SvtSaveOptions::SetLoadDocumentPrinter( sal_Bool _bEnable )
{
    pImp->pSaveOpt->SetLoadDocPrinter( _bEnable );
}

sal_Bool SvtSaveOptions::IsLoadDocumentPrinter() const
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

sal_Bool SvtSaveOptions::IsUseSHA1InODF12() const
{
    return pImp->pSaveOpt->IsUseSHA1InODF12();
}

sal_Bool SvtSaveOptions::IsUseBlowfishInODF12() const
{
    return pImp->pSaveOpt->IsUseBlowfishInODF12();
}

sal_Bool SvtSaveOptions::IsReadOnly( SvtSaveOptions::EOption eOption ) const
{
    return pImp->pSaveOpt->IsReadOnly(eOption);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
