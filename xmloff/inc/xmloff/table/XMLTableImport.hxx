/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

    SvXMLImportContext* CreateTableContext( sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::table::XColumnRowRange >& xColumnRowRange );

    SvXMLStyleContext* CreateTableTemplateContext( sal_uInt16 nPrfx, const ::rtl::OUString& rLName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
