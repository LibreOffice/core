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

#include "MColumnAlias.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <officecfg/Office/DataAccess.hxx>

#include <tools/diagnose_ex.h>

#include <algorithm>
#include <functional>

using namespace ::connectivity::mork;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;


OColumnAlias::OColumnAlias( const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxORB )
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
        m_aAliasMap[ OUString::createFromAscii( s_pProgrammaticNames[i] ) ] = AliasEntry( s_pProgrammaticNames[i], i );

    initialize( _rxORB );
}


void OColumnAlias::initialize( const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxORB )
{
    Reference< XNameAccess > xAliasesNode(
        officecfg::Office::DataAccess::DriverSettings::
        com_sun_star_comp_sdbc_MozabDriver::ColumnAliases::get(
            comphelper::getComponentContext(_rxORB)),
        UNO_QUERY_THROW);
    Sequence< OUString > aProgrammaticNames(xAliasesNode->getElementNames());
    for (sal_Int32 i = 0; i != aProgrammaticNames.getLength(); ++i) {
        OString sAsciiProgrammaticName(
            OUStringToOString(
                aProgrammaticNames[i], RTL_TEXTENCODING_ASCII_US));
        bool bFound = false;
        for (AliasMap::const_iterator j(m_aAliasMap.begin()); j != m_aAliasMap.end();
             ++j)
        {
            if (j->second.programmaticAsciiName == sAsciiProgrammaticName) {
                OUString sAssignedAlias;
                xAliasesNode->getByName(aProgrammaticNames[i]) >>=
                    sAssignedAlias;
                if (sAssignedAlias.isEmpty()) {
                    sAssignedAlias = aProgrammaticNames[i];
                }
                AliasEntry entry(j->second);
                m_aAliasMap.erase(j);
                m_aAliasMap[sAssignedAlias] = entry;
                bFound = true;
                break;
            }
        }
        SAL_WARN_IF(
            !bFound, "connectivity.mork",
            "unknown programmatic name " << aProgrammaticNames[i]
                <<" from configuration");
    }
}


OString OColumnAlias::getProgrammaticNameOrFallbackToUTF8Alias( const OUString& _rAlias ) const
{
    AliasMap::const_iterator pos = m_aAliasMap.find( _rAlias );
    if ( pos == m_aAliasMap.end() )
    {
        OSL_FAIL( "OColumnAlias::getProgrammaticNameOrFallbackToUTF8Alias: no programmatic name for this alias!" );
        return OUStringToOString( _rAlias, RTL_TEXTENCODING_UTF8 );
    }
    return pos->second.programmaticAsciiName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
