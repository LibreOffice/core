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

#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <tools/ref.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/drawing/XShape.hpp>
#include "PropertyIds.hxx"
#include <memory>
#include <optional>
#include <map>
#include <utility>
#include <vector>
#include <set>
#include <deque>

class SwXTextSection;
class SwXPageStyle;
namespace com::sun::star {
    namespace beans {
        struct PropertyValue;
    }
    namespace container {
        class XNameContainer;
    }
    namespace lang {
        class XMultiServiceFactory;
    }
    namespace text {
        class XTextRange;
        class XTextColumns;
        class XFootnote;
    }
    namespace table {
        struct BorderLine2;
        struct ShadowFormat;
    }
}

namespace writerfilter::dmapper {

class  DomainMapper_Impl;
struct AnchoredObjectInfo;

enum BorderPosition
{
    BORDER_LEFT,
    BORDER_RIGHT,
    BORDER_TOP,
    BORDER_BOTTOM
};

enum GrabBagType
{
    NO_GRAB_BAG,
    ROW_GRAB_BAG,
    CELL_GRAB_BAG,
    PARA_GRAB_BAG,
    CHAR_GRAB_BAG
};

struct RedlineParams : public SvRefBase
{
    OUString  m_sAuthor;
    OUString  m_sDate;
    sal_Int32 m_nToken;

    // This can hold properties of runs that had formatted 'track changes' properties
    css::uno::Sequence< css::beans::PropertyValue > m_aRevertProperties;
};

typedef tools::SvRef< RedlineParams > RedlineParamsPtr;

class PropValue
{
private:
    css::uno::Any m_aValue;
    GrabBagType   m_GrabBagType;
    bool          m_bIsDocDefault;

public:
    PropValue( css::uno::Any aValue, GrabBagType i_GrabBagType, bool bDocDefault )
        : m_aValue(std::move( aValue ))
        , m_GrabBagType( i_GrabBagType )
        , m_bIsDocDefault( bDocDefault )
    {
    }

    PropValue( css::uno::Any aValue, GrabBagType i_GrabBagType )
        : m_aValue(std::move( aValue ))
        , m_GrabBagType( i_GrabBagType )
        , m_bIsDocDefault( false )
    {
    }

    PropValue()
        : m_aValue()
        , m_GrabBagType( NO_GRAB_BAG )
        , m_bIsDocDefault( false )
    {
    }

    const css::uno::Any& getValue() const { return m_aValue; }

    GrabBagType getGrabBagType() const { return m_GrabBagType; }

    bool getIsDocDefault() const { return m_bIsDocDefault; }
};

class PropertyMap;
typedef tools::SvRef< PropertyMap > PropertyMapPtr;

class PropertyMap : public SvRefBase
{
private:
    // Cache the property values for the GetPropertyValues() call(s).
    std::vector< css::beans::PropertyValue >    m_aValues;

    // marks context as footnote context - ::text( ) events contain either the footnote character or can be ignored
    // depending on sprmCSymbol
    css::uno::Reference< css::text::XFootnote > m_xFootnote;
    OUString m_sFootnoteCharStyleName;
    std::map< PropertyIds, PropValue >          m_vMap;
    std::vector< RedlineParamsPtr >             m_aRedlines;

public:
    typedef std::pair< PropertyIds, css::uno::Any > Property;

    PropertyMap() {}

    // Sequence: Grab Bags: The CHAR_GRAB_BAG has Name "CharInteropGrabBag" and the PARA_GRAB_BAG has Name "ParaInteropGrabBag"
    // the contained properties are their Value.
    css::uno::Sequence< css::beans::PropertyValue > GetPropertyValues( bool bCharGrabBag = true );

    std::vector< PropertyIds > GetPropertyIds();

    // Add property, optionally overwriting existing attributes
    void Insert( PropertyIds eId, const css::uno::Any& rAny, bool bOverwrite = true, GrabBagType i_GrabBagType = NO_GRAB_BAG, bool bDocDefault = false );

    // Remove a named property from *this, does nothing if the property id has not been set
    void Erase( PropertyIds eId);

    // Imports properties from pMap (bOverwrite==false means m_bIsDocDefault=true setting)
    void InsertProps( const PropertyMapPtr& rMap, const bool bOverwrite = true );

    // Returns a copy of the property if it exists, .first is its PropertyIds and .second is its Value (type css::uno::Any)
    std::optional< Property > getProperty( PropertyIds eId ) const;

