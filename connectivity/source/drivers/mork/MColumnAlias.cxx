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

#include <sal/macros.h>
#include "MColumnAlias.hxx"
#include "MConnection.hxx"
#include "MExtConfigAccess.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <tools/diagnose_ex.h>

#include <algorithm>
#include <functional>

using namespace ::connectivity;
using namespace ::connectivity::mork;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

//------------------------------------------------------------------------------
OColumnAlias::OColumnAlias( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB )
{
    static const sal_Char* s_pProgrammaticNames[] =
    {
        "FirstName",
        "LastName",
        "DisplayName",
        "NickName",
        "PrimaryEmail",
        "SecondEmail",
        "PreferMailFormat",
        "WorkPhone",
        "HomePhone",
        "FaxNumber",
        "PagerNumber",
        "CellularNumber",
        "HomeAddress",
        "HomeAddress2",
        "HomeCity",
        "HomeState",
        "HomeZipCode",
        "HomeCountry",
        "WorkAddress",
        "WorkAddress2",
        "WorkCity",
        "WorkState",
        "WorkZipCode",
        "WorkCountry",
        "JobTitle",
        "Department",
        "Company",
        "WebPage1",
        "WebPage2",
        "BirthYear",
        "BirthMonth",
        "BirthDay",
        "Custom1",
        "Custom2",
        "Custom3",
        "Custom4",
        "Notes",
    };

    for ( size_t i = 0; i < sizeof( s_pProgrammaticNames ) / sizeof( s_pProgrammaticNames[0] ); ++i )
        m_aAliasMap[ ::rtl::OUString::createFromAscii( s_pProgrammaticNames[i] ) ] = AliasEntry( s_pProgrammaticNames[i], i );

    initialize( _rxORB );
}

//------------------------------------------------------------------------------
void OColumnAlias::initialize( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB )
{
    // open our driver settings config node

    // the config path for our own driver's settings
    Reference< XPropertySet > xDriverNode = createDriverConfigNode( _rxORB );
    if ( xDriverNode.is() )
    {
        try
        {
            //.............................................................
            Reference< XNameAccess > xAliasesNode;
            xDriverNode->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ColumnAliases")) ) >>= xAliasesNode;
            OSL_ENSURE( xAliasesNode.is(), "OColumnAlias::setAlias: missing the aliases node!" );

            // this is a set of string nodes
            Sequence< ::rtl::OUString > aProgrammaticNames;
            if ( xAliasesNode.is() )
                aProgrammaticNames = xAliasesNode->getElementNames();

            //.............................................................
            // travel through all the set elements
            const ::rtl::OUString* pProgrammaticNames = aProgrammaticNames.getConstArray();
            const ::rtl::OUString* pProgrammaticNamesEnd = pProgrammaticNames + aProgrammaticNames.getLength();
            ::rtl::OUString sAssignedAlias;

            for ( ; pProgrammaticNames < pProgrammaticNamesEnd; ++pProgrammaticNames )
            {
                OSL_VERIFY( xAliasesNode->getByName( *pProgrammaticNames ) >>= sAssignedAlias );

                // normalize in case the config data is corrupted
                // (what we really don't need is an empty alias ...)
                if ( sAssignedAlias.isEmpty() )
                      sAssignedAlias = *pProgrammaticNames;

                ::rtl::OString sAsciiProgrammaticName( ::rtl::OUStringToOString( *pProgrammaticNames, RTL_TEXTENCODING_ASCII_US ) );
                //.............................................................
            #if OSL_DEBUG_LEVEL > 0
                bool bFound = false;
            #endif
                for (   AliasMap::iterator search = m_aAliasMap.begin();
                        ( search != m_aAliasMap.end() );
                        ++search
                    )
                {
                    if ( search->second.programmaticAsciiName.equals( sAsciiProgrammaticName ) )
                    {
                        AliasEntry entry( search->second );
                        m_aAliasMap.erase( search );
                        m_aAliasMap[ sAssignedAlias ] = entry;

                    #if OSL_DEBUG_LEVEL > 0
                        bFound = true;
                    #endif

                        break;
                    }
                }

                OSL_ENSURE( bFound, "OColumnAlias::setAlias: did not find a programmatic name which exists in the configuration!" );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

//------------------------------------------------------------------
::rtl::OString OColumnAlias::getProgrammaticNameOrFallbackToUTF8Alias( const ::rtl::OUString& _rAlias ) const
{
    AliasMap::const_iterator pos = m_aAliasMap.find( _rAlias );
    if ( pos == m_aAliasMap.end() )
    {
        OSL_FAIL( "OColumnAlias::getProgrammaticNameOrFallbackToUTF8Alias: no programmatic name for this alias!" );
        return ::rtl::OUStringToOString( _rAlias, RTL_TEXTENCODING_UTF8 );
    }
    return pos->second.programmaticAsciiName;
}

//------------------------------------------------------------------
bool OColumnAlias::isColumnSearchable( const ::rtl::OUString _alias ) const
{
    ::rtl::OString sProgrammatic = getProgrammaticNameOrFallbackToUTF8Alias( _alias );

    return  (   !sProgrammatic.equals( "HomeCountry" )
            &&  !sProgrammatic.equals( "WorkCountry" )
            );
    // for those, we know that they're not searchable in the Mozilla/LDAP implementation.
    // There might be more ...
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
