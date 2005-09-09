/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PageHeaderFooterContext.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:25:36 $
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
#ifndef _XMLOFF_PAGEHEADERFOOTERCONTEXT_HXX
#define _XMLOFF_PAGEHEADERFOOTERCONTEXT_HXX

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

class PageHeaderFooterContext : public SvXMLImportContext
{
    ::std::vector< XMLPropertyState > &     rProperties;
    sal_Int32                               nStartIndex;
    sal_Int32                               nEndIndex;
    sal_Bool                                bHeader;
    const UniReference < SvXMLImportPropertyMapper > rMap;

public:

    PageHeaderFooterContext( SvXMLImport& rImport, USHORT nPrfx,
                       const rtl::OUString& rLName,
                       const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ::std::vector< XMLPropertyState > & rProperties,
                        const UniReference < SvXMLImportPropertyMapper > &rMap,
                        sal_Int32 nStartIndex, sal_Int32 nEndIndex,
                        const sal_Bool bHeader);

    virtual ~PageHeaderFooterContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

#endif


