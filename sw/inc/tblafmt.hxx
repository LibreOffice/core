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
#ifndef INCLUDED_SW_INC_TBLAFMT_HXX
#define INCLUDED_SW_INC_TBLAFMT_HXX
/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 *     The structure of table auto formatting should not be changed. It is used
 *     by different code of Writer and Calc. If a change is necessary, the
 *     source code of both applications must be changed!
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

#include <memory>

#include <svx/algitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/shaditem.hxx>
#include <svx/rotmodit.hxx>
#include <svl/intitem.hxx>
#include <editeng/lineitem.hxx>
#include "fmtpdsc.hxx"
#include "fmtornt.hxx"
#include "swdllapi.h"

struct SwAfVersions;

class SvNumberFormatter;
class SwTable;

class SwBoxAutoFormat
{
    // common attributes of Calc and Writer
    // --- from 641 on: CJK and CTL font settings
    SvxFontItem         m_aFont;
    SvxFontHeightItem   m_aHeight;
    SvxWeightItem       m_aWeight;
    SvxPostureItem      m_aPosture;

    SvxFontItem         m_aCJKFont;
    SvxFontHeightItem   m_aCJKHeight;
    SvxWeightItem       m_aCJKWeight;
    SvxPostureItem      m_aCJKPosture;

    SvxFontItem         m_aCTLFont;
    SvxFontHeightItem   m_aCTLHeight;
    SvxWeightItem       m_aCTLWeight;
    SvxPostureItem      m_aCTLPosture;

    SvxUnderlineItem    m_aUnderline;
    SvxOverlineItem     m_aOverline;
    SvxCrossedOutItem   m_aCrossedOut;
    SvxContourItem      m_aContour;
    SvxShadowedItem     m_aShadowed;
    SvxColorItem        m_aColor;
    SvxBoxItem          m_aBox;
    SvxLineItem         m_aTLBR;
    SvxLineItem         m_aBLTR;
    SvxBrushItem        m_aBackground;

    // Writer specific
    SvxAdjustItem       m_aAdjust;
    SvxFrameDirectionItem m_aTextOrientation;
    SwFormatVertOrient m_aVerticalAlignment;

    // Calc specific
    SvxHorJustifyItem   m_aHorJustify;
    SvxVerJustifyItem   m_aVerJustify;
    SfxBoolItem         m_aStacked;
    SvxMarginItem       m_aMargin;
    SfxBoolItem         m_aLinebreak;
    SfxInt32Item        m_aRotateAngle;
    SvxRotateModeItem   m_aRotateMode;

    // number format
    OUString            m_sNumFormatString;
    LanguageType        m_eSysLanguage, m_eNumFormatLanguage;

    css::uno::WeakReference<css::uno::XInterface> m_wXObject;
    friend class SwDocTest;

public:
    SwBoxAutoFormat();
    SwBoxAutoFormat( const SwBoxAutoFormat& rNew );
    ~SwBoxAutoFormat();

    SwBoxAutoFormat& operator=( const SwBoxAutoFormat& rNew );
    /// Comparing based of boxes backgrounds.
    bool operator==(const SwBoxAutoFormat& rRight);

    // The get-methods.
    const SvxFontItem       &GetFont() const        { return m_aFont; }
    const SvxFontHeightItem &GetHeight() const      { return m_aHeight; }
    const SvxWeightItem     &GetWeight() const      { return m_aWeight; }
    const SvxPostureItem    &GetPosture() const     { return m_aPosture; }
    const SvxFontItem       &GetCJKFont() const     { return m_aCJKFont; }
    const SvxFontHeightItem &GetCJKHeight() const   { return m_aCJKHeight; }
    const SvxWeightItem     &GetCJKWeight() const   { return m_aCJKWeight; }
    const SvxPostureItem    &GetCJKPosture() const  { return m_aCJKPosture; }
    const SvxFontItem       &GetCTLFont() const     { return m_aCTLFont; }
    const SvxFontHeightItem &GetCTLHeight() const   { return m_aCTLHeight; }
    const SvxWeightItem     &GetCTLWeight() const   { return m_aCTLWeight; }
    const SvxPostureItem    &GetCTLPosture() const  { return m_aCTLPosture; }
    const SvxUnderlineItem  &GetUnderline() const   { return m_aUnderline; }
    const SvxOverlineItem   &GetOverline() const    { return m_aOverline; }
    const SvxCrossedOutItem &GetCrossedOut() const  { return m_aCrossedOut; }
    const SvxContourItem    &GetContour() const     { return m_aContour; }
    const SvxShadowedItem   &GetShadowed() const    { return m_aShadowed; }
    const SvxColorItem      &GetColor() const       { return m_aColor; }
    const SvxAdjustItem     &GetAdjust() const      { return m_aAdjust; }
    const SvxFrameDirectionItem& GetTextOrientation() const { return m_aTextOrientation; }
    const SwFormatVertOrient& GetVerticalAlignment() const { return m_aVerticalAlignment; }
    const SvxBoxItem        &GetBox() const         { return m_aBox; }
    const SvxBrushItem      &GetBackground() const  { return m_aBackground; }
    void GetValueFormat( OUString& rFormat, LanguageType& rLng, LanguageType& rSys ) const
        { rFormat = m_sNumFormatString; rLng = m_eNumFormatLanguage; rSys = m_eSysLanguage; }