    // Has the property named been set (via Insert)?
    bool isSet( PropertyIds eId ) const;
    bool isDocDefault( PropertyIds eId ) const;

    const css::uno::Reference< css::text::XFootnote >& GetFootnote() const { return m_xFootnote; }
    const OUString& GetFootnoteStyle() const { return m_sFootnoteCharStyleName; }

    void SetFootnote(const css::uno::Reference< css::text::XFootnote >& xFootnote, const OUString& sStyleName)
    {
        m_xFootnote = xFootnote;
        m_sFootnoteCharStyleName = sStyleName;
    }

    virtual void insertTableProperties( const PropertyMap*, const bool bOverwrite = true );

    const std::vector< RedlineParamsPtr >& Redlines() const { return m_aRedlines; }

    std::vector< RedlineParamsPtr >& Redlines() { return m_aRedlines; }

    void printProperties();

#ifdef DBG_UTIL
    void dumpXml() const;
#endif

    static css::table::ShadowFormat getShadowFromBorder( const css::table::BorderLine2& rBorder );

protected:
    void Invalidate()
    {
        if ( m_aValues.size() )
            m_aValues.clear();
    }
};

/** Type of an page, which has an effect which headers and/or footers will be used. */
enum class PageType
{
    FIRST,
    LEFT,
    RIGHT
};

/** Which page part we are referring to.
 * This is mainly introduced to avoid having cryptic bools as input parameter.*/
enum class PagePartType
{
    Header,
    Footer
};

/** A page break type to force what the next page type will be.
 * For example - the next page is forced to be even (left) or odd (right). */
enum class PageBreakType
{
    Even,
    Odd,
    Both
};

class SectionPropertyMap : public PropertyMap
{
public:
    enum class BorderApply
    {
        ToAllInSection = 0,
        ToFirstPageInSection = 1,
        ToAllButFirstInSection = 2
    };
    enum class BorderOffsetFrom
    {
        Text = 0,
        Edge = 1,
    };
private:
#ifdef DBG_UTIL
    sal_Int32                                       m_nDebugSectionNumber;
#endif

    // 'temporarily' the section page settings are imported as page styles
    // empty strings mark page settings as not yet imported

    bool                                            m_bIsFirstSection;
    css::uno::Reference< css::text::XTextRange >    m_xStartingRange;

    OUString m_sPageStyleName;
    rtl::Reference<SwXPageStyle>                    m_aPageStyle;

    std::optional< css::table::BorderLine2 >      m_oBorderLines[4];
    sal_Int32                                       m_nBorderDistances[4];
    BorderApply                                     m_eBorderApply;
    BorderOffsetFrom                                m_eBorderOffsetFrom;
    bool                                            m_bBorderShadows[4];

    bool                                            m_bTitlePage;
    sal_Int16                                       m_nColumnCount;
    sal_Int32                                       m_nColumnDistance;
    css::uno::Reference< css::beans::XPropertySet > m_xColumnContainer;
    std::vector< sal_Int32 >                        m_aColWidth;
    std::vector< sal_Int32 >                        m_aColDistance;

    bool                                            m_bSeparatorLineIsOn;
    bool                                            m_bEvenlySpaced;

    sal_Int32                                       m_nPageNumber;
    // Page number type is a value from css::style::NumberingType.
    sal_Int16                                       m_nPageNumberType;
    sal_Int32                                       m_nBreakType;

    sal_Int32                                       m_nLeftMargin;
    sal_Int32                                       m_nRightMargin;
    sal_Int32                                       m_nGutterMargin;
    sal_Int32                                       m_nTopMargin;
    sal_Int32                                       m_nBottomMargin;
    sal_Int32                                       m_nHeaderTop;
    sal_Int32                                       m_nHeaderBottom;

    sal_Int32                                       m_nGridType;
    sal_Int32                                       m_nGridLinePitch;
    sal_Int32                                       m_nDxtCharSpace;
    bool                                            m_bGridSnapToChars;

    // line numbering
    sal_Int32                                       m_nLnnMod;
    sal_uInt32                                      m_nLnc;
    sal_Int32                                       m_ndxaLnn;
    sal_Int32                                       m_nLnnMin;

    sal_Int32                                       m_nPaperSourceFirst;
    sal_Int32                                       m_nPaperSourceOther;

    bool                                            m_bDynamicHeightTop;
    bool                                            m_bDynamicHeightBottom;

