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
#ifndef _PORTAB_HXX
#define _PORTAB_HXX

#include "porglue.hxx"
namespace binfilter {

/*************************************************************************
 *                      class SwTabPortion
 *************************************************************************/

class SwTabPortion : public SwFixPortion
{
    const KSHORT nTabPos;
    const xub_Unicode cFill;

    // Das Format() verzweigt entweder in Pre- oder PostFormat()
    sal_Bool PreFormat( SwTxtFormatInfo &rInf );
public:
    SwTabPortion( const KSHORT nTabPos, const xub_Unicode cFill = '\0' );
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
    sal_Bool PostFormat( SwTxtFormatInfo &rInf );
    inline  sal_Bool IsFilled() const { return 0 != cFill; }
    inline  KSHORT GetTabPos() const { return nTabPos; }

    // Accessibility: pass information about this portion to the PortionHandler

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                  class SwTabLeftPortion
 *************************************************************************/

class SwTabLeftPortion : public SwTabPortion
{
public:
    inline SwTabLeftPortion( const KSHORT nTabPos, const xub_Unicode cFill='\0' )
         : SwTabPortion( nTabPos, cFill )
    { SetWhichPor( POR_TABLEFT ); }
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                  class SwTabRightPortion
 *************************************************************************/

class SwTabRightPortion : public SwTabPortion
{
public:
    inline SwTabRightPortion( const KSHORT nTabPos, const xub_Unicode cFill='\0' )
         : SwTabPortion( nTabPos, cFill )
    { SetWhichPor( POR_TABRIGHT ); }
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                  class SwTabCenterPortion
 *************************************************************************/

class SwTabCenterPortion : public SwTabPortion
{
public:
    inline SwTabCenterPortion( const KSHORT nTabPos, const xub_Unicode cFill='\0' )
         : SwTabPortion( nTabPos, cFill )
    { SetWhichPor( POR_TABCENTER );	}
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                  class SwTabDecimalPortion
 *************************************************************************/

class SwTabDecimalPortion : public SwTabPortion
{
    const xub_Unicode cTab;
public:
    inline SwTabDecimalPortion( const KSHORT nTabPos, const xub_Unicode cTab,
                                const xub_Unicode cFill = '\0' )
         : SwTabPortion( nTabPos, cFill ), cTab(cTab)
    { SetWhichPor( POR_TABDECIMAL ); }
    inline xub_Unicode GetTabDecimal() const { return cTab; }
    OUTPUT_OPERATOR
};

CLASSIO( SwTabPortion )
CLASSIO( SwTabLeftPortion )
CLASSIO( SwTabRightPortion )
CLASSIO( SwTabCenterPortion )
CLASSIO( SwTabDecimalPortion )


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
