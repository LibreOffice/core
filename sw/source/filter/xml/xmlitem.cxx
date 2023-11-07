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
    m_rItemSet( rISet ),
    m_rIMapper( rIMap ),
    m_rUnitConv( rUnitConverter )
{
    rIMap.importXML( m_rItemSet, xAttrList, m_rUnitConv,
                           GetImport().GetNamespaceMap() );
}

SwXMLItemSetContext::~SwXMLItemSetContext()
{
    if( m_xBackground.is() )
    {
        const SvxBrushItem& rItem = m_xBackground->GetItem();
        m_rItemSet.Put( rItem );
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SwXMLItemSetContext::createFastChildContext(
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLItemMapEntriesRef xMapEntries = m_rIMapper.getMapEntries();
    SvXMLItemMapEntry const * pEntry = xMapEntries->getByName( nElement );

    if( pEntry && 0 != (pEntry->nMemberId & MID_SW_FLAG_ELEMENT_ITEM_IMPORT) )
    {
        return createFastChildContext( nElement, xAttrList, *pEntry );
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("sw", nElement);
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
    rtl::Reference<SwXMLBrushItemImportContext> xContext;

    switch( rEntry.nWhichId )
    {
    case RES_BACKGROUND:
        {
            if( const SvxBrushItem* pItem = m_rItemSet.GetItemIfSet( RES_BACKGROUND,
                                                       false ) )
            {
                xContext = new SwXMLBrushItemImportContext(
                                GetImport(), nElement, xAttrList,
                                m_rUnitConv, *pItem );
            }
            else
            {
                xContext = new SwXMLBrushItemImportContext(
                                GetImport(), nElement, xAttrList,
                                m_rUnitConv, RES_BACKGROUND );
            }
            m_xBackground = xContext;
        }
        break;
    }

    return xContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
