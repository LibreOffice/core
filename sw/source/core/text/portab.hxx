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

/*************************************************************************
 *                      class SwTabPortion
 *************************************************************************/

class SwTabPortion : public SwFixPortion
{
    const KSHORT nTabPos;
    const sal_Unicode cFill;
    const bool bAutoTabStop;

    // Format() branches either into PreFormat() or PostFormat()
    bool PreFormat( SwTxtFormatInfo &rInf );
public:
    SwTabPortion( const KSHORT nTabPos, const sal_Unicode cFill = '\0', const bool bAutoTab = true );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual bool Format( SwTxtFormatInfo &rInf );
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
    bool PostFormat( SwTxtFormatInfo &rInf );
    inline  bool IsFilled() const { return 0 != cFill; }
    inline  KSHORT GetTabPos() const { return nTabPos; }
    inline  bool IsAutoTabStop() const { return bAutoTabStop; }

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                  class SwTabLeftPortion
 *************************************************************************/

class SwTabLeftPortion : public SwTabPortion
{
public:
    inline SwTabLeftPortion( const KSHORT nTabPosVal, const sal_Unicode cFillChar='\0', bool bAutoTab = true )
         : SwTabPortion( nTabPosVal, cFillChar, bAutoTab )
    { SetWhichPor( POR_TABLEFT ); }
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                  class SwTabRightPortion
 *************************************************************************/

class SwTabRightPortion : public SwTabPortion
{
public:
    inline SwTabRightPortion( const KSHORT nTabPosVal, const sal_Unicode cFillChar='\0' )
         : SwTabPortion( nTabPosVal, cFillChar )
    { SetWhichPor( POR_TABRIGHT ); }
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                  class SwTabCenterPortion
 *************************************************************************/

class SwTabCenterPortion : public SwTabPortion
{
public:
    inline SwTabCenterPortion( const KSHORT nTabPosVal, const sal_Unicode cFillChar='\0' )
         : SwTabPortion( nTabPosVal, cFillChar )
    { SetWhichPor( POR_TABCENTER ); }
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                  class SwTabDecimalPortion
 *************************************************************************/

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
    inline SwTabDecimalPortion( const KSHORT nTabPosVal, const sal_Unicode cTab,
                                const sal_Unicode cFillChar = '\0' )
         : SwTabPortion( nTabPosVal, cFillChar ),
           mcTab(cTab),
           mnWidthOfPortionsUpTpDecimalPosition( USHRT_MAX )
    { SetWhichPor( POR_TABDECIMAL ); }

    inline sal_Unicode GetTabDecimal() const { return mcTab; }

    inline void SetWidthOfPortionsUpToDecimalPosition( sal_uInt16 nNew )
    {
        mnWidthOfPortionsUpTpDecimalPosition = nNew;
    }
    inline sal_uInt16 GetWidthOfPortionsUpToDecimalPosition() const
    {
        return mnWidthOfPortionsUpTpDecimalPosition;
    }

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                  class SwAutoTabDecimalPortion
 *************************************************************************/

class SwAutoTabDecimalPortion : public SwTabDecimalPortion
{
public:
    inline SwAutoTabDecimalPortion( const KSHORT nTabPosVal, const sal_Unicode cTab,
                                    const sal_Unicode cFillChar = '\0' )
         : SwTabDecimalPortion( nTabPosVal, cTab, cFillChar )
    { SetLen( 0 ); }
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
