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


#ifndef GCC
#endif

#include <bf_svtools/saveopt.hxx>

#ifndef INCLUDED_RTL_INSTANCE_HXX
#include "rtl/instance.hxx"
#endif

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#include <osl/mutex.hxx>
#include <comphelper/configurationhelper.hxx>
#include <unotools/processfactory.hxx>
#include <rtl/logfile.hxx>
#include "itemholder1.hxx"

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;
namespace css = ::com::sun::star;

namespace binfilter
{

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
                                        bSaveDocWins,
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
                                        bROSaveDocWins,
                                        bROSaveDocView,
                                        bROSaveRelINet,
                                        bROSaveRelFSys,
                                        bROSaveUnpacked,
                                        bROWarnAlienFormat,
                                        bRODoPrettyPrinting,
                                        bROLoadDocPrinter;
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
    BOOL                    IsSaveDocWins() const               { return bSaveDocWins; }
    BOOL                    IsSaveDocView() const               { return bSaveDocView; }
    BOOL                    IsSaveRelINet() const               { return bSaveRelINet; }
    BOOL                    IsSaveRelFSys() const               { return bSaveRelFSys; }
    BOOL                    IsSaveUnpacked() const              { return bSaveUnpacked; }
    sal_Bool                IsPrettyPrintingEnabled( ) const    { return bDoPrettyPrinting; }
    sal_Bool                IsWarnAlienFormat() const           { return bWarnAlienFormat; }
    sal_Bool                IsLoadDocPrinter() const            { return bLoadDocPrinter; }
};

#define FORMAT			 0
#define TIMEINTERVALL	 1
#define USEUSERDATA		 2
#define CREATEBACKUP	 3
#define AUTOSAVE		 4
#define PROMPT			 5
#define EDITPROPERTY	 6
#define SAVEDOCWINS		 7
#define SAVEVIEWINFO	 8
#define UNPACKED		 9
#define PRETTYPRINTING	10
#define WARNALIENFORMAT 11
#define LOADDOCPRINTER  12
#define FILESYSTEM      13
#define INTERNET        14
#define SAVEWORKINGSET  15

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
        "Document/DocumentWindows",
        "Document/ViewInfo",
        "Document/Unpacked",
        "Document/PrettyPrinting",
        "Document/WarnAlienFormat",
        "Document/LoadPrinter",
        "URL/FileSystem",
        "URL/Internet",
        "WorkingSet",
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
    : ConfigItem( OUString::createFromAscii("Office.Common/Save") )
    , nAutoSaveTime( 0 )
    , bUseUserData( sal_False )
    , bBackup( sal_False )
    , bAutoSave( sal_False )
    , bAutoSavePrompt( sal_False )
    , bDocInfSave( sal_False )
    , bSaveWorkingSet( sal_False )
    , bSaveDocWins( sal_False )
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
    , bROSaveDocWins( CFG_READONLY_DEFAULT )
    , bROSaveDocView( CFG_READONLY_DEFAULT )
    , bROSaveRelINet( CFG_READONLY_DEFAULT )
    , bROSaveRelFSys( CFG_READONLY_DEFAULT )
    , bROSaveUnpacked( CFG_READONLY_DEFAULT )
    , bROWarnAlienFormat( CFG_READONLY_DEFAULT )
    , bRODoPrettyPrinting( CFG_READONLY_DEFAULT )
    , bROLoadDocPrinter( CFG_READONLY_DEFAULT )
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
                    case TIMEINTERVALL :
                        if ( pValues[nProp] >>= nTemp )
                            nAutoSaveTime = nTemp;
                        else 
                        {
                            DBG_ERROR( "Wrong Type!" );
                        }
                        bROAutoSaveTime = pROStates[nProp];
                        break;

                    case FORMAT:
                        // not supported anymore
                        break;

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
                                case SAVEDOCWINS :
                                    bSaveDocWins = bTemp;
                                    bROSaveDocWins = pROStates[nProp];
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
        ::rtl::OUString::createFromAscii("org.openoffice.Office.Recovery"),
        ::comphelper::ConfigurationHelper::E_READONLY);

    ::comphelper::ConfigurationHelper::readRelativeKey(
        xCFG,
        ::rtl::OUString::createFromAscii("AutoSave"),
        ::rtl::OUString::createFromAscii("Enabled")) >>= bAutoSave;

    ::comphelper::ConfigurationHelper::readRelativeKey(
        xCFG,
        ::rtl::OUString::createFromAscii("AutoSave"),
        ::rtl::OUString::createFromAscii("TimeIntervall")) >>= nAutoSaveTime;
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
            case SAVEDOCWINS :
                if (!bROSaveDocWins)
                {
                    pValues[nRealCount] <<= bSaveDocWins;
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

            default:
                DBG_ERRORFILE( "invalid index to save a path" );
        }
    }

    aNames.realloc(nRealCount);
    aValues.realloc(nRealCount);
    PutProperties( aNames, aValues );

    css::uno::Reference< css::uno::XInterface > xCFG = ::comphelper::ConfigurationHelper::openConfig(
        ::utl::getProcessServiceFactory(),
        ::rtl::OUString::createFromAscii("org.openoffice.Office.Recovery"),
        ::comphelper::ConfigurationHelper::E_STANDARD);

    ::comphelper::ConfigurationHelper::writeRelativeKey(
        xCFG,
        ::rtl::OUString::createFromAscii("AutoSave"),
        ::rtl::OUString::createFromAscii("TimeIntervall"),
        css::uno::makeAny(nAutoSaveTime));

    ::comphelper::ConfigurationHelper::writeRelativeKey(
        xCFG,
        ::rtl::OUString::createFromAscii("AutoSave"),
        ::rtl::OUString::createFromAscii("Enabled"),
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
    : ConfigItem( OUString::createFromAscii("Office.Common/Load") )
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
        RTL_LOGFILE_CONTEXT(aLog, "svtools ( ??? ) ::SvtSaveOptions_Impl::ctor()");
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

sal_Bool SvtSaveOptions::IsBackup() const
{
    return pImp->pSaveOpt->IsBackup();
}

sal_Bool SvtSaveOptions::IsSaveDocView() const
{
    return pImp->pSaveOpt->IsSaveDocView();
}

sal_Bool SvtSaveOptions::IsSaveRelINet() const
{
    return pImp->pSaveOpt->IsSaveRelINet();
}

sal_Bool SvtSaveOptions::IsSaveRelFSys() const
{
    return pImp->pSaveOpt->IsSaveRelFSys();
}

sal_Bool SvtSaveOptions::IsSaveUnpacked() const
{
    return pImp->pSaveOpt->IsSaveUnpacked();
}

sal_Bool   SvtSaveOptions::IsLoadUserSettings() const
{
    return pImp->pLoadOpt->IsLoadUserSettings();
}

sal_Bool SvtSaveOptions::IsPrettyPrinting() const
{
    return pImp->pSaveOpt->IsPrettyPrintingEnabled();
}

}
