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
#ifndef _XMLOFF_TEXTSTYLE_HXX_
#define _XMLOFF_TEXTSTYLE_HXX_

#include <bf_xmloff/styleexp.hxx>
#include <bf_xmloff/uniref.hxx>
namespace binfilter {

class XMLPropertySetMapper;

class XMLTextStyleExport : public XMLStyleExport
{
    UniReference < XMLPropertySetMapper > xParaPropMapper;

    void exportNumStyles( sal_Bool bUsed );

public:

    XMLTextStyleExport(
        SvXMLExport& rExp,
//		const ::com::sun::star::uno::Reference<
//				::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
//		const ::com::sun::star::uno::Reference<
//				::com::sun::star::frame::XModel > & rModel,
        const ::rtl::OUString& rPoolStyleName,
//		const SvXMLNamespaceMap &rNamespaceMap,
//	 	MapUnit eDUnit,
        SvXMLAutoStylePoolP *pAutoStyleP=0 );
    virtual ~XMLTextStyleExport();

    void exportStyles( sal_Bool bUsed );

    UniReference < XMLPropertySetMapper >& getParaPropMapper();
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
