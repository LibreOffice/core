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
#pragma once

#include <queue>
#include <memory>

#include "LoggedResources.hxx"
#include "WrapPolygonHandler.hxx"

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>

class SwXTextGraphicObject;
namespace com::sun::star {
    namespace uno
    {
        class XComponentContext;
    }
    namespace lang
    {
        class XMultiServiceFactory;
    }
    namespace text
    {
        class XTextContent;
    }
    namespace drawing
    {
        class XShape;
    }
    namespace beans
    {
        struct PropertyValue;
    }
}

namespace writerfilter::dmapper
{
class GraphicImport_Impl;
class DomainMapper;

enum GraphicImportType
{
    IMPORT_AS_DETECTED_INLINE,
    IMPORT_AS_DETECTED_ANCHOR
};

struct GraphicBorderLine
{
    sal_Int32   nLineWidth;
    bool        bHasShadow;

    GraphicBorderLine() :
        nLineWidth(0)
        ,bHasShadow(false)
        {}

    bool isEmpty() const
    {
        return nLineWidth == 0 && !bHasShadow;
    }
};

class GraphicImport : public LoggedProperties, public LoggedTable
                    ,public BinaryObj, public LoggedStream
{
    sal_Int32 m_nXSize;
    bool      m_bXSizeValid;
    sal_Int32 m_nYSize;
    bool      m_bYSizeValid;
    GraphicImportType & m_rGraphicImportType;
    DomainMapper&   m_rDomainMapper;

    sal_Int32 m_nLeftPosition;
    sal_Int32 m_nTopPosition;

    bool      m_bUseSimplePos;
    std::optional<sal_Int64> m_oZOrder;

    sal_Int16 m_nHoriOrient;
    sal_Int16 m_nHoriRelation;
    bool m_bPageToggle = false;
    sal_Int16 m_nVertOrient;
    sal_Int16 m_nVertRelation;
    css::text::WrapTextMode m_nWrap;
    bool      m_bLayoutInCell;
    bool      m_bCompatForcedLayoutInCell;
    bool m_bAllowOverlap = true;

    // Opaque means not in the background (but instead, the graphic will be over top of the text)
    // This flag holds where LO will ACTUALLY put the graphic
    bool      m_bOpaque;
    // BehindDoc means in the background. This flag says the graphic REQUESTED to be behind the text
    bool      m_bBehindDoc;

    bool      m_bContour;
    bool      m_bContourOutside;
    WrapPolygon::Pointer_t mpWrapPolygon;

    sal_Int32 m_nLeftMargin;
    sal_Int32 m_nLeftMarginOrig = 0;
    sal_Int32 m_nRightMargin;
    sal_Int32 m_nTopMargin;
    sal_Int32 m_nBottomMargin;

    bool m_bShadow;
    sal_Int32 m_nShadowXDistance;
    sal_Int32 m_nShadowYDistance;
    sal_Int32 m_nShadowColor;
    sal_Int32 m_nShadowTransparence;

    sal_Int32 m_nContrast;
    sal_Int32 m_nBrightness;

    static constexpr sal_Int32 nFillColor = 0xffffffff;

    css::drawing::ColorMode m_eColorMode;

    GraphicBorderLine   m_aBorders[4];

    bool            m_bIsGraphic;

    bool            m_bSizeProtected;
    bool            m_bPositionProtected;
    bool            m_bHidden;
    bool            m_bDecorative = false;

    sal_Int32       m_nShapeOptionType;

    OUString m_sName;
    OUString m_sAlternativeText;
    OUString m_title;
    OUString m_sHyperlinkURL;
    std::pair<OUString, OUString>& m_rPositionOffsets;
    std::pair<OUString, OUString>& m_rAligns;
    std::queue<OUString>& m_rPositivePercentages;
    OUString m_sAnchorId;
    comphelper::SequenceAsHashMap m_aInteropGrabBag;
    std::optional<sal_Int32> m_oEffectExtentLeft;
    std::optional<sal_Int32> m_oEffectExtentTop;
    std::optional<sal_Int32> m_oEffectExtentRight;
    std::optional<sal_Int32> m_oEffectExtentBottom;
    std::optional<css::text::GraphicCrop> m_oCrop;

    css::uno::Reference<css::uno::XComponentContext>     m_xComponentContext;
    rtl::Reference<SwXTextDocument> m_xTextDoc;
    rtl::Reference<SwXTextGraphicObject> m_xGraphicObject;
    css::uno::Reference<css::drawing::XShape> m_xShape;

    void ProcessShapeOptions(Value const & val);

    rtl::Reference<SwXTextGraphicObject>
            createGraphicObject(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic,
                                css::uno::Reference<css::beans::XPropertySet> const & xShapeProps);

    void putPropertyToFrameGrabBag( const OUString& sPropertyName, const css::uno::Any& aPropertyValue );

public:
    explicit GraphicImport( css::uno::Reference<css::uno::XComponentContext>  xComponentContext,
                            rtl::Reference<SwXTextDocument> xTextDoc,
                            DomainMapper& rDomainMapper,
                            GraphicImportType & rGraphicImportType,
                            std::pair<OUString, OUString>& rPositionOffsets,
                            std::pair<OUString, OUString>& rAligns,
                            std::queue<OUString>& rPositivePercentages);
    virtual ~GraphicImport() override;

    // BinaryObj
    virtual void data(const sal_uInt8* buffer, size_t len) override;

    css::uno::Reference<css::text::XTextContent> GetGraphicObject();
    const css::uno::Reference<css::drawing::XShape>& GetXShapeObject() const { return m_xShape;}
    bool IsGraphic() const;
    sal_Int32 GetLeftMarginOrig() const;

    css::awt::Point GetGraphicObjectPosition() const;

    bool GetLayoutInCell() const;

 private:
    // Properties
    virtual void lcl_attribute(Id Name, const Value & val) override;
    virtual void lcl_sprm(Sprm & sprm) override;

    // Table
    virtual void lcl_entry(const writerfilter::Reference<Properties>::Pointer_t& ref) override;

    // Stream
    virtual void lcl_startSectionGroup() override;
    virtual void lcl_endSectionGroup() override;
    virtual void lcl_startParagraphGroup() override;
    virtual void lcl_endParagraphGroup() override;
    virtual void lcl_startCharacterGroup() override;
    virtual void lcl_endCharacterGroup() override;
    virtual void lcl_text(const sal_uInt8 * data, size_t len) override;
    virtual void lcl_utext(const sal_Unicode * data, size_t len) override;
    virtual void lcl_props(const writerfilter::Reference<Properties>::Pointer_t& ref) override;
    virtual void lcl_table(Id name,
                           const writerfilter::Reference<Table>::Pointer_t& ref) override;
    virtual void lcl_substream(Id name, const writerfilter::Reference<Stream>::Pointer_t& ref) override;
    virtual void lcl_startShape(css::uno::Reference<css::drawing::XShape> const& xShape) override;
    virtual void lcl_startTextBoxContent() override {};
    virtual void lcl_endTextBoxContent() override {};
    virtual void lcl_endShape() override;

    void handleWrapTextValue(sal_uInt32 nVal);
    void lcl_expandRectangleByEffectExtent(css::awt::Point& rLeftTop, css::awt::Size& rSize);
    void lcl_correctWord2007EffectExtent(const sal_Int32 nMSOAngle);
    void lcl_adjustMarginsAndOrientation();

    void setXSize(sal_Int32 _nXSize);
    sal_uInt32 getXSize() const;
    bool isXSizeValid() const;
    void setYSize(sal_Int32 _nYSize);
    sal_uInt32 getYSize() const;
    bool isYSizeValid() const;
    void applyMargins(const css::uno::Reference< css::beans::XPropertySet >& xGraphicObjectProperties) const;
    void applyPosition(const css::uno::Reference< css::beans::XPropertySet >& xGraphicObjectProperties) const;
    void applyRelativePosition(const css::uno::Reference< css::beans::XPropertySet >& xGraphicObjectProperties, bool bRelativeOnly = false) const;
    void applyZOrder(css::uno::Reference<css::beans::XPropertySet> const & xGraphicObjectProperties) const;
    void applyName(css::uno::Reference<css::beans::XPropertySet> const & xGraphicObjectProperties) const;
    void applyHyperlink(css::uno::Reference<css::beans::XPropertySet> const & xShapeProps, bool bIsShape);
    /// Getter for m_aInteropGrabBag, but also merges in the values from other members if they are set.
    comphelper::SequenceAsHashMap const & getInteropGrabBag();
};

typedef tools::SvRef<GraphicImport> GraphicImportPtr;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
