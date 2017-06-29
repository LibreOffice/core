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
#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_PROPERTYMAP_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_PROPERTYMAP_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/uno/Any.h>
#include "PropertyIds.hxx"
#include <memory>
#include <boost/optional.hpp>
#include <map>
#include <vector>
#include "TagLogger.hxx"

namespace com { namespace sun { namespace star {
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
}}}

namespace writerfilter {
namespace dmapper {

class  DomainMapper_Impl;
struct FloatingTableInfo;

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

struct RedlineParams
{
    OUString  m_sAuthor;
    OUString  m_sDate;
    sal_Int32 m_nToken;

    // This can hold properties of runs that had formatted 'track changes' properties
    css::uno::Sequence< css::beans::PropertyValue > m_aRevertProperties;
};

typedef std::shared_ptr< RedlineParams > RedlineParamsPtr;

class PropValue
{
private:
    css::uno::Any m_aValue;
    GrabBagType   m_GrabBagType;

public:
    PropValue( const css::uno::Any& rValue, GrabBagType i_GrabBagType )
        : m_aValue( rValue )
        , m_GrabBagType( i_GrabBagType )
    {
    }

    PropValue()
        : m_aValue()
        , m_GrabBagType( NO_GRAB_BAG )
    {
    }

    const css::uno::Any& getValue() const { return m_aValue; }

    GrabBagType getGrabBagType() const { return m_GrabBagType; }
};

class PropertyMap
{
private:
    // Cache the property values for the GetPropertyValues() call(s).
    std::vector< css::beans::PropertyValue >    m_aValues;

    // marks context as footnote context - ::text( ) events contain either the footnote character or can be ignored
    // depending on sprmCSymbol
    css::uno::Reference< css::text::XFootnote > m_xFootnote;
    std::map< PropertyIds, PropValue >          m_vMap;
    std::vector< RedlineParamsPtr >             m_aRedlines;

public:
    typedef std::pair< PropertyIds, css::uno::Any > Property;

    PropertyMap() {}
    virtual ~PropertyMap() {}

    // Sequence: Grab Bags: The CHAR_GRAB_BAG has Name "CharInteropGrabBag" and the PARA_GRAB_BAG has Name "ParaInteropGrabBag"
    // the contained properties are their Value.
    css::uno::Sequence< css::beans::PropertyValue > GetPropertyValues( bool bCharGrabBag = true );

    // Add property, optionally overwriting existing attributes
    void Insert( PropertyIds eId, const css::uno::Any& rAny, bool bOverwrite = true, GrabBagType i_GrabBagType = NO_GRAB_BAG );

    // Remove a named property from *this, does nothing if the property id has not been set
    void Erase( PropertyIds eId);

    // Imports properties from pMap, overwriting those with the same PropertyIds as the current map
    void InsertProps( const std::shared_ptr< PropertyMap >& rMap );

    // Returns a copy of the property if it exists, .first is its PropertyIds and .second is its Value (type css::uno::Any)
    boost::optional< Property > getProperty( PropertyIds eId ) const;

    // Has the property named been set (via Insert)?
    bool isSet( PropertyIds eId ) const;

    const css::uno::Reference< css::text::XFootnote >& GetFootnote() const { return m_xFootnote; }

    void SetFootnote( const css::uno::Reference< css::text::XFootnote >& xF ) { m_xFootnote = xF; }

    virtual void insertTableProperties( const PropertyMap* );

    const std::vector< RedlineParamsPtr >& Redlines() const { return m_aRedlines; }

    std::vector< RedlineParamsPtr >& Redlines() { return m_aRedlines; }

    void printProperties();

#ifdef DEBUG_WRITERFILTER
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

typedef std::shared_ptr< PropertyMap > PropertyMapPtr;

class SectionPropertyMap : public PropertyMap
{
private:
#ifdef DEBUG_WRITERFILTER
    sal_Int32 m_nDebugSectionNumber;
#endif

