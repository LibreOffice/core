/*************************************************************************
 *
 *  $RCSfile: saveopt.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: os $ $Date: 2001-07-02 07:45:12 $
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
                                        bSaveUnpacked;
public:
                            SvtSaveOptions_Impl();
                            ~SvtSaveOptions_Impl();

    virtual void            Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void            Commit();

    void                    SetAutoSaveTime( sal_Int32 n )      { nAutoSaveTime = n; SetModified();  }
    sal_Int32               GetAutoSaveTime() const             { return nAutoSaveTime; }
    void                    SetUseUserData( BOOL b )            { bUseUserData = b; SetModified();}
    BOOL                    IsUseUserData() const               { return bUseUserData; }
    void                    SetBackup( BOOL b )                 { bBackup = b; SetModified();}
    BOOL                    IsBackup() const                    { return bBackup; }
    void                    SetAutoSave( BOOL b )               { bAutoSave = b; SetModified();    }
    BOOL                    IsAutoSave() const                  { return bAutoSave; }
    void                    SetAutoSavePrompt( BOOL b )         { bAutoSavePrompt = b; SetModified();  }
    BOOL                    IsAutoSavePrompt() const            { return bAutoSavePrompt; }
    void                    SetDocInfoSave(BOOL b)              { bDocInfSave = b; SetModified(); }
    BOOL                    IsDocInfoSave() const               { return bDocInfSave; }
    void                    SetSaveGraphicsMode( SvtSaveOptions::SaveGraphicsMode eMode )   { eSaveGraphics = eMode; SetModified(); }
    SvtSaveOptions::SaveGraphicsMode        GetSaveGraphicsMode() const         { return eSaveGraphics; }
    void                    SetSaveWorkingSet( BOOL b )         { bSaveWorkingSet = b; SetModified();}
    BOOL                    IsSaveWorkingSet() const            { return bSaveWorkingSet;         }
    void                    SetSaveDocWins( BOOL b )            { bSaveDocWins = b; SetModified();}
    BOOL                    IsSaveDocWins() const               { return bSaveDocWins; }
    void                    SetSaveDocView( BOOL b )            { bSaveDocView = b; SetModified();}
    BOOL                    IsSaveDocView() const               { return bSaveDocView; }
    void                    SetSaveRelINet( BOOL b )            { bSaveRelINet = b; SetModified();}
    BOOL                    IsSaveRelINet() const               { return bSaveRelINet; }
    void                    SetSaveRelFSys( BOOL b )            { bSaveRelFSys = b; SetModified();}
    BOOL                    IsSaveRelFSys() const               { return bSaveRelFSys; }
    void                    SetSaveUnpacked( BOOL b )           { bSaveUnpacked = b; SetModified();}
    BOOL                    IsSaveUnpacked() const              { return bSaveUnpacked; }
};

#define FORMAT 0
#define TIMEINTERVALL 1
#define USEUSERDATA 2
#define CREATEBACKUP 3
#define AUTOSAVE 4
#define PROMPT 5
#define EDITPROPERTY 6
#define SAVEDOCWINS 7
#define SAVEVIEWINFO 8
#define UNPACKED 9
#define FILESYSTEM 10
#define INTERNET 11
#define SAVEWORKINGSET 12

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
        "URL/FileSystem",
        "URL/Internet",
        "WorkingSet"
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
{
    Sequence< OUString > aNames = GetPropertyNames();
    Sequence< Any > aValues = GetProperties( aNames );
    EnableNotification( aNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
    if ( aValues.getLength() == aNames.getLength() )
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
                        break;

                    case FORMAT :
                        if ( pValues[nProp] >>= nTemp )
                            eSaveGraphics = (SvtSaveOptions::SaveGraphicsMode) nTemp;
                        else
                            DBG_ERROR( "Wrong Type!" );
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
                                    break;
                                case CREATEBACKUP :
                                    bBackup = bTemp;
                                    break;
                                case AUTOSAVE :
                                    bAutoSave = bTemp;
                                    break;
                                case PROMPT :
                                    bAutoSavePrompt = bTemp;
                                    break;
                                case EDITPROPERTY :
                                    bDocInfSave = bTemp;
                                    break;
                                case SAVEWORKINGSET :
                                    bSaveWorkingSet = bTemp;
                                    break;
                                case SAVEDOCWINS :
                                    bSaveDocWins = bTemp;
                                    break;
                                case SAVEVIEWINFO :
                                    bSaveDocView = bTemp;
                                    break;
                                case FILESYSTEM :
                                    bSaveRelFSys = bTemp;
                                    break;
                                case INTERNET :
                                    bSaveRelINet = bTemp;
                                    break;
                                case UNPACKED :
                                    bSaveUnpacked = bTemp;
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
    Sequence< OUString > aNames = GetPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence< Any > aValues( aNames.getLength() );
    Any* pValues = aValues.getArray();
    for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
    {
        switch ( nProp )
        {
            case TIMEINTERVALL :
                pValues[nProp] <<= nAutoSaveTime;
                break;
            case FORMAT :
                pValues[nProp] <<= (sal_Int16 ) eSaveGraphics;
                break;
            case USEUSERDATA :
                pValues[nProp] <<= bUseUserData;
                break;
            case CREATEBACKUP :
                pValues[nProp] <<= bBackup;
                break;
            case AUTOSAVE :
                pValues[nProp] <<= bAutoSave;
                break;
            case PROMPT :
                pValues[nProp] <<= bAutoSavePrompt;
                break;
            case EDITPROPERTY :
                pValues[nProp] <<= bDocInfSave;
                break;
            case SAVEWORKINGSET :
                pValues[nProp] <<= bSaveWorkingSet;
                break;
            case SAVEDOCWINS :
                pValues[nProp] <<= bSaveDocWins;
                break;
            case SAVEVIEWINFO :
                pValues[nProp] <<= bSaveDocView;
                break;
            case FILESYSTEM :
                pValues[nProp] <<= bSaveRelFSys;
                break;
            case INTERNET :
                pValues[nProp] <<= bSaveRelINet;
                break;
            case UNPACKED :
                pValues[nProp] <<= bSaveUnpacked;
                break;
            default:
                DBG_ERRORFILE( "invalid index to save a path" );
        }
    }

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

void       SvtSaveOptions::SetLoadUserSettings(sal_Bool b)
{
    pImp->pLoadOpt->SetLoadUserSettings(b);
}

sal_Bool   SvtSaveOptions::IsLoadUserSettings() const
{
    return pImp->pLoadOpt->IsLoadUserSettings();
}

