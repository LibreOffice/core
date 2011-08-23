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


#ifndef _SFXPOOLITEM_HXX //autogen
#include <bf_svtools/poolitem.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
class IntlWrapper; 
namespace binfilter {



enum SwTextGrid { GRID_NONE, GRID_LINES_ONLY, GRID_LINES_CHARS };

class SwTextGridItem : public SfxPoolItem
{
    Color aColor;
    sal_uInt16 nLines;
    sal_uInt16 nBaseHeight, nRubyHeight;
    SwTextGrid eGridType;
    BOOL bRubyTextBelow;
    BOOL bPrintGrid;
    BOOL bDisplayGrid;

public:
    SwTextGridItem();
    virtual ~SwTextGridItem();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT nVer) const;
    virtual SvStream&       Store(SvStream &, USHORT nIVer) const;
    virtual USHORT			 GetVersion( USHORT nFFVer ) const;
    virtual BOOL             QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL             PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    SwTextGridItem&  operator=( const SwTextGridItem& );

    const Color& GetColor() const { return aColor; }
    void SetColor( const Color& rCol ) 	{ aColor = rCol; }

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
};

inline const SwTextGridItem &SwAttrSet::GetTextGrid(BOOL bInP) const
    {   return (const SwTextGridItem&)Get( RES_TEXTGRID, bInP ); }
inline const SwTextGridItem &SwFmt::GetTextGrid(BOOL bInP) const
    {   return (const SwTextGridItem&)aSet.Get( RES_TEXTGRID, bInP ); }

} //namespace binfilter
#endif

