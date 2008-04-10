/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: LColumnAlias.cxx,v $
 * $Revision: 1.7 $
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
#include "LColumnAlias.hxx"
#include "LDriver.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include "LConfigAccess.hxx"

using namespace ::connectivity;
using namespace ::connectivity::evoab;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

static const ::rtl::OUString sProgrammaticNames[] =
{
    ::rtl::OUString::createFromAscii("FirstName"),
    ::rtl::OUString::createFromAscii("LastName"),
    ::rtl::OUString::createFromAscii("DisplayName"),
    ::rtl::OUString::createFromAscii("NickName"),
    ::rtl::OUString::createFromAscii("PrimaryEmail"),
    ::rtl::OUString::createFromAscii("SecondEmail"),
    ::rtl::OUString::createFromAscii("PreferMailFormat"),
    ::rtl::OUString::createFromAscii("WorkPhone"),
    ::rtl::OUString::createFromAscii("HomePhone"),
    ::rtl::OUString::createFromAscii("FaxNumber"),
    ::rtl::OUString::createFromAscii("PagerNumber"),
    ::rtl::OUString::createFromAscii("CellularNumber"),
    ::rtl::OUString::createFromAscii("HomeAddress"),
    ::rtl::OUString::createFromAscii("HomeAddress2"),
    ::rtl::OUString::createFromAscii("HomeCity"),
    ::rtl::OUString::createFromAscii("HomeState"),
    ::rtl::OUString::createFromAscii("HomeZipCode"),
    ::rtl::OUString::createFromAscii("HomeCountry"),
    ::rtl::OUString::createFromAscii("WorkAddress"),
    ::rtl::OUString::createFromAscii("WorkAddress2"),
    ::rtl::OUString::createFromAscii("WorkCity"),
    ::rtl::OUString::createFromAscii("WorkState"),
    ::rtl::OUString::createFromAscii("WorkZipCode"),
    ::rtl::OUString::createFromAscii("WorkCountry"),
    ::rtl::OUString::createFromAscii("JobTitle"),
    ::rtl::OUString::createFromAscii("Department"),
    ::rtl::OUString::createFromAscii("Company"),
    ::rtl::OUString::createFromAscii("WebPage1"),
    ::rtl::OUString::createFromAscii("WebPage2"),
    ::rtl::OUString::createFromAscii("BirthYear"),
    ::rtl::OUString::createFromAscii("BirthMonth"),
    ::rtl::OUString::createFromAscii("BirthDay"),
    ::rtl::OUString::createFromAscii("Notes")
};
//------------------------------------------------------------------------------
OColumnAlias::OColumnAlias()
{
    // Initialise m_aAlias with the default values from sProgrammaticNames.
    initialise();

    // Initialise m_aAlias map with the default values from sProgrammaticNames.
    setAliasMap();
}
//------------------------------------------------------------------
OColumnAlias::~OColumnAlias()
{
}
//------------------------------------------------------------------
void OColumnAlias::initialise()
{
    m_aAlias.reserve( END - FIRSTNAME + 1 );
    for (sal_Int32 i(FIRSTNAME); i < END; ++i)
    {
        m_aAlias.push_back(sProgrammaticNames[i]);
        m_aHeadLineNames.push_back(sProgrammaticNames[i]);
    }

    return;
}
//------------------------------------------------------------------
void OColumnAlias::setAlias(const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::lang::XMultiServiceFactory >& _rxORB)
{
        OSL_TRACE("OColumnAlias::setAlias()entered");

        // open our driver settings config node

        // the config path for our own driver's settings
    Reference< XPropertySet > xEvoDriverNode = createDriverConfigNode( _rxORB, OEvoabDriver::getImplementationName_Static() );
    //Reference< XPropertySet > xMozDriverNode = createDriverConfigNode( _rxORB, mozab::OConnection::getDriverImplementationName() );
    Reference< XPropertySet > xMozDriverNode = createDriverConfigNode( _rxORB, ::rtl::OUString::createFromAscii ("com.sun.star.comp.sdbc.MozabDriver") );
    if ( xEvoDriverNode.is() && xMozDriverNode.is() )
    {
        try
        {
            //=============================================================
            Reference< XNameAccess > xEvoAliasesNode;
            Reference< XNameAccess > xMozAliasesNode;
            xEvoDriverNode->getPropertyValue( ::rtl::OUString::createFromAscii("ColumnAliases") ) >>= xEvoAliasesNode;
            xMozDriverNode->getPropertyValue( ::rtl::OUString::createFromAscii("ColumnAliases") ) >>= xMozAliasesNode;
            OSL_ENSURE( xEvoAliasesNode.is(), "OColumnAlias::setAlias: missing the evolution aliases node!" );
            OSL_ENSURE( xMozAliasesNode.is(), "OColumnAlias::setAlias: missing the mozilla aliases node!" );

            // this is a set of string nodes
            Sequence< ::rtl::OUString > aColumnProgrammaticNames;
            //OSL_TRACE("OColumnAlias::setAlias()before xAliasesNode->getElementNames()");
            if ( xEvoAliasesNode.is() )
                aColumnProgrammaticNames = xEvoAliasesNode->getElementNames();
            //OSL_TRACE("OColumnAlias::setAlias()after xAliasesNode->getElementNames()");

            //=============================================================
            // travel through all the set elements
            const ::rtl::OUString* pProgrammaticNames = aColumnProgrammaticNames.getConstArray();
            const ::rtl::OUString* pProgrammaticNamesEnd = pProgrammaticNames + aColumnProgrammaticNames.getLength();
            ::rtl::OUString sAssignedAlias;
            ::rtl::OUString sHeadLineName;

            for ( ; pProgrammaticNames < pProgrammaticNamesEnd; ++pProgrammaticNames )
            {
                //OSL_TRACE("OColumnAlias::setAlias()::*pProgrammaticNames = %s\n", ((OUtoCStr(*pProgrammaticNames)) ? (OUtoCStr(*pProgrammaticNames)):("NULL")) );
                OSL_ENSURE( m_aAliasMap.end() != m_aAliasMap.find( *pProgrammaticNames ),
                    "OColumnAlias::setAlias: found an invalid programmtic name!" );
                    // if this asserts, somebody stored a programmatic name in the configuration
                    // which is not allowed (i.e. not in the list of known programmatics).
                //OSL_TRACE("OColumnAlias::setAlias()before xAliasesNode->getByName()");

#if OSL_DEBUG_LEVEL > 0
                sal_Bool bExtractionSuccess =
#endif
                xMozAliasesNode->getByName( *pProgrammaticNames) >>= sAssignedAlias;
                OSL_ENSURE( bExtractionSuccess, "OColumnAlias::setAlias: invalid mozilla config data!" );
#if OSL_DEBUG_LEVEL > 0
                bExtractionSuccess =
#endif
                xEvoAliasesNode->getByName( *pProgrammaticNames) >>= sHeadLineName;
                OSL_ENSURE( bExtractionSuccess, "OColumnAlias::setAlias: invalid evolution config data!" );
                //OSL_TRACE("OColumnAlias::setAlias()after xAliasesNode->getByName()");

                // normalize in case the config data is corrupted
                // (what we really don't need is an empty alias ...)
                if ( 0 == sAssignedAlias.getLength() )
                      sAssignedAlias = *pProgrammaticNames;
                if ( 0 == sHeadLineName.getLength() )
                      sHeadLineName = *pProgrammaticNames;
                //OSL_TRACE("OColumnAlias::setAlias()::sAssignedAlias = %s\n", ((OUtoCStr(sAssignedAlias)) ? (OUtoCStr(sAssignedAlias)):("NULL")) );
                //OSL_TRACE("OColumnAlias::setAlias()::sHeadLineName = %s\n", ((OUtoCStr(sHeadLineName)) ? (OUtoCStr(sHeadLineName)):("NULL")) );

                //=============================================================
                // check the value
                // look up the programmatic indicated by pProgrammaticNames in the known programmatics
                const ::rtl::OUString* pProgrammatic = sProgrammaticNames + FIRSTNAME;
                const ::rtl::OUString* pProgrammaticEnd = sProgrammaticNames + END;

                OSL_ENSURE( (sal_Int32)m_aAlias.size() == pProgrammaticEnd - pProgrammatic,
                    "OColumnAlias::setAlias: aliases vector not yet initialized!" );

                // the destination where we want to remember the alias
                ::std::vector< ::rtl::OUString >::iterator aAlias = m_aAlias.begin();
                ::std::vector< ::rtl::OUString >::iterator aHeadLineName = m_aHeadLineNames.begin();

                for ( ; pProgrammatic < pProgrammaticEnd; ++pProgrammatic, ++aAlias, ++aHeadLineName )
                {
                    //OSL_TRACE("OColumnAlias::setAlias()::*pProgrammatic = %s\n", ((OUtoCStr(*pProgrammatic)) ? (OUtoCStr(*pProgrammatic)):("NULL")) );
                    if ( pProgrammaticNames->equals( *pProgrammatic ) )
                    {
                        // add alias to the vector
                        *aAlias = sAssignedAlias;
                        *aHeadLineName = sHeadLineName;
                        break;
                    }
                }
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "OColumnAlias::setAlias: could not read my driver's configuration data!" );
        }
    }

    // Initialise m_aAliasMap.
    setAliasMap();

    return;
}
//------------------------------------------------------------------
const ::std::vector< ::rtl::OUString> & OColumnAlias::getAlias() const
{
    return m_aAlias;
}
//------------------------------------------------------------------
const ::std::map< ::rtl::OUString, ::rtl::OUString> & OColumnAlias::getAliasMap() const
{
    return m_aAliasMap;
}
//------------------------------------------------------------------
void OColumnAlias::setAliasMap()
{
        // Fill the map with the values of m_aAlias
        // and the sProgrammaticNames array.
    for (sal_Int32 i(FIRSTNAME); i < END; ++i) {
        m_aAliasMap[m_aAlias[i]] = m_aHeadLineNames[i];
        //OSL_TRACE("OColumnAlias::setAliasMap()::m_aAlias[i] = %s\n", ((OUtoCStr(m_aAlias[i])) ? (OUtoCStr(m_aAlias[i])):("NULL")) );
        //OSL_TRACE("OColumnAlias::setAliasMap()::m_aAliasMap[m_aAlias[i]] = %s\n", ((OUtoCStr(m_aAliasMap[m_aAlias[i]])) ? (OUtoCStr(m_aAliasMap[m_aAlias[i]])):("NULL")) );
    }

        return;
}
//------------------------------------------------------------------
