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

#ifndef INCLUDED_SC_INC_AUTOFORM_HXX
#define INCLUDED_SC_INC_AUTOFORM_HXX

/*************************************************************************
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    The structure of auto formatting should not be changed. It is used
    by various code of Writer and Calc. If a change is necessary, the
    source code of both applications must be changed!

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
**************************************************************************/

#include "scitems.hxx"
#include <editeng/adjustitem.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svx/rotmodit.hxx>
#include <svl/intitem.hxx>
#include <editeng/lineitem.hxx>
#include "scdllapi.h"
#include "zforauto.hxx"

#include <array>
#include <memory>
#include <map>
#include <climits>

class ScDocument;

/**
A binary blob of writer-specific data. This data typically consists of types that are
unavailable to Calc (e.g. SwFmtVertOrient), or that Calc doesn't care about.

@remarks Note that in autoformat versions prior to AUTOFORMAT_DATA_ID_31005, Calc
logic handled and stored several writer-specific items (such as ScAutoFormatDataField::aAdjust).
That logic was preserved. From _31005 onward, writer-specific data should be handled by
blobs to avoid needlessly complicating the Calc logic.
*/
struct AutoFormatSwBlob
{
    std::unique_ptr<sal_uInt8[]> pData;
    std::size_t size;

    AutoFormatSwBlob() : size(0)
    {
    }
    AutoFormatSwBlob(const AutoFormatSwBlob&) = delete;
    const AutoFormatSwBlob& operator=(const AutoFormatSwBlob&) = delete;

    void Reset()
    {
        pData.reset();
        size = 0;
    }
};

/// Struct with version numbers of the Items
struct ScAfVersions
{
public:
    sal_uInt16 nFontVersion;
    sal_uInt16 nFontHeightVersion;
    sal_uInt16 nWeightVersion;
    sal_uInt16 nPostureVersion;

    sal_uInt16 nUnderlineVersion;
    sal_uInt16 nOverlineVersion;
    sal_uInt16 nCrossedOutVersion;
    sal_uInt16 nContourVersion;
    sal_uInt16 nShadowedVersion;
    sal_uInt16 nColorVersion;
    sal_uInt16 nBoxVersion;
    sal_uInt16 nLineVersion;
    sal_uInt16 nBrushVersion;

    sal_uInt16 nAdjustVersion;
    AutoFormatSwBlob swVersions;

    sal_uInt16 nHorJustifyVersion;
    sal_uInt16 nVerJustifyVersion;
    sal_uInt16 nOrientationVersion;
    sal_uInt16 nMarginVersion;
    sal_uInt16 nBoolVersion;
    sal_uInt16 nInt32Version;
    sal_uInt16 nRotateModeVersion;

    sal_uInt16 nNumFmtVersion;

    ScAfVersions();

    void Load( SvStream& rStream, sal_uInt16 nVer );
    void Write(SvStream& rStream, sal_uInt16 fileVersion);
};

/// Contains all items for one cell of a table autoformat.
class ScAutoFormatDataField
{
private:
    std::shared_ptr<SvxFontItem>                aFont;
    std::shared_ptr<SvxFontHeightItem>          aHeight;
    std::shared_ptr<SvxWeightItem>              aWeight;
    std::shared_ptr<SvxPostureItem>             aPosture;

    std::shared_ptr<SvxFontItem>                aCJKFont;
    std::shared_ptr<SvxFontHeightItem>          aCJKHeight;
    std::shared_ptr<SvxWeightItem>              aCJKWeight;
    std::shared_ptr<SvxPostureItem>             aCJKPosture;

    std::shared_ptr<SvxFontItem>                aCTLFont;
    std::shared_ptr<SvxFontHeightItem>          aCTLHeight;
    std::shared_ptr<SvxWeightItem>              aCTLWeight;
    std::shared_ptr<SvxPostureItem>             aCTLPosture;

    std::shared_ptr<SvxUnderlineItem>           aUnderline;
    std::shared_ptr<SvxOverlineItem>            aOverline;
    std::shared_ptr<SvxCrossedOutItem>          aCrossedOut;
    std::shared_ptr<SvxContourItem>             aContour;
    std::shared_ptr<SvxShadowedItem>            aShadowed;
    std::shared_ptr<SvxColorItem>               aColor;
    std::shared_ptr<SvxBoxItem>                 aBox;
    std::shared_ptr<SvxLineItem>                aTLBR;
    std::shared_ptr<SvxLineItem>                aBLTR;
    std::shared_ptr<SvxBrushItem>               aBackground;

    // Writer specific
    std::shared_ptr<SvxAdjustItem>              aAdjust;
    AutoFormatSwBlob                            m_swFields;

