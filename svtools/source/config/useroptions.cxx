/*************************************************************************
 *
 *  $RCSfile: useroptions.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 14:15:09 $
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

    sal_Bool        m_bIsROCompany;
    sal_Bool        m_bIsROFirstName;
    sal_Bool        m_bIsROLastName;
    sal_Bool        m_bIsROID;
    sal_Bool        m_bIsROStreet;
    sal_Bool        m_bIsROCity;
    sal_Bool        m_bIsROState;
    sal_Bool        m_bIsROZip;
    sal_Bool        m_bIsROCountry;
    sal_Bool        m_bIsROPosition;
    sal_Bool        m_bIsROTitle;
    sal_Bool        m_bIsROTelephoneHome;
    sal_Bool        m_bIsROTelephoneWork;
    sal_Bool        m_bIsROFax;
    sal_Bool        m_bIsROEmail;
    sal_Bool        m_bIsROCustomerNumber;

    typedef String SvtUserOptions_Impl:: *StrPtr;

    const String&   GetToken( StrPtr pPtr ) const;
    void            SetToken( StrPtr pPtr, const String& rNewToken );
    void            InitFullName();
    void            Load();

public:
    SvtUserOptions_Impl();

    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void    Commit();

    // get the user token
    const String&   GetCompany() const { return GetToken( &SvtUserOptions_Impl::m_aCompany ); }
    const String&   GetFirstName() const { return GetToken( &SvtUserOptions_Impl::m_aFirstName ); }
    const String&   GetLastName() const { return GetToken( &SvtUserOptions_Impl::m_aLastName ); }
    const String&   GetID() const { return GetToken( &SvtUserOptions_Impl::m_aID ); }
    const String&   GetStreet() const { return GetToken( &SvtUserOptions_Impl::m_aStreet ); }
    const String&   GetCity() const { return GetToken( &SvtUserOptions_Impl::m_aCity ); }
    const String&   GetState() const { return GetToken( &SvtUserOptions_Impl::m_aState ); }
    const String&   GetZip() const { return GetToken( &SvtUserOptions_Impl::m_aZip ); }
    const String&   GetCountry() const { return GetToken( &SvtUserOptions_Impl::m_aCountry ); }
    const String&   GetPosition() const { return GetToken( &SvtUserOptions_Impl::m_aPosition ); }
    const String&   GetTitle() const { return GetToken( &SvtUserOptions_Impl::m_aTitle ); }
    const String&   GetTelephoneHome() const { return GetToken( &SvtUserOptions_Impl::m_aTelephoneHome ); }
    const String&   GetTelephoneWork() const { return GetToken( &SvtUserOptions_Impl::m_aTelephoneWork ); }
    const String&   GetFax() const { return GetToken( &SvtUserOptions_Impl::m_aFax ); }
    const String&   GetEmail() const { return GetToken( &SvtUserOptions_Impl::m_aEmail ); }
    const String&   GetCustomerNumber() const { return GetToken( &SvtUserOptions_Impl::m_aCustomerNumber ); }

    const String&   GetFullName();
    const String&   GetLocale() const { return m_aLocale; }

    // set the address token
    void            SetCompany( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aCompany, rNewToken ); }
    void            SetFirstName( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aFirstName, rNewToken ); InitFullName();}
    void            SetLastName( const String& rNewToken )
                        { SetToken( &SvtUserOptions_Impl::m_aLastName, rNewToken ); InitFullName();}
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

    sal_Bool        IsTokenReadonly( USHORT nToken ) const;
};

// global ----------------------------------------------------------------

static SvtUserOptions_Impl* pOptions = NULL;
static sal_Int32            nRefCount = 0;

#define READONLY_DEFAULT    sal_False

// functions -------------------------------------------------------------

Sequence< OUString > GetUserPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Data/l",                       // USER_OPT_CITY
        "Data/o",                       // USER_OPT_COMPANY
        "Data/c",                       // USER_OPT_COUNTRY
        "Data/mail",                    // USER_OPT_EMAIL
        "Data/facsimiletelephonenumber",// USER_OPT_FAX
        "Data/givenname",               // USER_OPT_FIRSTNAME
        "Data/sn",                      // USER_OPT_LASTNAME
        "Data/position",                // USER_OPT_POSITION
        "Data/st",                      // USER_OPT_STATE
        "Data/street",                  // USER_OPT_STREET
        "Data/homephone",               // USER_OPT_TELEPHONEHOME
        "Data/telephonenumber",         // USER_OPT_TELEPHONEWORK
        "Data/title",                   // USER_OPT_TITLE
        "Data/initials",                // USER_OPT_ID
        "Data/postalcode",              // USER_OPT_ZIP
    };
    const int nCount = sizeof( aPropNames ) / sizeof( const char* );
    Sequence< OUString > seqNames( nCount );
    OUString* pNames = seqNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return seqNames;
}

// class SvtUserOptions_Impl ---------------------------------------------

const String& SvtUserOptions_Impl::GetToken( StrPtr pPtr ) const
{
    return this->*pPtr;
}

// -----------------------------------------------------------------------

void SvtUserOptions_Impl::SetToken( StrPtr pPtr, const String& rNewToken )
{
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

    ConfigItem( OUString::createFromAscii("UserProfile") ),

    m_bIsROCompany( READONLY_DEFAULT ),
    m_bIsROFirstName( READONLY_DEFAULT ),
    m_bIsROLastName( READONLY_DEFAULT ),
    m_bIsROID( READONLY_DEFAULT ),
    m_bIsROStreet( READONLY_DEFAULT ),
    m_bIsROCity( READONLY_DEFAULT ),
    m_bIsROState( READONLY_DEFAULT ),
    m_bIsROZip( READONLY_DEFAULT ),
    m_bIsROCountry( READONLY_DEFAULT ),
    m_bIsROPosition( READONLY_DEFAULT ),
    m_bIsROTitle( READONLY_DEFAULT ),
    m_bIsROTelephoneHome( READONLY_DEFAULT ),
    m_bIsROTelephoneWork( READONLY_DEFAULT ),
    m_bIsROFax( READONLY_DEFAULT ),
    m_bIsROEmail( READONLY_DEFAULT ),
    m_bIsROCustomerNumber( READONLY_DEFAULT )

{
    Load();
    Any aAny = ConfigManager::GetConfigManager()->GetDirectConfigProperty( ConfigManager::LOCALE );
    OUString aLocale;
    if ( aAny >>= aLocale )
        m_aLocale = String( aLocale );
    else
    {
        DBG_ERRORFILE( "SvtUserOptions_Impl::SvtUserOptions_Impl(): no locale found" );
    }
}
// -----------------------------------------------------------------------
void SvtUserOptions_Impl::Load()
{
    Sequence< OUString > seqNames = GetUserPropertyNames();
    Sequence< Any > seqValues = GetProperties( seqNames );
    Sequence< sal_Bool > seqRO = GetReadOnlyStates( seqNames );
    EnableNotification( seqNames );
    const Any* pValues = seqValues.getConstArray();
    DBG_ASSERT( seqValues.getLength() == seqNames.getLength(), "GetProperties failed" );
    if ( seqValues.getLength() == seqNames.getLength() )
    {
        OUString aTempStr;

        for ( int nProp = 0; nProp < seqNames.getLength(); nProp++ )
        {
            if ( pValues[nProp].hasValue() )
            {
                if ( pValues[nProp] >>= aTempStr )
                {
                    String* pToken = NULL;
                    sal_Bool* pBool = NULL;

                    switch ( nProp )
                    {
                        case USER_OPT_COMPANY:
                            pToken = &m_aCompany; pBool = &m_bIsROCompany; break;
                        case USER_OPT_FIRSTNAME:
                            pToken = &m_aFirstName; pBool = &m_bIsROFirstName; break;
                        case USER_OPT_LASTNAME:
                            pToken = &m_aLastName; pBool = &m_bIsROLastName; break;
                        case USER_OPT_ID:
                            pToken = &m_aID; pBool = &m_bIsROID; break;
                        case USER_OPT_STREET:
                            pToken = &m_aStreet; pBool = &m_bIsROStreet; break;
                        case USER_OPT_CITY:
                            pToken = &m_aCity; pBool = &m_bIsROCity; break;
                        case USER_OPT_STATE:
                            pToken = &m_aState; pBool = &m_bIsROState; break;
                        case USER_OPT_ZIP:
                            pToken = &m_aZip; pBool = &m_bIsROZip; break;
                        case USER_OPT_COUNTRY:
                            pToken = &m_aCountry; pBool = &m_bIsROCountry; break;
                        case USER_OPT_POSITION:
                            pToken = &m_aPosition; pBool = &m_bIsROPosition; break;
                        case USER_OPT_TITLE:
                            pToken = &m_aTitle; pBool = &m_bIsROTitle; break;
                        case USER_OPT_TELEPHONEHOME:
                            pToken = &m_aTelephoneHome; pBool = &m_bIsROTelephoneHome; break;
                        case USER_OPT_TELEPHONEWORK:
                            pToken = &m_aTelephoneWork; pBool = &m_bIsROTelephoneWork; break;
                        case USER_OPT_FAX:
                            pToken = &m_aFax; pBool = &m_bIsROFax; break;
                        case USER_OPT_EMAIL:
                            pToken = &m_aEmail; pBool = &m_bIsROEmail; break;
                        default:
                            DBG_ERRORFILE( "invalid index to load a user token" );
                    }

                    if ( pToken )
                        *pToken = String( aTempStr );
                    if ( pBool )
                        *pBool = seqRO[nProp];
                }
                else
                {
                    DBG_ERRORFILE( "Wrong any type" );
                }
            }
        }
    }
    InitFullName();
}
// -----------------------------------------------------------------------

void SvtUserOptions_Impl::Commit()
{
    Sequence< OUString > seqOrgNames = GetUserPropertyNames();
    sal_Int32 nOrgCount = seqOrgNames.getLength();

    Sequence< OUString > seqNames( nOrgCount );
    Sequence< Any > seqValues( nOrgCount );
    sal_Int32 nRealCount = 0;

    OUString aTempStr;

    for ( int nProp = 0; nProp < nOrgCount; nProp++ )
    {
        sal_Bool* pbReadonly = NULL;

        switch ( nProp )
        {
            case USER_OPT_COMPANY:
                aTempStr = OUString( m_aCompany ); pbReadonly = &m_bIsROCompany; break;
            case USER_OPT_FIRSTNAME:
                aTempStr = OUString( m_aFirstName ); pbReadonly = &m_bIsROFirstName; break;
            case USER_OPT_LASTNAME:
                aTempStr = OUString( m_aLastName ); pbReadonly = &m_bIsROLastName; break;
            case USER_OPT_ID:
                aTempStr = OUString( m_aID ); pbReadonly = &m_bIsROID; break;
            case USER_OPT_STREET:
                aTempStr = OUString( m_aStreet ); pbReadonly = &m_bIsROStreet; break;
            case USER_OPT_CITY:
                aTempStr = OUString( m_aCity ); pbReadonly = &m_bIsROCity; break;
            case USER_OPT_STATE:
                aTempStr = OUString( m_aState ); pbReadonly = &m_bIsROState; break;
            case USER_OPT_ZIP:
                aTempStr = OUString( m_aZip ); pbReadonly = &m_bIsROZip; break;
            case USER_OPT_COUNTRY:
                aTempStr = OUString( m_aCountry ); pbReadonly = &m_bIsROCountry; break;
            case USER_OPT_POSITION:
                aTempStr = OUString( m_aPosition ); pbReadonly = &m_bIsROPosition; break;
            case USER_OPT_TITLE:
                aTempStr = OUString( m_aTitle ); pbReadonly = &m_bIsROTitle; break;
            case USER_OPT_TELEPHONEHOME:
                aTempStr = OUString( m_aTelephoneHome ); pbReadonly = &m_bIsROTelephoneHome; break;
            case USER_OPT_TELEPHONEWORK:
                aTempStr = OUString( m_aTelephoneWork ); pbReadonly = &m_bIsROTelephoneWork; break;
            case USER_OPT_FAX:
                aTempStr = OUString( m_aFax ); pbReadonly = &m_bIsROFax; break;
            case USER_OPT_EMAIL:
                aTempStr = OUString( m_aEmail ); pbReadonly = &m_bIsROEmail; break;
            default:
                DBG_ERRORFILE( "invalid index to save a user token" );
        }

        if ( pbReadonly && !(*pbReadonly) )
        {
            seqValues[nRealCount] <<= aTempStr;
            seqNames[nRealCount] = seqOrgNames[nProp];
            ++nRealCount;
        }
    }

    // Set properties in configuration.
    seqNames.realloc( nRealCount );
    seqValues.realloc( nRealCount );
    PutProperties( seqNames, seqValues );
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
    Load();
}

// -----------------------------------------------------------------------

sal_Bool SvtUserOptions_Impl::IsTokenReadonly( USHORT nToken ) const
{
    sal_Bool bRet = sal_False;

    switch ( nToken )
    {
        case USER_OPT_COMPANY:          bRet = m_bIsROCompany;          break;
        case USER_OPT_FIRSTNAME:        bRet = m_bIsROFirstName;        break;
        case USER_OPT_LASTNAME:         bRet = m_bIsROLastName;         break;
        case USER_OPT_ID:               bRet = m_bIsROID;               break;
        case USER_OPT_STREET:           bRet = m_bIsROStreet;           break;
        case USER_OPT_CITY:             bRet = m_bIsROCity;             break;
        case USER_OPT_STATE:            bRet = m_bIsROState;            break;
        case USER_OPT_ZIP:              bRet = m_bIsROZip;              break;
        case USER_OPT_COUNTRY:          bRet = m_bIsROCountry;          break;
        case USER_OPT_POSITION:         bRet = m_bIsROPosition;         break;
        case USER_OPT_TITLE:            bRet = m_bIsROTitle;            break;
        case USER_OPT_TELEPHONEHOME:    bRet = m_bIsROTelephoneHome;    break;
        case USER_OPT_TELEPHONEWORK:    bRet = m_bIsROTelephoneWork;    break;
        case USER_OPT_FAX:              bRet = m_bIsROFax;              break;
        case USER_OPT_EMAIL:            bRet = m_bIsROEmail;            break;
        default:
            DBG_ERRORFILE( "SvtUserOptions_Impl::IsTokenReadonly(): invalid token" );
    }

    return bRet;
}

// class SvtUserOptions --------------------------------------------------

SvtUserOptions::SvtUserOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( GetInitMutex() );

    if ( !pOptions )
        pOptions = new SvtUserOptions_Impl;
    ++nRefCount;
    pImp = pOptions;
}

// -----------------------------------------------------------------------

SvtUserOptions::~SvtUserOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( GetInitMutex() );

    if ( !--nRefCount )
    {
        if ( pOptions->IsModified() )
            pOptions->Commit();
        DELETEZ( pOptions );
    }
}

// -----------------------------------------------------------------------

::osl::Mutex& SvtUserOptions::GetInitMutex()
{
    // Initialize static mutex only for one time!
    static ::osl::Mutex* pMutex = NULL;
    // If these method first called (Mutex not already exist!) ...
    if ( pMutex == NULL )
    {
        // ... we must create a new one. Protect follow code with the global mutex -
        // It must be - we create a static variable!
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        // We must check our pointer again -
        // because another instance of our class will be faster then this instance!
        if ( pMutex == NULL )
        {
            // Create the new mutex and set it for return on static variable.
            static ::osl::Mutex aMutex;
            pMutex = &aMutex;
        }
    }
    // Return new created or already existing mutex object.
    return *pMutex;
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetCompany() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetCompany();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetFirstName() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetFirstName();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetLastName() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetLastName();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetID() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetID();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetStreet() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetStreet();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetCity() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetCity();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetState() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetState();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetZip() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetZip();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetCountry() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetCountry();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetPosition() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetPosition();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetTitle() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetTitle();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetTelephoneHome() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetTelephoneHome();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetTelephoneWork() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetTelephoneWork();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetFax() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetFax();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetEmail() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetEmail();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetCustomerNumber() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetCustomerNumber();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetFullName() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetFullName();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetLocale() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetLocale();
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetCompany( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetCompany( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetFirstName( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetFirstName( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetLastName( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetLastName( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetID( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetID( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetStreet( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetStreet( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetCity( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetCity( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetState( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetState( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetZip( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetZip( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetCountry( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetCountry( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetPosition( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetPosition( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetTitle( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetTitle( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetTelephoneHome( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetTelephoneHome( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetTelephoneWork( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetTelephoneWork( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetFax( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetFax( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetEmail( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetEmail( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetCustomerNumber( const String& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetCustomerNumber( rNewToken );
}

// -----------------------------------------------------------------------

sal_Bool SvtUserOptions::IsTokenReadonly( USHORT nToken ) const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->IsTokenReadonly( nToken );
}

