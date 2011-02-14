/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_AUTOFORM_HXX
#define SC_AUTOFORM_HXX

/*************************************************************************
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

JP 20.07.95:

    Die akt. Struktur der Autoformatierung darf nicht mehr veraendert werden.
    Diese wird durch unterschiedlichen Code vom StartWriter und vom StarCalc
    eingelesen/geschrieben.
    Sollte sich doch mal eine Aenderung nicht vermeiden lassen, dann auf
    jedenfall in beiden Applikationen aendern.

    The structure of table auto formatting should not changed. It is used
    by different code of Writer and Calc. If a change is necessary, the
    source code of both applications must be changed!

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
**************************************************************************/

#include "scitems.hxx"
#include <editeng/adjitem.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <svx/rotmodit.hxx>
#include <svl/intitem.hxx>
#include <editeng/bolnitem.hxx>
#include "scdllapi.h"
#include "collect.hxx"
#include "global.hxx"
#include "zforauto.hxx"


struct ScAfVersions;


/// Contains all items for one cell of a table autoformat.
class ScAutoFormatDataField
{
private:
    SvxFontItem                 aFont;
    SvxFontHeightItem           aHeight;
    SvxWeightItem               aWeight;
    SvxPostureItem              aPosture;

    SvxFontItem                 aCJKFont;
    SvxFontHeightItem           aCJKHeight;
    SvxWeightItem               aCJKWeight;
    SvxPostureItem              aCJKPosture;

    SvxFontItem                 aCTLFont;
    SvxFontHeightItem           aCTLHeight;
    SvxWeightItem               aCTLWeight;
    SvxPostureItem              aCTLPosture;

    SvxUnderlineItem            aUnderline;
    SvxOverlineItem             aOverline;
    SvxCrossedOutItem           aCrossedOut;
    SvxContourItem              aContour;
    SvxShadowedItem             aShadowed;
    SvxColorItem                aColor;
    SvxBoxItem                  aBox;
    SvxLineItem                 aTLBR;
    SvxLineItem                 aBLTR;
    SvxBrushItem                aBackground;

    // Writer specific
    SvxAdjustItem               aAdjust;

    // Calc specific
    SvxHorJustifyItem           aHorJustify;
    SvxVerJustifyItem           aVerJustify;
    SfxBoolItem                 aStacked;
    SvxMarginItem               aMargin;
    SfxBoolItem                 aLinebreak;
    // from SO5, 504k on, rotated text
    SfxInt32Item                aRotateAngle;
    SvxRotateModeItem           aRotateMode;

    // number format
    ScNumFormatAbbrev           aNumFormat;

public:
                                ScAutoFormatDataField();
                                ScAutoFormatDataField( const ScAutoFormatDataField& rCopy );
                                ~ScAutoFormatDataField();

    const ScNumFormatAbbrev&    GetNumFormat() const    { return aNumFormat; }
    const SvxFontItem&          GetFont() const         { return aFont; }
    const SvxFontHeightItem&    GetHeight() const       { return aHeight; }
    const SvxWeightItem&        GetWeight() const       { return aWeight; }
    const SvxPostureItem&       GetPosture() const      { return aPosture; }
    const SvxFontItem&          GetCJKFont() const      { return aCJKFont; }
    const SvxFontHeightItem&    GetCJKHeight() const    { return aCJKHeight; }
    const SvxWeightItem&        GetCJKWeight() const    { return aCJKWeight; }
    const SvxPostureItem&       GetCJKPosture() const   { return aCJKPosture; }
    const SvxFontItem&          GetCTLFont() const      { return aCTLFont; }
    const SvxFontHeightItem&    GetCTLHeight() const    { return aCTLHeight; }
    const SvxWeightItem&        GetCTLWeight() const    { return aCTLWeight; }
    const SvxPostureItem&       GetCTLPosture() const   { return aCTLPosture; }
    const SvxUnderlineItem&     GetUnderline() const    { return aUnderline; }
    const SvxOverlineItem&      GetOverline() const     { return aOverline; }
    const SvxCrossedOutItem&    GetCrossedOut() const   { return aCrossedOut; }
    const SvxContourItem&       GetContour() const      { return aContour; }
    const SvxShadowedItem&      GetShadowed() const     { return aShadowed; }
    const SvxColorItem&         GetColor() const        { return aColor; }
    const SvxHorJustifyItem&    GetHorJustify() const   { return aHorJustify; }
    const SvxVerJustifyItem&    GetVerJustify() const   { return aVerJustify; }
    const SfxBoolItem&          GetStacked() const      { return aStacked; }
    const SfxBoolItem&          GetLinebreak() const    { return aLinebreak; }
    const SvxMarginItem&        GetMargin() const       { return aMargin; }
    const SvxBoxItem&           GetBox() const          { return aBox; }
    const SvxLineItem&          GetTLBR() const         { return aTLBR; }
    const SvxLineItem&          GetBLTR() const         { return aBLTR; }
    const SvxBrushItem&         GetBackground() const   { return aBackground; }
    const SvxAdjustItem&        GetAdjust() const       { return aAdjust; }
    const SfxInt32Item&         GetRotateAngle() const  { return aRotateAngle; }
    const SvxRotateModeItem&    GetRotateMode() const   { return aRotateMode; }

