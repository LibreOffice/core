/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/uno/Reference.hxx>

#include <vcl/bitmapex.hxx>
#include <vcl/dllapi.h>

namespace com::sun::star::awt
{
class XBitmap;
}
namespace com::sun::star::uno
{
class Any;
}

namespace vcl
{
VCL_DLLPUBLIC BitmapEx GetBitmap(const css::uno::Reference<css::awt::XBitmap>& xBitmap);

// Checks if the Any contains graphic::XGraphic; or if awt::XBitmap is there, converts it
VCL_DLLPUBLIC css::uno::Reference<css::graphic::XGraphic> GetGraphic(const css::uno::Any& any);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
