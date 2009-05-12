/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MColumnAlias.cxx,v $
 * $Revision: 1.10 $
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
#include "MColumnAlias.hxx"
#include "MConnection.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#ifndef CONNECTIVITY_MOZAB_MEXTCONFIGACCESS_HXX
#include "MExtConfigAccess.hxx"
#endif


using namespace ::connectivity;
using namespace ::connectivity::mozab;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

static const ::rtl::OUString sProgrammaticNames[] =
{
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FirstName")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LastName")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("DisplayName")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NickName")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PrimaryEmail")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SecondEmail")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PreferMailFormat")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WorkPhone")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HomePhone")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FaxNumber")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PagerNumber")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CellularNumber")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HomeAddress")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HomeAddress2")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HomeCity")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HomeState")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HomeZipCode")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HomeCountry")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WorkAddress")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WorkAddress2")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WorkCity")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WorkState")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WorkZipCode")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WorkCountry")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("JobTitle")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Department")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Company")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WebPage1")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WebPage2")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BirthYear")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BirthMonth")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BirthDay")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Custom1")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Custom2")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Custom3")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Custom4")),
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Notes"))
};
//------------------------------------------------------------------------------
OColumnAlias::OColumnAlias( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB )
{
    for ( size_t i = 0; i < END - BEGIN; ++i )
        m_aAliasMap[ sProgrammaticNames[i] ] = AliasDescription( sProgrammaticNames[i], static_cast< ProgrammaticName>( i ) );

    initialize( _rxORB );
}

//------------------------------------------------------------------
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
            Sequence< ::rtl::OUString > aColumnProgrammaticNames;
            if ( xAliasesNode.is() )
                aColumnProgrammaticNames = xAliasesNode->getElementNames();

            //.............................................................
            // travel through all the set elements
            const ::rtl::OUString* pProgrammaticNames = aColumnProgrammaticNames.getConstArray();
            const ::rtl::OUString* pProgrammaticNamesEnd = pProgrammaticNames + aColumnProgrammaticNames.getLength();
            ::rtl::OUString sAssignedAlias;

            for ( ; pProgrammaticNames < pProgrammaticNamesEnd; ++pProgrammaticNames )
            {
                OSL_ENSURE( m_aAliasMap.end() != m_aAliasMap.find( *pProgrammaticNames ),
                    "OColumnAlias::setAlias: found an invalid programmtic name!" );
                    // if this asserts, somebody stored a programmatic name in the configuration
                    // which is not allowed (i.e. not in the list of known programmatics).

#if OSL_DEBUG_LEVEL > 0
                sal_Bool bExtractionSuccess =
#endif
                xAliasesNode->getByName( *pProgrammaticNames) >>= sAssignedAlias;
                OSL_ENSURE( bExtractionSuccess, "OColumnAlias::setAlias: invalid config data!" );

                // normalize in case the config data is corrupted
                // (what we really don't need is an empty alias ...)
                if ( 0 == sAssignedAlias.getLength() )
                      sAssignedAlias = *pProgrammaticNames;

                //.............................................................
            #if OSL_DEBUG_LEVEL > 0
                bool bFound = false;
            #endif
                for (   AliasMap::iterator search = m_aAliasMap.begin();
                        ( search != m_aAliasMap.end() );
                        ++search
                    )
                {
                    if ( search->second.sProgrammaticName == *pProgrammaticNames )
                    {
                        AliasDescription aDescription( search->second );

                        // delete this old entry for this programmatic name
                        m_aAliasMap.erase( search );

                        // insert the same AliasDescription under a new name - its alias
                        m_aAliasMap[ sAssignedAlias ] = aDescription;

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
            OSL_ENSURE( sal_False, "OColumnAlias::setAlias: could not read my driver's configuration data!" );
        }
    }
}

//------------------------------------------------------------------
OColumnAlias::ProgrammaticName OColumnAlias::getProgrammaticNameIndex( const ::rtl::OUString& _rAliasName ) const
{
    AliasMap::const_iterator pos = m_aAliasMap.find( _rAliasName );
    if ( pos == m_aAliasMap.end() )
    {
        OSL_ENSURE( false, "OColumnAlias::getProgrammaticNameIndex: unknown column alias!" );
        return END;
    }

    return pos->second.eProgrammaticNameIndex;
}

//------------------------------------------------------------------
::rtl::OUString OColumnAlias::getProgrammaticNameOrFallbackToAlias( const ::rtl::OUString& _rAlias ) const
{
    AliasMap::const_iterator pos = m_aAliasMap.find( _rAlias );
    if ( pos == m_aAliasMap.end() )
        return _rAlias;
    return pos->second.sProgrammaticName;
}
