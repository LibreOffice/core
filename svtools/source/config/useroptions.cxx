/*************************************************************************
 *
 *  $RCSfile: useroptions.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: pb $ $Date: 2000-11-06 12:58:00 $
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

#include "useroptions.hxx"

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

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

// define ----------------------------------------------------------------

#define USER_CITY               0
#define USER_COMPANY            1
#define USER_COUNTRY            2
#define USER_CUSTOMERNUMBER     3
#define USER_EMAIL              4
#define USER_FAX                5
#define USER_FIRSTNAME          6
#define USER_LASTNAME           7
#define USER_POSITION           8
#define USER_STATE              9
#define USER_STREET             10
#define USER_TELEPHONEHOME      11
#define USER_TELEPHONEWORK      12
#define USER_TITLE              13
#define USER_ID                 14
#define USER_ZIP                15

// class SvtUserOptions_Impl ---------------------------------------------

class SvtUserOptions_Impl : public utl::ConfigItem
{
private:
    String          m_aCompany;
    String          m_aFirstName;
    String          m_aLastName;
    String          m_aID;
    String          m_aStreet;
    String          m_aCity;
    String          m_aState;
    String          m_aZip;
    String          m_aCountry;
    String          m_aPosition;
    String          m_aTitle;
    String          m_aTelephoneHome;
    String          m_aTelephoneWork;
    String          m_aFax;
    String          m_aEmail;
    String          m_aCustomerNumber;

    String          m_aEmptyString;
    String          m_aFullName;
    String          m_aLocale;

    ::osl::Mutex    m_aMutex;

    typedef String SvtUserOptions_Impl:: *StrPtr;

    // not const because of using a mutex
    const String&   GetToken( StrPtr pPtr );
    void            SetToken( StrPtr pPtr, const String& rNewToken );
    void            InitFullName();

public:
    SvtUserOptions_Impl();

    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void    Commit();

    // get the user token, not const because of using a mutex
    const String&   GetCompany() { return GetToken( &SvtUserOptions_Impl::m_aCompany ); }
    const String&   GetFirstName() { return GetToken( &SvtUserOptions_Impl::m_aFirstName ); }
    const String&   GetLastName() { return GetToken( &SvtUserOptions_Impl::m_aLastName ); }
    const String&   GetID() { return GetToken( &SvtUserOptions_Impl::m_aID ); }
    const String&   GetStreet() { return GetToken( &SvtUserOptions_Impl::m_aStreet ); }
    const String&   GetCity() { return GetToken( &SvtUserOptions_Impl::m_aCity ); }
    const String&   GetState() { return GetToken( &SvtUserOptions_Impl::m_aState ); }
    const String&   GetZip() { return GetToken( &SvtUserOptions_Impl::m_aZip ); }
    const String&   GetCountry() { return GetToken( &SvtUserOptions_Impl::m_aCountry ); }
    const String&   GetPosition() { return GetToken( &SvtUserOptions_Impl::m_aPosition ); }
    const String&   GetTitle() { return GetToken( &SvtUserOptions_Impl::m_aTitle ); }
    const String&   GetTelephoneHome() { return GetToken( &SvtUserOptions_Impl::m_aTelephoneHome ); }
    const String&   GetTelephoneWork() { return GetToken( &SvtUserOptions_Impl::m_aTelephoneWork ); }
    const String&   GetFax() { return GetToken( &SvtUserOptions_Impl::m_aFax ); }
    const String&   GetEmail() { return GetToken( &SvtUserOptions_Impl::m_aEmail ); }
    const String&   GetCustomerNumber() { return GetToken( &SvtUserOptions_Impl::m_aCustomerNumber ); }

    const String&   GetFullName();
    const String&   GetLocale() const { return m_aLocale; }

    // set the address token
    void            SetCompany( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aCompany, rNewToken ); }
    void            SetFirstName( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aFirstName, rNewToken ); }
    void            SetLastName( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aLastName, rNewToken ); }
    void            SetID( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aID, rNewToken ); }
    void            SetStreet( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aStreet, rNewToken ); }
    void            SetCity( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aCity, rNewToken ); }
    void            SetState( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aState, rNewToken ); }
    void            SetZip( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aZip, rNewToken ); }
    void            SetCountry( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aCountry, rNewToken ); }
    void            SetPosition( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aPosition, rNewToken ); }
    void            SetTitle( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aTitle, rNewToken ); }
    void            SetTelephoneHome( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aTelephoneHome, rNewToken ); }
    void            SetTelephoneWork( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aTelephoneWork, rNewToken ); }
    void            SetFax( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aFax, rNewToken ); }
    void            SetEmail( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aEmail, rNewToken ); }
    void            SetCustomerNumber( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aCustomerNumber, rNewToken ); }
};

// global ----------------------------------------------------------------

static SvtUserOptions_Impl* pOptions = NULL;
static sal_Int32            nRefCount = 0;

// functions -------------------------------------------------------------

Sequence< OUString > GetUserPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Data/City",            // USER_CITY
        "Data/Company",         // USER_COMPANY
        "Data/Country",         // USER_COUNTRY
        "Data/CustomerNumber",  // USER_CUSTOMERNUMBER
        "Data/EMail",           // USER_EMAIL
        "Data/Fax",             // USER_FAX
        "Data/FirstName",       // USER_FIRSTNAME
        "Data/LastName",        // USER_LASTNAME
        "Data/Position",        // USER_POSITION
        "Data/State",           // USER_STATE
        "Data/Street",          // USER_STREET
        "Data/TelephoneHome",   // USER_TELEPHONEHOME
        "Data/TelephoneWork",   // USER_TELEPHONEWORK
        "Data/Title",           // USER_TITLE
        "Data/UserID",          // USER_ID
        "Data/Zip",             // USER_ZIP
    };

    const int nCount = sizeof( aPropNames ) / sizeof( const char* );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

// class SvtUserOptions_Impl ---------------------------------------------

const String& SvtUserOptions_Impl::GetToken( StrPtr pPtr )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return this->*pPtr;
}

// -----------------------------------------------------------------------

void SvtUserOptions_Impl::SetToken( StrPtr pPtr, const String& rNewToken )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    this->*pPtr = rNewToken;
    SetModified();
}

// -----------------------------------------------------------------------

void SvtUserOptions_Impl::InitFullName()
{
    m_aFullName = GetFirstName();
    m_aFullName.EraseLeadingAndTrailingChars();
    if ( m_aFullName.Len() )
        m_aFullName += ' ';
    m_aFullName += GetLastName();
    m_aFullName.EraseTrailingChars();
}

// -----------------------------------------------------------------------

SvtUserOptions_Impl::SvtUserOptions_Impl() :

    ConfigItem( OUString::createFromAscii("UserProfile") )

{
    Sequence< OUString > aNames = GetUserPropertyNames();
    Sequence< Any > aValues = GetProperties( aNames );
    EnableNotification( aNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
    if ( aValues.getLength() == aNames.getLength() )
    {
        OUString aTempStr;

        for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
        {
            DBG_ASSERT( pValues[nProp].hasValue(), "property value missing" );
            if ( pValues[nProp].hasValue() )
            {
                if ( pValues[nProp] >>= aTempStr )
                {
                    switch ( nProp )
                    {
                        case USER_COMPANY:          m_aCompany = String( aTempStr );        break;
                        case USER_FIRSTNAME:        m_aFirstName = String( aTempStr );      break;
                        case USER_LASTNAME:         m_aLastName = String( aTempStr );       break;
                        case USER_ID:               m_aID = String( aTempStr );             break;
                        case USER_STREET:           m_aStreet = String( aTempStr );         break;
                        case USER_CITY:             m_aCity = String( aTempStr );           break;
                        case USER_STATE:            m_aState = String( aTempStr );          break;
                        case USER_ZIP:              m_aZip = String( aTempStr );            break;
                        case USER_COUNTRY:          m_aCountry = String( aTempStr );        break;
                        case USER_POSITION:         m_aPosition = String( aTempStr );       break;
                        case USER_TITLE:            m_aTitle = String( aTempStr );          break;
                        case USER_TELEPHONEHOME:    m_aTelephoneHome = String( aTempStr );  break;
                        case USER_TELEPHONEWORK:    m_aTelephoneWork = String( aTempStr );  break;
                        case USER_FAX:              m_aFax = String( aTempStr );            break;
                        case USER_EMAIL:            m_aEmail = String( aTempStr );          break;
                        case USER_CUSTOMERNUMBER:   m_aCustomerNumber = String( aTempStr ); break;

                        default:
                            DBG_ERRORFILE( "invalid index to load a user token" );
                    }
                }
                else
                {
                    DBG_ERRORFILE( "Wrong any type" );
                }
            }
        }
    }
    InitFullName();

    Any aAny = ConfigManager::GetConfigManager()->GetDirectConfigProperty( ConfigManager::LOCALE );
    OUString aLocale;
    if ( aAny >>= aLocale )
        m_aLocale = String( aLocale );
    else
    {
        DBG_ERRORFILE( "no locale found" );
    }
}

// -----------------------------------------------------------------------

void SvtUserOptions_Impl::Commit()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Sequence< OUString > aNames = GetUserPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence< Any > aValues( aNames.getLength() );
    Any* pValues = aValues.getArray();
    const Type& rType = ::getBooleanCppuType();
    OUString aTempStr;
    for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
    {
        switch ( nProp )
        {
            case USER_COMPANY:          aTempStr = OUString( m_aCompany );          break;
            case USER_FIRSTNAME:        aTempStr = OUString( m_aFirstName );        break;
            case USER_LASTNAME:         aTempStr = OUString( m_aLastName );         break;
            case USER_ID:               aTempStr = OUString( m_aID );               break;
            case USER_STREET:           aTempStr = OUString( m_aStreet );           break;
            case USER_CITY:             aTempStr = OUString( m_aCity );             break;
            case USER_STATE:            aTempStr = OUString( m_aState );            break;
            case USER_ZIP:              aTempStr = OUString( m_aZip );              break;
            case USER_COUNTRY:          aTempStr = OUString( m_aCountry );          break;
            case USER_POSITION:         aTempStr = OUString( m_aPosition );         break;
            case USER_TITLE:            aTempStr = OUString( m_aTitle );            break;
            case USER_TELEPHONEHOME:    aTempStr = OUString( m_aTelephoneHome );    break;
            case USER_TELEPHONEWORK:    aTempStr = OUString( m_aTelephoneWork );    break;
            case USER_FAX:              aTempStr = OUString( m_aFax );              break;
            case USER_EMAIL:            aTempStr = OUString( m_aEmail );            break;
            case USER_CUSTOMERNUMBER:   aTempStr = OUString( m_aCustomerNumber );   break;

            default:
                DBG_ERRORFILE( "invalid index to save a user token" );
        }
        pValues[nProp] <<= aTempStr;
    }
    PutProperties( aNames, aValues );
}

// -----------------------------------------------------------------------

const String& SvtUserOptions_Impl::GetFullName()
{
    if ( IsModified() )
        InitFullName();
    return m_aFullName;
}

// -----------------------------------------------------------------------

void SvtUserOptions_Impl::Notify( const Sequence<rtl::OUString>& aPropertyNames )
{
    DBG_ERRORFILE( "properties have been changed" );
}

// class SvtUserOptions --------------------------------------------------

SvtUserOptions::SvtUserOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() );
    if ( !pOptions )
        pOptions = new SvtUserOptions_Impl;
    ++nRefCount;
    pImp = pOptions;
}

// -----------------------------------------------------------------------

SvtUserOptions::~SvtUserOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if ( !--nRefCount )
    {
        if ( pOptions->IsModified() )
            pOptions->Commit();
        DELETEZ( pOptions );
    }
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetCompany() const
{
    return pImp->GetCompany();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetFirstName() const
{
    return pImp->GetFirstName();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetLastName() const
{
    return pImp->GetLastName();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetID() const
{
    return pImp->GetID();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetStreet() const
{
    return pImp->GetStreet();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetCity() const
{
    return pImp->GetCity();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetState() const
{
    return pImp->GetState();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetZip() const
{
    return pImp->GetZip();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetCountry() const
{
    return pImp->GetCountry();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetPosition() const
{
    return pImp->GetPosition();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetTitle() const
{
    return pImp->GetTitle();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetTelephoneHome() const
{
    return pImp->GetTelephoneHome();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetTelephoneWork() const
{
    return pImp->GetTelephoneWork();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetFax() const
{
    return pImp->GetFax();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetEmail() const
{
    return pImp->GetEmail();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetCustomerNumber() const
{
    return pImp->GetCustomerNumber();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetFullName() const
{
    return pImp->GetFullName();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetLocale() const
{
    return pImp->GetLocale();
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetCompany( const String& rNewToken )
{
    pImp->SetCompany( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetFirstName( const String& rNewToken )
{
    pImp->SetFirstName( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetLastName( const String& rNewToken )
{
    pImp->SetLastName( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetID( const String& rNewToken )
{
    pImp->SetID( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetStreet( const String& rNewToken )
{
    pImp->SetStreet( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetCity( const String& rNewToken )
{
    pImp->SetCity( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetState( const String& rNewToken )
{
    pImp->SetState( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetZip( const String& rNewToken )
{
    pImp->SetZip( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetCountry( const String& rNewToken )
{
    pImp->SetCountry( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetPosition( const String& rNewToken )
{
    pImp->SetPosition( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetTitle( const String& rNewToken )
{
    pImp->SetTitle( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetTelephoneHome( const String& rNewToken )
{
    pImp->SetTelephoneHome( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetTelephoneWork( const String& rNewToken )
{
    pImp->SetTelephoneWork( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetFax( const String& rNewToken )
{
    pImp->SetFax( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetEmail( const String& rNewToken )
{
    pImp->SetEmail( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetCustomerNumber( const String& rNewToken )
{
    pImp->SetCustomerNumber( rNewToken );
}