    // The set-methods.
    void SetFont( const SvxFontItem& rNew )             { m_aFont = rNew; }
    void SetHeight( const SvxFontHeightItem& rNew )     { m_aHeight = rNew; }
    void SetWeight( const SvxWeightItem& rNew )         { m_aWeight = rNew; }
    void SetPosture( const SvxPostureItem& rNew )       { m_aPosture = rNew; }
    void SetCJKFont( const SvxFontItem& rNew )          { m_aCJKFont = rNew; }
    void SetCJKHeight( const SvxFontHeightItem& rNew )  { m_aCJKHeight = rNew; }
    void SetCJKWeight( const SvxWeightItem& rNew )      { m_aCJKWeight = rNew; }
    void SetCJKPosture( const SvxPostureItem& rNew )    { m_aCJKPosture = rNew; }
    void SetCTLFont( const SvxFontItem& rNew )          { m_aCTLFont = rNew; }
    void SetCTLHeight( const SvxFontHeightItem& rNew )  { m_aCTLHeight = rNew; }
    void SetCTLWeight( const SvxWeightItem& rNew )      { m_aCTLWeight = rNew; }
    void SetCTLPosture( const SvxPostureItem& rNew )    { m_aCTLPosture = rNew; }
    void SetUnderline( const SvxUnderlineItem& rNew )   { m_aUnderline = rNew; }
    void SetOverline( const SvxOverlineItem& rNew )     { m_aOverline = rNew; }
    void SetCrossedOut( const SvxCrossedOutItem& rNew ) { m_aCrossedOut = rNew; }
    void SetContour( const SvxContourItem& rNew )       { m_aContour = rNew; }
    void SetShadowed( const SvxShadowedItem& rNew )     { m_aShadowed = rNew; }
    void SetColor( const SvxColorItem& rNew )           { m_aColor = rNew; }
    void SetAdjust( const SvxAdjustItem& rNew )
        {
            m_aAdjust.SetAdjust( rNew.GetAdjust() );
            m_aAdjust.SetOneWord( rNew.GetOneWord() );
            m_aAdjust.SetLastBlock( rNew.GetLastBlock() );
        }
    void SetTextOrientation(const SvxFrameDirectionItem& rNew) { m_aTextOrientation = rNew; }
    void SetVerticalAlignment(const SwFormatVertOrient& rNew) { m_aVerticalAlignment = rNew; }
    void SetBox( const SvxBoxItem& rNew )               { m_aBox = rNew; }
    void SetBackground( const SvxBrushItem& rNew )      { m_aBackground = rNew; }
    void SetValueFormat( const OUString& rFormat, LanguageType eLng, LanguageType eSys )
        { m_sNumFormatString = rFormat; m_eNumFormatLanguage = eLng; m_eSysLanguage = eSys; }

    css::uno::WeakReference<css::uno::XInterface> const& GetXObject() const
        { return m_wXObject; }
    void SetXObject(css::uno::Reference<css::uno::XInterface> const& xObject)
        { m_wXObject = xObject; }

    bool Load( SvStream& rStream, const SwAfVersions& rVersions, sal_uInt16 nVer );
    bool Save( SvStream& rStream, sal_uInt16 fileVersion ) const;
    void SaveVersionNo( SvStream& rStream, sal_uInt16 fileVersion ) const;
};

enum class SwTableAutoFormatUpdateFlags { Char = 1, Box = 2 };
namespace o3tl {
    template<> struct typed_flags<SwTableAutoFormatUpdateFlags> : is_typed_flags<SwTableAutoFormatUpdateFlags, 0x03> {};
};

