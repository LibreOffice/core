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
#include <stack>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
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
#include <vcl/graph.hxx>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <vcl/mapmod.hxx>
#include <svx/EnhancedCustomShape2d.hxx>
#include <basegfx/utils/bgradient.hxx>

class Graphic;
class SdrObjCustomShape;
enum class SvxDateFormat;
enum class SvxTimeFormat;

namespace com::sun::star {
namespace awt {
    struct FontDescriptor;
    struct Gradient2;
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
    class XTextFrame;
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
}

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

class OOX_DLLPUBLIC URLTransformer
{
public:
    virtual ~URLTransformer();

    virtual OUString getTransformedString(const OUString& rURL) const;

    virtual bool isExternalURL(const OUString& rURL) const;
};

// Our rotation is counter-clockwise and is in 100ths of a degree.
// drawingML rotation is clockwise and is in 60000ths of a degree.
inline sal_Int32 ExportRotateClockwisify(Degree100 input)
{
    return ((21600000 - input.get() * 600) % 21600000);
}

/// Interface to be implemented by the parent exporter that knows how to handle shape text.
class OOX_DLLPUBLIC DMLTextExport
{
public:
    virtual void WriteOutliner(const OutlinerParaObject& rParaObj) = 0;
    /// Write the contents of the textbox that is associated to this shape.
    virtual void WriteTextBox(css::uno::Reference<css::drawing::XShape> xShape) = 0;
    ///  Get textbox which belongs to the shape.
    virtual css::uno::Reference<css::text::XTextFrame> GetUnoTextFrame(
        css::uno::Reference<css::drawing::XShape> xShape) = 0;
protected:
    DMLTextExport() {}
    virtual ~DMLTextExport() {}
};

constexpr std::u16string_view getComponentDir(DocumentType eDocumentType)
{
    switch (eDocumentType)
    {
        case DOCUMENT_DOCX: return u"word";
        case DOCUMENT_PPTX: return u"ppt";
        case DOCUMENT_XLSX: return u"xl";
    }

    return u"";
}

constexpr std::u16string_view getRelationCompPrefix(DocumentType eDocumentType)
{
    switch (eDocumentType)
    {
        case DOCUMENT_DOCX: return u"";
        case DOCUMENT_PPTX:
        case DOCUMENT_XLSX: return u"../";
    }

    return u"";
}

class OOX_DLLPUBLIC GraphicExportCache
{
private:
    std::stack<sal_Int32> mnImageCounter;
    std::stack<std::unordered_map<BitmapChecksum, OUString>> maExportGraphics;
    std::stack<sal_Int32> mnWdpImageCounter;
    std::stack<std::map<OUString, OUString>> maWdpCache;

    GraphicExportCache() = default;
public:
    static GraphicExportCache& get();

    void push()
    {
        mnImageCounter.push(1);
        maExportGraphics.emplace();
        mnWdpImageCounter.push(1);
        maWdpCache.emplace();
    }

    void pop()
    {
        mnImageCounter.pop();
        maExportGraphics.pop();
        mnWdpImageCounter.pop();
        maWdpCache.pop();
    }

    bool hasExportGraphics()
    {
        return !maExportGraphics.empty();
    }

    void addExportGraphics(BitmapChecksum aChecksum, OUString const& sPath)
    {
        maExportGraphics.top()[aChecksum] = sPath;
    }

    OUString findExportGraphics(BitmapChecksum aChecksum)
    {
        OUString sPath;
        if (!hasExportGraphics())
            return sPath;

        auto aIterator = maExportGraphics.top().find(aChecksum);
        if (aIterator != maExportGraphics.top().end())
            sPath = aIterator->second;
        return sPath;
    }

    sal_Int32 nextImageCount()
    {
        sal_Int32 nCount = mnImageCounter.top();
        mnImageCounter.top()++;
        return nCount;
    }

    bool hasWdpCache()
    {
        return !maWdpCache.empty();
    }

    OUString findWdpID(OUString const& rFileId)
    {
        OUString aPath;
        if (!hasWdpCache())
            return aPath;
        auto aCachedItem = maWdpCache.top().find(rFileId);
        if (aCachedItem != maWdpCache.top().end())
            aPath = aCachedItem->second;
        return aPath;
    }

    void addToWdpCache(OUString const& rFileId, OUString const& rId)
    {
        if (hasWdpCache())
            maWdpCache.top()[rFileId] = rId;
    }