    void    SetNumFormat( const ScNumFormatAbbrev& rNumFormat )     { aNumFormat = rNumFormat; }
    void    SetFont( const SvxFontItem& rFont )                     { aFont = rFont; }
    void    SetHeight( const SvxFontHeightItem& rHeight )           { aHeight = rHeight; }
    void    SetWeight( const SvxWeightItem& rWeight )               { aWeight = rWeight; }
    void    SetPosture( const SvxPostureItem& rPosture )            { aPosture = rPosture; }
    void    SetCJKFont( const SvxFontItem& rCJKFont )               { aCJKFont = rCJKFont; }
    void    SetCJKHeight( const SvxFontHeightItem& rCJKHeight )     { aCJKHeight = rCJKHeight; }
    void    SetCJKWeight( const SvxWeightItem& rCJKWeight )         { aCJKWeight = rCJKWeight; }
    void    SetCJKPosture( const SvxPostureItem& rCJKPosture )      { aCJKPosture = rCJKPosture; }
    void    SetCTLFont( const SvxFontItem& rCTLFont )               { aCTLFont = rCTLFont; }
    void    SetCTLHeight( const SvxFontHeightItem& rCTLHeight )     { aCTLHeight = rCTLHeight; }
    void    SetCTLWeight( const SvxWeightItem& rCTLWeight )         { aCTLWeight = rCTLWeight; }
    void    SetCTLPosture( const SvxPostureItem& rCTLPosture )      { aCTLPosture = rCTLPosture; }
    void    SetUnderline( const SvxUnderlineItem& rUnderline )      { aUnderline = rUnderline; }
    void    SetOverline( const SvxOverlineItem& rOverline )         { aOverline = rOverline; }
    void    SetCrossedOut( const SvxCrossedOutItem& rCrossedOut )   { aCrossedOut = rCrossedOut; }
    void    SetContour( const SvxContourItem& rContour )            { aContour = rContour; }
    void    SetShadowed( const SvxShadowedItem& rShadowed )         { aShadowed = rShadowed; }
    void    SetColor( const SvxColorItem& rColor )                  { aColor = rColor; }
    void    SetHorJustify( const SvxHorJustifyItem& rHorJustify )   { aHorJustify = rHorJustify; }
    void    SetVerJustify( const SvxVerJustifyItem& rVerJustify )   { aVerJustify = rVerJustify; }
    void    SetStacked( const SfxBoolItem& rStacked )               { aStacked.SetValue( rStacked.GetValue() ); }
    void    SetLinebreak( const SfxBoolItem& rLinebreak )           { aLinebreak.SetValue( rLinebreak.GetValue() ); }
    void    SetMargin( const SvxMarginItem& rMargin )               { aMargin = rMargin; }
    void    SetBox( const SvxBoxItem& rBox )                        { aBox = rBox; }
    void    SetTLBR( const SvxLineItem& rTLBR )                     { aTLBR = rTLBR; }
    void    SetBLTR( const SvxLineItem& rBLTR )                     { aBLTR = rBLTR; }
    void    SetBackground( const SvxBrushItem& rBackground )        { aBackground = rBackground; }
    void    SetAdjust( const SvxAdjustItem& rAdjust );
    void    SetRotateAngle( const SfxInt32Item& rRotateAngle )      { aRotateAngle.SetValue( rRotateAngle.GetValue() ); }
    void    SetRotateMode( const SvxRotateModeItem& rRotateMode )   { aRotateMode.SetValue( rRotateMode.GetValue() ); }

    sal_Bool                        Load( SvStream& rStream, const ScAfVersions& rVersions, sal_uInt16 nVer );
    sal_Bool                        Save( SvStream& rStream );

#ifdef READ_OLDVERS
    sal_Bool                        LoadOld( SvStream& rStream, const ScAfVersions& rVersions );
#endif
};