/*
@remarks
A table has a number of lines. These lines seem to correspond with rows, except in the case of
rows spanning more than one line. Each line contains a number of boxes/cells.

AutoFormat properties are retrieved and stored in a grid of 16 table boxes. A sampling approach
is used to read the data. 4 lines are picked, and 4 boxes are picked from each.

The line picking and box picking algorithms are similar. We start at the first line/box, and pick
lines/boxes one by one for a maximum of 3. The 4th line/box is the last line/box in the current
table/line. If we hit the end of lines/boxes, the last line/box encountered is picked several times.

For example, in a 2x3 table, the 4 lines will be [0, 1, 1, 1]. In each line, the boxes will be
[0, 1, 2, 2]. In a 6x5 table, the 4 lines will be [0, 1, 2, 4] and the boxes per line will be
[0, 1, 2, 5].

As you can see, property extraction/application is lossless for tables that are 4x4 or smaller
(and in fact has a bit of redundancy). For larger tables, we lose any individual cell formatting
for the range [(3,rows - 1) -> (3, cols - 1)]. That formatting is replaced by formatting from
the saved cells:

            0            1            2           3           4           5
        +-----------------------------------------------------------------------+
     0  |   Saved   |  Saved    |  Saved    |           |           |  Saved    |
        +-----------------------------------------------------------------------+
     1  |   Saved   |  Saved    |  Saved    |           |           |  Saved    |
        +-----------------------------------------------------------------------+
     2  |   Saved   |  Saved    |  Saved    |           |           |  Saved    |
        +-----------------------------------------------------------------------+
     3  |           |           |           |           |           |           |
        +-----------------------------------------------------------------------+
     4  |           |           |           |           |           |           |
        +-----------------------------------------------------------------------+
     5  |   Saved   |  Saved    |  Saved    |           |           |  Saved    |
        +-----------+-----------+-----------+-----------+-----------+-----------+

The properties saved are divided into three categories:
    1. Character properties: Font, font size, weight, etc.
    2. Box properties: Box, cell background
    3. Table properties: Properties that are set in the Table->Table Properties dialog.

Character and box properties are stored per cell (and are lossy for tables larger than 4x4). Table
properties are stored per-table, and are lossless.
*/
class SW_DLLPUBLIC SwTableAutoFormat
{
    friend class SwDocTest;
    friend void FinitCore();       // To destroy default pointer.
    static SwBoxAutoFormat* pDfltBoxAutoFormat;

    css::uno::WeakReference<css::uno::XInterface> m_wXObject;

    OUString m_aName;
    sal_uInt16 m_nStrResId;

    // Common flags of Calc and Writer.
    bool m_bInclFont : 1;
    bool m_bInclJustify : 1;
    bool m_bInclFrame : 1;
    bool m_bInclBackground : 1;
    bool m_bInclValueFormat : 1;

    // Calc specific flags.
    bool m_bInclWidthHeight : 1;

    SwBoxAutoFormat* m_aBoxAutoFormat[ 16 ];

    // Writer-specific options
    SvxFormatBreakItem m_aBreak;
    SwFormatPageDesc m_aPageDesc;
    SvxFormatKeepItem m_aKeepWithNextPara;
    sal_uInt16 m_aRepeatHeading;
    bool m_bLayoutSplit;
    bool m_bRowSplit;
    bool m_bCollapsingBorders;
    SvxShadowItem m_aShadow;

    bool m_bHidden;
    bool m_bUserDefined;
public:
    SwTableAutoFormat( const OUString& rName );
    SwTableAutoFormat( const SwTableAutoFormat& rNew );
    ~SwTableAutoFormat();

    SwTableAutoFormat& operator=( const SwTableAutoFormat& rNew );

    void SetBoxFormat( const SwBoxAutoFormat& rNew, sal_uInt8 nPos );
    const SwBoxAutoFormat& GetBoxFormat( sal_uInt8 nPos ) const;
    SwBoxAutoFormat& GetBoxFormat( sal_uInt8 nPos );
    static const SwBoxAutoFormat& GetDefaultBoxFormat();

    void SetName( const OUString& rNew ) { m_aName = rNew; m_nStrResId = USHRT_MAX; }
    const OUString& GetName() const { return m_aName; }

    void UpdateFromSet( sal_uInt8 nPos, const SfxItemSet& rSet,
                                SwTableAutoFormatUpdateFlags eFlags, SvNumberFormatter const * );
    void UpdateToSet( sal_uInt8 nPos, SfxItemSet& rSet, SwTableAutoFormatUpdateFlags eFlags,
                        SvNumberFormatter* ) const ;

    void RestoreTableProperties(SwTable &table) const;
    void StoreTableProperties(const SwTable &table);

    bool IsFont() const         { return m_bInclFont; }
    bool IsJustify() const      { return m_bInclJustify; }
    bool IsFrame() const        { return m_bInclFrame; }
    bool IsBackground() const   { return m_bInclBackground; }
    bool IsValueFormat() const  { return m_bInclValueFormat; }

    /// Check if style is hidden.
    bool IsHidden() const       { return m_bHidden; }
    /// Check if style is defined by user.
    bool IsUserDefined() const  { return m_bUserDefined; }