    sal_Int32 nextWdpImageCount()
    {
        sal_Int32 nCount = mnWdpImageCounter.top();
        mnWdpImageCounter.top()++;
        return nCount;
    }
};

class OOX_DLLPUBLIC GraphicExport
{
private:
    sax_fastparser::FSHelperPtr mpFS;
    oox::core::XmlFilterBase* mpFilterBase;
    DocumentType meDocumentType;

    OUString writeNewEntryToStorage(const Graphic& rGraphic, bool bRelPathToMedia);
    OUString writeNewSvgEntryToStorage(const Graphic& rGraphic, bool bRelPathToMedia);

public:
    enum class TypeHint
    {
        Detect,
        SVG
    };

    GraphicExport(sax_fastparser::FSHelperPtr pFS, ::oox::core::XmlFilterBase* pFilterBase, DocumentType eDocumentType)
        : mpFS(std::move(pFS))
        , mpFilterBase(pFilterBase)
        , meDocumentType(eDocumentType)
    {}

    OUString writeToStorage(Graphic const& rGraphic, bool bRelPathToMedia = false, TypeHint eHint = TypeHint::Detect);

    void writeBlip(Graphic const& rGraphic, std::vector<model::BlipEffect> const& rEffects);
    void writeSvgExtension(OUString const& rSvgRelId);
};

class DrawingML
{

private:
    OOX_DLLPUBLIC static sal_Int32 mnDrawingMLCount;
    OOX_DLLPUBLIC static sal_Int32 mnVmlCount;

    /// To specify where write eg. the images to (like 'ppt', or 'word' - according to the OPC).
    DocumentType meDocumentType;
    /// Parent exporter, used for text callback.
    DMLTextExport* mpTextExport;


protected:
    css::uno::Any                             mAny;
    ::sax_fastparser::FSHelperPtr             mpFS;
    ::oox::core::XmlFilterBase*               mpFB;
    /// If set, this is the parent of the currently handled shape.
    css::uno::Reference<css::drawing::XShape> m_xParent;
    bool                                      mbIsBackgroundDark;
    OOX_DLLPUBLIC static sal_Int32 mnChartCount;

    /// True when exporting presentation placeholder shape.
    bool mbPlaceholder;

    bool GetProperty( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet, const OUString& aName );
    bool GetPropertyAndState( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet,
                  const css::uno::Reference< css::beans::XPropertyState >& rXPropState,
                  const OUString& aName, css::beans::PropertyState& eState );
    OUString GetFieldValue( const css::uno::Reference< css::text::XTextRange >& rRun, bool& bIsURLField );
    /** Gets OOXML datetime field type from LO Date format

        @param eDate LO Date format
    */
    static OUString GetDatetimeTypeFromDate(SvxDateFormat eDate);
    /** Gets OOXML datetime field type from LO Time format

        @param eTime LO Time format
    */
    static OUString GetDatetimeTypeFromTime(SvxTimeFormat eTime);
    /** Gets OOXML datetime field type from combination of LO Time and Date formats

        @param eDate LO Date format
        @param eTime LO Time format
    */
    OOX_DLLPUBLIC static OUString GetDatetimeTypeFromDateTime(SvxDateFormat eDate, SvxTimeFormat eTime);

    /// Output the media (including copying a video from vnd.sun.star.Package: to the output if necessary).
    void WriteMediaNonVisualProperties(const css::uno::Reference<css::drawing::XShape>& xShape);

    void WriteStyleProperties( sal_Int32 nTokenId, const css::uno::Sequence< css::beans::PropertyValue >& aProperties );

    OUString GetComponentDir() const;
    OUString GetRelationCompPrefix() const;

    static bool EqualGradients( const css::awt::Gradient2& rGradient1, const css::awt::Gradient2& rGradient2 );
    bool IsFontworkShape(const css::uno::Reference< css::beans::XPropertySet >& rXShapePropSet);

