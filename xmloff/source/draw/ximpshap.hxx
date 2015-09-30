/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_XMLOFF_SOURCE_DRAW_XIMPSHAP_HXX
#define INCLUDED_XMLOFF_SOURCE_DRAW_XIMPSHAP_HXX

#include <config_features.h>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/container/XIdentifierContainer.hpp>
#include <xmloff/xmlictxt.hxx>
#include "sdxmlimp_impl.hxx"
#include <xmloff/nmspmap.hxx>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <tools/rtti.hxx>
#include "xexptran.hxx"
#include <vector>
#include <xmloff/shapeimport.hxx>
#include <xmloff/xmlmultiimagehelper.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

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

    OUString               maDrawStyleName;
    OUString               maTextStyleName;
    OUString               maPresentationClass;
    OUString               maShapeName;
    OUString               maThumbnailURL;

    /// whether to restore list context (#91964#)
    bool                        mbListContextPushed;

    sal_uInt16                  mnStyleFamily;
    sal_uInt16                  mnClass;
    bool                    mbIsPlaceholder;
    bool                        mbClearDefaultAttributes;
    bool                    mbIsUserTransformed;
    sal_Int32                   mnZOrder;
    OUString               maShapeId;
    OUString               maLayerName;

    // #i68101#
    OUString               maShapeTitle;
    OUString               maShapeDescription;

    SdXMLImExTransform2D        mnTransform;
    com::sun::star::awt::Size   maSize;
    sal_Int16                   mnRelWidth;
    sal_Int16                   mnRelHeight;
    com::sun::star::awt::Point  maPosition;
    basegfx::B2DHomMatrix       maUsedTransformation;

    bool                        mbVisible;
    bool                        mbPrintable;
    bool mbHaveXmlId;
    bool mbTextBox; ///< If the text of this shape is handled by a Writer TextFrame.

    /** if bSupportsStyle is false, auto styles will be set but not a style */
    void SetStyle( bool bSupportsStyle = true );
    void SetLayer();
    void SetThumbnail();

    void AddShape(com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape);
    void AddShape(OUString const & serviceName);
    void SetTransformation();

    SvXMLImport& GetImport() { return SvXMLImportContext::GetImport(); }
    const SvXMLImport& GetImport() const { return SvXMLImportContext::GetImport(); }

    void addGluePoint( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

    bool isPresentationShape() const;

public:
    TYPEINFO_OVERRIDE();

    SdXMLShapeContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLShapeContext();

    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList) SAL_OVERRIDE;
    virtual void EndElement() SAL_OVERRIDE;

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue );

    // allow to copy evtl. useful data from another temporary import context, e.g. used to
    // support multiple images
    virtual void onDemandRescueUsefulDataFromTemporary( const SvXMLImportContext& rCandidate ) SAL_OVERRIDE;
};

// draw:rect context

class SdXMLRectShapeContext : public SdXMLShapeContext
{
    sal_Int32                   mnRadius;

public:
    TYPEINFO_OVERRIDE();

    SdXMLRectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLRectShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// draw:line context

class SdXMLLineShapeContext : public SdXMLShapeContext
{
    sal_Int32                   mnX1;
    sal_Int32                   mnY1;
    sal_Int32                   mnX2;
    sal_Int32                   mnY2;

public:
    TYPEINFO_OVERRIDE();

    SdXMLLineShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLLineShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// draw:ellipse and draw:circle context

class SdXMLEllipseShapeContext : public SdXMLShapeContext
{
    sal_Int32                   mnCX;
    sal_Int32                   mnCY;
    sal_Int32                   mnRX;
    sal_Int32                   mnRY;

    sal_uInt16                      meKind;
    sal_Int32                   mnStartAngle;
    sal_Int32                   mnEndAngle;
public:
    TYPEINFO_OVERRIDE();

    SdXMLEllipseShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLEllipseShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// draw:polyline and draw:polygon context

class SdXMLPolygonShapeContext : public SdXMLShapeContext
{
    OUString               maPoints;
    OUString               maViewBox;
    bool                    mbClosed;

public:
    TYPEINFO_OVERRIDE();

    SdXMLPolygonShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes, bool bClosed, bool bTemporaryShape);
    virtual ~SdXMLPolygonShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// draw:path context

class SdXMLPathShapeContext : public SdXMLShapeContext
{
    OUString               maD;
    OUString               maViewBox;

public:
    TYPEINFO_OVERRIDE();

    SdXMLPathShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLPathShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// draw:text-box context

class SdXMLTextBoxShapeContext : public SdXMLShapeContext
{
    sal_Int32                   mnRadius;
    OUString                    maChainNextName;

public:
    TYPEINFO_OVERRIDE();

    SdXMLTextBoxShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLTextBoxShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// draw:control context

class SdXMLControlShapeContext : public SdXMLShapeContext
{
private:
    OUString maFormId;

public:
    TYPEINFO_OVERRIDE();

    SdXMLControlShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLControlShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// draw:connector context

class SdXMLConnectorShapeContext : public SdXMLShapeContext
{
private:
    ::com::sun::star::awt::Point maStart;
    ::com::sun::star::awt::Point maEnd;

    sal_uInt16      mnType;

