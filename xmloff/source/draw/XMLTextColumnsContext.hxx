/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <com/sun/star/xml/sax/XFastContextHandler.hpp>

#include <sal/types.h>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltkmap.hxx>

#include <vector>

#include "XMLElementPropertyContext.hxx"

class SvxXMLTextColumnsContext final : public XMLElementPropertyContext
{
public:
    SvxXMLTextColumnsContext(
        SvXMLImport& rImport, sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
        const XMLPropertyState& rProp, std::vector<XMLPropertyState>& rProps);

    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& AttrList) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

private:
    sal_Int16 mnColumns = 1;
    sal_Int32 mnSpacing = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
