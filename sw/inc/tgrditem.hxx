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
#ifndef INCLUDED_SW_INC_TGRDITEM_HXX
#define INCLUDED_SW_INC_TGRDITEM_HXX

#include <tools/color.hxx>
#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <format.hxx>

class IntlWrapper;

enum SwTextGrid { GRID_NONE, GRID_LINES_ONLY, GRID_LINES_CHARS };

class SW_DLLPUBLIC SwTextGridItem : public SfxPoolItem
{
private:
    Color m_aColor;
    sal_uInt16 m_nLines;
    sal_uInt16 m_nBaseHeight;
    sal_uInt16 m_nRubyHeight;
    SwTextGrid m_eGridType;
    sal_Bool m_bRubyTextBelow;
    sal_Bool m_bPrintGrid;
    sal_Bool m_bDisplayGrid;

    //for textgrid enhancement
    sal_uInt16 m_nBaseWidth;
    sal_Bool m_bSnapToChars;
    sal_Bool m_bSquaredMode;

public:
    SwTextGridItem();
    virtual ~SwTextGridItem();

    // "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const SAL_OVERRIDE;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const SAL_OVERRIDE;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

    SwTextGridItem&  operator=( const SwTextGridItem& );

    const Color& GetColor() const { return m_aColor; }
    void SetColor( const Color& rCol )  { m_aColor = rCol; }

    sal_uInt16 GetLines() const { return m_nLines; }
    void SetLines( sal_uInt16 nNew ) { m_nLines = nNew; }

    sal_uInt16 GetBaseHeight() const { return m_nBaseHeight; }
    void SetBaseHeight( sal_uInt16 nNew ) { m_nBaseHeight = nNew; }

    sal_uInt16 GetRubyHeight() const { return m_nRubyHeight; }
    void SetRubyHeight( sal_uInt16 nNew ) { m_nRubyHeight = nNew; }

    SwTextGrid GetGridType() const { return m_eGridType; }
    void SetGridType( SwTextGrid eNew ) { m_eGridType = eNew; }

    sal_Bool IsRubyTextBelow() const { return m_bRubyTextBelow; }
    sal_Bool GetRubyTextBelow() const { return m_bRubyTextBelow; }
    void SetRubyTextBelow( sal_Bool bNew ) { m_bRubyTextBelow = bNew; }

    sal_Bool IsPrintGrid() const { return m_bPrintGrid; }
    sal_Bool GetPrintGrid() const { return m_bPrintGrid; }
    void SetPrintGrid( sal_Bool bNew ) { m_bPrintGrid = bNew; }

    sal_Bool IsDisplayGrid() const { return m_bDisplayGrid; }
    sal_Bool GetDisplayGrid() const { return m_bDisplayGrid; }
    void SetDisplayGrid( sal_Bool bNew ) { m_bDisplayGrid = bNew; }

    //for textgrid enhancement
    sal_uInt16 GetBaseWidth() const { return m_nBaseWidth;}
    void SetBaseWidth( sal_uInt16 nNew ) { m_nBaseWidth = nNew; }

    sal_Bool IsSnapToChars() const { return m_bSnapToChars; }
    sal_Bool GetSnapToChars() const { return m_bSnapToChars; }
    void SetSnapToChars( sal_Bool bNew ) { m_bSnapToChars = bNew; }

    sal_Bool IsSquaredMode() const { return m_bSquaredMode; }
    sal_Bool GetSquaredMode() const { return m_bSquaredMode; }
    void SetSquaredMode( sal_Bool bNew ) { m_bSquaredMode = bNew; }
    void SwitchPaperMode(sal_Bool bNew );

    void Init();
};

inline const SwTextGridItem &SwAttrSet::GetTextGrid(sal_Bool bInP) const
    {   return (const SwTextGridItem&)Get( RES_TEXTGRID, bInP ); }
inline const SwTextGridItem &SwFmt::GetTextGrid(sal_Bool bInP) const
    {   return (const SwTextGridItem&)aSet.Get( RES_TEXTGRID, bInP ); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
