/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typedetectionexport.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:15:26 $
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

#ifndef _TYPEDETECTION_EXPORT_HXX
#define _TYPEDETECTION_EXPORT_HXX

#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HXX
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#include "xmlfilterjar.hxx"

class TypeDetectionExporter
{
public:
    TypeDetectionExporter( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMSF );

    void doExport(com::sun::star::uno::Reference < com::sun::star::io::XOutputStream > xOS,  const XMLFilterVector& rFilters );

private:
    void addProperty( com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > xWriter, const rtl::OUString& rName, const rtl::OUString& rValue );
    void addLocaleProperty( com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > xWriter, const rtl::OUString& rName, const rtl::OUString& rValue );

    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;
};

#endif