    // Calc specific
    std::shared_ptr<SvxHorJustifyItem>          aHorJustify;
    std::shared_ptr<SvxVerJustifyItem>          aVerJustify;
    std::shared_ptr<SfxBoolItem>                aStacked;
    std::shared_ptr<SvxMarginItem>              aMargin;
    std::shared_ptr<SfxBoolItem>                aLinebreak;

    // from SO5, 504k on, rotated text
    std::shared_ptr<SfxInt32Item>               aRotateAngle;
    std::shared_ptr<SvxRotateModeItem>          aRotateMode;

    // number format
    ScNumFormatAbbrev                           aNumFormat;

public:
                                ScAutoFormatDataField();
                                ScAutoFormatDataField( const ScAutoFormatDataField& rCopy );
                                ~ScAutoFormatDataField();

    const SvxFontItem&          GetFont() const         { return *aFont; }
    const SvxFontHeightItem&    GetHeight() const       { return *aHeight; }
    const SvxWeightItem&        GetWeight() const       { return *aWeight; }
    const SvxPostureItem&       GetPosture() const      { return *aPosture; }

    const SvxFontItem&          GetCJKFont() const      { return *aCJKFont; }
    const SvxFontHeightItem&    GetCJKHeight() const    { return *aCJKHeight; }
    const SvxWeightItem&        GetCJKWeight() const    { return *aCJKWeight; }
    const SvxPostureItem&       GetCJKPosture() const   { return *aCJKPosture; }

    const SvxFontItem&          GetCTLFont() const      { return *aCTLFont; }
    const SvxFontHeightItem&    GetCTLHeight() const    { return *aCTLHeight; }
    const SvxWeightItem&        GetCTLWeight() const    { return *aCTLWeight; }
    const SvxPostureItem&       GetCTLPosture() const   { return *aCTLPosture; }

    const SvxUnderlineItem&     GetUnderline() const    { return *aUnderline; }
    const SvxOverlineItem&      GetOverline() const     { return *aOverline; }
    const SvxCrossedOutItem&    GetCrossedOut() const   { return *aCrossedOut; }
    const SvxContourItem&       GetContour() const      { return *aContour; }
    const SvxShadowedItem&      GetShadowed() const     { return *aShadowed; }
    const SvxColorItem&         GetColor() const        { return *aColor; }
    const SvxBoxItem&           GetBox() const          { return *aBox; }
    const SvxLineItem&          GetTLBR() const         { return *aTLBR; }
    const SvxLineItem&          GetBLTR() const         { return *aBLTR; }
    const SvxBrushItem&         GetBackground() const   { return *aBackground; }

    // Writer specific
    const SvxAdjustItem&        GetAdjust() const       { return *aAdjust; }
    const AutoFormatSwBlob&     GetAutoFormatSwBlob() const { return m_swFields; }

    // Calc specific
    const SvxHorJustifyItem&    GetHorJustify() const   { return *aHorJustify; }
    const SvxVerJustifyItem&    GetVerJustify() const   { return *aVerJustify; }
    const SfxBoolItem&          GetStacked() const      { return *aStacked; }
    const SvxMarginItem&        GetMargin() const       { return *aMargin; }
    const SfxBoolItem&          GetLinebreak() const    { return *aLinebreak; }

    // from SO5, 504k on, rotated text
    const SfxInt32Item&         GetRotateAngle() const  { return *aRotateAngle; }
    const SvxRotateModeItem&    GetRotateMode() const   { return *aRotateMode; }

    // number format
    const ScNumFormatAbbrev&    GetNumFormat() const    { return aNumFormat; }

    void    SetFont( const SvxFontItem& rFont )                     { aFont.reset(static_cast<SvxFontItem*>(rFont.Clone())); }
    void    SetHeight( const SvxFontHeightItem& rHeight )           { aHeight.reset(static_cast<SvxFontHeightItem*>(rHeight.Clone())); }
    void    SetWeight( const SvxWeightItem& rWeight )               { aWeight.reset(static_cast<SvxWeightItem*>(rWeight.Clone())); }
    void    SetPosture( const SvxPostureItem& rPosture )            { aPosture.reset(static_cast<SvxPostureItem*>(rPosture.Clone())); }

    void    SetCJKFont( const SvxFontItem& rCJKFont )               { aCJKFont.reset(static_cast<SvxFontItem*>(rCJKFont.Clone())); }
    void    SetCJKHeight( const SvxFontHeightItem& rCJKHeight )     { aCJKHeight.reset(static_cast<SvxFontHeightItem*>(rCJKHeight.Clone())); }
    void    SetCJKWeight( const SvxWeightItem& rCJKWeight )         { aCJKWeight.reset(static_cast<SvxWeightItem*>(rCJKWeight.Clone())); }
    void    SetCJKPosture( const SvxPostureItem& rCJKPosture )      { aCJKPosture.reset(static_cast<SvxPostureItem*>(rCJKPosture.Clone())); }

