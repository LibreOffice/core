/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <bf_svx/adjitem.hxx>
#include <bf_svx/algitem.hxx>
#include <bf_svx/boxitem.hxx>
#include <bf_svx/brshitem.hxx>
#include <bf_svx/cntritem.hxx>
#include <bf_svx/colritem.hxx>
#include <bf_svx/crsditem.hxx>
#include <bf_svx/fhgtitem.hxx>
#include <bf_svx/fontitem.hxx>
#include <bf_svx/postitem.hxx>
#include <bf_svx/shdditem.hxx>
#include <bf_svx/udlnitem.hxx>
#include <bf_svx/wghtitem.hxx>
#include <bf_svx/rotmodit.hxx>
#include <bf_svtools/intitem.hxx>

#include "collect.hxx"
#include "global.hxx"
#include "zforauto.hxx"

namespace binfilter {

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
     SvxCrossedOutItem           aCrossedOut;
     SvxContourItem              aContour;
     SvxShadowedItem             aShadowed;
     SvxColorItem                aColor;
     SvxBoxItem                  aBox;
     SvxBrushItem                aBackground;
 
     // Writer specific
     SvxAdjustItem               aAdjust;
 
     // Calc specific
     SvxHorJustifyItem           aHorJustify;
     SvxVerJustifyItem           aVerJustify;
     SvxOrientationItem          aOrientation;
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
     const SvxCrossedOutItem&    GetCrossedOut() const   { return aCrossedOut; }
     const SvxContourItem&       GetContour() const      { return aContour; }
     const SvxShadowedItem&      GetShadowed() const     { return aShadowed; }
     const SvxColorItem&         GetColor() const        { return aColor; }
     const SvxHorJustifyItem&    GetHorJustify() const   { return aHorJustify; }
     const SvxVerJustifyItem&    GetVerJustify() const   { return aVerJustify; }
     const SvxOrientationItem&   GetOrientation() const  { return aOrientation; }
     const SfxBoolItem&          GetLinebreak() const    { return aLinebreak; }
     const SvxMarginItem&        GetMargin() const       { return aMargin; }
     const SvxBoxItem&           GetBox() const          { return aBox; }
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
     void    SetCrossedOut( const SvxCrossedOutItem& rCrossedOut )   { aCrossedOut = rCrossedOut; }
     void    SetContour( const SvxContourItem& rContour )            { aContour = rContour; }
     void    SetShadowed( const SvxShadowedItem& rShadowed )         { aShadowed = rShadowed; }
     void    SetColor( const SvxColorItem& rColor )                  { aColor = rColor; }
     void    SetHorJustify( const SvxHorJustifyItem& rHorJustify )   { aHorJustify = rHorJustify; }
     void    SetVerJustify( const SvxVerJustifyItem& rVerJustify )   { aVerJustify = rVerJustify; }
     void    SetOrientation( const SvxOrientationItem& rOrient )     { aOrientation = rOrient; }
     void    SetLinebreak( const SfxBoolItem& rLinebreak )           { aLinebreak.SetValue( rLinebreak.GetValue() ); }
     void    SetMargin( const SvxMarginItem& rMargin )               { aMargin = rMargin; }
     void    SetBox( const SvxBoxItem& rBox )                        { aBox = rBox; }
     void    SetBackground( const SvxBrushItem& rBackground )        { aBackground = rBackground; }
     void    SetAdjust( const SvxAdjustItem& rAdjust );
     void    SetRotateAngle( const SfxInt32Item& rRotateAngle )      { aRotateAngle.SetValue( rRotateAngle.GetValue() ); }
     void    SetRotateMode( const SvxRotateModeItem& rRotateMode )   { aRotateMode.SetValue( rRotateMode.GetValue() ); }
 
    BOOL                        Load( SvStream& rStream, const ScAfVersions& rVersions, USHORT nVer );
    BOOL                        Save( SvStream& rStream );

#ifdef READ_OLDVERS
    BOOL                        LoadOld( SvStream& rStream, const ScAfVersions& rVersions );
#endif
};


class ScAutoFormatData : public DataObject
{
private:
    String                      aName;
    USHORT                      nStrResId;
    // common flags of Calc and Writer
    BOOL                        bIncludeFont : 1;
    BOOL                        bIncludeJustify : 1;
    BOOL                        bIncludeFrame : 1;
    BOOL                        bIncludeBackground : 1;

    // Calc specific flags
    BOOL                        bIncludeValueFormat : 1;
    BOOL                        bIncludeWidthHeight : 1;

    ScAutoFormatDataField**     ppDataField;

    ScAutoFormatDataField&      GetField( USHORT nIndex );
    const ScAutoFormatDataField& GetField( USHORT nIndex ) const;

public:
                    ScAutoFormatData();
                    ScAutoFormatData( const ScAutoFormatData& rData );
    virtual         ~ScAutoFormatData();

    virtual         DataObject* Clone() const { return new ScAutoFormatData( *this ); }

    void            SetName( const String& rName )              { aName = rName; nStrResId = USHRT_MAX; }
    void            GetName( String& rName ) const              { rName = aName; }

    BOOL            GetIncludeValueFormat() const               { return bIncludeValueFormat; }
    BOOL            GetIncludeFont() const                      { return bIncludeFont; }
    BOOL            GetIncludeJustify() const                   { return bIncludeJustify; }
    BOOL            GetIncludeFrame() const                     { return bIncludeFrame; }
    BOOL            GetIncludeBackground() const                { return bIncludeBackground; }
    BOOL            GetIncludeWidthHeight() const               { return bIncludeWidthHeight; }

    void            SetIncludeValueFormat( BOOL bValueFormat )  { bIncludeValueFormat = bValueFormat; }
    void            SetIncludeFont( BOOL bFont )                { bIncludeFont = bFont; }
    void            SetIncludeJustify( BOOL bJustify )          { bIncludeJustify = bJustify; }
    void            SetIncludeFrame( BOOL bFrame )              { bIncludeFrame = bFrame; }
    void            SetIncludeBackground( BOOL bBackground )    { bIncludeBackground = bBackground; }
    void            SetIncludeWidthHeight( BOOL bWidthHeight )  { bIncludeWidthHeight = bWidthHeight; }

    const SfxPoolItem*          GetItem( USHORT nIndex, USHORT nWhich ) const;
    void                        PutItem( USHORT nIndex, const SfxPoolItem& rItem );
    BOOL                        Load( SvStream& rStream, const ScAfVersions& rVersions );
    BOOL                        Save( SvStream& rStream );

#ifdef READ_OLDVERS
    BOOL                        LoadOld( SvStream& rStream, const ScAfVersions& rVersions );
#endif
};

class ScAutoFormat : public SortedCollection
{
private:
    BOOL                        bSaveLater;

public:
                                ScAutoFormat( USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE );
                                ScAutoFormat( const ScAutoFormat& AutoFormat );
    virtual                     ~ScAutoFormat();
    virtual                     DataObject*         Clone() const { return new ScAutoFormat( *this ); }
                                ScAutoFormatData*   operator[]( const USHORT nIndex ) const {return (ScAutoFormatData*)At( nIndex );}
    virtual short               Compare( DataObject* pKey1, DataObject* pKey2 ) const{DBG_BF_ASSERT(0, "STRIP"); return 0;} //STRIP001 virtual short               Compare( DataObject* pKey1, DataObject* pKey2 ) const;
    BOOL                        Load();
    BOOL                        Save();
    void                        SetSaveLater( BOOL bSet );
    BOOL                        IsSaveLater() const         { return bSaveLater; }
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