    // 'temporarily' the section page settings are imported as page styles
    // empty strings mark page settings as not yet imported
    enum class SectionProp {
        BorderDistanceLeft,
        BorderDistanceRight,
        BorderDistanceTop,
        BorderDistanceBottom,
        BorderParams,
        BorderShadowLeft,
        BorderShadowRight,
        BorderShadowTop,
        BorderShadowBottom,
        TitlePage,
        ColumnCount,
        ColumnDistance,
        SeparatorLineIsOn,
        EvenlySpaced,
        PageNoRestart,
        PageNumber,
        // Page number type is a value from css::style::NumberingType.
        PageNumberType,
        BreakType,
        PaperBin,
        FirstPaperBin,
        LeftMargin,
        RightMargin,
        TopMargin,
        BottomMargin,
        HeaderTop,
        HeaderBottom,
        DzaGutter,
        GridType,
        GridLinePitch,
        DxtCharSpace,
        GridSnapToChars,

        // line numbering
        LnnMod,
        Lnc,
        dxaLnn,
        LnnMin,

        // The "Link To Previous" flag indicates whether the header/footer
        // content should be taken from the previous section
        DefaultHeaderLinkToPrevious,
        EvenPageHeaderLinkToPrevious,
        FirstPageHeaderLinkToPrevious,
        DefaultFooterLinkToPrevious,
        EvenPageFooterLinkToPrevious,
        FirstPageFooterLinkToPrevious,
    };

    typedef std::map<SectionProp, css::uno::Any> SecPropMapImpl;
    // We only store explicitly set properties; properties not set return default values in getters
    SecPropMapImpl                                  m_aPropMap;

    bool                                            m_bIsFirstSection;
    css::uno::Reference< css::text::XTextRange >    m_xStartingRange;

    OUString                                        m_sFirstPageStyleName;
    OUString                                        m_sFollowPageStyleName;
    css::uno::Reference< css::beans::XPropertySet > m_xFirstPageStyle;
    css::uno::Reference< css::beans::XPropertySet > m_xFollowPageStyle;

    boost::optional< css::table::BorderLine2 >      m_oBorderLines[4];

    css::uno::Reference< css::beans::XPropertySet > m_xColumnContainer;
    std::vector< sal_Int32 >                        m_aColWidth;
    std::vector< sal_Int32 >                        m_aColDistance;

    template<typename T>
    void set(SectionProp key, T val) { m_aPropMap.insert_or_assign(key, css::uno::Any(val)); }

    template<typename T>
    T get(SectionProp key, T defVal) const
    {
        const auto& it = m_aPropMap.find(key);
        if (it == m_aPropMap.end())
            return defVal;

        return it->second.get<T>();
    }

    static SectionProp BorderDistance(BorderPosition ePos)
    {
        switch (ePos)
        {
        case BORDER_LEFT:
            return SectionProp::BorderDistanceLeft;
        case BORDER_RIGHT:
            return SectionProp::BorderDistanceRight;
        case BORDER_TOP:
            return SectionProp::BorderDistanceTop;
        case BORDER_BOTTOM:
            return SectionProp::BorderDistanceBottom;
        }
        return SectionProp::BorderDistanceLeft;
    }

    static SectionProp BorderShadow(BorderPosition ePos)
    {
        switch (ePos)
        {
        case BORDER_LEFT:
            return SectionProp::BorderShadowLeft;
        case BORDER_RIGHT:
            return SectionProp::BorderShadowRight;
        case BORDER_TOP:
            return SectionProp::BorderShadowTop;
        case BORDER_BOTTOM:
            return SectionProp::BorderShadowBottom;
        }
        return SectionProp::BorderShadowLeft;
    }