    std::vector<css::uno::Reference<css::drawing::XShape>>    m_xRelativeWidthShapes;

    // The "Link To Previous" flag indicates whether the header/footer
    // content should be taken from the previous section
    bool m_bDefaultHeaderLinkToPrevious = true;
    bool m_bEvenPageHeaderLinkToPrevious = true;
    bool m_bFirstPageHeaderLinkToPrevious = true;
    bool m_bDefaultFooterLinkToPrevious = true;
    bool m_bEvenPageFooterLinkToPrevious = true;
    bool m_bFirstPageFooterLinkToPrevious = true;

    void ApplyProperties_( const css::uno::Reference< css::beans::XPropertySet >& xStyle );

    void DontBalanceTextColumns();

    /// Apply section-specific properties: only valid to use after PageStyle has been determined by InheritOrFinalizePageStyles
    void ApplySectionProperties( const css::uno::Reference< css::beans::XPropertySet >& xSection, DomainMapper_Impl& rDM_Impl );

    /// Check if document is protected. If so, ensure a section exists, and apply its protected value.
    void ApplyProtectionProperties( rtl::Reference<SwXTextSection>& xSection, DomainMapper_Impl& rDM_Impl );

    css::uno::Reference< css::text::XTextColumns > ApplyColumnProperties(const css::uno::Reference<css::beans::XPropertySet>& xPageStyle,
                                                                         DomainMapper_Impl& rDM_Impl);

    void CopyLastHeaderFooter(DomainMapper_Impl& rDM_Impl);

    void CreateEvenOddPageStyleCopy(DomainMapper_Impl& rDM_Impl, PageBreakType eBreakType);

    void PrepareHeaderFooterProperties();

    bool HasHeader() const;
    bool HasFooter() const;

    static void SetBorderDistance( const css::uno::Reference< css::beans::XPropertySet >& xStyle,
                                   PropertyIds eMarginId,
                                   PropertyIds eDistId,
                                   sal_Int32 nDistance,
                                   BorderOffsetFrom eOffsetFrom,
                                   sal_uInt32 nLineWidth,
                                   DomainMapper_Impl& rDM_Impl );

    /// Increases paragraph spacing according to Word 2013+ needs if necessary.
    void HandleIncreasedAnchoredObjectSpacing(DomainMapper_Impl& rDM_Impl);

public:
    explicit SectionPropertyMap( bool bIsFirstSection );

    bool IsFirstSection() const { return m_bIsFirstSection; }

    void SetStart( const css::uno::Reference< css::text::XTextRange >& xRange ) { m_xStartingRange = xRange; }

    const css::uno::Reference< css::text::XTextRange >& GetStartingRange() const { return m_xStartingRange; }

    rtl::Reference<SwXPageStyle> GetPageStyle(DomainMapper_Impl& rDM_Impl);

    const OUString& GetPageStyleName()
    {
        return m_sPageStyleName;
    }

    bool getFirstPageHeader() { return m_bFirstPageHeaderLinkToPrevious; }
    bool getFirstPageFooter() { return m_bFirstPageFooterLinkToPrevious; }

    // @throws css::beans::UnknownPropertyException
    // @throws css::beans::PropertyVetoException
    // @throws css::lang::IllegalArgumentException
    // @throws css::lang::WrappedTargetException
    // @throws css::uno::RuntimeException
    void InheritOrFinalizePageStyles( DomainMapper_Impl& rDM_Impl );

    void SetBorder( BorderPosition ePos, sal_Int32 nLineDistance, const css::table::BorderLine2& rBorderLine, bool bShadow );
    void SetBorderApply( BorderApply nSet ) { m_eBorderApply = nSet; }
    void SetBorderOffsetFrom( BorderOffsetFrom nSet ) { m_eBorderOffsetFrom = nSet; }

    void      SetColumnCount( sal_Int16 nCount ) { m_nColumnCount = nCount; }
    sal_Int16 ColumnCount() const                { return m_nColumnCount; }

    void SetColumnDistance( sal_Int32 nDist )   { m_nColumnDistance = nDist; }
    void AppendColumnWidth( sal_Int32 nWidth )  { m_aColWidth.push_back( nWidth ); }
    void AppendColumnSpacing( sal_Int32 nDist ) { m_aColDistance.push_back( nDist ); }