    void WriteGlowEffect(const css::uno::Reference<css::beans::XPropertySet>& rXPropSet);
    void WriteSoftEdgeEffect(const css::uno::Reference<css::beans::XPropertySet>& rXPropSet);
    void WriteCustomGeometryPoint(const css::drawing::EnhancedCustomShapeParameterPair& rParamPair,
                                  const EnhancedCustomShape2d& rCustomShape2d,
                                  const bool bReplaceGeoWidth, const bool bReplaceGeoHeight);
    bool WriteCustomGeometrySegment(
        const sal_Int16 eCommand, const sal_Int32 nCount,
        const css::uno::Sequence<css::drawing::EnhancedCustomShapeParameterPair>& rPairs,
        sal_Int32& rnPairIndex, double& rfCurrentX, double& rfCurrentY, bool& rbCurrentValid,
        const EnhancedCustomShape2d& rCustomShape2d,
        const bool bReplaceGeoWidth, const bool bReplaceGeoHeight);

public:
    DrawingML( ::sax_fastparser::FSHelperPtr pFS, ::oox::core::XmlFilterBase* pFB, DocumentType eDocumentType = DOCUMENT_PPTX, DMLTextExport* pTextExport = nullptr )
        : meDocumentType( eDocumentType ), mpTextExport(pTextExport), mpFS(std::move( pFS )), mpFB( pFB ), mbIsBackgroundDark( false ), mbPlaceholder(false) {}
    void SetFS(const ::sax_fastparser::FSHelperPtr& pFS) { mpFS = pFS; }
    const ::sax_fastparser::FSHelperPtr& GetFS() const { return mpFS; }
    ::oox::core::XmlFilterBase* GetFB() { return mpFB; }
    DocumentType GetDocumentType() const { return meDocumentType; }
    /// The application-specific text exporter callback, if there is one.
    DMLTextExport* GetTextExport() { return mpTextExport; }

    void SetBackgroundDark(bool bIsDark) { mbIsBackgroundDark = bIsDark; }
    /// If bRelPathToMedia is true add "../" to image folder path while adding the image relationship
    OOX_DLLPUBLIC OUString writeGraphicToStorage(const Graphic &rGraphic , bool bRelPathToMedia = false, GraphicExport::TypeHint eHint = GraphicExport::TypeHint::Detect);

    void WriteColor( ::Color nColor, sal_Int32 nAlpha = MAX_PERCENT );
    void WriteColor( const OUString& sColorSchemeName, const css::uno::Sequence< css::beans::PropertyValue >& aTransformations, sal_Int32 nAlpha = MAX_PERCENT );
    void WriteColor( const ::Color nColor, const css::uno::Sequence< css::beans::PropertyValue >& aTransformations, sal_Int32 nAlpha = MAX_PERCENT );
    void WriteColorTransformations( const css::uno::Sequence< css::beans::PropertyValue >& aTransformations, sal_Int32 nAlpha = MAX_PERCENT );
    void WriteGradientStop(double fOffset, const basegfx::BColor& rColor, const basegfx::BColor& rAlpha);
    void WriteLineArrow( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet, bool bLineStart );
    void WriteConnectorConnections( sal_Int32 nStartGlueId, sal_Int32 nEndGlueId, sal_Int32 nStartID, sal_Int32 nEndID );

    bool WriteCharColor(const css::uno::Reference<css::beans::XPropertySet>& xPropertySet);
    bool WriteSchemeColor(OUString const& rPropertyName, const css::uno::Reference<css::beans::XPropertySet>& xPropertySet);

    void WriteSolidFill( ::Color nColor, sal_Int32 nAlpha = MAX_PERCENT );
    void WriteSolidFill( const OUString& sSchemeName, const css::uno::Sequence< css::beans::PropertyValue >& aTransformations, sal_Int32 nAlpha = MAX_PERCENT );
    void WriteSolidFill( const ::Color nColor, const css::uno::Sequence< css::beans::PropertyValue >& aTransformations, sal_Int32 nAlpha = MAX_PERCENT );
    void WriteSolidFill( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );
    OOX_DLLPUBLIC void WriteGradientFill( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );

    /* New API for WriteGradientFill:
       If a BGradient is given, it will be used. Else, the 'Fix' entry will be used for
       Color or Transparency. That way, less Pseudo(Color|Transparency)Gradients have to be
       created at caller side.
       NOTE: Giving no Gradient at all (both nullptr) is an error.
    */
    void WriteGradientFill(
        const basegfx::BGradient* pColorGradient, sal_Int32 nFixColor,
        const basegfx::BGradient* pTransparenceGradient, double fFixTransparence = 0.0);

    void WriteGrabBagGradientFill( const css::uno::Sequence< css::beans::PropertyValue >& aGradientStops, const basegfx::BGradient& rGradient);

    void WriteBlipOrNormalFill(const css::uno::Reference<css::beans::XPropertySet>& rXPropSet,
                               const OUString& rURLPropName, const css::awt::Size& rSize = {});
    OOX_DLLPUBLIC void WriteBlipFill(const css::uno::Reference<css::beans::XPropertySet>& rXPropSet,
                       const OUString& sURLPropName, const css::awt::Size& rSize = {});
    void WriteBlipFill(const css::uno::Reference<css::beans::XPropertySet>& rXPropSet,
                       const css::awt::Size& rSize, const OUString& sURLPropName,
                       sal_Int32 nXmlNamespace);

