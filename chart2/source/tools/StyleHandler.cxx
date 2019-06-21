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

#include <StyleHandler.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <vector>
#include <memory>
#include <map>
#include <config_folders.h>

using osl::FileBase;

namespace
{
// File loading code goes here
OUString lcl_GetFileUrl()
{
    OUString aURL = OUString("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/chart2/styles.xml");
    rtl::Bootstrap::expandMacros(aURL);
    return aURL;
}

void lcl_InitializeStyleMap()
{}

void lcl_exportStyle()
{};

} // namespace anonymous

namespace chart
{

StyleHandler::StyleHandler( const StyleType eType ):
    m_eType( eType )
{
    if ( !bIsDataLoaded )
    {
        bIsDataLoaded = true;
        lcl_InitializeStyleMap(); // think I should hard code the current chart defaults in LibreOffice
        loadStyleData();
    }
}

StyleHandler::~StyleHandler()
{}

tStylePropertyMap& StyleHandler::getStyle()
{
    if ( m_nLocalStyle >= 0 )
    {
        // locally applied style will be used to supply default values
    }
    else
    {
        // global default for charts will be used if no local style is set
    }
}

void StyleHandler::createStyle( const tStylePropertyMap& rPropMap, StyleType eType )
{
    //chart export code goes here
}

void StyleHandler::deleteStyle( const sal_Int16 nValue )
{
    //remove the style from loaded data and remove the data from the style sheel xml
}

std::vector< std::vector< tStylePropertyMap > > StyleHandler::m_pStyleMap =
                            std::vector< std::vector< tStylePropertyMap > >();

sal_Int16 StyleHandler::m_nDefaultStyle = 1;
std::unique_ptr< osl::File > StyleHandler::m_pStyleSheet = std::make_unique< osl::File >( lcl_GetFileUrl() );
bool StyleHandler::bIsDataLoaded = false;

void StyleHandler::loadStyleData()
{
    // chart import code goes here ( to be used only once to cache all the styles )
}

void StyleHandler::setLocalStyle( const sal_Int16 nValue )
{
    if ( nValue >= 0 )
        m_nLocalStyle = nValue;
}

void StyleHandler::setDefaultStyle( const sal_Int16 nValue )
{
    if ( nValue >= 0 )
        m_nDefaultStyle = nValue;
}

} // namespace chart

 /* vim:set shiftwidth=4 softtabstop=4 expandtab: */
