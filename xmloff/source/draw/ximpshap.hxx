/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef _XIMPSHAPE_HXX
#define _XIMPSHAPE_HXX

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
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>

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

    sal_uInt16                  mnStyleFamily;
    sal_uInt16                  mnClass;
    sal_Int32                   mnZOrder;
    rtl::OUString               maShapeId;
    rtl::OUString               maLayerName;

    // #i68101#
    rtl::OUString               maShapeTitle;
    rtl::OUString               maShapeDescription;

    SdXMLImExTransform2D        mnTransform;
    basegfx::B2DVector          maObjectSize;
    basegfx::B2DPoint           maObjectPosition;
    //com::sun::star::awt::Size   maSize;
    //com::sun::star::awt::Point  maPosition;
    basegfx::B2DHomMatrix       maUsedTransformation;

    /// bitfield
    bool                        mbVisible : 1;
    bool                        mbPrintable : 1;

    /// whether to restore list context (#91964#)
    bool                        mbListContextPushed : 1;

    bool                        mbIsPlaceholder : 1;
    bool                        mbClearDefaultAttributes : 1;
    bool                        mbIsUserTransformed : 1;

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
    SdXMLShapeContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLShapeContext();

    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );

    /// access to ShapeId for evtl. late adding
    const rtl::OUString& getShapeId() const { return maShapeId; }

    // allow to copy evtl. useful data from another temporary import context, e.g. used to
    // support multiple images
    virtual void onDemandRescueUsefulDataFromTemporary( const SvXMLImportContext& rCandidate );
};

//////////////////////////////////////////////////////////////////////////////
// draw:rect context

class SdXMLRectShapeContext : public SdXMLShapeContext
{
    sal_Int32                   mnRadius;

public:
    SdXMLRectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLRectShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:line context

class SdXMLLineShapeContext : public SdXMLShapeContext
{
private:
    double              mfX1;
    double              mfY1;
    double              mfX2;
    double              mfY2;

public:
    SdXMLLineShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLLineShapeContext();
    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:ellipse and draw:circle context

class SdXMLEllipseShapeContext : public SdXMLShapeContext
{
    double                      mfCX;
    double                      mfCY;
    double                      mfRX;
    double                      mfRY;

    sal_uInt16                  meKind;
    sal_Int32                   mnStartAngle;
    sal_Int32                   mnEndAngle;
public:
    SdXMLEllipseShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
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
    bool                        mbClosed;

public:
    SdXMLPolygonShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bClosed,
        bool bTemporaryShape);
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
    bool                        mbClosed;

public:
    SdXMLPathShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
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
    SdXMLTextBoxShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
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
    SdXMLControlShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
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

    sal_uInt16      mnType;

    rtl::OUString   maStartShapeId;
    sal_Int32       mnStartGlueId;
    rtl::OUString   maEndShapeId;
    sal_Int32       mnEndGlueId;

    sal_Int32   mnDelta1;
    sal_Int32   mnDelta2;
    sal_Int32   mnDelta3;

    com::sun::star::uno::Any maPath;

public:
    SdXMLConnectorShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
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
    SdXMLMeasureShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
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
    SdXMLPageShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
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
    SdXMLCaptionShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
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

    /// bitfield
    bool                mbLateAddToIdentifierMapper : 1;

public:
    SdXMLGraphicObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLGraphicObjectShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );

    /// support for LateAddToIdentifierMapper
    bool getLateAddToIdentifierMapper() const { return mbLateAddToIdentifierMapper; }
    void setLateAddToIdentifierMapper(bool bNew) { mbLateAddToIdentifierMapper = bNew; }
};

//////////////////////////////////////////////////////////////////////////////
// chart:chart context

class SdXMLChartShapeContext : public SdXMLShapeContext
{
    SvXMLImportContext*         mpChartContext;

public:
    SdXMLChartShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLChartShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual void Characters( const ::rtl::OUString& rChars );
    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
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
    SdXMLObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLObjectShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    // #100592#
    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
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
    SdXMLAppletShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLAppletShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
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
    SdXMLPluginShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLPluginShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
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
    SdXMLFloatingFrameShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLFloatingFrameShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:-frame

class SdXMLFrameShapeContext : public SdXMLShapeContext, public multiImageImportHelper
{
private:
    sal_Bool mbSupportsReplacement;
    SvXMLImportContextRef mxImplContext;
    SvXMLImportContextRef mxReplImplContext;

protected:
    /// helper to get the created xShape instance, needs to be overloaded
    virtual rtl::OUString getGraphicURLFromImportContext(const SvXMLImportContext& rContext) const;
    virtual void removeGraphicFromImportContext(const SvXMLImportContext& rContext) const;

public:
    SdXMLFrameShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLFrameShapeContext();

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
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

    SdXMLCustomShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        bool bTemporaryShape);
    virtual ~SdXMLCustomShapeContext();

    virtual void StartElement( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
};

//////////////////////////////////////////////////////////////////////////////
// draw:table

class SdXMLTableShapeContext : public SdXMLShapeContext
{
public:
    SdXMLTableShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes );
    virtual ~SdXMLTableShapeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

    // this is called from the parent group for each unparsed attribute in the attribute list
    virtual void processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );

private:
    SvXMLImportContextRef mxTableImportContext;
    rtl::OUString msTemplateStyleName;
    sal_Bool maTemplateStylesUsed[6];
};

#endif  //  _XIMPSHAPE_HXX
