/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "StylePropertiesContext.hxx"

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/txtprmap.hxx>

#include <xmloff/XMLComplexColorContext.hxx>

StylePropertiesContext::StylePropertiesContext(
    SvXMLImport& rImport, sal_Int32 nElement,
    const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList, sal_uInt32 nFamily,
    std::vector<XMLPropertyState>& rProps, SvXMLImportPropertyMapper* pMap)
    : SvXMLPropertySetContext(rImport, nElement, xAttrList, nFamily, rProps, pMap)
{
}

StylePropertiesContext::~StylePropertiesContext() {}

css::uno::Reference<css::xml::sax::XFastContextHandler>
StylePropertiesContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
    std::vector<XMLPropertyState>& rProperties, const XMLPropertyState& rProperty)
{
    // Character, fill and stroke complex colors all share this context id.
    if (mpMapper->getPropertySetMapper()->GetEntryContextId(rProperty.mnIndex) == CTF_COMPLEX_COLOR)
    {
        return new XMLPropertyComplexColorContext(GetImport(), nElement, xAttrList, rProperty,
                                                  rProperties);
    }
    return SvXMLPropertySetContext::createFastChildContext(nElement, xAttrList, rProperties,
                                                           rProperty);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
