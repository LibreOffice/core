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

#ifndef _SC_XMLTEXTPCONTEXT_HXX
#define _SC_XMLTEXTPCONTEXT_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <bf_xmloff/xmlictxt.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
namespace binfilter {

class ScXMLImport;
class ScXMLTableRowCellContext;

class ScXMLTextPContext : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList> xAttrList;
    SvXMLImportContext*			pTextPContext;
    ScXMLTableRowCellContext*	pCellContext;
    ::rtl::OUString				sLName;
    ::rtl::OUStringBuffer			sOUText;
    USHORT						nPrefix;
    sal_Bool					bIsOwn : 1;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLTextPContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLTableRowCellContext* pCellContext);

    virtual ~ScXMLTextPContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void Characters( const ::rtl::OUString& rChars );

    virtual void EndElement();

    void AddSpaces(sal_Int32 nSpaceCount);
};

} //namespace binfilter
#endif
