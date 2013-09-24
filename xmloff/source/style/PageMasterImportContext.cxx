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

#include "PageMasterImportContext.hxx"
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include "PageMasterPropHdl.hxx"
#include "PagePropertySetContext.hxx"
#include "PageHeaderFooterContext.hxx"
#include "PageMasterPropMapper.hxx"
#include "PageMasterImportPropMapper.hxx"
#include <xmloff/PageMasterStyleMap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

void PageStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    // TODO: use a map here
    if( XML_NAMESPACE_STYLE == nPrefixKey && IsXMLToken( rLocalName, XML_PAGE_USAGE ) )
    {
        sPageUsage = rValue;
    }
    else
    {
        XMLPropStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}

TYPEINIT1( PageStyleContext, XMLPropStyleContext );

PageStyleContext::PageStyleContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles,
        sal_Bool bDefaultStyle) :
    XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, XML_STYLE_FAMILY_PAGE_MASTER, bDefaultStyle),
    sPageUsage()
{
}

PageStyleContext::~PageStyleContext()
{
}

SvXMLImportContext *PageStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = NULL;

    if( XML_NAMESPACE_STYLE == nPrefix &&
        ((IsXMLToken(rLocalName, XML_HEADER_STYLE )) ||
         (IsXMLToken(rLocalName, XML_FOOTER_STYLE )) ) )
    {
        sal_Bool bHeader = IsXMLToken(rLocalName, XML_HEADER_STYLE);
        UniReference < SvXMLImportPropertyMapper > xImpPrMap =
            GetStyles()->GetImportPropertyMapper( GetFamily() );
        if( xImpPrMap.is() )
        {
            const UniReference< XMLPropertySetMapper >& rMapper = xImpPrMap->getPropertySetMapper();
            sal_Int32 nFlag;
            if (bHeader)
                nFlag = CTF_PM_HEADERFLAG;
            else
                nFlag = CTF_PM_FOOTERFLAG;
            sal_Int32 nStartIndex (-1);
            sal_Int32 nEndIndex (-1);
            bool bFirst(false);
            bool bEnd(false);
            sal_Int32 nIndex = 0;
            while ( nIndex < rMapper->GetEntryCount() && !bEnd)
            {
                if ((rMapper->GetEntryContextId( nIndex ) & CTF_PM_FLAGMASK) == nFlag)
                {
                    if (!bFirst)
                    {
                        bFirst = true;
                        nStartIndex = nIndex;
                    }
                }
                else if (bFirst)
                {
                    bEnd = true;
                    nEndIndex = nIndex;
                }
                nIndex++;
            }
            if (!bEnd)
                nEndIndex = nIndex;
            pContext = new PageHeaderFooterContext(GetImport(), nPrefix, rLocalName,
                            xAttrList, GetProperties(), xImpPrMap, nStartIndex, nEndIndex, bHeader);
        }
    }
    if( XML_NAMESPACE_STYLE == nPrefix &&
        IsXMLToken(rLocalName, XML_PAGE_LAYOUT_PROPERTIES) )
    {
        UniReference < SvXMLImportPropertyMapper > xImpPrMap =
            GetStyles()->GetImportPropertyMapper( GetFamily() );
        if( xImpPrMap.is() )
        {
            const UniReference< XMLPropertySetMapper >& rMapper = xImpPrMap->getPropertySetMapper();
            sal_Int32 nEndIndex (-1);
            bool bEnd(false);
            sal_Int32 nIndex = 0;
            sal_Int16 nContextID;
            while ( nIndex < rMapper->GetEntryCount() && !bEnd)
            {
                nContextID = rMapper->GetEntryContextId( nIndex );
                if (nContextID && ((nContextID & CTF_PM_FLAGMASK) != XML_PM_CTF_START))
                {
                    nEndIndex = nIndex;
                    bEnd = true;
                }
                nIndex++;
            }
            if (!bEnd)
                nEndIndex = nIndex;
            PageContextType aType = Page;
            pContext = new PagePropertySetContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    XML_TYPE_PROP_PAGE_LAYOUT,
                                                    GetProperties(),
                                                    xImpPrMap, 0, nEndIndex, aType);
        }
    }

    if (!pContext)
        pContext = XMLPropStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                           xAttrList );
    return pContext;
}

void PageStyleContext::FillPropertySet(
            const uno::Reference<beans::XPropertySet > & rPropSet )
{
    XMLPropStyleContext::FillPropertySet(rPropSet);
    if (!sPageUsage.isEmpty())
    {
        uno::Any aPageUsage;
        XMLPMPropHdl_PageStyleLayout aPageUsageHdl;
        if (aPageUsageHdl.importXML(sPageUsage, aPageUsage, GetImport().GetMM100UnitConverter()))
            rPropSet->setPropertyValue("PageStyleLayout", aPageUsage);
    }
}

// text grid enhancement for better CJK support
//set default page layout style
void PageStyleContext::SetDefaults( )
{
    Reference < XMultiServiceFactory > xFactory ( GetImport().GetModel(), UNO_QUERY);
    if (xFactory.is())
    {
        Reference < XInterface > xInt = xFactory->createInstance (
        OUString ( "com.sun.star.text.Defaults" ) );
        Reference < beans::XPropertySet > xProperties ( xInt, UNO_QUERY );
        if ( xProperties.is() )
            FillPropertySet ( xProperties );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
