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
    css::uno::Reference< css::drawing::XShapes >  mxShapes;
    css::uno::Reference< css::text::XTextCursor > mxCursor;
    css::uno::Reference< css::text::XTextCursor > mxOldCursor;
    css::uno::Reference< css::xml::sax::XAttributeList> mxAttrList;
    css::uno::Reference< css::container::XIdentifierContainer > mxGluePoints;
    css::uno::Reference< css::document::XActionLockable > mxLockable;

    OUString               maDrawStyleName;
    OUString               maTextStyleName;
    OUString               maPresentationClass;
    OUString               maShapeName;
    OUString               maThumbnailURL;

    /// whether to restore list context (#91964#)
    bool                        mbListContextPushed;

    sal_uInt16                  mnStyleFamily;
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
    css::awt::Size              maSize;
    sal_Int16                   mnRelWidth;
    sal_Int16                   mnRelHeight;
    css::awt::Point             maPosition;
    basegfx::B2DHomMatrix       maUsedTransformation;

    bool                        mbVisible;
    bool                        mbPrintable;
    bool mbHaveXmlId;
    bool mbTextBox; ///< If the text of this shape is handled by a Writer TextFrame.

    /** if bSupportsStyle is false, auto styles will be set but not a style */
    void SetStyle( bool bSupportsStyle = true );
    void SetLayer();
    void SetThumbnail();

    void AddShape(css::uno::Reference< css::drawing::XShape >& xShape);
    void AddShape(OUString const & serviceName);
    void SetTransformation();

    SvXMLImport& GetImport() { return SvXMLImportContext::GetImport(); }
    const SvXMLImport& GetImport() const { return SvXMLImportContext::GetImport(); }

    void addGluePoint( const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList );

    bool isPresentationShape() const;

public:

    SdXMLShapeContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLShapeContext();

    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;
    virtual void EndElement() override;

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue );

    // allow to copy evtl. useful data from another temporary import context, e.g. used to
    // support multiple images
    virtual void onDemandRescueUsefulDataFromTemporary( const SvXMLImportContext& rCandidate ) override;
};

// draw:rect context

class SdXMLRectShapeContext : public SdXMLShapeContext
{
    sal_Int32                   mnRadius;

public:

    SdXMLRectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLRectShapeContext();
    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
};

// draw:line context

class SdXMLLineShapeContext : public SdXMLShapeContext
{
    sal_Int32                   mnX1;
    sal_Int32                   mnY1;
    sal_Int32                   mnX2;
    sal_Int32                   mnY2;

public:

    SdXMLLineShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLLineShapeContext();
    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
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

    SdXMLEllipseShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLEllipseShapeContext();
    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
};

// draw:polyline and draw:polygon context

class SdXMLPolygonShapeContext : public SdXMLShapeContext
{
    OUString               maPoints;
    OUString               maViewBox;
    bool                    mbClosed;

public:

    SdXMLPolygonShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes, bool bClosed, bool bTemporaryShape);
    virtual ~SdXMLPolygonShapeContext();
    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
};

// draw:path context

class SdXMLPathShapeContext : public SdXMLShapeContext
{
    OUString               maD;
    OUString               maViewBox;

public:

    SdXMLPathShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLPathShapeContext();
    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
};

// draw:text-box context

class SdXMLTextBoxShapeContext : public SdXMLShapeContext
{
    sal_Int32                   mnRadius;
    OUString                    maChainNextName;

public:

    SdXMLTextBoxShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLTextBoxShapeContext();
    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
};

// draw:control context

class SdXMLControlShapeContext : public SdXMLShapeContext
{
private:
    OUString maFormId;

public:

    SdXMLControlShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLControlShapeContext();
    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
};

// draw:connector context

class SdXMLConnectorShapeContext : public SdXMLShapeContext
{
private:
    css::awt::Point maStart;
    css::awt::Point maEnd;

    sal_uInt16      mnType;

