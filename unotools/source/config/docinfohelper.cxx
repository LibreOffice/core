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

#include <rtl/ustrbuf.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/docinfohelper.hxx>
#include <rtl/bootstrap.hxx>

using namespace ::com::sun::star;

namespace utl
{

OUString DocInfoHelper::GetGeneratorString()
{
    OUStringBuffer aResult;

    // First product: branded name + version
    // version is <product_versions>_<product_extension>$<platform>

    // plain product name
    OUString aValue( utl::ConfigManager::getProductName() );
    if ( !aValue.isEmpty() )
    {
        aResult.append( aValue.replace( ' ', '_' ) );
        aResult.append( '/' );

        aValue = utl::ConfigManager::getProductVersion();
        if ( !aValue.isEmpty() )
        {
            aResult.append( aValue.replace( ' ', '_' ) );

            aValue = utl::ConfigManager::getProductExtension();
            if ( !aValue.isEmpty() )
            {
                aResult.append( aValue.replace( ' ', '_' ) );
            }
        }

        OUString os( "$_OS" );
        OUString arch( "$_ARCH" );
        ::rtl::Bootstrap::expandMacros(os);
        ::rtl::Bootstrap::expandMacros(arch);
        aResult.append( '$' );
        aResult.append( os );
        aResult.append( '_' );
        aResult.append( arch );
        aResult.append( ' ' );
    }

    // second product: LibreOffice_project/<build_information>
    // build_information has '(' and '[' encoded as '$', ')' and ']' ignored
    // and ':' replaced by '-'
    {
        aResult.append( "LibreOffice_project/" );
        OUString aBuildId( Bootstrap::getBuildIdData( OUString() ) );
        for( sal_Int32 i=0; i < aBuildId.getLength(); i++ )
        {
            sal_Unicode c = aBuildId[i];
            switch( c )
            {
            case '(':
            case '[':
                aResult.append( '$' );
                break;
            case ')':
            case ']':
                break;
            case ':':
                aResult.append( '-' );
                break;
            default:
                aResult.append( c );
                break;
            }
        }
    }

    return aResult.makeStringAndClear();
}

} // end of namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
