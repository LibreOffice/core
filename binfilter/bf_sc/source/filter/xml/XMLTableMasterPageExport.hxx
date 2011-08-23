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

#ifndef _SC_XMLTABLEMASTERPAGEEXPORT_HXX
#define _SC_XMLTABLEMASTERPAGEEXPORT_HXX

#include <rtl/ustring.hxx>

#include <bf_xmloff/xmltoken.hxx>
#include <bf_xmloff/XMLTextMasterPageExport.hxx>

#include <com/sun/star/sheet/XHeaderFooterContent.hpp>

#include "xmlexprt.hxx"
namespace binfilter {

namespace com { namespace sun { namespace star {
    namespace text { class XText; }
} } }

class XMLTableMasterPageExport : public XMLTextMasterPageExport
{
    void exportHeaderFooter(const ::com::sun::star::uno::Reference < ::com::sun::star::sheet::XHeaderFooterContent >& xHeaderFooter,
                            const xmloff::token::XMLTokenEnum aName,
                            const sal_Bool bDisplay);

protected:
    virtual void exportHeaderFooterContent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XText >& rText,
            sal_Bool bAutoStyles, sal_Bool bProgress );

    virtual void exportMasterPageContent(
                const ::com::sun::star::uno::Reference <
                    ::com::sun::star::beans::XPropertySet > & rPropSet,
                 sal_Bool bAutoStyles );

public:
    XMLTableMasterPageExport( ScXMLExport& rExp );
    ~XMLTableMasterPageExport();
};


} //namespace binfilter
#endif	//  _XMLOFF_XMLTABLEMASTERPAGEEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
