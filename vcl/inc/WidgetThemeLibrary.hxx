/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_WIDGETTHEME_HXX
#define INCLUDED_VCL_INC_WIDGETTHEME_HXX

#include <cairo.h>
#include <vcl/dllapi.h>

namespace vcl
{
class SAL_DLLPUBLIC_RTTI WidgetThemeLibrary
{
public:
    WidgetThemeLibrary();
    virtual ~WidgetThemeLibrary();

    virtual bool drawPushButtonFocus(cairo_t* pCairo, long nX, long nY, long nWidth, long nHeight);
    virtual bool drawPushButton(cairo_t* pCairo, long nX, long nY, long nWidth, long nHeight);
};

extern "C" vcl::WidgetThemeLibrary* CreateWidgetThemeLibrary();

} // end vcl namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