    void    SetCTLFont( const SvxFontItem& rCTLFont )               { aCTLFont.reset(static_cast<SvxFontItem*>(rCTLFont.Clone())); }
    void    SetCTLHeight( const SvxFontHeightItem& rCTLHeight )     { aCTLHeight.reset(static_cast<SvxFontHeightItem*>(rCTLHeight.Clone())); }
    void    SetCTLWeight( const SvxWeightItem& rCTLWeight )         { aCTLWeight.reset(static_cast<SvxWeightItem*>(rCTLWeight.Clone())); }
    void    SetCTLPosture( const SvxPostureItem& rCTLPosture )      { aCTLPosture.reset(static_cast<SvxPostureItem*>(rCTLPosture.Clone())); }

    void    SetUnderline( const SvxUnderlineItem& rUnderline )      { aUnderline.reset(static_cast<SvxUnderlineItem*>(rUnderline.Clone())); }
    void    SetOverline( const SvxOverlineItem& rOverline )         { aOverline.reset(static_cast<SvxOverlineItem*>(rOverline.Clone())); }
    void    SetCrossedOut( const SvxCrossedOutItem& rCrossedOut )   { aCrossedOut.reset(static_cast<SvxCrossedOutItem*>(rCrossedOut.Clone())); }
    void    SetContour( const SvxContourItem& rContour )            { aContour.reset(static_cast<SvxContourItem*>(rContour.Clone())); }
    void    SetShadowed( const SvxShadowedItem& rShadowed )         { aShadowed.reset(static_cast<SvxShadowedItem*>(rShadowed.Clone())); }
    void    SetColor( const SvxColorItem& rColor )                  { aColor.reset(static_cast<SvxColorItem*>(rColor.Clone())); }
    void    SetBox( const SvxBoxItem& rBox )                        { aBox.reset(static_cast<SvxBoxItem*>(rBox.Clone())); }
    void    SetTLBR( const SvxLineItem& rTLBR )                     { aTLBR.reset(static_cast<SvxLineItem*>(rTLBR.Clone())); }
    void    SetBLTR( const SvxLineItem& rBLTR )                     { aBLTR.reset(static_cast<SvxLineItem*>(rBLTR.Clone())); }
    void    SetBackground( const SvxBrushItem& rBackground )        { aBackground.reset(static_cast<SvxBrushItem*>(rBackground.Clone())); }

    // Writer specific
    void    SetAdjust( const SvxAdjustItem& rAdjust )               { aAdjust.reset(static_cast<SvxAdjustItem*>(rAdjust.Clone())); }
//    void    SetAutoFormatSwBlob( const AutoFormatSwBlob& r_swFields )   { m_swFields = r_swFields; }

    // Calc specific
    void    SetHorJustify( const SvxHorJustifyItem& rHorJustify )   { aHorJustify.reset(static_cast<SvxHorJustifyItem*>(rHorJustify.Clone())); }
    void    SetVerJustify( const SvxVerJustifyItem& rVerJustify )   { aVerJustify.reset(static_cast<SvxVerJustifyItem*>(rVerJustify.Clone())); }
    void    SetStacked( const SfxBoolItem& rStacked )               { aStacked.reset(static_cast<SfxBoolItem*>(rStacked.Clone())); }
    void    SetMargin( const SvxMarginItem& rMargin )               { aMargin.reset(static_cast<SvxMarginItem*>(rMargin.Clone())); }
    void    SetLinebreak( const SfxBoolItem& rLinebreak )           { aLinebreak.reset(static_cast<SfxBoolItem*>(rLinebreak.Clone())); }

    // from SO5, 504k on, rotated text
    void    SetRotateAngle( const SfxInt32Item& rRotateAngle )      { aRotateAngle.reset(static_cast<SfxInt32Item*>(rRotateAngle.Clone())); }
    void    SetRotateMode( const SvxRotateModeItem& rRotateMode )   { aRotateMode.reset(static_cast<SvxRotateModeItem*>(rRotateMode.Clone())); }

    // number format
    void    SetNumFormat( const ScNumFormatAbbrev& rNumFormat )     { aNumFormat = rNumFormat; }

    bool    Load( SvStream& rStream, const ScAfVersions& rVersions, sal_uInt16 nVer );
    bool    Save( SvStream& rStream, sal_uInt16 fileVersion );
};

