/*************************************************************************
 *
 *  $RCSfile: ximpshap.hxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: cl $ $Date: 2001-02-21 18:04:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XIMPSHAPE_HXX
#define _XIMPSHAPE_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _SDXMLIMP_IMPL_HXX
#include "sdxmlimp_impl.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTCURSOR_HPP_
#include <com/sun/star/text/XTextCursor.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

//////////////////////////////////////////////////////////////////////////////
// common shape context

class SdXMLShapeContext : public SvXMLImportContext
{
protected:
    // the shape group this object should be created inside
    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >  mxShapes;
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape >   mxShape;
    com::sun::star::uno::Reference< com::sun::star::text::XTextCursor > mxCursor;
    com::sun::star::uno::Reference< com::sun::star::text::XTextCursor > mxOldCursor;
    com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList> mxAttrList;
    rtl::OUString               maDrawStyleName;
    rtl::OUString               maPresentationClass;
    rtl::OUString               maShapeName;
    sal_uInt16                  mnStyleFamily;
    sal_uInt16                  mnClass;
    sal_Bool                    mbIsPlaceholder;
    sal_Bool                    mbIsUserTransformed;
    sal_Int32                   mnZOrder;
    sal_Int32                   mnShapeId;
    rtl::OUString               maLayerName;

    SdXMLImExTransform2D        mnTransform;

    void SetStyle();
    void SetLayer();
    void AddShape(com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape);
    void AddShape(const char* pServiceName );
    void SetTransformation();

    SvXMLImport& GetImport() { return SvXMLImportContext::GetImport(); }
    const SvXMLImport& GetImport() const { return SvXMLImportContext::GetImport(); }

public:
    TYPEINFO();

    SdXMLShapeContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLShapeContext();

    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual SvXMLImportContext * CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:rect context

class SdXMLRectShapeContext : public SdXMLShapeContext
{
    sal_Int32                   mnRadius;

public:
    TYPEINFO();

    SdXMLRectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLRectShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:line context

class SdXMLLineShapeContext : public SdXMLShapeContext
{
    sal_Int32                   mnX1;
    sal_Int32                   mnY1;
    sal_Int32                   mnX2;
    sal_Int32                   mnY2;

public:
    TYPEINFO();

    SdXMLLineShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLLineShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:ellipse and draw:circle context

class SdXMLEllipseShapeContext : public SdXMLShapeContext
{
    sal_Int32                   mnCX;
    sal_Int32                   mnCY;
    sal_Int32                   mnRX;
    sal_Int32                   mnRY;

    USHORT                      meKind;
    sal_Int32                   mnStartAngle;
    sal_Int32                   mnEndAngle;
public:
    TYPEINFO();

    SdXMLEllipseShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLEllipseShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:polyline and draw:polygon context

class SdXMLPolygonShapeContext : public SdXMLShapeContext
{
    rtl::OUString               maPoints;
    rtl::OUString               maViewBox;
    sal_Bool                    mbClosed;

public:
    TYPEINFO();

    SdXMLPolygonShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes, sal_Bool bClosed);
    virtual ~SdXMLPolygonShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:path context

class SdXMLPathShapeContext : public SdXMLShapeContext
{
    rtl::OUString               maD;
    rtl::OUString               maViewBox;
    sal_Bool                    mbClosed;

public:
    TYPEINFO();

    SdXMLPathShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLPathShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:text-box context

class SdXMLTextBoxShapeContext : public SdXMLShapeContext
{
public:
    TYPEINFO();

    SdXMLTextBoxShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLTextBoxShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);
};

//////////////////////////////////////////////////////////////////////////////
// draw:control context

class SdXMLControlShapeContext : public SdXMLShapeContext
{
private:
    rtl::OUString maFormId;

public:
    TYPEINFO();

    SdXMLControlShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLControlShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:connector context

class SdXMLConnectorShapeContext : public SdXMLShapeContext
{
private:
    ::com::sun::star::awt::Point maStart;
    ::com::sun::star::awt::Point maEnd;

    USHORT      mnType;

    sal_Int32   mnStartShapeId;
    sal_Int32   mnStartGlueId;
    sal_Int32   mnEndShapeId;
    sal_Int32   mnEndGlueId;

    sal_Int32   mnDelta1;
    sal_Int32   mnDelta2;
    sal_Int32   mnDelta3;
public:
    TYPEINFO();

    SdXMLConnectorShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLConnectorShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:measure context

class SdXMLMeasureShapeContext : public SdXMLShapeContext
{
private:
    ::com::sun::star::awt::Point maStart;
    ::com::sun::star::awt::Point maEnd;

public:
    TYPEINFO();

    SdXMLMeasureShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLMeasureShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:page context

class SdXMLPageShapeContext : public SdXMLShapeContext
{
public:
    TYPEINFO();

    SdXMLPageShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLPageShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);
};

//////////////////////////////////////////////////////////////////////////////
// draw:caption context

class SdXMLCaptionShapeContext : public SdXMLShapeContext
{
public:
    TYPEINFO();

    SdXMLCaptionShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLCaptionShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);
};

//////////////////////////////////////////////////////////////////////////////
// office:image context

class SdXMLGraphicObjectShapeContext : public SdXMLShapeContext
{
    ::rtl::OUString maURL;
public:
    TYPEINFO();

    SdXMLGraphicObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLGraphicObjectShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// chart:chart context

class SdXMLChartShapeContext : public SdXMLShapeContext
{
    SvXMLImportContext*         mpChartContext;

public:
    TYPEINFO();

    SdXMLChartShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLChartShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual void Characters( const ::rtl::OUString& rChars );
    virtual SvXMLImportContext * CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );
};

//////////////////////////////////////////////////////////////////////////////
// draw:object and draw:object_ole context

class SdXMLObjectShapeContext : public SdXMLShapeContext
{
private:
    rtl::OUString maCLSID;
    rtl::OUString maClass;
    rtl::OUString maHref;
    sal_Bool mbPlaceHolder;
    sal_Bool mbUserTransformed;

public:
    TYPEINFO();

    SdXMLObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLObjectShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

#endif  //  _XIMPSHAPE_HXX