    // Getters with default values
    sal_Int32 GetBorderDistance(BorderPosition ePos) const { return get(BorderDistance(ePos),                       sal_Int32(-1)); }
    sal_Int32 GetBorderParams()                      const { return get(SectionProp::BorderParams,                  sal_Int32(0)); }
    bool      GetBorderShadow(BorderPosition ePos)   const { return get(BorderShadow(ePos),                         false); }
    bool      GetTitlePage()                         const { return get(SectionProp::TitlePage,                     false); }
    sal_Int32 GetColumnDistance()                    const { return get(SectionProp::ColumnDistance,                sal_Int32(1249)); }
    bool      GetSeparatorLineIsOn()                 const { return get(SectionProp::SeparatorLineIsOn,             false); }
    bool      GetEvenlySpaced()                      const { return get(SectionProp::EvenlySpaced,                  false); }
    bool      GetPageNoRestart()                     const { return get(SectionProp::PageNoRestart,                 false); }
    sal_Int32 GetPageNumber()                        const { return get(SectionProp::PageNumber,                    sal_Int32(-1)); }
    sal_Int16 GetPageNumberType()                    const { return get(SectionProp::PageNumberType,                sal_Int16(-1)); }
    sal_Int32 GetPaperBin()                          const { return get(SectionProp::PaperBin,                      sal_Int32(-1)); }
    sal_Int32 GetFirstPaperBin()                     const { return get(SectionProp::FirstPaperBin,                 sal_Int32(-1)); }
    sal_Int32 GetTopMargin()                         const { return get(SectionProp::TopMargin,                     sal_Int32(2540)); } // 1440 twip
    sal_Int32 GetBottomMargin()                      const { return get(SectionProp::BottomMargin,                  sal_Int32(2540)); } // 1440 twip
    sal_Int32 GetHeaderTop()                         const { return get(SectionProp::HeaderTop,                     sal_Int32(1270)); } // 720 twip
    sal_Int32 GetHeaderBottom()                      const { return get(SectionProp::HeaderBottom,                  sal_Int32(1270)); } // 720 twip
    sal_Int32 GetDzaGutter()                         const { return get(SectionProp::DzaGutter,                     sal_Int32(0)); }
    sal_Int32 GetGridType()                          const { return get(SectionProp::GridType,                      sal_Int32(0)); }
    sal_Int32 GetGridLinePitch()                     const { return get(SectionProp::GridLinePitch,                 sal_Int32(1)); }
    sal_Int32 GetDxtCharSpace()                      const { return get(SectionProp::DxtCharSpace,                  sal_Int32(0)); }
    bool      GetGridSnapToChars()                   const { return get(SectionProp::GridSnapToChars,               true); }
    sal_Int32 GetLnnMod()                            const { return get(SectionProp::LnnMod,                        sal_Int32(0)); }
    sal_Int32 GetLnc()                               const { return get(SectionProp::Lnc,                           sal_Int32(0)); }
    sal_Int32 GetdxaLnn()                            const { return get(SectionProp::dxaLnn,                        sal_Int32(0)); }
    sal_Int32 GetLnnMin()                            const { return get(SectionProp::LnnMin,                        sal_Int32(0)); }
    bool      GetDefaultHeaderLinkToPrevious()       const { return get(SectionProp::DefaultHeaderLinkToPrevious,   true); }
    bool      GetEvenPageHeaderLinkToPrevious()      const { return get(SectionProp::EvenPageHeaderLinkToPrevious,  true); }
    bool      GetFirstPageHeaderLinkToPrevious()     const { return get(SectionProp::FirstPageHeaderLinkToPrevious, true); }
    bool      GetDefaultFooterLinkToPrevious()       const { return get(SectionProp::DefaultFooterLinkToPrevious,   true); }
    bool      GetEvenPageFooterLinkToPrevious()      const { return get(SectionProp::EvenPageFooterLinkToPrevious,  true); }
    bool      GetFirstPageFooterLinkToPrevious()     const { return get(SectionProp::FirstPageFooterLinkToPrevious, true); }

    void ApplyProperties_( const css::uno::Reference< css::beans::XPropertySet >& xStyle );

    void DontBalanceTextColumns();

    css::uno::Reference< css::text::XTextColumns > ApplyColumnProperties( const css::uno::Reference< css::beans::XPropertySet >& xFollowPageStyle,
                                                                          DomainMapper_Impl& rDM_Impl);

    void CopyLastHeaderFooter( bool bFirstPage, DomainMapper_Impl& rDM_Impl );

    static void CopyHeaderFooter( const css::uno::Reference< css::beans::XPropertySet >& xPrevStyle,
                                  const css::uno::Reference< css::beans::XPropertySet >& xStyle,
                                  bool bOmitRightHeader = false, bool bOmitLeftHeader = false,
                                  bool bOmitRightFooter = false, bool bOmitLeftFooter = false );

    static void CopyHeaderFooterTextProperty( const css::uno::Reference< css::beans::XPropertySet >& xPrevStyle,
                                              const css::uno::Reference< css::beans::XPropertySet >& xStyle,
                                              PropertyIds ePropId );

    void PrepareHeaderFooterProperties( bool bFirstPage );

    bool HasHeader( bool bFirstPage ) const;
    bool HasFooter( bool bFirstPage ) const;