class SC_DLLPUBLIC ScAutoFormatData : public ScDataObject
{
private:
    String                      aName;
    sal_uInt16                      nStrResId;
    // common flags of Calc and Writer
    sal_Bool                        bIncludeFont : 1;
    sal_Bool                        bIncludeJustify : 1;
    sal_Bool                        bIncludeFrame : 1;
    sal_Bool                        bIncludeBackground : 1;

    // Calc specific flags
    sal_Bool                        bIncludeValueFormat : 1;
    sal_Bool                        bIncludeWidthHeight : 1;

    ScAutoFormatDataField**     ppDataField;

    SC_DLLPRIVATE ScAutoFormatDataField&       GetField( sal_uInt16 nIndex );
    SC_DLLPRIVATE const ScAutoFormatDataField& GetField( sal_uInt16 nIndex ) const;

public:
                    ScAutoFormatData();
                    ScAutoFormatData( const ScAutoFormatData& rData );
    virtual         ~ScAutoFormatData();

    virtual         ScDataObject* Clone() const { return new ScAutoFormatData( *this ); }

    void            SetName( const String& rName )              { aName = rName; nStrResId = USHRT_MAX; }
    void            GetName( String& rName ) const              { rName = aName; }

    sal_Bool            GetIncludeValueFormat() const               { return bIncludeValueFormat; }
    sal_Bool            GetIncludeFont() const                      { return bIncludeFont; }
    sal_Bool            GetIncludeJustify() const                   { return bIncludeJustify; }
    sal_Bool            GetIncludeFrame() const                     { return bIncludeFrame; }
    sal_Bool            GetIncludeBackground() const                { return bIncludeBackground; }
    sal_Bool            GetIncludeWidthHeight() const               { return bIncludeWidthHeight; }

    void            SetIncludeValueFormat( sal_Bool bValueFormat )  { bIncludeValueFormat = bValueFormat; }
    void            SetIncludeFont( sal_Bool bFont )                { bIncludeFont = bFont; }
    void            SetIncludeJustify( sal_Bool bJustify )          { bIncludeJustify = bJustify; }
    void            SetIncludeFrame( sal_Bool bFrame )              { bIncludeFrame = bFrame; }
    void            SetIncludeBackground( sal_Bool bBackground )    { bIncludeBackground = bBackground; }
    void            SetIncludeWidthHeight( sal_Bool bWidthHeight )  { bIncludeWidthHeight = bWidthHeight; }

    const SfxPoolItem*          GetItem( sal_uInt16 nIndex, sal_uInt16 nWhich ) const;
    void                        PutItem( sal_uInt16 nIndex, const SfxPoolItem& rItem );
    void                        CopyItem( sal_uInt16 nToIndex, sal_uInt16 nFromIndex, sal_uInt16 nWhich );

    const ScNumFormatAbbrev&    GetNumFormat( sal_uInt16 nIndex ) const;

    sal_Bool                        IsEqualData( sal_uInt16 nIndex1, sal_uInt16 nIndex2 ) const;

    void                        FillToItemSet( sal_uInt16 nIndex, SfxItemSet& rItemSet, ScDocument& rDoc ) const;
    void                        GetFromItemSet( sal_uInt16 nIndex, const SfxItemSet& rItemSet, const ScNumFormatAbbrev& rNumFormat );

    sal_Bool                        Load( SvStream& rStream, const ScAfVersions& rVersions );
    sal_Bool                        Save( SvStream& rStream );

#ifdef READ_OLDVERS
    sal_Bool                        LoadOld( SvStream& rStream, const ScAfVersions& rVersions );
#endif
};

class SC_DLLPUBLIC ScAutoFormat : public ScSortedCollection
{
private:
    sal_Bool                        bSaveLater;

public:
                                ScAutoFormat( sal_uInt16 nLim = 4, sal_uInt16 nDel = 4, sal_Bool bDup = sal_False );
                                ScAutoFormat( const ScAutoFormat& AutoFormat );
    virtual                     ~ScAutoFormat();
    virtual                     ScDataObject*         Clone() const { return new ScAutoFormat( *this ); }
                                ScAutoFormatData*   operator[]( const sal_uInt16 nIndex ) const {return (ScAutoFormatData*)At( nIndex );}
    virtual short               Compare( ScDataObject* pKey1, ScDataObject* pKey2 ) const;
    sal_Bool                        Load();
    sal_Bool                        Save();
    sal_uInt16                      FindIndexPerName( const String& rName ) const;
    void                        SetSaveLater( sal_Bool bSet );
    sal_Bool                        IsSaveLater() const         { return bSaveLater; }
};


#endif
