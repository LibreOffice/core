/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

#include <unotools/saveopt.hxx>
#include "rtl/instance.hxx"
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <osl/mutex.hxx>
#include <comphelper/configurationhelper.hxx>
#include <unotools/processfactory.hxx>
#include <rtl/logfile.hxx>
#include "itemholder1.hxx"

using namespace utl;
using namespace com::sun::star::uno;

using ::rtl::OUString;

namespace css = ::com::sun::star;

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
                                        bDocInfSave,
                                        bSaveWorkingSet,
                                        bSaveDocView,
                                        bSaveRelINet,
                                        bSaveRelFSys,
                                        bSaveUnpacked,
                                        bDoPrettyPrinting,
                                        bWarnAlienFormat,
                                        bLoadDocPrinter;

    sal_Bool                            bROAutoSaveTime,
                                        bROUseUserData,
                                        bROBackup,
                                        bROAutoSave,
                                        bROAutoSavePrompt,
                                        bRODocInfSave,
                                        bROSaveWorkingSet,
                                        bROSaveDocView,
                                        bROSaveRelINet,
                                        bROSaveRelFSys,
                                        bROSaveUnpacked,
                                        bROWarnAlienFormat,
                                        bRODoPrettyPrinting,
                                        bROLoadDocPrinter,
                                        bROODFDefaultVersion;

    SvtSaveOptions::ODFDefaultVersion   eODFDefaultVersion;

public:
                            SvtSaveOptions_Impl();
                            ~SvtSaveOptions_Impl();

    virtual void            Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void            Commit();

    sal_Int32               GetAutoSaveTime() const             { return nAutoSaveTime; }
    BOOL                    IsUseUserData() const               { return bUseUserData; }
    BOOL                    IsBackup() const                    { return bBackup; }
    BOOL                    IsAutoSave() const                  { return bAutoSave; }
    BOOL                    IsAutoSavePrompt() const            { return bAutoSavePrompt; }
    BOOL                    IsDocInfoSave() const               { return bDocInfSave; }
    BOOL                    IsSaveWorkingSet() const            { return bSaveWorkingSet;         }
    BOOL                    IsSaveDocView() const               { return bSaveDocView; }
    BOOL                    IsSaveRelINet() const               { return bSaveRelINet; }
    BOOL                    IsSaveRelFSys() const               { return bSaveRelFSys; }
    BOOL                    IsSaveUnpacked() const              { return bSaveUnpacked; }
    sal_Bool                IsPrettyPrintingEnabled( ) const    { return bDoPrettyPrinting; }
    sal_Bool                IsWarnAlienFormat() const           { return bWarnAlienFormat; }
    sal_Bool                IsLoadDocPrinter() const            { return bLoadDocPrinter; }
    SvtSaveOptions::ODFDefaultVersion
                            GetODFDefaultVersion() const        { return eODFDefaultVersion; }

    void                    SetAutoSaveTime( sal_Int32 n );
    void                    SetUseUserData( BOOL b );
    void                    SetBackup( BOOL b );
    void                    SetAutoSave( BOOL b );
    void                    SetAutoSavePrompt( BOOL b );
    void                    SetDocInfoSave( BOOL b );
    void                    SetSaveWorkingSet( BOOL b );
    void                    SetSaveDocView( BOOL b );
    void                    SetSaveRelINet( BOOL b );
    void                    SetSaveRelFSys( BOOL b );
    void                    SetSaveUnpacked( BOOL b );
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

