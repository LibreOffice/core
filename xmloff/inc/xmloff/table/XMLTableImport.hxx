/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTableImport.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:24:45 $
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

#ifndef XMLTABLEIMPORT_HXX_
#define XMLTABLEIMPORT_HXX_

#include <com/sun/star/table/XColumnRowRange.hpp>

#include "xmloff/dllapi.h"
#include "xmloff/xmlictxt.hxx"
#include "xmloff/uniref.hxx"
#include "xmloff/xmlimppr.hxx"
#include "xmloff/prhdlfac.hxx"

#include <rtl/ref.hxx>

#include <boost/shared_ptr.hpp>
#include <map>

class SvXMLStyleContext;

typedef std::map< ::rtl::OUString, ::rtl::OUString > XMLTableTemplate;
typedef std::map < ::rtl::OUString, boost::shared_ptr< XMLTableTemplate > > XMLTableTemplateMap;

class XMLTableImport : public UniRefBase
{
    friend class XMLTableImportContext;

public:
    XMLTableImport( SvXMLImport& rImport, const rtl::Reference< XMLPropertySetMapper >& xCellPropertySetMapper, const rtl::Reference< XMLPropertyHandlerFactory >& xFactoryRef );
    virtual ~XMLTableImport();

    SvXMLImportContext* CreateTableContext( USHORT nPrfx, const ::rtl::OUString& rLName,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::table::XColumnRowRange >& xColumnRowRange );

    SvXMLStyleContext* CreateTableTemplateContext( USHORT nPrfx, const ::rtl::OUString& rLName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    rtl::Reference< SvXMLImportPropertyMapper > GetCellImportPropertySetMapper() const { return mxCellImportPropertySetMapper; }
    rtl::Reference< SvXMLImportPropertyMapper > GetRowImportPropertySetMapper() const { return mxRowImportPropertySetMapper; }
    rtl::Reference< SvXMLImportPropertyMapper > GetColumnImportPropertySetMapper() const { return mxColumnImportPropertySetMapper; }

    void addTableTemplate( const rtl::OUString& rsStyleName, XMLTableTemplate& xTableTemplate );
    void finishStyles();

private:
    SvXMLImport&                                 mrImport;
    rtl::Reference< SvXMLImportPropertyMapper > mxCellImportPropertySetMapper;
    rtl::Reference< SvXMLImportPropertyMapper > mxRowImportPropertySetMapper;
    rtl::Reference< SvXMLImportPropertyMapper > mxColumnImportPropertySetMapper;

    XMLTableTemplateMap                         maTableTemplates;
};

#endif /*XMLTABLEIMPORT_HXX_*/
