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
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FirstName")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LastName")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DisplayName")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NickName")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PrimaryEmail")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SecondEmail")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PreferMailFormat")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WorkPhone")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HomePhone")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FaxNumber")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PagerNumber")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CellularNumber")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HomeAddress")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HomeAddress2")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HomeCity")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HomeState")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HomeZipCode")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HomeCountry")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WorkAddress")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WorkAddress2")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WorkCity")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WorkState")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WorkZipCode")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WorkCountry")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("JobTitle")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Department")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Company")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WebPage1")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WebPage2")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BirthYear")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BirthMonth")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BirthDay")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Notes"))
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
    Reference< XPropertySet > xMozDriverNode = createDriverConfigNode( _rxORB, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.comp.sdbc.MozabDriver")) );
    if ( xEvoDriverNode.is() && xMozDriverNode.is() )
    {
        try
        {
            //=============================================================
            Reference< XNameAccess > xEvoAliasesNode;
            Reference< XNameAccess > xMozAliasesNode;
            xEvoDriverNode->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ColumnAliases")) ) >>= xEvoAliasesNode;
            xMozDriverNode->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ColumnAliases")) ) >>= xMozAliasesNode;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