    void SetFont( const bool bNew )         { m_bInclFont = bNew; }
    void SetJustify( const  bool bNew )     { m_bInclJustify = bNew; }
    void SetFrame( const bool bNew )        { m_bInclFrame = bNew; }
    void SetBackground( const bool bNew )   { m_bInclBackground = bNew; }
    void SetValueFormat( const bool bNew )  { m_bInclValueFormat = bNew; }
    void SetWidthHeight( const bool bNew )  { m_bInclWidthHeight = bNew; }

    /// Set if style is hidden.
    void SetHidden(bool bHidden)            { m_bHidden = bHidden; }
    /// Set if style is user defined.
    void SetUserDefined(bool bUserDefined)  { m_bUserDefined = bUserDefined; }

    /// These methods returns what style (row or column) is applied first on given Cell
    bool FirstRowEndColumnIsRow();
    bool FirstRowStartColumnIsRow();
    bool LastRowEndColumnIsRow();
    bool LastRowStartColumnIsRow();

    bool Load( SvStream& rStream, const SwAfVersions& );
    bool Save( SvStream& rStream, sal_uInt16 fileVersion ) const;

    css::uno::WeakReference<css::uno::XInterface> const& GetXObject() const
        { return m_wXObject; }
    void SetXObject(css::uno::Reference<css::uno::XInterface> const& xObject)
        { m_wXObject = xObject; }

    /// Returns the cell's name postfix. eg. ".1"
    OUString GetTableTemplateCellSubName(const SwBoxAutoFormat& rBoxFormat) const;
    /// Returns a vector of indexes in aBoxAutoFormat array. Returned indexes points to cells which are mapped to a table-template.
    static const std::vector<sal_Int32>& GetTableTemplateMap();

    /**
     * Calculates the relevant position in the table autoformat for a given
     * cell in a given table.
     */
    static sal_uInt8 CountPos(sal_uInt32 nCol, sal_uInt32 nCols, sal_uInt32 nRow, sal_uInt32 nRows);
};

class SW_DLLPUBLIC SwTableAutoFormatTable
{
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;

    SAL_DLLPRIVATE bool Load( SvStream& rStream );
    SAL_DLLPRIVATE bool Save( SvStream& rStream ) const;

public:
    explicit SwTableAutoFormatTable();
    ~SwTableAutoFormatTable();

    size_t size() const;
    SwTableAutoFormat const& operator[](size_t i) const;
    SwTableAutoFormat      & operator[](size_t i);

    /// Append table style to the existing styles.
    void AddAutoFormat(const SwTableAutoFormat& rFormat);

    void InsertAutoFormat(size_t i, std::unique_ptr<SwTableAutoFormat> pFormat);
    void EraseAutoFormat(size_t i);
    void EraseAutoFormat(const OUString& rName);
    std::unique_ptr<SwTableAutoFormat> ReleaseAutoFormat(size_t i);
    /// Removes an autoformat. Returns pointer to the removed autoformat or nullptr.
    std::unique_ptr<SwTableAutoFormat> ReleaseAutoFormat(const OUString& rName);

    /// Find table style with the provided name, return nullptr when not found.
    SwTableAutoFormat* FindAutoFormat(const OUString& rName) const;

    void Load();
    bool Save() const;
};

class SwCellStyleDescriptor
{
    const std::pair<OUString, std::unique_ptr<SwBoxAutoFormat>>& m_rCellStyleDesc;
public:
    SwCellStyleDescriptor(const std::pair<OUString, std::unique_ptr<SwBoxAutoFormat>>& rCellStyleDesc) : m_rCellStyleDesc(rCellStyleDesc) { }

    const OUString&  GetName()   { return m_rCellStyleDesc.first; }
};

class SwCellStyleTable
{
    std::vector<std::pair<OUString, std::unique_ptr<SwBoxAutoFormat>>> m_aCellStyles;
public:
    SwCellStyleTable();
    ~SwCellStyleTable();

    size_t size() const;
    SwCellStyleDescriptor operator[](size_t i) const;
    void clear();

    /// Add a copy of rBoxFormat
    void AddBoxFormat(const SwBoxAutoFormat& rBoxFormat, const OUString& sName);
    void RemoveBoxFormat(const OUString& sName);
    void ChangeBoxFormatName(const OUString& sFromName, const OUString& sToName);
    /// If found returns its name. If not found returns an empty OUString
    OUString GetBoxFormatName(const SwBoxAutoFormat& rBoxFormat) const;
    /// If found returns a ptr to a BoxFormat. If not found returns nullptr
    SwBoxAutoFormat* GetBoxFormat(const OUString& sName) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