    static void SetBorderDistance( const css::uno::Reference< css::beans::XPropertySet >& xStyle,
                                   PropertyIds eMarginId,
                                   PropertyIds eDistId,
                                   sal_Int32 nDistance,
                                   sal_Int32 nOffsetFrom,
                                   sal_uInt32 nLineWidth );

    // Determines if conversion of a given floating table is wanted or not.
    bool FloatingTableConversion( DomainMapper_Impl& rDM_Impl, FloatingTableInfo& rInfo );

public:
    enum PageType
    {
        PAGE_FIRST,
        PAGE_LEFT,
        PAGE_RIGHT
    };

    explicit SectionPropertyMap( bool bIsFirstSection );

    void InsertSectionProps(const PropertyMapPtr& pFrom);

    void SetStart( const css::uno::Reference< css::text::XTextRange >& xRange ) { m_xStartingRange = xRange; }

    const css::uno::Reference< css::text::XTextRange >& GetStartingRange() const { return m_xStartingRange; }

    css::uno::Reference< css::beans::XPropertySet > GetPageStyle( const css::uno::Reference< css::container::XNameContainer >& xStyles,
                                                                  const css::uno::Reference< css::lang::XMultiServiceFactory >& xTextFactory,
                                                                  bool bFirst );

    const OUString& GetPageStyleName( bool bFirstPage = false ) const
    {
        return bFirstPage ? m_sFirstPageStyleName : m_sFollowPageStyleName;
    }

    // @throws css::beans::UnknownPropertyException
    // @throws css::beans::PropertyVetoException
    // @throws css::lang::IllegalArgumentException
    // @throws css::lang::WrappedTargetException
    // @throws css::uno::RuntimeException
    void InheritOrFinalizePageStyles( DomainMapper_Impl& rDM_Impl );

    void SetBorder( BorderPosition ePos, sal_Int32 nLineDistance, const css::table::BorderLine2& rBorderLine, bool bShadow );
    void SetBorderParams( sal_Int32 nSet ) { set(SectionProp::BorderParams, nSet); }

    void SetColumnCount( sal_Int16 nCount ) { set(SectionProp::ColumnCount, nCount); }
    sal_Int16 GetColumnCount() const        { return get(SectionProp::ColumnCount, sal_Int16(0)); }

    void SetColumnDistance( sal_Int32 nDist )   { set(SectionProp::ColumnDistance, nDist); }
    void AppendColumnWidth( sal_Int32 nWidth )  { m_aColWidth.push_back( nWidth ); }
    void AppendColumnSpacing( sal_Int32 nDist ) { m_aColDistance.push_back( nDist ); }

    void SetTitlePage( bool bSet )           { set(SectionProp::TitlePage, bSet); }
    void SetSeparatorLine( bool bSet )       { set(SectionProp::SeparatorLineIsOn, bSet); }
    void SetEvenlySpaced( bool bSet )        { set(SectionProp::EvenlySpaced, bSet); }
    void SetPageNumber( sal_Int32 nSet )     { set(SectionProp::PageNumber, nSet); }
    void SetPageNumberType( sal_Int16 nSet ) { set(SectionProp::PageNumberType, nSet); }
    void SetBreakType( sal_Int32 nSet )      { set(SectionProp::BreakType, nSet); }
    // GetBreakType returns -1 if the breakType has not yet been identified for the section
    sal_Int32 GetBreakType() const           { return get(SectionProp::BreakType, sal_Int32(-1)); }

    void SetLeftMargin( sal_Int32 nSet )   { set(SectionProp::LeftMargin, nSet); }
    sal_Int32 GetLeftMargin() const        { return get(SectionProp::LeftMargin, sal_Int32(3175)); } // page left margin, default 0x708 (1800) twip -> 3175 1/100 mm
    void SetRightMargin( sal_Int32 nSet )  { set(SectionProp::RightMargin, nSet); }
    sal_Int32 GetRightMargin() const       { return get(SectionProp::RightMargin, sal_Int32(3175)); } // page right margin, default 0x708 (1800) twip -> 3175 1/100 mm
    void SetTopMargin( sal_Int32 nSet )    { set(SectionProp::TopMargin, nSet); }
    void SetBottomMargin( sal_Int32 nSet ) { set(SectionProp::BottomMargin, nSet); }
    void SetHeaderTop( sal_Int32 nSet )    { set(SectionProp::HeaderTop, nSet); }
    void SetHeaderBottom( sal_Int32 nSet ) { set(SectionProp::HeaderBottom, nSet); }
    sal_Int32 GetPageWidth() const;

