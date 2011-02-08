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
#ifndef SW_TGRDITEM_HXX
#define SW_TGRDITEM_HXX

#include <tools/color.hxx>
#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <format.hxx>

class IntlWrapper;

enum SwTextGrid { GRID_NONE, GRID_LINES_ONLY, GRID_LINES_CHARS };

class SW_DLLPUBLIC SwTextGridItem : public SfxPoolItem
{
    Color aColor;
    sal_uInt16 nLines;
    sal_uInt16 nBaseHeight, nRubyHeight;
    SwTextGrid eGridType;
    sal_Bool bRubyTextBelow;
    sal_Bool bPrintGrid;
    sal_Bool bDisplayGrid;

    //for textgrid enhancement
    sal_uInt16 nBaseWidth;
    sal_Bool bSnapToChars;
    sal_Bool bSquaredMode;
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
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

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

    sal_Bool IsRubyTextBelow() const { return bRubyTextBelow; }
    sal_Bool GetRubyTextBelow() const { return bRubyTextBelow; }
    void SetRubyTextBelow( sal_Bool bNew ) { bRubyTextBelow = bNew; }

    sal_Bool IsPrintGrid() const { return bPrintGrid; }
    sal_Bool GetPrintGrid() const { return bPrintGrid; }
    void SetPrintGrid( sal_Bool bNew ) { bPrintGrid = bNew; }

    sal_Bool IsDisplayGrid() const { return bDisplayGrid; }
    sal_Bool GetDisplayGrid() const { return bDisplayGrid; }
    void SetDisplayGrid( sal_Bool bNew ) { bDisplayGrid = bNew; }

    //for textgrid enhancement
    sal_uInt16 GetBaseWidth() const { return nBaseWidth;}
    void SetBaseWidth( sal_uInt16 nNew ) { nBaseWidth = nNew; }

    sal_Bool IsSnapToChars() const { return bSnapToChars; }
    sal_Bool GetSnapToChars() const { return bSnapToChars; }
    void SetSnapToChars( sal_Bool bNew ) { bSnapToChars = bNew; }

    sal_Bool IsSquaredMode() const { return bSquaredMode; }
    sal_Bool GetSquaredMode() const { return bSquaredMode; }
    void SetSquaredMode( sal_Bool bNew ) { bSquaredMode = bNew; }
    void SwitchPaperMode(sal_Bool bNew );

    void Init();
};

inline const SwTextGridItem &SwAttrSet::GetTextGrid(sal_Bool bInP) const
    {   return (const SwTextGridItem&)Get( RES_TEXTGRID, bInP ); }
inline const SwTextGridItem &SwFmt::GetTextGrid(sal_Bool bInP) const
    {   return (const SwTextGridItem&)aSet.Get( RES_TEXTGRID, bInP ); }

#endif

