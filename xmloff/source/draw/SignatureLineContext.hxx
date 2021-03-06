/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/drawing/XShape.hpp>
#include <xmloff/xmlictxt.hxx>

// signatureline inside a shape

class SignatureLineContext : public SvXMLImportContext
{
public:
    SignatureLineContext(SvXMLImport& rImport, sal_Int32 nElement,
                         const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
                         const css::uno::Reference<css::drawing::XShape>& rxShape);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
