/*************************************************************************
 *
 *  $RCSfile: MColumnAlias.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:37:35 $
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
 *  Contributor(s): Willem van Dorp
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_MAB_COLUMNALIAS_HXX_
#include "MColumnAlias.hxx"
#endif
#ifndef CONNECTIVITY_SCONNECTION_HXX
#include "MConnection.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
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
    ::rtl::OUString::createFromAscii("Custom1"),
    ::rtl::OUString::createFromAscii("Custom2"),
    ::rtl::OUString::createFromAscii("Custom3"),
    ::rtl::OUString::createFromAscii("Custom4"),
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
        m_aAlias.push_back(sProgrammaticNames[i]);

    return;
}
//------------------------------------------------------------------
void OColumnAlias::setAlias(const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::lang::XMultiServiceFactory >& _rxORB)
{
        // open our driver settings config node

        // the config path for our own driver's settings
    Reference< XPropertySet > xDriverNode = createDriverConfigNode( _rxORB );
    if ( xDriverNode.is() )
    {
        try
        {
            //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
            Reference< XNameAccess > xAliasesNode;
            xDriverNode->getPropertyValue( ::rtl::OUString::createFromAscii("ColumnAliases") ) >>= xAliasesNode;
            OSL_ENSURE( xAliasesNode.is(), "OColumnAlias::setAlias: missing the aliases node!" );

            // this is a set of string nodes
            Sequence< ::rtl::OUString > aColumnProgrammaticNames;
            if ( xAliasesNode.is() )
                aColumnProgrammaticNames = xAliasesNode->getElementNames();

            //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
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

                //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
                // check the value
                // look up the programmatic indicated by pProgrammaticNames in the known programmatics
                const ::rtl::OUString* pProgrammatic = sProgrammaticNames + FIRSTNAME;
                const ::rtl::OUString* pProgrammaticEnd = sProgrammaticNames + END;

                OSL_ENSURE( (sal_Int32)m_aAlias.size() == pProgrammaticEnd - pProgrammatic,
                    "OColumnAlias::setAlias: aliases vector not yet initialized!" );

                // the destination where we want to remember the alias
                ::std::vector< ::rtl::OUString >::iterator aDestination = m_aAlias.begin();

                for ( ; pProgrammatic < pProgrammaticEnd; ++pProgrammatic, ++aDestination )
                {
                    if ( pProgrammaticNames->equals( *pProgrammatic ) )
                    {
                        // add alias to the vector
                        *aDestination = sAssignedAlias;
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
        m_aAliasMap[m_aAlias[i]] = sProgrammaticNames[i];
    }

        return;
}
//------------------------------------------------------------------
