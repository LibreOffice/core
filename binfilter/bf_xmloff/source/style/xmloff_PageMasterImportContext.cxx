/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _XMLOFF_PAGEMASTERIMPORTCONTEXT_HXX
#include "PageMasterImportContext.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_PAGEMASTERPROPHDL_HXX_
#include "PageMasterPropHdl.hxx"
#endif
#ifndef _XMLOFF_PAGEPROPERTYSETCONTEXT_HXX
#include "PagePropertySetContext.hxx"
#endif
#ifndef _XMLOFF_PAGEPHEADERFOOTERCONTEXT_HXX
#include "PageHeaderFooterContext.hxx"
#endif
#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include "PageMasterStyleMap.hxx"
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

void PageStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const ::rtl::OUString& rLocalName,
                                        const ::rtl::OUString& rValue )
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
        sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles) :
    XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, XML_STYLE_FAMILY_PAGE_MASTER ),
    sPageUsage()
{
}

PageStyleContext::~PageStyleContext()
{
}

SvXMLImportContext *PageStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
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
            sal_Bool bFirst(sal_False);
            sal_Bool bEnd(sal_False);
            sal_Int32 nIndex = 0;
            while ( nIndex < rMapper->GetEntryCount() && !bEnd)
            {
                if ((rMapper->GetEntryContextId( nIndex ) & CTF_PM_FLAGMASK) == nFlag)
                {
                    if (!bFirst)
                    {
                        bFirst = sal_True;
                        nStartIndex = nIndex;
                    }
                }
                else if (bFirst)
                {
                    bEnd = sal_True;
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
        IsXMLToken(rLocalName, XML_PROPERTIES) )
    {
        UniReference < SvXMLImportPropertyMapper > xImpPrMap =
            GetStyles()->GetImportPropertyMapper( GetFamily() );
        if( xImpPrMap.is() )
        {
            const UniReference< XMLPropertySetMapper >& rMapper = xImpPrMap->getPropertySetMapper();
            sal_Int32 nEndIndex (-1);
            sal_Bool bEnd(sal_False);
            sal_Int32 nIndex = 0;
            sal_Int16 nContextID;
            while ( nIndex < rMapper->GetEntryCount() && !bEnd)
            {
                nContextID = rMapper->GetEntryContextId( nIndex );
                if (nContextID && ((nContextID & CTF_PM_FLAGMASK) != XML_PM_CTF_START))
                {
                    nEndIndex = nIndex;
                    bEnd = sal_True;
                }
                nIndex++;
            }
            if (!bEnd)
                nEndIndex = nIndex;
            PageContextType aType = Page;
            pContext = new PagePropertySetContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
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
    if (sPageUsage.getLength())
    {
        uno::Any aPageUsage;
        XMLPMPropHdl_PageStyleLayout aPageUsageHdl;
        if (aPageUsageHdl.importXML(sPageUsage, aPageUsage, GetImport().GetMM100UnitConverter()))
            rPropSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PageStyleLayout")), aPageUsage);
    }
}

}//end of namespace binfilter