    OUString   maStartShapeId;
    sal_Int32       mnStartGlueId;
    OUString   maEndShapeId;
    sal_Int32       mnEndGlueId;

    sal_Int32   mnDelta1;
    sal_Int32   mnDelta2;
    sal_Int32   mnDelta3;

    com::sun::star::uno::Any maPath;

public:
    TYPEINFO_OVERRIDE();

    SdXMLConnectorShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLConnectorShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// draw:measure context

class SdXMLMeasureShapeContext : public SdXMLShapeContext
{
private:
    ::com::sun::star::awt::Point maStart;
    ::com::sun::star::awt::Point maEnd;

public:
    TYPEINFO_OVERRIDE();

    SdXMLMeasureShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLMeasureShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList) SAL_OVERRIDE;
    virtual void EndElement() SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// draw:page context

class SdXMLPageShapeContext : public SdXMLShapeContext
{
private:
    sal_Int32   mnPageNumber;
public:
    TYPEINFO_OVERRIDE();

    SdXMLPageShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLPageShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// draw:caption context

class SdXMLCaptionShapeContext : public SdXMLShapeContext
{
private:
    com::sun::star::awt::Point maCaptionPoint;
    sal_Int32 mnRadius;

public:
    TYPEINFO_OVERRIDE();

    SdXMLCaptionShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLCaptionShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// office:image context

class SdXMLGraphicObjectShapeContext : public SdXMLShapeContext
{
private:
    OUString maURL;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > mxBase64Stream;

public:
    TYPEINFO_OVERRIDE();

    SdXMLGraphicObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLGraphicObjectShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;
    virtual void EndElement() SAL_OVERRIDE;
    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// chart:chart context

class SdXMLChartShapeContext : public SdXMLShapeContext
{
    SvXMLImportContext*         mpChartContext;

public:
    TYPEINFO_OVERRIDE();

    SdXMLChartShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLChartShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;
    virtual void EndElement() SAL_OVERRIDE;
    virtual void Characters( const OUString& rChars ) SAL_OVERRIDE;
    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;
};

// draw:object and draw:object_ole context

class SdXMLObjectShapeContext : public SdXMLShapeContext
{
private:
    OUString maCLSID;
    OUString maHref;

    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > mxBase64Stream;

public:
    TYPEINFO_OVERRIDE();

    SdXMLObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLObjectShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;
    virtual void EndElement() SAL_OVERRIDE;

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// draw:applet

class SdXMLAppletShapeContext : public SdXMLShapeContext
{
private:
    OUString maAppletName;
    OUString maAppletCode;
    OUString maHref;
    bool mbIsScript;

    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > maParams;

public:
    TYPEINFO_OVERRIDE();

    SdXMLAppletShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLAppletShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;
    virtual void EndElement() SAL_OVERRIDE;

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// draw:plugin

class SdXMLPluginShapeContext : public SdXMLShapeContext
{
private:
    OUString   maMimeType;
    OUString   maHref;
    bool            mbMedia;

    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > maParams;

public:
    TYPEINFO_OVERRIDE();

    SdXMLPluginShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLPluginShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;
    virtual void EndElement() SAL_OVERRIDE;

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;

#if !HAVE_FEATURE_GLTF
    const OUString& getMimeType() const { return maMimeType; }
#endif
};

// draw:floating-frame

class SdXMLFloatingFrameShapeContext : public SdXMLShapeContext
{
private:
    OUString maFrameName;
    OUString maHref;

public:
    TYPEINFO_OVERRIDE();

    SdXMLFloatingFrameShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLFloatingFrameShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;
    virtual void EndElement() SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// draw:-frame

class SdXMLFrameShapeContext : public SdXMLShapeContext, public MultiImageImportHelper
{
private:
    bool mbSupportsReplacement;
    SvXMLImportContextRef mxImplContext;
    SvXMLImportContextRef mxReplImplContext;

protected:
    /// helper to get the created xShape instance, needs to be overridden
    virtual OUString getGraphicURLFromImportContext(const SvXMLImportContext& rContext) const SAL_OVERRIDE;
    virtual void removeGraphicFromImportContext(const SvXMLImportContext& rContext) const SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();

    SdXMLFrameShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLFrameShapeContext();

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;
    virtual void EndElement() SAL_OVERRIDE;

    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

class SdXMLCustomShapeContext : public SdXMLShapeContext
{

protected:

    OUString maCustomShapeEngine;
    OUString maCustomShapeData;

    std::vector< com::sun::star::beans::PropertyValue > maCustomShapeGeometry;

public:

    TYPEINFO_OVERRIDE();

    SdXMLCustomShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLCustomShapeContext();

    virtual void StartElement( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;
    virtual void EndElement() SAL_OVERRIDE;

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;
};

// draw:table

class SdXMLTableShapeContext : public SdXMLShapeContext
{
public:
    TYPEINFO_OVERRIDE();

    SdXMLTableShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes );
    virtual ~SdXMLTableShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;
    virtual void EndElement() SAL_OVERRIDE;

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) SAL_OVERRIDE;

private:
    SvXMLImportContextRef mxTableImportContext;
    OUString msTemplateStyleName;
    sal_Bool maTemplateStylesUsed[6];
};

#endif // INCLUDED_XMLOFF_SOURCE_DRAW_XIMPSHAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
