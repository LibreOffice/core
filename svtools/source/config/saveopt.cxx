/*************************************************************************
 *
 *  $RCSfile: saveopt.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 14:37:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include "saveopt.hxx"

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
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

using namespace utl;
using namespace rtl;
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
    SvtSaveOptions::SaveGraphicsMode    eSaveGraphics;
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
                                        bDoPrettyPrinting;

    sal_Bool                            bROAutoSaveTime,
                                        bROSaveGraphics,
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
                                        bRODoPrettyPrinting;
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
    SvtSaveOptions::SaveGraphicsMode        GetSaveGraphicsMode() const         { return eSaveGraphics; }
    BOOL                    IsSaveWorkingSet() const            { return bSaveWorkingSet;         }
    BOOL                    IsSaveDocWins() const               { return bSaveDocWins; }
    BOOL                    IsSaveDocView() const               { return bSaveDocView; }
    BOOL                    IsSaveRelINet() const               { return bSaveRelINet; }
    BOOL                    IsSaveRelFSys() const               { return bSaveRelFSys; }
    BOOL                    IsSaveUnpacked() const              { return bSaveUnpacked; }
    sal_Bool                IsPrettyPrintingEnabled( ) const    { return bDoPrettyPrinting; }

    void                    SetAutoSaveTime( sal_Int32 n );
    void                    SetUseUserData( BOOL b );
    void                    SetBackup( BOOL b );
    void                    SetAutoSave( BOOL b );
    void                    SetAutoSavePrompt( BOOL b );
    void                    SetDocInfoSave( BOOL b );
    void                    SetSaveGraphicsMode( SvtSaveOptions::SaveGraphicsMode eMode );
    void                    SetSaveWorkingSet( BOOL b );
    void                    SetSaveDocWins( BOOL b );
    void                    SetSaveDocView( BOOL b );
    void                    SetSaveRelINet( BOOL b );
    void                    SetSaveRelFSys( BOOL b );
    void                    SetSaveUnpacked( BOOL b );
    void                    EnablePrettyPrinting( sal_Bool _bDoPP );

    sal_Bool                IsReadOnly( SvtSaveOptions::EOption eOption ) const;
};

void SvtSaveOptions_Impl::SetAutoSaveTime( sal_Int32 n )
{
    if (!bROAutoSaveTime && nAutoSaveTime!=n)
    {
        nAutoSaveTime = n;
        SetModified();
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

void SvtSaveOptions_Impl::SetSaveGraphicsMode( SvtSaveOptions::SaveGraphicsMode eMode )
{
    if (!bROSaveGraphics && eSaveGraphics!=eMode)
    {
        eSaveGraphics = eMode;
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

void SvtSaveOptions_Impl::SetSaveDocWins( BOOL b )
{
    if (!bROSaveDocWins && bSaveDocWins!=b)
    {
        bSaveDocWins = b;
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

sal_Bool SvtSaveOptions_Impl::IsReadOnly( SvtSaveOptions::EOption eOption ) const
{
    sal_Bool bReadOnly = CFG_READONLY_DEFAULT;
    switch(eOption)
    {
        case SvtSaveOptions::E_AUTOSAVETIME :
            bReadOnly = bROAutoSaveTime;
            break;
        case SvtSaveOptions::E_SAVEGRAPHICS :
            bReadOnly = bROSaveGraphics;
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
        case SvtSaveOptions::E_SAVEDOCWINS :
            bReadOnly = bROSaveDocWins;
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
    }
    return bReadOnly;
}

#define FORMAT           0
#define TIMEINTERVALL    1
#define USEUSERDATA      2
#define CREATEBACKUP     3
#define AUTOSAVE         4
#define PROMPT           5
#define EDITPROPERTY     6
#define SAVEDOCWINS      7
#define SAVEVIEWINFO     8
#define UNPACKED         9
#define PRETTYPRINTING  10
#define FILESYSTEM      11
#define INTERNET        12
#define SAVEWORKINGSET  13

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
    , eSaveGraphics( SvtSaveOptions::SaveGraphicsNormal )
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
    , bROAutoSaveTime( CFG_READONLY_DEFAULT )
    , bROSaveGraphics( CFG_READONLY_DEFAULT )
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
    , bRODoPrettyPrinting( CFG_READONLY_DEFAULT )
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
                sal_Int32 nTemp;
                switch ( nProp )
                {
                    case TIMEINTERVALL :
                        if ( pValues[nProp] >>= nTemp )
                            nAutoSaveTime = nTemp;
                        else
                            DBG_ERROR( "Wrong Type!" );
                        bROAutoSaveTime = pROStates[nProp];
                        break;

                    case FORMAT :
                        if ( pValues[nProp] >>= nTemp )
                            eSaveGraphics = (SvtSaveOptions::SaveGraphicsMode) nTemp;
                        else
                            DBG_ERROR( "Wrong Type!" );
                        bROSaveGraphics = pROStates[nProp];
                        break;

                    default:
                    {
                        sal_Bool bTemp;
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

                                default :
                                    DBG_ERRORFILE( "invalid index to load a path" );
                            }
                        }
                        else
                            DBG_ERROR( "Wrong Type!" );

                    }
                }
            }
        }
    }
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
            case FORMAT :
                if (!bROSaveGraphics)
                {
                    pValues[nRealCount] <<= (sal_Int16)eSaveGraphics;
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

            default:
                DBG_ERRORFILE( "invalid index to save a path" );
        }
    }

    aNames.realloc(nRealCount);
    aValues.realloc(nRealCount);
    PutProperties( aNames, aValues );
}

// -----------------------------------------------------------------------

void SvtSaveOptions_Impl::Notify( const Sequence<rtl::OUString>& aPropertyNames )
{
    DBG_ERRORFILE( "properties have been changed" );
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
void SvtLoadOptions_Impl::Notify( const Sequence<rtl::OUString>& aPropertyNames )
{
    DBG_ERRORFILE( "properties have been changed" );
}
// -----------------------------------------------------------------------

SvtSaveOptions::SvtSaveOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() );
    if ( !pOptions )
    {
        pOptions = new SvtLoadSaveOptions_Impl;
        pOptions->pSaveOpt = new SvtSaveOptions_Impl;
        pOptions->pLoadOpt = new SvtLoadOptions_Impl;
    }
    ++nRefCount;
    pImp = pOptions;
}

// -----------------------------------------------------------------------

SvtSaveOptions::~SvtSaveOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
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

void SvtSaveOptions::SetSaveDocWins( sal_Bool b )
{
    pImp->pSaveOpt->SetSaveDocWins( b );
}

sal_Bool SvtSaveOptions::IsSaveDocWins() const
{
    return pImp->pSaveOpt->IsSaveDocWins();
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
SvtSaveOptions::SaveGraphicsMode SvtSaveOptions::GetSaveGraphicsMode() const
{
    return pImp->pSaveOpt->GetSaveGraphicsMode();
}

void SvtSaveOptions::SetSaveGraphicsMode( SvtSaveOptions::SaveGraphicsMode eMode )
{
    // #87097#: don't allow setting of this property (it isn't needed anymore)
    // pImp->pSaveOpt->SetSaveGraphicsMode( eMode );
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

sal_Bool SvtSaveOptions::IsReadOnly( SvtSaveOptions::EOption eOption ) const
{
    return pImp->pSaveOpt->IsReadOnly(eOption);
}