void SvtSaveOptions_Impl::SetUseUserData( BOOL b )
{
    if (!bROUseUserData && bUseUserData!=b)
    {
        bUseUserData = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetBackup( BOOL b )
{
    if (!bROBackup && bBackup!=b)
    {
        bBackup = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetAutoSave( BOOL b )
{
    if (!bROAutoSave && bAutoSave!=b)
    {
        bAutoSave = b;
        SetModified();
        Commit();
    }
}

void SvtSaveOptions_Impl::SetAutoSavePrompt( BOOL b )
{
    if (!bROAutoSavePrompt && bAutoSavePrompt!=b)
    {
        bAutoSavePrompt = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetDocInfoSave(BOOL b)
{
    if (!bRODocInfSave && bDocInfSave!=b)
    {
        bDocInfSave = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetSaveWorkingSet( BOOL b )
{
    if (!bROSaveWorkingSet && bSaveWorkingSet!=b)
    {
        bSaveWorkingSet = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetSaveDocView( BOOL b )
{
    if (!bROSaveDocView && bSaveDocView!=b)
    {
        bSaveDocView = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetSaveRelINet( BOOL b )
{
    if (!bROSaveRelINet && bSaveRelINet!=b)
    {
        bSaveRelINet = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetSaveRelFSys( BOOL b )
{
    if (!bROSaveRelFSys && bSaveRelFSys!=b)
    {
        bSaveRelFSys = b;
        SetModified();
    }
}

void SvtSaveOptions_Impl::SetSaveUnpacked( BOOL b )
{
    if (!bROSaveUnpacked && bSaveUnpacked!=b)
    {
        bSaveUnpacked = b;
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
        "ODF/DefaultVersion"
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
    : ConfigItem( OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/Save")) )
    , nAutoSaveTime( 0 )
    , bUseUserData( sal_False )
    , bBackup( sal_False )
    , bAutoSave( sal_False )
    , bAutoSavePrompt( sal_False )
    , bDocInfSave( sal_False )
    , bSaveWorkingSet( sal_False )
    , bSaveDocView( sal_False )
    , bSaveRelINet( sal_False )
    , bSaveRelFSys( sal_False )
    , bSaveUnpacked( sal_False )
    , bDoPrettyPrinting( sal_False )
    , bWarnAlienFormat( sal_True )
    , bLoadDocPrinter( sal_True )
    , bROAutoSaveTime( CFG_READONLY_DEFAULT )
    , bROUseUserData( CFG_READONLY_DEFAULT )
    , bROBackup( CFG_READONLY_DEFAULT )
    , bROAutoSave( CFG_READONLY_DEFAULT )
    , bROAutoSavePrompt( CFG_READONLY_DEFAULT )
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
    , eODFDefaultVersion( SvtSaveOptions::ODFVER_LATEST )
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
            DBG_ASSERT( pValues[nProp].hasValue(), "property value missing" );
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
                            DBG_ERROR( "Wrong Type!" );
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
                            DBG_ERRORFILE( "SvtSaveOptions_Impl::SvtSaveOptions_Impl(): Wrong Type!" );
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

                                default :
                                    DBG_ERRORFILE( "invalid index to load a path" );
                            }
                        }
                        else
                        {
                            DBG_ERROR( "Wrong Type!" );
                        }
                    }
                }
            }
        }
    }

    try
    {
    css::uno::Reference< css::uno::XInterface > xCFG = ::comphelper::ConfigurationHelper::openConfig(
        ::utl::getProcessServiceFactory(),
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Recovery")),
        ::comphelper::ConfigurationHelper::E_READONLY);

    ::comphelper::ConfigurationHelper::readRelativeKey(
        xCFG,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AutoSave")),
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Enabled"))) >>= bAutoSave;

    ::comphelper::ConfigurationHelper::readRelativeKey(
        xCFG,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AutoSave")),
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TimeIntervall"))) >>= nAutoSaveTime;
    }
    catch(const css::uno::Exception&)
        { DBG_ERROR("Could not find needed informations for AutoSave feature."); }
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
            default:
                DBG_ERRORFILE( "invalid index to save a path" );
        }
    }

    aNames.realloc(nRealCount);
    aValues.realloc(nRealCount);
    PutProperties( aNames, aValues );

    css::uno::Reference< css::uno::XInterface > xCFG = ::comphelper::ConfigurationHelper::openConfig(
        ::utl::getProcessServiceFactory(),
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Recovery")),
        ::comphelper::ConfigurationHelper::E_STANDARD);

    ::comphelper::ConfigurationHelper::writeRelativeKey(
        xCFG,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AutoSave")),
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TimeIntervall")),
        css::uno::makeAny(nAutoSaveTime));

    ::comphelper::ConfigurationHelper::writeRelativeKey(
        xCFG,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AutoSave")),
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Enabled")),
        css::uno::makeAny(bAutoSave));

    ::comphelper::ConfigurationHelper::flush(xCFG);
}

// -----------------------------------------------------------------------

void SvtSaveOptions_Impl::Notify( const Sequence<rtl::OUString>& )
{
}


class SvtLoadOptions_Impl : public utl::ConfigItem
{

    sal_Bool                            bLoadUserDefinedSettings;

public:
                            SvtLoadOptions_Impl();
                            ~SvtLoadOptions_Impl();

    virtual void            Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void            Commit();

    void                    SetLoadUserSettings(sal_Bool b){bLoadUserDefinedSettings = b; SetModified();}
    sal_Bool                IsLoadUserSettings() const {return bLoadUserDefinedSettings;}
};
// -----------------------------------------------------------------------
const sal_Char cUserDefinedSettings[] = "UserDefinedSettings";

SvtLoadOptions_Impl::SvtLoadOptions_Impl()
    : ConfigItem( OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/Load")) )
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
void SvtLoadOptions_Impl::Notify( const Sequence<rtl::OUString>& )
{
    DBG_ERRORFILE( "properties have been changed" );
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
        RTL_LOGFILE_CONTEXT(aLog, "unotools ( ??? ) ::SvtSaveOptions_Impl::ctor()");
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

void SvtSaveOptions::SetSaveUnpacked( sal_Bool b )
{
    pImp->pSaveOpt->SetSaveUnpacked( b );
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

sal_Bool SvtSaveOptions::IsReadOnly( SvtSaveOptions::EOption eOption ) const
{
    return pImp->pSaveOpt->IsReadOnly(eOption);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
