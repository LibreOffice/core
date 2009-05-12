/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xmlannoi.hxx,v $
 * $Revision: 1.8 $
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
#ifndef SC_XMLANNOI_HXX
#define SC_XMLANNOI_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>

class ScXMLImport;
class ScXMLTableRowCellContext;

class ScXMLAnnotationContext : public SvXMLImportContext
{
    rtl::OUStringBuffer sOUText;
    rtl::OUStringBuffer sAuthorBuffer;
    rtl::OUStringBuffer sCreateDateBuffer;
    rtl::OUStringBuffer sCreateDateStringBuffer;
    sal_Int32       nParagraphCount;
    sal_Bool        bDisplay;
    sal_Bool        bHasTextP;
    sal_Bool        bHasPos;
    ScXMLTableRowCellContext*   pCellContext;
    SvXMLImportContext*         pShapeContext;
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape;
    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > xShapes;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLAnnotationContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLTableRowCellContext* pCellContext);

    virtual ~ScXMLAnnotationContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual void Characters( const ::rtl::OUString& rChars );

    virtual void EndElement();

    void SetShape(const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xTempShape,
        const com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& xTempShapes) { xShape.set(xTempShape); xShapes.set(xTempShapes); }
};


#endif

