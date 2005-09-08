/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlannoi.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:03:03 $
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
#ifndef SC_XMLANNOI_HXX
#define SC_XMLANNOI_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif

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

