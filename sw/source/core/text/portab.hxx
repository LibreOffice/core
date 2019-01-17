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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_PORTAB_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_PORTAB_HXX

#include "porglue.hxx"

class SwTabPortion : public SwFixPortion
{
    const sal_uInt16 nTabPos;
    const sal_Unicode cFill;
    const bool bAutoTabStop;

    // Format() branches either into PreFormat() or PostFormat()
    bool PreFormat( SwTextFormatInfo &rInf );
public:
    SwTabPortion( const sal_uInt16 nTabPos, const sal_Unicode cFill, const bool bAutoTab = true );
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual void FormatEOL( SwTextFormatInfo &rInf ) override;
    bool PostFormat( SwTextFormatInfo &rInf );
    bool IsFilled() const { return 0 != cFill; }
    sal_uInt16 GetTabPos() const { return nTabPos; }

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;
};

class SwTabLeftPortion : public SwTabPortion
{
public:
    SwTabLeftPortion( const sal_uInt16 nTabPosVal, const sal_Unicode cFillChar, bool bAutoTab )
         : SwTabPortion( nTabPosVal, cFillChar, bAutoTab )
    { SetWhichPor( PortionType::TabLeft ); }
};

class SwTabRightPortion : public SwTabPortion
{
public:
    SwTabRightPortion( const sal_uInt16 nTabPosVal, const sal_Unicode cFillChar )
         : SwTabPortion( nTabPosVal, cFillChar )
    { SetWhichPor( PortionType::TabRight ); }
};

class SwTabCenterPortion : public SwTabPortion
{
public:
    SwTabCenterPortion( const sal_uInt16 nTabPosVal, const sal_Unicode cFillChar )
         : SwTabPortion( nTabPosVal, cFillChar )
    { SetWhichPor( PortionType::TabCenter ); }
};

class SwTabDecimalPortion : public SwTabPortion
{
    const sal_Unicode mcTab;

    /*
     * During text formatting, we already store the width of the portions
     * following the tab stop up to the decimal position. This value is
     * evaluated during pLastTab->FormatEOL. FME 2006-01-06 #127428#.
     */
    sal_uInt16 mnWidthOfPortionsUpTpDecimalPosition;

public:
    SwTabDecimalPortion( const sal_uInt16 nTabPosVal, const sal_Unicode cTab,
                                const sal_Unicode cFillChar )
         : SwTabPortion( nTabPosVal, cFillChar ),
           mcTab(cTab),
           mnWidthOfPortionsUpTpDecimalPosition( USHRT_MAX )
    { SetWhichPor( PortionType::TabDecimal ); }

    sal_Unicode GetTabDecimal() const { return mcTab; }

    void SetWidthOfPortionsUpToDecimalPosition( sal_uInt16 nNew )
    {
        mnWidthOfPortionsUpTpDecimalPosition = nNew;
    }
    sal_uInt16 GetWidthOfPortionsUpToDecimalPosition() const
    {
        return mnWidthOfPortionsUpTpDecimalPosition;
    }
};

class SwAutoTabDecimalPortion : public SwTabDecimalPortion
{
public:
    SwAutoTabDecimalPortion( const sal_uInt16 nTabPosVal, const sal_Unicode cTab,
                                    const sal_Unicode cFillChar )
         : SwTabDecimalPortion( nTabPosVal, cTab, cFillChar )
    {
        SetLen(TextFrameIndex(0));
    }
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