    void SetTitlePage( bool bSet )           { m_bTitlePage = bSet; }
    void SetSeparatorLine( bool bSet )       { m_bSeparatorLineIsOn = bSet; }
    void SetEvenlySpaced( bool bSet )        { m_bEvenlySpaced = bSet; }
    void SetPageNumber( sal_Int32 nSet )     { m_nPageNumber = nSet; }
    void SetPageNumberType( sal_Int32 nSet ) { m_nPageNumberType = nSet; }
    void SetBreakType( sal_Int32 nSet )      { m_nBreakType = nSet; }
    // GetBreakType returns -1 if the breakType has not yet been identified for the section
    sal_Int32 GetBreakType() const           { return m_nBreakType; }

    void SetLeftMargin( sal_Int32 nSet )   { m_nLeftMargin = nSet; }
    sal_Int32 GetLeftMargin() const        { return m_nLeftMargin; }
    void SetRightMargin( sal_Int32 nSet )  { m_nRightMargin = nSet; }
    sal_Int32 GetRightMargin() const       { return m_nRightMargin; }
    void SetTopMargin(sal_Int32 nSet)      { m_bDynamicHeightTop = nSet >= 0; m_nTopMargin = std::abs(nSet); }
    void SetBottomMargin( sal_Int32 nSet ) { m_bDynamicHeightBottom = nSet >= 0; m_nBottomMargin = std::abs(nSet); }
    void SetHeaderTop( sal_Int32 nSet )    { m_nHeaderTop = nSet; }
    void SetHeaderBottom( sal_Int32 nSet ) { m_nHeaderBottom = nSet; }
    void SetGutterMargin( sal_Int32 nGutterMargin ) { m_nGutterMargin = nGutterMargin; }
    sal_Int32 GetPageWidth() const;

    void SetGridType( sal_Int32 nSet )      { m_nGridType = nSet; }
    void SetGridLinePitch( sal_Int32 nSet ) { m_nGridLinePitch = nSet; }
    void SetGridSnapToChars( bool bSet )    { m_bGridSnapToChars = bSet; }
    void SetDxtCharSpace( sal_Int32 nSet )  { m_nDxtCharSpace = nSet; }

    void SetLnnMod( sal_Int32 nValue ) { m_nLnnMod = nValue; }
    void SetLnc( sal_Int32 nValue )    { m_nLnc = nValue; }
    void SetdxaLnn( sal_Int32 nValue ) { m_ndxaLnn = nValue; }
    void SetLnnMin( sal_Int32 nValue ) { m_nLnnMin = nValue; }

    void SetPaperSource(sal_Int32 first, sal_Int32 other) { m_nPaperSourceFirst = first; m_nPaperSourceOther = other;}

    void addRelativeWidthShape( css::uno::Reference<css::drawing::XShape> xShape ) { m_xRelativeWidthShapes.push_back( xShape ); }

    // determine which style gets the borders
    void ApplyBorderToPageStyles( DomainMapper_Impl &rDM_Impl,
                                  BorderApply eBorderApply, BorderOffsetFrom eOffsetFrom );
    void ApplyPaperSource(DomainMapper_Impl& rDM_Impl);
    void CloseSectionGroup( DomainMapper_Impl& rDM_Impl );
    // Handling of margins, header and footer for any kind of sections breaks.
    void HandleMarginsHeaderFooter(DomainMapper_Impl& rDM_Impl);
    void clearHeaderFooterLinkToPrevious(PagePartType ePartType, PageType eType);
    void setHeaderFooterProperties(DomainMapper_Impl& rDM_Impl);

    // Which headers/footer types are used (have been imported) by the current section.
    bool m_bFirstHeader = false;
    bool m_bFirstFooter = false;
    bool m_bLeftHeader = false;
    bool m_bLeftFooter = false;
    bool m_bRightHeader = false;
    bool m_bRightFooter = false;
    bool m_bHadFirstHeader = false;
    bool m_bHadLeftHeader = false;
    bool m_bHadRightHeader = false;

