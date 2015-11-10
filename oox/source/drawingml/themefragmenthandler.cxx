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

#include "oox/drawingml/themefragmenthandler.hxx"
#include "drawingml/objectdefaultcontext.hxx"
#include "oox/drawingml/theme.hxx"
#include "drawingml/themeelementscontext.hxx"

using namespace ::oox::core;

namespace oox {
namespace drawingml {

ThemeFragmentHandler::ThemeFragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath, Theme& rTheme ) :
    FragmentHandler2( rFilter, rFragmentPath ),
    mrTheme( rTheme )
{
}

ThemeFragmentHandler::~ThemeFragmentHandler()
{
}

ContextHandlerRef ThemeFragmentHandler::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    // CT_OfficeStyleSheet
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            switch( nElement )
            {
                case A_TOKEN( theme ):
                    return this;
            }
        break;

        case A_TOKEN( theme ):
            switch( nElement )
            {
                case A_TOKEN( themeElements ):              // CT_BaseStyles
                    return new ThemeElementsContext( *this, mrTheme );
                case A_TOKEN( objectDefaults ):             // CT_ObjectStyleDefaults
                    return new objectDefaultContext( *this, mrTheme );
                case A_TOKEN( extraClrSchemeLst ):          // CT_ColorSchemeList
                    return nullptr;
                case A_TOKEN( custClrLst ):                 // CustomColorList
                    return nullptr;
                case A_TOKEN( ext ):                        // CT_OfficeArtExtension
                    return nullptr;
            }
        break;
    }
    return nullptr;
}

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
