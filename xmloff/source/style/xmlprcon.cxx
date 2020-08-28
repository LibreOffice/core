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

#include <xmloff/xmlprcon.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlprmap.hxx>

using namespace ::com::sun::star;
using namespace ::std;

SvXMLPropertySetContext::SvXMLPropertySetContext(
    SvXMLImport& rImp, sal_Int32 /*nElement*/,
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList,
    sal_uInt32 nFam,
    vector< XMLPropertyState > &rProps,
    const rtl::Reference < SvXMLImportPropertyMapper >  &rMap,
    sal_Int32 nSIdx, sal_Int32 nEIdx )
:   SvXMLImportContext( rImp )
,   mnStartIdx( nSIdx )
,   mnEndIdx( nEIdx )
,   mnFamily( nFam )
,   mrProperties( rProps )
,   mxMapper( rMap )
{
    mxMapper->importXML( mrProperties, xAttrList,
                        GetImport().GetMM100UnitConverter(),
                        GetImport().GetNamespaceMap(), mnFamily,
                        mnStartIdx, mnEndIdx );
}

SvXMLPropertySetContext::~SvXMLPropertySetContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SvXMLPropertySetContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    rtl::Reference< XMLPropertySetMapper > aSetMapper(
            mxMapper->getPropertySetMapper() );
    sal_Int32 nEntryIndex = aSetMapper->GetEntryIndex( nElement, mnFamily, mnStartIdx );

    if( ( nEntryIndex != -1 ) && (-1 == mnEndIdx || nEntryIndex < mnEndIdx ) &&
        ( 0 != ( aSetMapper->GetEntryFlags( nEntryIndex )
                         & MID_FLAG_ELEMENT_ITEM_IMPORT ) ) )
    {
        XMLPropertyState aProp( nEntryIndex );
        return createFastChildContext( nElement, xAttrList, mrProperties, aProp );
    }
    SAL_WARN("xmloff", "unknown element " << SvXMLImport::getPrefixAndNameFromToken(nElement));
    return nullptr;
}

/** This method is called from this instance implementation of
    CreateChildContext if the element matches an entry in the
    SvXMLImportItemMapper with the mid flag MID_FLAG_ELEMENT
*/
css::uno::Reference< css::xml::sax::XFastContextHandler > SvXMLPropertySetContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/,
    ::std::vector< XMLPropertyState > &/*rProperties*/,
    const XMLPropertyState& /*rProp*/ )
{
    SAL_WARN("xmloff", "unknown element " << SvXMLImport::getPrefixAndNameFromToken(nElement));
    return nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
