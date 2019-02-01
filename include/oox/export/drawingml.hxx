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

#ifndef INCLUDED_OOX_EXPORT_DRAWINGML_HXX
#define INCLUDED_OOX_EXPORT_DRAWINGML_HXX

#include <map>
#include <vector>

#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <oox/dllapi.h>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/token/tokens.hxx>
#include <oox/export/utils.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <sax/fshelper.hxx>
#include <svx/msdffdef.hxx>
#include <vcl/checksum.hxx>
#include <tools/gen.hxx>
#include <vcl/mapmod.hxx>

class Graphic;
class SdrObjCustomShape;

namespace com { namespace sun { namespace star {
namespace awt {
    struct FontDescriptor;
    struct Gradient;
}
namespace beans {
    struct PropertyValue;
    class XPropertySet;
    class XPropertyState;
}
namespace drawing {
    class XShape;
    struct EnhancedCustomShapeParameterPair;
    struct EnhancedCustomShapeParameter;
}
namespace graphic {
    class XGraphic;
}
namespace style {
    struct LineSpacing;
}
namespace text {
    class XTextContent;
    class XTextRange;
}
namespace io {
    class XOutputStream;
}
namespace uno {
    class XInterface;
}
namespace frame {
    class XModel;
}
}}}

struct EscherConnectorListEntry;
class OutlinerParaObject;
namespace tools { class Rectangle; }

namespace tools {
    class PolyPolygon;
}

namespace oox {
namespace core {
    class XmlFilterBase;
}

namespace drawingml {

// Our rotation is counter-clockwise and is in 100ths of a degree.
// drawingML rotation is clockwise and is in 60000ths of a degree.
template <typename T> T ExportRotateClockwisify(T input)
{
    return ((21600000 - input * 600) % 21600000);
}

/// Interface to be implemented by the parent exporter that knows how to handle shape text.
class OOX_DLLPUBLIC DMLTextExport
{
public:
    virtual void WriteOutliner(const OutlinerParaObject& rParaObj) = 0;
    /// Write the contents of the textbox that is associated to this shape.
    virtual void WriteTextBox(css::uno::Reference<css::drawing::XShape> xShape) = 0;
    /// Look up the RelId of a graphic based on its checksum.
    virtual OUString FindRelId(BitmapChecksum nChecksum) = 0;
    /// Store the RelId of a graphic based on its checksum.
    virtual void CacheRelId(BitmapChecksum nChecksum, const OUString& rRelId) = 0;
protected:
    DMLTextExport() {}
    virtual ~DMLTextExport() {}
};

class OOX_DLLPUBLIC DrawingML
{

private:
    static int mnImageCounter;
    static int mnWdpImageCounter;
    static std::map<OUString, OUString> maWdpCache;

    /// To specify where write eg. the images to (like 'ppt', or 'word' - according to the OPC).
    DocumentType const meDocumentType;
    /// Parent exporter, used for text callback.
    DMLTextExport* mpTextExport;

protected:
    css::uno::Any                             mAny;
    ::sax_fastparser::FSHelperPtr             mpFS;
    ::oox::core::XmlFilterBase*               mpFB;
    /// If set, this is the parent of the currently handled shape.
    css::uno::Reference<css::drawing::XShape> m_xParent;
    bool                                      mbIsBackgroundDark;

    bool GetProperty( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet, const OUString& aName );
    bool GetPropertyAndState( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet,
                  const css::uno::Reference< css::beans::XPropertyState >& rXPropState,
                  const OUString& aName, css::beans::PropertyState& eState );
    OUString GetFieldValue( const css::uno::Reference< css::text::XTextRange >& rRun, bool& bIsURLField );

    /// Output the media (including copying a video from vnd.sun.star.Package: to the output if necessary).
    void WriteMediaNonVisualProperties(const css::uno::Reference<css::drawing::XShape>& xShape);

    void WriteStyleProperties( sal_Int32 nTokenId, const css::uno::Sequence< css::beans::PropertyValue >& aProperties );

    const char* GetComponentDir();
    const char* GetRelationCompPrefix();

