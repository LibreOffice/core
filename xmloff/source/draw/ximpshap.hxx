/*************************************************************************
 *
 *  $RCSfile: ximpshap.hxx,v $
 *
 *  $Revision: 1.38 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 08:50:48 $
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

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XACTIONLOCKABLE_HPP_
#include <com/sun/star/document/XActionLockable.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XIDENTIFIERCONTAINER_HPP_
#include <com/sun/star/container/XIdentifierContainer.hpp>
#endif

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

#include <vector>

#ifndef _XMLOFF_SHAPEIMPORT_HXX_
#include "shapeimport.hxx"
#endif

//////////////////////////////////////////////////////////////////////////////
// common shape context

class SdXMLShapeContext : public SvXMLShapeContext
{
protected:
    // the shape group this object should be created inside
    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >  mxShapes;
    com::sun::star::uno::Reference< com::sun::star::text::XTextCursor > mxCursor;
    com::sun::star::uno::Reference< com::sun::star::text::XTextCursor > mxOldCursor;
    com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList> mxAttrList;
    com::sun::star::uno::Reference< com::sun::star::container::XIdentifierContainer > mxGluePoints;
    com::sun::star::uno::Reference< com::sun::star::document::XActionLockable > mxLockable;

    rtl::OUString               maDrawStyleName;
    rtl::OUString               maTextStyleName;
    rtl::OUString               maPresentationClass;
    rtl::OUString               maShapeName;
    rtl::OUString               maThumbnailURL;

    /// old list item and block (#91964#)
    SvXMLImportContextRef       mxOldListBlock;
    SvXMLImportContextRef       mxOldListItem;

    sal_uInt16                  mnStyleFamily;
    sal_uInt16                  mnClass;
    sal_Bool                    mbIsPlaceholder;
    sal_Bool                    mbIsUserTransformed;
    sal_Int32                   mnZOrder;
    sal_Int32                   mnShapeId;
    rtl::OUString               maLayerName;

    SdXMLImExTransform2D        mnTransform;
    com::sun::star::awt::Size   maSize;
    com::sun::star::awt::Point  maPosition;

    /** if bSupportsStyle is false, auto styles will be set but not a style */
    void SetStyle( bool bSupportsStyle = true );
    void SetLayer();
    void SetThumbnail();

    void AddShape(com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape);
    void AddShape(const char* pServiceName );
    void SetTransformation();

    SvXMLImport& GetImport() { return SvXMLImportContext::GetImport(); }
    const SvXMLImport& GetImport() const { return SvXMLImportContext::GetImport(); }

    void addGluePoint( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

    sal_Bool isPresentationShape() const;

public:
    TYPEINFO();

    SdXMLShapeContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLShapeContext();

    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();

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
    sal_Int32                   mnRadius;

public:
    TYPEINFO();

    SdXMLTextBoxShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLTextBoxShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
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
private:
    sal_Int32   mnPageNumber;
public:
    TYPEINFO();

    SdXMLPageShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLPageShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:caption context

class SdXMLCaptionShapeContext : public SdXMLShapeContext
{
private:
    com::sun::star::awt::Point maCaptionPoint;
    sal_Int32 mnRadius;

public:
    TYPEINFO();

    SdXMLCaptionShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLCaptionShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// office:image context

class SdXMLGraphicObjectShapeContext : public SdXMLShapeContext
{
private:
    ::rtl::OUString maURL;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > mxBase64Stream;

public:
    TYPEINFO();

    SdXMLGraphicObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLGraphicObjectShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual SvXMLImportContext * CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

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
    rtl::OUString maHref;

    // #100592#
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > mxBase64Stream;

public:
    TYPEINFO();

    SdXMLObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLObjectShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    // #100592#
    virtual SvXMLImportContext * CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:applet

class SdXMLAppletShapeContext : public SdXMLShapeContext
{
private:
    rtl::OUString maAppletName;
    rtl::OUString maAppletCode;
    rtl::OUString maHref;
    sal_Bool mbIsScript;

    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > maParams;

public:
    TYPEINFO();

    SdXMLAppletShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLAppletShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    virtual SvXMLImportContext * CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:plugin

class SdXMLPluginShapeContext : public SdXMLShapeContext
{
private:
    rtl::OUString   maMimeType;
    rtl::OUString   maHref;
    bool            mbMedia;

    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > maParams;

public:
    TYPEINFO();

    SdXMLPluginShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLPluginShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    virtual SvXMLImportContext * CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:floating-frame

class SdXMLFloatingFrameShapeContext : public SdXMLShapeContext
{
private:
    rtl::OUString maFrameName;
    rtl::OUString maHref;

public:
    TYPEINFO();

    SdXMLFloatingFrameShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLFloatingFrameShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:-frame

class SdXMLFrameShapeContext : public SdXMLShapeContext
{
private:
    SvXMLImportContextRef mxImplContext;

public:
    TYPEINFO();

    SdXMLFrameShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLFrameShapeContext();

    virtual SvXMLImportContext * CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

class SdXMLCustomShapeContext : public SdXMLShapeContext
{

protected :

    rtl::OUString maCustomShapeEngine;
    rtl::OUString maCustomShapeData;

    std::vector< com::sun::star::beans::PropertyValue > maCustomShapeGeometry;

public:

    TYPEINFO();

    SdXMLCustomShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXMLCustomShapeContext();

    virtual void StartElement( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();

    virtual SvXMLImportContext * CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

#endif  //  _XIMPSHAPE_HXX