    OUString        maStartShapeId;
    sal_Int32       mnStartGlueId;
    OUString        maEndShapeId;
    sal_Int32       mnEndGlueId;

    sal_Int32   mnDelta1;
    sal_Int32   mnDelta2;
    sal_Int32   mnDelta3;

    css::uno::Any maPath;

public:

    SdXMLConnectorShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLConnectorShapeContext();
    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
};

// draw:measure context

class SdXMLMeasureShapeContext : public SdXMLShapeContext
{
private:
    css::awt::Point maStart;
    css::awt::Point maEnd;

public:

    SdXMLMeasureShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLMeasureShapeContext();
    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;
    virtual void EndElement() override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
};

// draw:page context

class SdXMLPageShapeContext : public SdXMLShapeContext
{
private:
    sal_Int32   mnPageNumber;
public:

    SdXMLPageShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLPageShapeContext();
    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
};

// draw:caption context

class SdXMLCaptionShapeContext : public SdXMLShapeContext
{
private:
    css::awt::Point maCaptionPoint;
    sal_Int32 mnRadius;

public:

    SdXMLCaptionShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLCaptionShapeContext();
    virtual void StartElement(const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
};

// office:image context

class SdXMLGraphicObjectShapeContext : public SdXMLShapeContext
{
private:
    OUString maURL;
    css::uno::Reference < css::io::XOutputStream > mxBase64Stream;

public:

    SdXMLGraphicObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLGraphicObjectShapeContext();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;
    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
};

// chart:chart context

class SdXMLChartShapeContext : public SdXMLShapeContext
{
    SvXMLImportContext*         mpChartContext;

public:

    SdXMLChartShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLChartShapeContext();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;
    virtual void Characters( const OUString& rChars ) override;
    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;
};

// draw:object and draw:object_ole context

class SdXMLObjectShapeContext : public SdXMLShapeContext
{
private:
    OUString maCLSID;
    OUString maHref;

    css::uno::Reference < css::io::XOutputStream > mxBase64Stream;

public:

    SdXMLObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLObjectShapeContext();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
};

// draw:applet

class SdXMLAppletShapeContext : public SdXMLShapeContext
{
private:
    OUString maAppletName;
    OUString maAppletCode;
    OUString maHref;
    bool mbIsScript;

    css::uno::Sequence< css::beans::PropertyValue > maParams;

public:

    SdXMLAppletShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLAppletShapeContext();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
};

// draw:plugin

class SdXMLPluginShapeContext : public SdXMLShapeContext
{
private:
    OUString   maMimeType;
    OUString   maHref;
    bool            mbMedia;

    css::uno::Sequence< css::beans::PropertyValue > maParams;

public:

    SdXMLPluginShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLPluginShapeContext();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;

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

    SdXMLFloatingFrameShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLFloatingFrameShapeContext();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
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
    virtual OUString getGraphicURLFromImportContext(const SvXMLImportContext& rContext) const override;
    virtual void removeGraphicFromImportContext(const SvXMLImportContext& rContext) const override;

public:

    SdXMLFrameShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLFrameShapeContext();

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;

    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
};

class SdXMLCustomShapeContext : public SdXMLShapeContext
{

protected:

    OUString maCustomShapeEngine;
    OUString maCustomShapeData;

    std::vector< css::beans::PropertyValue > maCustomShapeGeometry;

public:


    SdXMLCustomShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLCustomShapeContext();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void EndElement() override;

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;
};

// draw:table

class SdXMLTableShapeContext : public SdXMLShapeContext
{
public:

    SdXMLTableShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes );
    virtual ~SdXMLTableShapeContext();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue ) override;

private:
    SvXMLImportContextRef mxTableImportContext;
    OUString msTemplateStyleName;
    sal_Bool maTemplateStylesUsed[6];
};

#endif // INCLUDED_XMLOFF_SOURCE_DRAW_XIMPSHAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