    static bool EqualGradients( css::awt::Gradient aGradient1, css::awt::Gradient aGradient2 );

public:
    DrawingML( ::sax_fastparser::FSHelperPtr pFS, ::oox::core::XmlFilterBase* pFB, DocumentType eDocumentType = DOCUMENT_PPTX, DMLTextExport* pTextExport = nullptr )
        : meDocumentType( eDocumentType ), mpTextExport(pTextExport), mpFS( pFS ), mpFB( pFB ), mbIsBackgroundDark( false ) {}
    void SetFS( ::sax_fastparser::FSHelperPtr pFS ) { mpFS = pFS; }
    const ::sax_fastparser::FSHelperPtr& GetFS() { return mpFS; }
    ::oox::core::XmlFilterBase* GetFB() { return mpFB; }
    DocumentType GetDocumentType() { return meDocumentType; }
    /// The application-specific text exporter callback, if there is one.
    DMLTextExport* GetTextExport() { return mpTextExport; }

    void SetBackgroundDark(bool bIsDark) { mbIsBackgroundDark = bIsDark; }
    /// If bRelPathToMedia is true add "../" to image folder path while adding the image relationship
    OUString WriteImage( const Graphic &rGraphic , bool bRelPathToMedia = false);

    void WriteColor( ::Color nColor, sal_Int32 nAlpha = MAX_PERCENT );
    void WriteColor( const OUString& sColorSchemeName, const css::uno::Sequence< css::beans::PropertyValue >& aTransformations );
    void WriteColorTransformations( const css::uno::Sequence< css::beans::PropertyValue >& aTransformations );
    void WriteGradientStop( sal_uInt16 nStop, ::Color nColor );
    void WriteLineArrow( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet, bool bLineStart );
    void WriteConnectorConnections( EscherConnectorListEntry& rConnectorEntry, sal_Int32 nStartID, sal_Int32 nEndID );

    void WriteSolidFill( ::Color nColor, sal_Int32 nAlpha = MAX_PERCENT );
    void WriteSolidFill( const OUString& sSchemeName, const css::uno::Sequence< css::beans::PropertyValue >& aTransformations );
    void WriteSolidFill( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );
    void WriteGradientFill( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );
    void WriteGradientFill( css::awt::Gradient rGradient );
    void WriteGrabBagGradientFill( const css::uno::Sequence< css::beans::PropertyValue >& aGradientStops, css::awt::Gradient rGradient);

    void WriteBlipOrNormalFill( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet,
            const OUString& rURLPropName );
    void WriteBlipFill( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet,
            const OUString& sURLPropName );
    void WriteBlipFill( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet,
                         const OUString& sURLPropName, sal_Int32 nXmlNamespace );

    void WriteXGraphicBlipFill(css::uno::Reference<css::beans::XPropertySet> const & rXPropSet,
                               css::uno::Reference<css::graphic::XGraphic> const & rxGraphic,
                               sal_Int32 nXmlNamespace, bool bWriteMode, bool bRelPathToMedia = false);

    void WritePattFill( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );
    void WritePattFill(const css::uno::Reference<css::beans::XPropertySet>& rXPropSet,
            const css::drawing::Hatch& rHatch);

    void WriteGraphicCropProperties(css::uno::Reference<css::beans::XPropertySet> const & rxPropertySet,
                                    Size const & rOriginalSize, MapMode const & rMapMode);

    void WriteSrcRectXGraphic(css::uno::Reference<css::beans::XPropertySet> const & rxPropertySet,
                              css::uno::Reference<css::graphic::XGraphic> const & rxGraphic);

    void WriteOutline( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet,
                              css::uno::Reference< css::frame::XModel> const & xModel = nullptr );

    void WriteXGraphicStretch(css::uno::Reference<css::beans::XPropertySet> const & rXPropSet,
                              css::uno::Reference<css::graphic::XGraphic> const & rxGraphic);

    void WriteLinespacing( const css::style::LineSpacing& rLineSpacing );

    OUString WriteXGraphicBlip(css::uno::Reference<css::beans::XPropertySet> const & rXPropSet,
                               css::uno::Reference<css::graphic::XGraphic> const & rxGraphic,
                               bool bRelPathToMedia);

    void WriteImageBrightnessContrastTransparence(css::uno::Reference<css::beans::XPropertySet> const & rXPropSet);