    void SetGridType( sal_Int32 nSet )      { set(SectionProp::GridType, nSet); }
    void SetGridLinePitch( sal_Int32 nSet ) { set(SectionProp::GridLinePitch, nSet); }
    void SetGridSnapToChars( bool bSet )    { set(SectionProp::GridSnapToChars, bSet); }
    void SetDxtCharSpace( sal_Int32 nSet )  { set(SectionProp::DxtCharSpace, nSet); }

    void SetLnnMod( sal_Int32 nValue ) { set(SectionProp::LnnMod, nValue); }
    void SetLnc( sal_Int32 nValue )    { set(SectionProp::Lnc, nValue); }
    void SetdxaLnn( sal_Int32 nValue ) { set(SectionProp::dxaLnn, nValue); }
    void SetLnnMin( sal_Int32 nValue ) { set(SectionProp::LnnMin, nValue); }

    // determine which style gets the borders
    void ApplyBorderToPageStyles( const css::uno::Reference< css::container::XNameContainer >& xStyles,
                                  const css::uno::Reference< css::lang::XMultiServiceFactory >& xTextFactory,
                                  sal_Int32 nValue );

    void CloseSectionGroup( DomainMapper_Impl& rDM_Impl );
    // Handling of margins, header and footer for any kind of sections breaks.
    void HandleMarginsHeaderFooter( bool bFirstPage, DomainMapper_Impl& rDM_Impl );
    void ClearHeaderFooterLinkToPrevious( bool bHeader, PageType eType );
};

class ParagraphProperties
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
    bool                                         m_bAnchorLock;
    sal_Int8                                     m_nDropCapLength; // number of characters
    OUString                                     m_sParaStyleName;

    css::uno::Reference< css::text::XTextRange > m_xStartingRange; // start of a frame
    css::uno::Reference< css::text::XTextRange > m_xEndingRange;   // end of the frame

public:
    ParagraphProperties();
    ParagraphProperties( const ParagraphProperties& );
    virtual ~ParagraphProperties() {}

    // Does not compare the starting/ending range, m_sParaStyleName and m_nDropCapLength
    bool operator==( const ParagraphProperties& );

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

    void ResetFrameProperties();
};

typedef std::shared_ptr< ParagraphProperties > ParagraphPropertiesPtr;

/*-------------------------------------------------------------------------
    property map of a stylesheet
  -----------------------------------------------------------------------*/

#define WW_OUTLINE_MAX  sal_Int16( 9 )
#define WW_OUTLINE_MIN  sal_Int16( 0 )

class StyleSheetPropertyMap
    : public PropertyMap
    , public ParagraphProperties
{
private:
    sal_Int32 mnListId;
    sal_Int16 mnListLevel;
    sal_Int16 mnOutlineLevel;
    sal_Int32 mnNumId;

public:
    explicit StyleSheetPropertyMap();

    sal_Int32 GetListId() const          { return mnListId; }
    void      SetListId( sal_Int32 nId ) { mnListId = nId; }

    sal_Int16 GetListLevel() const             { return mnListLevel; }
    void      SetListLevel( sal_Int16 nLevel ) { mnListLevel = nLevel; }

    sal_Int16 GetOutlineLevel() const             { return mnOutlineLevel; }
    void      SetOutlineLevel( sal_Int16 nLevel ) { if ( nLevel < WW_OUTLINE_MAX ) mnOutlineLevel = nLevel; }

    sal_Int32 GetNumId() const        { return mnNumId; }
    void      SetNumId(sal_Int32 nId) { mnNumId = nId; }
};

class ParagraphPropertyMap
    : public PropertyMap
    , public ParagraphProperties
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
        GAP_HALF,
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

    virtual void insertTableProperties( const PropertyMap* ) override;
};

typedef std::shared_ptr< TablePropertyMap > TablePropertyMapPtr;

} // namespace dmapper
} // namespace writerfilter

#endif // INCLUDED_WRITERFILTER_SOURCE_DMAPPER_PROPERTYMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