    void WriteXGraphicBlipFill(css::uno::Reference<css::beans::XPropertySet> const & rXPropSet,
                               css::uno::Reference<css::graphic::XGraphic> const & rxGraphic,
                               sal_Int32 nXmlNamespace, bool bWriteMode,
                               bool bRelPathToMedia = false, css::awt::Size const& rSize = {});

    void WritePattFill( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );
    void WritePattFill(const css::uno::Reference<css::beans::XPropertySet>& rXPropSet,
            const css::drawing::Hatch& rHatch);

    void WriteGraphicCropProperties(css::uno::Reference<css::beans::XPropertySet> const & rxPropertySet,
                                    Size const & rOriginalSize, MapMode const & rMapMode);

    void WriteSrcRectXGraphic(css::uno::Reference<css::beans::XPropertySet> const & rxPropertySet,
                              css::uno::Reference<css::graphic::XGraphic> const & rxGraphic);

    OOX_DLLPUBLIC void WriteOutline( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet,
                              css::uno::Reference< css::frame::XModel> const & xModel = nullptr );

    void WriteXGraphicStretch(css::uno::Reference<css::beans::XPropertySet> const & rXPropSet,
                              css::uno::Reference<css::graphic::XGraphic> const & rxGraphic);

    void WriteXGraphicTile(css::uno::Reference<css::beans::XPropertySet> const& rXPropSet,
                           css::uno::Reference<css::graphic::XGraphic> const& rxGraphic,
                           css::awt::Size const& rSize);

    void WriteXGraphicCustomPosition(css::uno::Reference<css::beans::XPropertySet> const& rXPropSet,
                                     css::uno::Reference<css::graphic::XGraphic> const& rxGraphic,
                                     css::awt::Size const& rSize);

    void WriteLinespacing(const css::style::LineSpacing& rLineSpacing, float fFirstCharHeight);

    void WriteXGraphicBlip(css::uno::Reference<css::beans::XPropertySet> const & rXPropSet,
                               css::uno::Reference<css::graphic::XGraphic> const & rxGraphic,
                               bool bRelPathToMedia);

    void WriteImageBrightnessContrastTransparence(css::uno::Reference<css::beans::XPropertySet> const & rXPropSet);

    void WriteXGraphicBlipMode(css::uno::Reference<css::beans::XPropertySet> const& rXPropSet,
                               css::uno::Reference<css::graphic::XGraphic> const& rxGraphic,
                               css::awt::Size const& rSize);

    OOX_DLLPUBLIC void WriteShapeTransformation(const css::uno::Reference< css::drawing::XShape >& rXShape,
                  sal_Int32 nXmlNamespace, bool bFlipH = false, bool bFlipV = false, bool bSuppressRotation = false, bool bSuppressFlipping = false, bool bFlippedBeforeRotation = false);
    void WriteTransformation(const css::uno::Reference< css::drawing::XShape >& xShape, const tools::Rectangle& rRectangle,
                  sal_Int32 nXmlNamespace, bool bFlipH = false, bool bFlipV = false, sal_Int32 nRotation = 0, bool bIsGroupShape = false);

    void WriteText( const css::uno::Reference< css::uno::XInterface >& rXIface, bool bBodyPr, bool bText = true, sal_Int32 nXmlNamespace = 0, bool bWritePropertiesAsLstStyles = false);

    /** Populates the lstStyle with the shape's text run and paragraph properties */
    void WriteLstStyles(const css::uno::Reference<css::text::XTextContent>& rParagraph,
                       bool& rbOverridingCharHeight, sal_Int32& rnCharHeight,
                       const css::uno::Reference<css::beans::XPropertySet>& rXShapePropSet);
    void WriteParagraph( const css::uno::Reference< css::text::XTextContent >& rParagraph,
                         bool& rbOverridingCharHeight, sal_Int32& rnCharHeight, const css::uno::Reference< css::beans::XPropertySet >& rXShapePropSet);
    /** Writes paragraph properties

        @returns true if any paragraph properties were written
    */
    bool WriteParagraphProperties(const css::uno::Reference< css::text::XTextContent >& rParagraph, float fFirstCharHeight, sal_Int32 nElement);
    void WriteParagraphNumbering(const css::uno::Reference< css::beans::XPropertySet >& rXPropSet, float fFirstCharHeight,
                                  sal_Int16 nLevel );
    void WriteParagraphTabStops(const css::uno::Reference<css::beans::XPropertySet>& rXPropSet);
    void WriteRun( const css::uno::Reference< css::text::XTextRange >& rRun,
                   bool& rbOverridingCharHeight, sal_Int32& rnCharHeight,
                   const css::uno::Reference< css::beans::XPropertySet >& rXShapePropSet);
    void WriteRunProperties( const css::uno::Reference< css::beans::XPropertySet >& rRun, bool bIsField, sal_Int32 nElement, bool bCheckDirect,
                             bool& rbOverridingCharHeight, sal_Int32& rnCharHeight,
                             sal_Int16 nScriptType = css::i18n::ScriptType::LATIN,
                             const css::uno::Reference< css::beans::XPropertySet >& rXShapePropSet = {});

