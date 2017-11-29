/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_GMH_HXX
#define INCLUDED_COMPHELPER_GMH_HXX

#include <comphelper/comphelperdllapi.h>
#include <rtl/ustring.hxx>

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/Reference.hxx>

namespace comphelper
{
class COMPHELPER_DLLPUBLIC GraphicMimeTypeHelper
{
public:
    static OUString GetMimeTypeForExtension(const OString& rExt);
    static OUString GetMimeTypeForXGraphic(css::uno::Reference<css::graphic::XGraphic> xGraphic);
    static OUString GetMimeTypeForImageUrl(const OUString& rImageUrl);
    static OUString
    GetMimeTypeForImageStream(css::uno::Reference<css::io::XInputStream> xInputStream);
};
}

#endif // INCLUDED_COMPHELPER_GMH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
