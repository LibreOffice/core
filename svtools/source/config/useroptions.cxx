/*************************************************************************
 *
 *  $RCSfile: useroptions.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:56 $
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

// class SvtUserOptions --------------------------------------------------

// initialize the static member
SvtUserOptions* SvtUserOptions::m_pOptions = NULL;

// -----------------------------------------------------------------------

Sequence< OUString > SvtUserOptions::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Company",          // USER_COMPANY
        "FirstName",        // USER_FIRSTNAME
        "LastName",         // USER_LASTNAME
        "UserID",           // USER_ID
        "Street",           // USER_STREET
        "City",             // USER_CITY
        "State",            // USER_STATE
        "Zip",              // USER_ZIP
        "Country",          // USER_COUNTRY
        "Position",         // USER_POSITION
        "Title",            // USER_TITLE
        "TelephoneHome",    // USER_TELEPHONEHOME
        "TelephoneWork",    // USER_TELEPHONEWORK
        "Fax",              // USER_FAX
        "EMail",            // USER_EMAIL
        "CustomerNumber"    // USER_CUSTOMERNUMBER
    };

    const int nCount = 16;
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetUserToken( int nIndex )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( USER_COMPANY == nIndex )
        return m_aCompany;
    else if ( USER_FIRSTNAME == nIndex )
        return m_aFirstName;
    else if ( USER_LASTNAME == nIndex )
        return m_aLastName;
    else if ( USER_ID == nIndex )
        return m_aID;
    else if ( USER_STREET == nIndex )
        return m_aStreet;
    else if ( USER_CITY == nIndex )
        return m_aCity;
    else if ( USER_STATE == nIndex )
        return m_aState;
    else if ( USER_ZIP == nIndex )
        return m_aZip;
    else if ( USER_COUNTRY == nIndex )
        return m_aCountry;
    else if ( USER_POSITION == nIndex )
        return m_aPosition;
    else if ( USER_TITLE == nIndex )
        return m_aTitle;
    else if ( USER_TELEPHONEHOME == nIndex )
        return m_aTelephoneHome;
    else if ( USER_TELEPHONEWORK == nIndex )
        return m_aTelephoneWork;
    else if ( USER_FAX == nIndex )
        return m_aFax;
    else if ( USER_EMAIL == nIndex )
        return m_aEmail;
    else if ( USER_CUSTOMERNUMBER == nIndex )
        return m_aCustomerNumber;
    else
    {
        DBG_ERRORFILE( "invalid index to get a user token" );
        return m_aEmptyString;
    }
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetUserToken( int nIndex, const String& rNewToken )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( USER_COMPANY == nIndex )
        m_aCompany = rNewToken;
    else if ( USER_FIRSTNAME == nIndex )
        m_aFirstName = rNewToken;
    else if ( USER_LASTNAME == nIndex )
        m_aLastName = rNewToken;
    else if ( USER_ID == nIndex )
        m_aID = rNewToken;
    else if ( USER_STREET == nIndex )
        m_aStreet = rNewToken;
    else if ( USER_CITY == nIndex )
        m_aCity = rNewToken;
    else if ( USER_STATE == nIndex )
        m_aState = rNewToken;
    else if ( USER_ZIP == nIndex )
        m_aZip = rNewToken;
    else if ( USER_COUNTRY == nIndex )
        m_aCountry = rNewToken;
    else if ( USER_POSITION == nIndex )
        m_aPosition = rNewToken;
    else if ( USER_TITLE == nIndex )
        m_aTitle = rNewToken;
    else if ( USER_TELEPHONEHOME == nIndex )
        m_aTelephoneHome = rNewToken;
    else if ( USER_TELEPHONEWORK == nIndex )
        m_aTelephoneWork = rNewToken;
    else if ( USER_FAX == nIndex )
        m_aFax = rNewToken;
    else if ( USER_EMAIL == nIndex )
        m_aEmail = rNewToken;
    else if ( USER_CUSTOMERNUMBER == nIndex )
        m_aCustomerNumber = rNewToken;
    else
        DBG_ERRORFILE( "invalid index to set a user token" );
    SetModified();
}

// -----------------------------------------------------------------------

SvtUserOptions::SvtUserOptions() : ConfigItem( OUString::createFromAscii("UserProfile/Data") )
{
    Sequence< OUString > aNames = GetPropertyNames();
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
                pValues[nProp] >>= aTempStr;
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
        }
    }
}

// -----------------------------------------------------------------------

SvtUserOptions::~SvtUserOptions()
{
}

// -----------------------------------------------------------------------

SvtUserOptions* SvtUserOptions::GetUserOptions()
{
    if ( !m_pOptions )
        m_pOptions = new SvtUserOptions;
    return m_pOptions;
}

// -----------------------------------------------------------------------

void SvtUserOptions::DestroyUserOptions()
{
    if ( m_pOptions )
        DELETEZ( m_pOptions );
}

// -----------------------------------------------------------------------

void SvtUserOptions::Commit()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Sequence< OUString > aNames = GetPropertyNames();
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

void SvtUserOptions::Notify( const Sequence<rtl::OUString>& aPropertyNames )
{
    DBG_ERRORFILE( "properties have been changed" );
}