    void WritePresetShape( const OString& pShape , std::vector< std::pair<sal_Int32,sal_Int32>> & rAvList );
    OOX_DLLPUBLIC void WritePresetShape( const OString& pShape );
    void WritePresetShape( const OString& pShape, MSO_SPT eShapeType, bool bPredefinedHandlesUsed, const css::beans::PropertyValue& rProp );
    bool WriteCustomGeometry(
        const css::uno::Reference<css::drawing::XShape>& rXShape,
        const SdrObjCustomShape& rSdrObjCustomShape);
    void WriteEmptyCustomGeometry();
    void WritePolyPolygon(const css::uno::Reference<css::drawing::XShape>& rXShape,
                          const bool bClosed);
    OOX_DLLPUBLIC void WriteFill(const css::uno::Reference<css::beans::XPropertySet>& xPropSet,
                   const css::awt::Size& rSize = {});
    void WriteShapeStyle( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );
    OOX_DLLPUBLIC void WriteShapeEffects( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );
    void WriteShapeEffect( std::u16string_view sName, const css::uno::Sequence< css::beans::PropertyValue >& aEffectProps );
    /** Populates scene3d tag
        @param rXPropSet Prop set
        @param bIsText True if the 3D effects are for a text body, false if it is for a shape
     */
    OOX_DLLPUBLIC void Write3DEffects(const css::uno::Reference<css::beans::XPropertySet>& rXPropSet, bool bIsText);
    void WriteArtisticEffect( const css::uno::Reference< css::beans::XPropertySet >& rXPropSet );
    OString WriteWdpPicture( const OUString& rFileId, const css::uno::Sequence< sal_Int8 >& rPictureData );
    OOX_DLLPUBLIC void WriteDiagram(const css::uno::Reference<css::drawing::XShape>& rXShape, int nDiagramId);
    void writeDiagramRels(const css::uno::Sequence<css::uno::Sequence<css::uno::Any>>& xRelSeq,
                          const css::uno::Reference<css::io::XOutputStream>& xOutStream,
                          std::u16string_view sGrabBagProperyName, int nDiagramId);
    static void WriteFromTo(const css::uno::Reference<css::drawing::XShape>& rXShape, const css::awt::Size& aPageSize,
                            const sax_fastparser::FSHelperPtr& pDrawing);

    static bool IsGroupShape( const css::uno::Reference< css::drawing::XShape >& rXShape );
    sal_Int32 getBulletMarginIndentation (const css::uno::Reference< css::beans::XPropertySet >& rXPropSet,sal_Int16 nLevel, std::u16string_view propName);

    OOX_DLLPUBLIC static void ResetMlCounters();

    static sal_Int32 getNewDrawingUniqueId() { return ++mnDrawingMLCount; }
    static sal_Int32 getNewVMLUniqueId() { return ++mnVmlCount; }
    static sal_Int32 getNewChartUniqueId() { return ++mnChartCount; }

    // A Helper to decide the script type for given text in order to call WriteRunProperties.
    static sal_Int16 GetScriptType(const OUString& rStr);

    static sal_Unicode SubstituteBullet( sal_Unicode cBulletId, css::awt::FontDescriptor& rFontDesc );

    static ::Color ColorWithIntensity( sal_uInt32 nColor, sal_uInt32 nIntensity );

    static const char* GetAlignment( css::style::ParagraphAdjust nAlignment );

    sax_fastparser::FSHelperPtr     CreateOutputStream (
                                        const OUString& sFullStream,
                                        std::u16string_view sRelativeStream,
                                        const css::uno::Reference< css::io::XOutputStream >& xParentRelation,
                                        const OUString& sContentType,
                                        const OUString& sRelationshipType,
                                        OUString* pRelationshipId );

    OOX_DLLPUBLIC std::shared_ptr<GraphicExport> createGraphicExport();
};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
