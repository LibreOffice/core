/*************************************************************************
 *
 *  $RCSfile: saveopt.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mba $ $Date: 2000-09-20 12:29:15 $
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

static SvtSaveOptions_Impl* pOptions = NULL;
static sal_Int32           nRefCount = 0;

class SvtSaveOptions_Impl : public utl::ConfigItem
{
    sal_Int32               nAutoSaveTime;
    sal_Int32               nSaveGraphics;
    BOOL                    bUseUserData:1,
                            bBackup:1,
                            bAutoSave:1,
                            bAutoSavePrompt:1,
                            bDocInfSave:1,
                            bSaveWorkingSet:1,
                            bSaveDocWins:1,
                            bSaveDocView:1,
                            bSaveRelINet:1,
                            bSaveRelFSys:1,
                            bIndepGrfFmt:1;
public:
                            SvtSaveOptions_Impl();

    virtual void            Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void            Commit();

    void                    SetAutoSaveTime( sal_Int32 n )      { nAutoSaveTime = n; SetModified();  }
    sal_Int32               GetAutoSaveTime() const             { return nAutoSaveTime; }
    void                    SetBackup( BOOL b )                 { bBackup = b; SetModified();}
    BOOL                    IsBackup() const                    { return bBackup; }
    void                    SetAutoSave( BOOL b )               { bAutoSave = b; SetModified();    }
    BOOL                    IsAutoSave() const                  { return bAutoSave; }
    void                    SetAutoSavePrompt( BOOL b )         { bAutoSavePrompt = b; SetModified();  }
    BOOL                    IsAutoSavePrompt() const            { return bAutoSavePrompt; }
    void                    SetDocInfoSave(BOOL b)              { bDocInfSave = b; SetModified(); }
    BOOL                    IsDocInfoSave() const               { return bDocInfSave; }
    void                    SetSaveOriginalGraphics(BOOL b)     { nSaveGraphics = 1; SetModified(); }
    BOOL                    IsSaveOriginalGraphics() const      { return nSaveGraphics == 1; }
    void                    SetSaveGraphicsCompressed(BOOL b)   { nSaveGraphics = 2; SetModified(); }
    BOOL                    IsSaveGraphicsCompressed() const    { return nSaveGraphics == 2; }
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
    void                    SetIndepGrfFmt(BOOL b)              { bIndepGrfFmt = b; SetModified();}
    BOOL                    IsIndepGrfFmt() const               { return bIndepGrfFmt; }
};

#define TIMEINTERVALL 0
#define FORMAT 1
#define USEUSERDATA 2
#define CREATEBACKUP 3
#define AUTOSAVE 4
#define PROMPT 5
#define EDITPROPERTY 6
#define SAVEWORKINGSET 7
#define SAVEDOCWINS 8
#define SAVEDOCVIEW 9
#define FILESYSTEM 10
#define INTERNET 11

Sequence< OUString > GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Document/TimeIntervall",
        "Graphic/Format"
        "Document/UseUserData",
        "Document/CreateBackup",
        "Document/AutoSave",
        "Document/Prompt",
        "Document/EditProperty",
        "WorkingSet",
        "DocumentWins",
        "ViewInfo",
        "URL/FileSystem",
        "URL/Internet",
        "IndepGraph"                    // not supported anymore
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
    : ConfigItem( OUString::createFromAscii("Common/Save") )
    , nAutoSaveTime( 0 )
    , nSaveGraphics( 0 )
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
    , bIndepGrfFmt( sal_False )
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
                            nSaveGraphics = nTemp;
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
                                    bUseUserData = bTemp;
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
                                case SAVEDOCVIEW :
                                    bSaveDocView = bTemp;
                                    break;
                                case FILESYSTEM :
                                    bSaveRelFSys = bTemp;
                                    break;
                                case INTERNET :
                                    bSaveRelINet = bTemp;
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
                pValues[nProp] <<= nSaveGraphics;
                break;
            case USEUSERDATA :
                pValues[nProp] <<= bUseUserData;
                break;
            case CREATEBACKUP :
                pValues[nProp] <<= bUseUserData;
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
            case SAVEDOCVIEW :
                pValues[nProp] <<= bSaveDocView;
                break;
            case FILESYSTEM :
                pValues[nProp] <<= bSaveRelFSys;
                break;
            case INTERNET :
                pValues[nProp] <<= bSaveRelINet;
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

SvtSaveOptions::SvtSaveOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() );
    if ( !pOptions )
        pOptions = new SvtSaveOptions_Impl;
    ++nRefCount;
    pImp = pOptions;
}

// -----------------------------------------------------------------------

SvtSaveOptions::~SvtSaveOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if ( !--nRefCount )
        DELETEZ( pOptions );
}

void SvtSaveOptions::SetAutoSaveTime( sal_Int32 n )
{
    pImp->SetAutoSaveTime( n );
}

sal_Int32 SvtSaveOptions::GetAutoSaveTime() const
{
    return pImp->GetAutoSaveTime();
}

void SvtSaveOptions::SetBackup( sal_Bool b )
{
    pImp->SetBackup( b );
}

sal_Bool SvtSaveOptions::IsBackup() const
{
    return pImp->IsBackup();
}

void SvtSaveOptions::SetAutoSave( sal_Bool b )
{
    pImp->SetAutoSave( b );
}

sal_Bool SvtSaveOptions::IsAutoSave() const
{
    return pImp->IsAutoSave();
}

void SvtSaveOptions::SetAutoSavePrompt( sal_Bool b )
{
    pImp->SetAutoSavePrompt( b );
}

sal_Bool SvtSaveOptions::IsAutoSavePrompt() const
{
    return pImp->IsAutoSavePrompt();
}

void SvtSaveOptions::SetDocInfoSave(sal_Bool b)
{
    pImp->SetDocInfoSave( b );
}

sal_Bool SvtSaveOptions::IsDocInfoSave() const
{
    return pImp->IsDocInfoSave();
}

void SvtSaveOptions::SetSaveOriginalGraphics(sal_Bool b)
{
    pImp->SetSaveOriginalGraphics( b );
}

sal_Bool SvtSaveOptions::IsSaveOriginalGraphics() const
{
    return pImp->IsSaveOriginalGraphics();
}

void SvtSaveOptions::SetSaveGraphicsCompressed(sal_Bool b)
{
    pImp->SetSaveGraphicsCompressed( b );
}

sal_Bool SvtSaveOptions::IsSaveGraphicsCompressed() const
{
    return pImp->IsSaveGraphicsCompressed();
}

void SvtSaveOptions::SetSaveWorkingSet( sal_Bool b )
{
    pImp->SetSaveWorkingSet( b );
}

sal_Bool SvtSaveOptions::IsSaveWorkingSet() const
{
    return pImp->IsSaveWorkingSet();
}

void SvtSaveOptions::SetSaveDocWins( sal_Bool b )
{
    pImp->SetSaveDocWins( b );
}

sal_Bool SvtSaveOptions::IsSaveDocWins() const
{
    return pImp->IsSaveDocWins();
}

void SvtSaveOptions::SetSaveDocView( sal_Bool b )
{
    pImp->SetSaveDocView( b );
}

sal_Bool SvtSaveOptions::IsSaveDocView() const
{
    return pImp->IsSaveDocView();
}

void SvtSaveOptions::SetSaveRelINet( sal_Bool b )
{
    pImp->SetSaveRelINet( b );
}

sal_Bool SvtSaveOptions::IsSaveRelINet() const
{
    return pImp->IsSaveRelINet();
}

void SvtSaveOptions::SetSaveRelFSys( sal_Bool b )
{
    pImp->SetSaveRelFSys( b );
}

sal_Bool SvtSaveOptions::IsSaveRelFSys() const
{
    return pImp->IsSaveRelFSys();
}

void SvtSaveOptions::SetIndepGrfFmt(sal_Bool b)
{
    pImp->SetIndepGrfFmt( b );
}

sal_Bool SvtSaveOptions::IsIndepGrfFmt() const
{
    return pImp->IsIndepGrfFmt();
}





