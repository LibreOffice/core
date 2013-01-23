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

    // "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
