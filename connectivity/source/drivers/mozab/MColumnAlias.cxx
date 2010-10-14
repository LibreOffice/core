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
#include "precompiled_connectivity.hxx"
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
using namespace ::connectivity::mozab;
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

    for ( size_t i = 0; i < SAL_N_ELEMENTS( s_pProgrammaticNames ); ++i )
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
                if ( 0 == sAssignedAlias.getLength() )
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
        OSL_ENSURE( false, "OColumnAlias::getProgrammaticNameOrFallbackToUTF8Alias: no programmatic name for this alias!" );
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
