/*************************************************************************
 *
 *  $RCSfile: autoform.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:50:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef _SVX_ADJITEM_HXX
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_ALGITEM_HXX
#include <svx/algitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_ITEM_HXX
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_ROTMODIT_HXX
#include <svx/rotmodit.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SVX_BOLNITEM_HXX
#include <svx/bolnitem.hxx>
#endif

#ifndef SC_COLLECT_HXX
#include "collect.hxx"
#endif
#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef _ZFORAUTO_HXX_
#include "zforauto.hxx"
#endif


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
    void                        CopyItem( USHORT nToIndex, USHORT nFromIndex, USHORT nWhich );

    const ScNumFormatAbbrev&    GetNumFormat( USHORT nIndex ) const;
    void                        SetNumFormat( USHORT nIndex, const ScNumFormatAbbrev& rNumFormat );

    BOOL                        IsEqualData( USHORT nIndex1, USHORT nIndex2 ) const;

    void                        FillToItemSet( USHORT nIndex, SfxItemSet& rItemSet, ScDocument& rDoc ) const;
    void                        GetFromItemSet( USHORT nIndex, const SfxItemSet& rItemSet, const ScNumFormatAbbrev& rNumFormat );

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
    virtual short               Compare( DataObject* pKey1, DataObject* pKey2 ) const;
    BOOL                        Load();
    BOOL                        Save();
    USHORT                      FindIndexPerName( const String& rName ) const;
    void                        SetSaveLater( BOOL bSet );
    BOOL                        IsSaveLater() const         { return bSaveLater; }
};


#endif