    void WriteXGraphicBlipMode(css::uno::Reference<css::beans::XPropertySet> const & rXPropSet,
                               css::uno::Reference<css::graphic::XGraphic> const & rxGraphic);

    void WriteShapeTransformation(const css::uno::Reference< css::drawing::XShape >& rXShape,
                  sal_Int32 nXmlNamespace, bool bFlipH = false, bool bFlipV = false, bool bSuppressRotation = false, bool bSuppressFlipping = false, bool bFlippedBeforeRotation = false);
    void WriteTransformation(const tools::Rectangle& rRectangle,
                  sal_Int32 nXmlNamespace, bool bFlipH = false, bool bFlipV = false, sal_Int32 nRotation = 0, bool bIsGroupShape = false);

    void WriteText( const css::uno::Reference< css::uno::XInterface >& rXIface, const OUString& presetWarp, bool bBodyPr, bool bText = true, sal_Int32 nXmlNamespace = 0);
    void WriteParagraph( const css::uno::Reference< css::text::XTextContent >& rParagraph,
                         bool& rbOverridingCharHeight, sal_Int32& rnCharHeight );
    void WriteParagraphProperties(const css::uno::Reference< css::text::XTextContent >& rParagraph, float fFirstCharHeight);
    void WriteParagraphNumbering(const css::uno::Reference< css::beans::XPropertySet >& rXPropSet, float fFirstCharHeight,
                                  sal_Int16 nLevel );
    void WriteRun( const css::uno::Reference< css::text::XTextRange >& rRun,
                   bool& rbOverridingCharHeight, sal_Int32& rnCharHeight );
    void WriteRunProperties( const css::uno::Reference< css::beans::XPropertySet >& rRun, bool bIsField, sal_Int32 nElement, bool bCheckDirect,
                             bool& rbOverridingCharHeight, sal_Int32& rnCharHeight );

    void WritePresetShape( const char* pShape , std::vector< std::pair<sal_Int32,sal_Int32>> & rAvList );
    void WritePresetShape( const char* pShape );
    void WritePresetShape( const char* pShape, MSO_SPT eShapeType, bool bPredefinedHandlesUsed, const css::beans::PropertyValue& rProp );
    bool WriteCustomGeometry(
        const css::uno::Reference<css::drawing::XShape>& rXShape,
        const SdrObjCustomShape& rSdrObjCustomShape);
    void WriteCustomGeometryPoint(
        const css::drawing::EnhancedCustomShapeParameterPair& rParamPair,
        const SdrObjCustomShape& rSdrObjCustomShape);
    static sal_Int32 GetCustomGeometryPointValue(
        const css::drawing::EnhancedCustomShapeParameter& rParam,
        const SdrObjCustomShape& rSdrObjCustomShape);
    void WritePolyPolygon( const tools::PolyPolygon& rPolyPolygon );
    void WriteFill( const css::uno::Reference< css::beans::XPropertySet >& xPropSet );
    void WriteShapeStyle( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );
    void WriteShapeEffects( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );
    void WriteShapeEffect( const OUString& sName, const css::uno::Sequence< css::beans::PropertyValue >& aEffectProps );
    void WriteShape3DEffects( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );
    void WriteArtisticEffect( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );
    OString WriteWdpPicture( const OUString& rFileId, const css::uno::Sequence< sal_Int8 >& rPictureData );
    static bool IsGroupShape( const css::uno::Reference< css::drawing::XShape >& rXShape );
    sal_Int32 getBulletMarginIndentation (const css::uno::Reference< css::beans::XPropertySet >& rXPropSet,sal_Int16 nLevel, const OUString& propName);

    static void ResetCounters();

    static sal_Unicode SubstituteBullet( sal_Unicode cBulletId, css::awt::FontDescriptor& rFontDesc );

    static ::Color ColorWithIntensity( sal_uInt32 nColor, sal_uInt32 nIntensity );

    static const char* GetAlignment( css::style::ParagraphAdjust nAlignment );

    sax_fastparser::FSHelperPtr     CreateOutputStream (
                                        const OUString& sFullStream,
                                        const OUString& sRelativeStream,
                                        const css::uno::Reference< css::io::XOutputStream >& xParentRelation,
                                        const char* sContentType,
                                        const char* sRelationshipType,
                                        OUString* pRelationshipId );

};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
