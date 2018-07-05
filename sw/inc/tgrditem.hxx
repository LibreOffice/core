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
#include "hintids.hxx"
#include "format.hxx"

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
    bool m_bRubyTextBelow;
    bool m_bPrintGrid;
    bool m_bDisplayGrid;

    //for textgrid enhancement
    sal_uInt16 m_nBaseWidth;
    bool m_bSnapToChars;
    bool m_bSquaredMode;

public:
    SwTextGridItem();
    virtual ~SwTextGridItem() override;

    SwTextGridItem(SwTextGridItem const &) = default;
    SwTextGridItem(SwTextGridItem &&) = default;
    SwTextGridItem & operator =(SwTextGridItem const &) = default;
    SwTextGridItem & operator =(SwTextGridItem &&) = default;

    // "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

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

    bool IsRubyTextBelow() const { return m_bRubyTextBelow; }
    bool GetRubyTextBelow() const { return m_bRubyTextBelow; }
    void SetRubyTextBelow( bool bNew ) { m_bRubyTextBelow = bNew; }

    bool IsPrintGrid() const { return m_bPrintGrid; }
    bool GetPrintGrid() const { return m_bPrintGrid; }
    void SetPrintGrid( bool bNew ) { m_bPrintGrid = bNew; }

    bool IsDisplayGrid() const { return m_bDisplayGrid; }
    bool GetDisplayGrid() const { return m_bDisplayGrid; }
    void SetDisplayGrid( bool bNew ) { m_bDisplayGrid = bNew; }

    //for textgrid enhancement
    sal_uInt16 GetBaseWidth() const { return m_nBaseWidth;}
    void SetBaseWidth( sal_uInt16 nNew ) { m_nBaseWidth = nNew; }

    bool IsSnapToChars() const { return m_bSnapToChars; }
    bool GetSnapToChars() const { return m_bSnapToChars; }
    void SetSnapToChars( bool bNew ) { m_bSnapToChars = bNew; }

    bool IsSquaredMode() const { return m_bSquaredMode; }
    bool GetSquaredMode() const { return m_bSquaredMode; }
    void SetSquaredMode( bool bNew ) { m_bSquaredMode = bNew; }
    void SwitchPaperMode(bool bNew );

    void Init();
};

inline const SwTextGridItem &SwFormat::GetTextGrid(bool bInP) const
    {   return m_aSet.Get( RES_TEXTGRID, bInP ); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
