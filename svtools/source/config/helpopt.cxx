/*************************************************************************
 *
 *  $RCSfile: helpopt.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: pb $ $Date: 2000-10-26 12:58:47 $
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

#include "helpopt.hxx"

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

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

static SvtHelpOptions_Impl* pOptions = NULL;
static sal_Int32           nRefCount = 0;

#define EXTENDEDHELP 0
#define HELPTIPS 1
#define AUTOSTART 2
#define WELCOMESCREEN 3

class SvtHelpOptions_Impl : public utl::ConfigItem
{
    IdList*         pList;
    sal_Bool        bExtendedHelp;
    sal_Bool        bHelpTips;
    sal_Bool        bHelpAgentAutoStartMode;
    sal_Bool        bWelcomeScreen;

public:
                    SvtHelpOptions_Impl();

    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void    Commit();

    void            SetExtendedHelp( sal_Bool b )           { bExtendedHelp= b; SetModified(); }
    sal_Bool        IsExtendedHelp() const                  { return bExtendedHelp; }
    void            SetHelpTips( sal_Bool b )               { bHelpTips = b; SetModified(); }
    sal_Bool        IsHelpTips() const                      { return bHelpTips; }
    void            SetHelpAgentAutoStartMode( sal_Bool b ) { bHelpAgentAutoStartMode = b; SetModified(); }
    sal_Bool        IsHelpAgentAutoStartMode() const        { return bHelpAgentAutoStartMode; }
    void            SetWelcomeScreen( sal_Bool b )          { bWelcomeScreen = b; SetModified(); }
    sal_Bool        IsWelcomeScreen() const                 { return bWelcomeScreen; }
    IdList*         GetPIStarterList()                      { return pList; }
    void            AddToPIStarterList( sal_Int32 nId );
    void            RemoveFromPIStarterList( sal_Int32 nId );
};

static Sequence< OUString > GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "ExtendedTip",
        "Tip",
        "Agent/AutoStart",
        "HowTo/Show"
    };

    const int nCount = sizeof( aPropNames ) / sizeof( const char* );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

static ::osl::Mutex & getInitMutex()
{
    static ::osl::Mutex *pMutex = 0;

    if( ! pMutex )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if( ! pMutex )
        {
            static ::osl::Mutex mutex;
            pMutex = &mutex;
        }
    }
    return *pMutex;
}


// -----------------------------------------------------------------------

SvtHelpOptions_Impl::SvtHelpOptions_Impl()
    : ConfigItem( OUString::createFromAscii("Office.Common/Help") )
    , pList( 0 )
    , bExtendedHelp( sal_False )
    , bHelpTips( sal_True )
    , bHelpAgentAutoStartMode( sal_False )
    , bWelcomeScreen( sal_False )
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
                sal_Bool bTmp;
                if ( pValues[nProp] >>= bTmp )
                {
                    switch ( nProp )
                    {
                        case EXTENDEDHELP :
                            bExtendedHelp = bTmp;
                            break;
                        case HELPTIPS :
                            bHelpTips = bTmp;
                            break;
                        case AUTOSTART :
                            bHelpAgentAutoStartMode = bTmp;
                            break;
                        case WELCOMESCREEN :
                            bWelcomeScreen = bTmp;
                            break;
                        default:
                            DBG_ERROR( "Wrong Member!" );
                            break;
                    }
                }
                else
                    DBG_ERROR( "Wrong Type!" );
            }
        }
    }
}

void SvtHelpOptions_Impl::Commit()
{
    Sequence< OUString > aNames = GetPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence< Any > aValues( aNames.getLength() );
    Any* pValues = aValues.getArray();
    for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
    {
        switch ( nProp )
        {
            case EXTENDEDHELP :
                pValues[nProp] <<= bExtendedHelp;
                break;
            case HELPTIPS :
                pValues[nProp] <<= bHelpTips;
                break;
            case AUTOSTART :
                pValues[nProp] <<= bHelpAgentAutoStartMode;
                break;
            case WELCOMESCREEN :
                pValues[nProp] <<= bWelcomeScreen;
                break;
            default:
                DBG_ERRORFILE( "invalid index to save a path" );
        }
    }

    PutProperties( aNames, aValues );
}

// -----------------------------------------------------------------------

void SvtHelpOptions_Impl::Notify( const Sequence<rtl::OUString>& aPropertyNames )
{
    DBG_ERRORFILE( "properties have been changed" );
}

SvtHelpOptions::SvtHelpOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( getInitMutex() );
    if ( !pOptions )
        pOptions = new SvtHelpOptions_Impl;
    ++nRefCount;
    pImp = pOptions;
}

// -----------------------------------------------------------------------

SvtHelpOptions::~SvtHelpOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( getInitMutex() );
    if ( !--nRefCount )
    {
        if ( pOptions->IsModified() )
            pOptions->Commit();
        DELETEZ( pOptions );
    }
}

void SvtHelpOptions::SetExtendedHelp( sal_Bool b )
{
    pImp->SetExtendedHelp( b );
}

sal_Bool SvtHelpOptions::IsExtendedHelp() const
{
    return pImp->IsExtendedHelp();
}

void SvtHelpOptions::SetHelpTips( sal_Bool b )
{
    pImp->SetHelpTips( b );
}

sal_Bool SvtHelpOptions::IsHelpTips() const
{
    return pImp->IsHelpTips();
}

void SvtHelpOptions::SetHelpAgentAutoStartMode( sal_Bool b )
{
    pImp->SetHelpAgentAutoStartMode( b );
}

sal_Bool SvtHelpOptions::IsHelpAgentAutoStartMode() const
{
    return pImp->IsHelpAgentAutoStartMode();
}

void SvtHelpOptions::SetWelcomeScreen( sal_Bool b )
{
    pImp->SetWelcomeScreen( b );
}

sal_Bool SvtHelpOptions::IsWelcomeScreen() const
{
    return pImp->IsWelcomeScreen();
}

IdList* SvtHelpOptions::GetPIStarterList()
{
    return pImp->GetPIStarterList();
}

void SvtHelpOptions::AddToPIStarterList( sal_Int32 nId )
{
}

void SvtHelpOptions::RemoveFromPIStarterList( sal_Int32 nId )
{
}