    static void removeXTextContent(css::uno::Reference<css::text::XText> const& rxText);
};

void BeforeConvertToTextFrame(std::deque<css::uno::Any>& rFramedRedlines, std::vector<sal_Int32>& redPos, std::vector<sal_Int32>& redLen, std::vector<OUString>& redCell, std::vector<OUString>& redTable);

void AfterConvertToTextFrame(DomainMapper_Impl& rDM_Impl, std::deque<css::uno::Any>& aFramedRedlines, std::vector<sal_Int32>& redPos, std::vector<sal_Int32>& redLen, std::vector<OUString>& redCell, std::vector<OUString>& redTable);

class ParagraphProperties : public SvRefBase
{
private:
    bool                                         m_bFrameMode;
    sal_Int32                                    m_nDropCap;       // drop, margin ST_DropCap
    sal_Int32                                    m_nLines;         // number of lines of the drop cap
    sal_Int32                                    m_w;              // width
    sal_Int32                                    m_h;              // height
    css::text::WrapTextMode                      m_nWrap;          // from ST_Wrap around, auto, none, notBeside, through, tight
    sal_Int32                                    m_hAnchor;        // page, from ST_HAnchor  margin, page, text
    sal_Int32                                    m_vAnchor;        // around from ST_VAnchor margin, page, text
    sal_Int32                                    m_x;              // x-position
    bool                                         m_bxValid;
    sal_Int32                                    m_y;              // y-position
    bool                                         m_byValid;
    sal_Int32                                    m_hSpace;         // frame padding h
    sal_Int32                                    m_vSpace;         // frame padding v
    sal_Int32                                    m_hRule;          // from ST_HeightRule exact, atLeast, auto
    sal_Int32                                    m_xAlign;         // from ST_XAlign center, inside, left, outside, right
    sal_Int32                                    m_yAlign;         // from ST_YAlign bottom, center, inline, inside, outside, top
    sal_Int8                                     m_nDropCapLength; // number of characters
    OUString                                     m_sParaStyleName;
    OUString                                     m_sParaId;        // [MS-DOCX] sect. 2.2.4 "p and tr Extensions"

    css::uno::Reference< css::text::XTextRange > m_xStartingRange; // start of a frame
    css::uno::Reference< css::text::XTextRange > m_xEndingRange;   // end of the frame
    sal_Int32 m_nListId = -1;

public:
    ParagraphProperties();

    ParagraphProperties(ParagraphProperties const &) = default;
    ParagraphProperties(ParagraphProperties &&) = default;
    ParagraphProperties & operator =(ParagraphProperties const &) = default;
    ParagraphProperties & operator =(ParagraphProperties &&) = default;

    sal_Int32 GetListId() const          { return m_nListId; }
    void      SetListId( sal_Int32 nId ) { m_nListId = nId; }

    bool IsFrameMode() const             { return m_bFrameMode; }
    void SetFrameMode( bool set = true ) { m_bFrameMode = set; }

    sal_Int32 GetDropCap() const           { return m_nDropCap; }
    void      SetDropCap( sal_Int32 nSet ) { m_nDropCap = nSet; }

    sal_Int32 GetLines() const           { return m_nLines; }
    void      SetLines( sal_Int32 nSet ) { m_nLines = nSet; }

    sal_Int32 Getw() const           { return m_w; }
    void      Setw( sal_Int32 nSet ) { m_w = nSet; }

    sal_Int32 Geth() const           { return m_h; }
    void      Seth( sal_Int32 nSet ) { m_h = nSet; }

    css::text::WrapTextMode GetWrap() const      { return m_nWrap; }
    void SetWrap( css::text::WrapTextMode nSet ) { m_nWrap = nSet; }

    sal_Int32 GethAnchor() const           { return m_hAnchor; }
    void      SethAnchor( sal_Int32 nSet ) { m_hAnchor = nSet; }

    sal_Int32 GetvAnchor() const           { return m_vAnchor; }
    void      SetvAnchor( sal_Int32 nSet ) { m_vAnchor = nSet; }

    sal_Int32 Getx() const           { return m_x; }
    void      Setx( sal_Int32 nSet ) { m_x = nSet; m_bxValid = true; }
    bool      IsxValid() const       { return m_bxValid; }

    sal_Int32 Gety() const           { return m_y; }
    void      Sety( sal_Int32 nSet ) { m_y = nSet; m_byValid = true; }
    bool      IsyValid() const       { return m_byValid; }

    void      SethSpace( sal_Int32 nSet ) { m_hSpace = nSet; }
    sal_Int32 GethSpace() const           { return m_hSpace; }

    sal_Int32 GetvSpace() const           { return m_vSpace; }
    void      SetvSpace( sal_Int32 nSet ) { m_vSpace = nSet; }

    sal_Int32 GethRule() const           { return m_hRule; }
    void      SethRule( sal_Int32 nSet ) { m_hRule = nSet; }