class SC_DLLPUBLIC ScAutoFormatData
{
private:
    OUString               aName;
    sal_uInt16                  nStrResId;
    // common flags of Calc and Writer
    bool                        bIncludeFont : 1;
    bool                        bIncludeJustify : 1;
    bool                        bIncludeFrame : 1;
    bool                        bIncludeBackground : 1;

    // Calc specific flags
    bool                        bIncludeValueFormat : 1;
    bool                        bIncludeWidthHeight : 1;

    // Writer-specific data
    AutoFormatSwBlob m_swFields;

    std::array<std::unique_ptr<ScAutoFormatDataField>,16> ppDataField;

    SAL_DLLPRIVATE ScAutoFormatDataField&       GetField( sal_uInt16 nIndex );
    SAL_DLLPRIVATE const ScAutoFormatDataField& GetField( sal_uInt16 nIndex ) const;

public:
    ScAutoFormatData();
    ScAutoFormatData( const ScAutoFormatData& rData );
    ~ScAutoFormatData();

    void            SetName( const OUString& rName )              { aName = rName; nStrResId = USHRT_MAX; }
    const OUString& GetName() const { return aName; }

    bool            GetIncludeValueFormat() const               { return bIncludeValueFormat; }
    bool            GetIncludeFont() const                      { return bIncludeFont; }
    bool            GetIncludeJustify() const                   { return bIncludeJustify; }
    bool            GetIncludeFrame() const                     { return bIncludeFrame; }
    bool            GetIncludeBackground() const                { return bIncludeBackground; }
    bool            GetIncludeWidthHeight() const               { return bIncludeWidthHeight; }

    void            SetIncludeValueFormat( bool bValueFormat )  { bIncludeValueFormat = bValueFormat; }
    void            SetIncludeFont( bool bFont )                { bIncludeFont = bFont; }
    void            SetIncludeJustify( bool bJustify )          { bIncludeJustify = bJustify; }
    void            SetIncludeFrame( bool bFrame )              { bIncludeFrame = bFrame; }
    void            SetIncludeBackground( bool bBackground )    { bIncludeBackground = bBackground; }
    void            SetIncludeWidthHeight( bool bWidthHeight )  { bIncludeWidthHeight = bWidthHeight; }

    const SfxPoolItem*          GetItem( sal_uInt16 nIndex, sal_uInt16 nWhich ) const;
    template<class T> const T*  GetItem( sal_uInt16 nIndex, TypedWhichId<T> nWhich ) const
    {
        return static_cast<const T*>(GetItem(nIndex, sal_uInt16(nWhich)));
    }
    void                        PutItem( sal_uInt16 nIndex, const SfxPoolItem& rItem );
    void                        CopyItem( sal_uInt16 nToIndex, sal_uInt16 nFromIndex, sal_uInt16 nWhich );

    const ScNumFormatAbbrev&    GetNumFormat( sal_uInt16 nIndex ) const;

    bool                        IsEqualData( sal_uInt16 nIndex1, sal_uInt16 nIndex2 ) const;

    void                        FillToItemSet( sal_uInt16 nIndex, SfxItemSet& rItemSet, const ScDocument& rDoc ) const;
    void                        GetFromItemSet( sal_uInt16 nIndex, const SfxItemSet& rItemSet, const ScNumFormatAbbrev& rNumFormat );

    bool                        Load( SvStream& rStream, const ScAfVersions& rVersions );
    bool                        Save( SvStream& rStream, sal_uInt16 fileVersion );
};

struct DefaultFirstEntry {
    bool operator() (const OUString& left, const OUString& right) const;
};

class SC_DLLPUBLIC ScAutoFormat
{
    typedef std::map<OUString, std::unique_ptr<ScAutoFormatData>, DefaultFirstEntry> MapType;
    MapType m_Data;
    bool mbSaveLater;
    ScAfVersions m_aVersions;

    ScAutoFormat(const ScAutoFormat&) = delete;
    const ScAutoFormat operator=(const ScAutoFormat&) = delete;

public:
    typedef MapType::const_iterator const_iterator;
    typedef MapType::iterator iterator;

    ScAutoFormat();
    void Load();
    bool Save();

    void SetSaveLater( bool bSet );
    bool IsSaveLater() const { return mbSaveLater; }

    const ScAutoFormatData* findByIndex(size_t nIndex) const;
    ScAutoFormatData* findByIndex(size_t nIndex);
    iterator find(const OUString& rName);

    iterator insert(std::unique_ptr<ScAutoFormatData> pNew);
    void erase(const iterator& it);

    size_t size() const;
    const_iterator begin() const;
    const_iterator end() const;
    iterator begin();
    iterator end();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
