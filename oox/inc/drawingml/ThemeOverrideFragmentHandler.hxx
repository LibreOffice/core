/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OOX_DRAWINGML_THEMEOVERRICEFRAGMENTHANDLER_HXX
#define INCLUDED_OOX_DRAWINGML_THEMEOVERRICEFRAGMENTHANDLER_HXX

#include <oox/core/fragmenthandler2.hxx>

namespace oox {
namespace drawingml {

class Theme;

class ThemeOverrideFragmentHandler : public ::oox::core::FragmentHandler2
{
public:
    explicit            ThemeOverrideFragmentHandler(
                            ::oox::core::XmlFilterBase& rFilter,
                            const OUString& rFragmentPath,
                            Theme& rTheme );
    virtual             ~ThemeOverrideFragmentHandler();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;

private:
    Theme&              mrTheme;
};

} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
