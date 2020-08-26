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

#include <editeng/brushitem.hxx>
#include <sal/log.hxx>
#include <xmloff/xmlimp.hxx>
#include "xmlimpit.hxx"
#include "xmlitem.hxx"
#include "xmlbrshi.hxx"
#include <hintids.hxx>

using namespace ::com::sun::star;

SwXMLItemSetContext::SwXMLItemSetContext( SvXMLImport& rImp, sal_Int32 /*nElement*/,
                                          const uno::Reference< xml::sax::XFastAttributeList >& xAttrList,
                                          SfxItemSet& rISet,
                                          SvXMLImportItemMapper& rIMap,
                                          const SvXMLUnitConverter& rUnitConverter ):
    SvXMLImportContext( rImp ),
    rItemSet( rISet ),
    rIMapper( rIMap ),
    rUnitConv( rUnitConverter )
{
    rIMap.importXML( rItemSet, xAttrList, rUnitConv,
                           GetImport().GetNamespaceMap() );
}

SwXMLItemSetContext::~SwXMLItemSetContext()
{
    if( xBackground.is() )
    {
        const SvxBrushItem& rItem =
            static_cast<SwXMLBrushItemImportContext*>(xBackground.get())->GetItem();
        rItemSet.Put( rItem );
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SwXMLItemSetContext::createFastChildContext(
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLItemMapEntriesRef xMapEntries = rIMapper.getMapEntries();
    SvXMLItemMapEntry const * pEntry = xMapEntries->getByName( nElement );

    if( pEntry && 0 != (pEntry->nMemberId & MID_SW_FLAG_ELEMENT_ITEM_IMPORT) )
    {
        return createFastChildContext( nElement, xAttrList, *pEntry ).get();
    }
    else
        SAL_WARN("sw", "unknown element " << SvXMLImport::getPrefixAndNameFromToken(nElement));
    return nullptr;
}

/** This method is called from this instance implementation of
    CreateChildContext if the element matches an entry in the
    SvXMLImportItemMapper with the mid flag MID_SW_FLAG_ELEMENT
*/
SvXMLImportContextRef SwXMLItemSetContext::createFastChildContext( sal_Int32 nElement,
                                   const uno::Reference< xml::sax::XFastAttributeList >& xAttrList,
                                   const SvXMLItemMapEntry& rEntry )
{
    SvXMLImportContextRef xContext;

    switch( rEntry.nWhichId )
    {
    case RES_BACKGROUND:
        {
            const SfxPoolItem *pItem;
            if( SfxItemState::SET == rItemSet.GetItemState( RES_BACKGROUND,
                                                       false, &pItem ) )
            {
                xContext = new SwXMLBrushItemImportContext(
                                GetImport(), nElement, xAttrList,
                                rUnitConv, *static_cast<const SvxBrushItem *>(pItem) );
            }
            else
            {
                xContext = new SwXMLBrushItemImportContext(
                                GetImport(), nElement, xAttrList,
                                rUnitConv, RES_BACKGROUND );
            }
            xBackground = xContext;
        }
        break;
    }

    return xContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