    sal_Int32 GetxAlign() const           { return m_xAlign; }
    void      SetxAlign( sal_Int32 nSet ) { m_xAlign = nSet; }

    sal_Int32 GetyAlign() const           { return m_yAlign; }
    void      SetyAlign( sal_Int32 nSet ) { m_yAlign = nSet; }

    sal_Int8  GetDropCapLength() const          { return m_nDropCapLength; }
    void      SetDropCapLength( sal_Int8 nSet ) { m_nDropCapLength = nSet; }

    const css::uno::Reference< css::text::XTextRange >& GetStartingRange() const      { return m_xStartingRange; }
    void SetStartingRange( const css::uno::Reference< css::text::XTextRange >& xSet ) { m_xStartingRange = xSet; }

    const css::uno::Reference< css::text::XTextRange >& GetEndingRange() const    { return m_xEndingRange; }
    void SetEndingRange( const css::uno::Reference< css::text::XTextRange >& xSet ) { m_xEndingRange = xSet; }

    const OUString& GetParaStyleName() const      { return m_sParaStyleName; }
    void SetParaStyleName( const OUString& rSet ) { m_sParaStyleName = rSet; }

    const OUString& GetParaId() const { return m_sParaId; }
    void SetParaId(const OUString& rSet) { m_sParaId = rSet; }

    void ResetFrameProperties();
};

typedef tools::SvRef< ParagraphProperties > ParagraphPropertiesPtr;

class ParagraphPropertiesPropertyMap: public PropertyMap {
public:
    ParagraphProperties & props() { return m_props; }

    ParagraphProperties const & props() const { return m_props; }

private:
    ParagraphProperties m_props;
};

/*-------------------------------------------------------------------------
    property map of a stylesheet
  -----------------------------------------------------------------------*/

#define WW_OUTLINE_MAX  sal_Int16( 9 )
#define WW_OUTLINE_MIN  sal_Int16( 0 )

class StyleSheetPropertyMap
    : public ParagraphPropertiesPropertyMap
{
private:
    sal_Int16 mnListLevel;
    sal_Int16 mnOutlineLevel;

public:
    explicit StyleSheetPropertyMap();

    sal_Int16 GetListLevel() const             { return mnListLevel; }
    void      SetListLevel( sal_Int16 nLevel ) { mnListLevel = nLevel; }

    sal_Int16 GetOutlineLevel() const             { return mnOutlineLevel; }
    void SetOutlineLevel(sal_Int16 nLevel) { if (nLevel <= WW_OUTLINE_MAX) mnOutlineLevel = nLevel; }
};

class ParagraphPropertyMap
    : public ParagraphPropertiesPropertyMap
{
public:
    explicit ParagraphPropertyMap() {}
};

class TablePropertyMap
    : public PropertyMap
{
public:
    enum TablePropertyMapTarget
    {
        TablePropertyMapTarget_START,
        CELL_MAR_LEFT = TablePropertyMapTarget_START,
        CELL_MAR_RIGHT,
        CELL_MAR_TOP,
        CELL_MAR_BOTTOM,
        TABLE_WIDTH,
        TABLE_WIDTH_TYPE,
        LEFT_MARGIN,
        HORI_ORIENT,
        TablePropertyMapTarget_MAX
    };

private:
    struct ValidValue
    {
        sal_Int32 nValue;
        bool      bValid;

        ValidValue()
            : nValue( 0 )
            , bValid( false )
        {
        }
    };

    ValidValue m_aValidValues[TablePropertyMapTarget_MAX];

public:
    explicit TablePropertyMap() {}

    bool getValue( TablePropertyMapTarget eWhich, sal_Int32& nFill );
    void setValue( TablePropertyMapTarget eWhich, sal_Int32 nSet );

    virtual void insertTableProperties( const PropertyMap*, const bool bOverwrite = true ) override;
};

typedef tools::SvRef< TablePropertyMap > TablePropertyMapPtr;

/// Information about a paragraph to be finished after a table end.
struct TableParagraph
{
    css::uno::Reference<css::text::XTextRange> m_rStartParagraph;
    css::uno::Reference<css::text::XTextRange> m_rEndParagraph;
    PropertyMapPtr m_pPropertyMap;
    css::uno::Reference<css::beans::XPropertySet> m_rPropertySet;
};

typedef std::shared_ptr< std::vector<TableParagraph> > TableParagraphVectorPtr;

} // namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
