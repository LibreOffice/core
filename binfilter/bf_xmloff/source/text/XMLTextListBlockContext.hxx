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

#ifndef _XMLTEXTLISTBLOCKCONTEXT_HXX
#define _XMLTEXTLISTBLOCKCONTEXT_HXX

#include <com/sun/star/container/XIndexReplace.hpp>

#include "xmlictxt.hxx"
namespace binfilter {

class XMLTextImportHelper;

class XMLTextListBlockContext : public SvXMLImportContext
{
    XMLTextImportHelper&	rTxtImport;

    ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XIndexReplace > xNumRules;

    const ::rtl::OUString	sNumberingRules;
    ::rtl::OUString			sStyleName;
    SvXMLImportContextRef	xParentListBlock;
    sal_Int16				nLevel;
    sal_Int16				nLevels;
    sal_Bool				bOrdered : 1;
    sal_Bool				bRestartNumbering : 1;
    sal_Bool				bSetDefaults : 1;


public:

    TYPEINFO();

    XMLTextListBlockContext( SvXMLImport& rImport,
            XMLTextImportHelper& rTxtImp, sal_uInt16 nPrfx,
                const ::rtl::OUString& rLName,
                   const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
                sal_Bool bOrdered );
    virtual ~XMLTextListBlockContext();

    virtual void EndElement();

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    const ::rtl::OUString& GetStyleName() const { return sStyleName; }
    sal_Int16 GetLevel() const { return nLevel; }
    sal_Bool IsRestartNumbering() const { return bRestartNumbering; }
    void ResetRestartNumbering() { bRestartNumbering = sal_False; }

//	sal_Bool HasGeneratedStyle() const { return xGenNumRule.is(); }
    const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace >& GetNumRules() const
        { return xNumRules; }
};


}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
