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

#include <oox/drawingml/themefragmenthandler.hxx>
#include <drawingml/objectdefaultcontext.hxx>
#include <oox/drawingml/theme.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/helper/attributelist.hxx>
#include <drawingml/themeelementscontext.hxx>

using namespace ::oox::core;

namespace oox::drawingml
{

ThemeFragmentHandler::ThemeFragmentHandler(XmlFilterBase& rFilter, const OUString& rFragmentPath, Theme& rOoxTheme, model::Theme& rTheme)
    : FragmentHandler2(rFilter, rFragmentPath)
    , mrOoxTheme(rOoxTheme)
    , mrTheme(rTheme)
{
}

ThemeFragmentHandler::~ThemeFragmentHandler()
{
}

ContextHandlerRef ThemeFragmentHandler::onCreateContext( sal_Int32 nElement, const AttributeList& /*rAttribs*/)
{
    // CT_OfficeStyleSheet
    if (getCurrentElement() == A_TOKEN(theme))
    {
        switch (nElement)
        {
            case A_TOKEN( themeElements ):              // CT_BaseStyles
                return new ThemeElementsContext(*this, mrOoxTheme, mrTheme);
            case A_TOKEN( objectDefaults ):             // CT_ObjectStyleDefaults
                return new objectDefaultContext(*this, mrOoxTheme);
            case A_TOKEN( extraClrSchemeLst ):          // CT_ColorSchemeList
                return nullptr;
            case A_TOKEN( custClrLst ):                 // CustomColorList
                return nullptr;
            case A_TOKEN( ext ):                        // CT_OfficeArtExtension
                return nullptr;
        }
    }
    else if (getCurrentElement() == XML_ROOT_CONTEXT)
    {
        return this;
    }

    return nullptr;
}

void ThemeFragmentHandler::onStartElement(const AttributeList& rAttribs)
{
    if (getCurrentElement() == A_TOKEN(theme))
    {
        OUString aName = rAttribs.getStringDefaulted(XML_name);
        mrOoxTheme.setThemeName(aName);
        mrTheme.SetName(aName);
    }
}

} // namespace oox::drawingml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
