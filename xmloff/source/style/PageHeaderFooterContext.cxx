/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PageHeaderFooterContext.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:44:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"




// INCLUDE ---------------------------------------------------------------

#ifndef _XMLOFF_PAGEPHEADERFOOTERCONTEXT_HXX
#include "PageHeaderFooterContext.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif
#ifndef _XMLOFF_PAGEPROPERTYSETCONTEXT_HXX
#include "PagePropertySetContext.hxx"
#endif

using namespace com::sun::star;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_HEADER_FOOTER_PROPERTIES;

//------------------------------------------------------------------

PageHeaderFooterContext::PageHeaderFooterContext( SvXMLImport& rImport,
                                      USHORT nPrfx,
                                      const rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>&,
                                      ::std::vector< XMLPropertyState > & rTempProperties,
                                      const UniReference < SvXMLImportPropertyMapper > &rTempMap,
                                      sal_Int32 nStart, sal_Int32 nEnd,
                                      const sal_Bool bTempHeader ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rProperties(rTempProperties),
    nStartIndex(nStart),
    nEndIndex(nEnd),
    rMap(rTempMap)
{
    bHeader = bTempHeader;
}

PageHeaderFooterContext::~PageHeaderFooterContext()
{
}

SvXMLImportContext *PageHeaderFooterContext::CreateChildContext( USHORT nPrefix,
                                            const rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix && IsXMLToken( rLName, XML_HEADER_FOOTER_PROPERTIES ) )
    {
        PageContextType aType = Header;
        if (!bHeader)
            aType = Footer;
        pContext = new PagePropertySetContext( GetImport(), nPrefix,
                                                rLName, xAttrList,
                                                XML_TYPE_PROP_HEADER_FOOTER,
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

