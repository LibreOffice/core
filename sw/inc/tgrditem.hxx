/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tgrditem.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 16:23:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SW_TGRDITEM_HXX
#define SW_TGRDITEM_HXX

#include <tools/color.hxx>

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif

class IntlWrapper;

enum SwTextGrid { GRID_NONE, GRID_LINES_ONLY, GRID_LINES_CHARS };

class SW_DLLPUBLIC SwTextGridItem : public SfxPoolItem
{
    Color aColor;
    sal_uInt16 nLines;
    sal_uInt16 nBaseHeight, nRubyHeight;
    SwTextGrid eGridType;
    BOOL bRubyTextBelow;
    BOOL bPrintGrid;
    BOOL bDisplayGrid;

    //for textgrid enhancement
    sal_uInt16 nBaseWidth;
    BOOL bSnapToChars;
    BOOL bSquaredMode;
public:
    SwTextGridItem();
    virtual ~SwTextGridItem();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
    virtual BOOL             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    SwTextGridItem&  operator=( const SwTextGridItem& );

    const Color& GetColor() const { return aColor; }
    void SetColor( const Color& rCol )  { aColor = rCol; }

    sal_uInt16 GetLines() const { return nLines; }
    void SetLines( sal_uInt16 nNew ) { nLines = nNew; }

    sal_uInt16 GetBaseHeight() const { return nBaseHeight; }
    void SetBaseHeight( sal_uInt16 nNew ) { nBaseHeight = nNew; }

    sal_uInt16 GetRubyHeight() const { return nRubyHeight; }
    void SetRubyHeight( sal_uInt16 nNew ) { nRubyHeight = nNew; }

    SwTextGrid GetGridType() const { return eGridType; }
    void SetGridType( SwTextGrid eNew ) { eGridType = eNew; }

    BOOL IsRubyTextBelow() const { return bRubyTextBelow; }
    BOOL GetRubyTextBelow() const { return bRubyTextBelow; }
    void SetRubyTextBelow( BOOL bNew ) { bRubyTextBelow = bNew; }

    BOOL IsPrintGrid() const { return bPrintGrid; }
    BOOL GetPrintGrid() const { return bPrintGrid; }
    void SetPrintGrid( BOOL bNew ) { bPrintGrid = bNew; }

    BOOL IsDisplayGrid() const { return bDisplayGrid; }
    BOOL GetDisplayGrid() const { return bDisplayGrid; }
    void SetDisplayGrid( BOOL bNew ) { bDisplayGrid = bNew; }

    //for textgrid enhancement
    sal_uInt16 GetBaseWidth() const { return nBaseWidth;}
    void SetBaseWidth( sal_uInt16 nNew ) { nBaseWidth = nNew; }

    BOOL IsSnapToChars() const { return bSnapToChars; }
    BOOL GetSnapToChars() const { return bSnapToChars; }
    void SetSnapToChars( BOOL bNew ) { bSnapToChars = bNew; }

    BOOL IsSquaredMode() const { return bSquaredMode; }
    BOOL GetSquaredMode() const { return bSquaredMode; }
    void SetSquaredMode( BOOL bNew ) { bSquaredMode = bNew; }
    void SwitchPaperMode(BOOL bNew );

    void Init();
};

inline const SwTextGridItem &SwAttrSet::GetTextGrid(BOOL bInP) const
    {   return (const SwTextGridItem&)Get( RES_TEXTGRID, bInP ); }
inline const SwTextGridItem &SwFmt::GetTextGrid(BOOL bInP) const
    {   return (const SwTextGridItem&)aSet.Get( RES_TEXTGRID, bInP ); }

#endif

