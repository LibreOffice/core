/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawingml/ThemeOverrideFragmentHandler.hxx>
#include "oox/drawingml/theme.hxx"
#include "drawingml/themeelementscontext.hxx"

using namespace ::oox::core;

namespace oox {
namespace drawingml {

ThemeOverrideFragmentHandler::ThemeOverrideFragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath, Theme& rTheme ) :
    FragmentHandler2( rFilter, rFragmentPath ),
    mrTheme( rTheme )
{
}

ThemeOverrideFragmentHandler::~ThemeOverrideFragmentHandler()
{
}

ContextHandlerRef ThemeOverrideFragmentHandler::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    // CT_OfficeStyleSheet
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            switch( nElement )
            {
                case A_TOKEN( themeOverride ): // CT_BaseStylesOverride
                    return new ThemeElementsContext( *this, mrTheme );
            }
        break;
    }
    return nullptr;
}

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
