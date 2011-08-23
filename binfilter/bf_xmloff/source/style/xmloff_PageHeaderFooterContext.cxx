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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#ifndef _XMLOFF_PAGEPHEADERFOOTERCONTEXT_HXX
#include "PageHeaderFooterContext.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_PAGEPROPERTYSETCONTEXT_HXX
#include "PagePropertySetContext.hxx"
#endif
namespace binfilter {

using namespace ::com::sun::star;
using ::binfilter::xmloff::token::IsXMLToken;
using ::binfilter::xmloff::token::XML_PROPERTIES;

//------------------------------------------------------------------

PageHeaderFooterContext::PageHeaderFooterContext( SvXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ::std::vector< XMLPropertyState > & rTempProperties,
                                      const UniReference < SvXMLImportPropertyMapper > &rTempMap,
                                      sal_Int32 nStart, sal_Int32 nEnd,
                                      const sal_Bool bTempHeader ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rProperties(rTempProperties),
    rMap(rTempMap),
    nStartIndex(nStart),
    nEndIndex(nEnd)
{
    bHeader = bTempHeader;
}

PageHeaderFooterContext::~PageHeaderFooterContext()
{
}

SvXMLImportContext *PageHeaderFooterContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix && IsXMLToken( rLName, XML_PROPERTIES ) )
    {
        PageContextType aType = Header;
        if (!bHeader)
            aType = Footer;
        pContext = new PagePropertySetContext( GetImport(), nPrefix,
                                                rLName, xAttrList,
                                                rProperties,
                                                rMap,  nStartIndex, nEndIndex, aType);
    }


    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void PageHeaderFooterContext::EndElement()
{
}

}//end of namespace binfilter
