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
#ifndef _PORRST_HXX
#define _PORRST_HXX

#ifndef _SVARRAY_HXX //autogen
#include <bf_svtools/svarray.hxx>
#endif

#include "porlay.hxx"
#include "porexp.hxx"
namespace binfilter {

#ifdef VERTICAL_LAYOUT
#define LINE_BREAK_WIDTH        150
#define SPECIAL_FONT_HEIGHT     200
#endif

class SwTxtFormatInfo;

/*************************************************************************
 *						class SwBreakPortion
 *************************************************************************/

class SwBreakPortion : public SwLinePortion
{
#ifdef VERTICAL_LAYOUT
#else
    void CalcViewWidth( const SwTxtSizeInfo &rInf );
#endif

protected:
#ifndef VERTICAL_LAYOUT
    KSHORT	nViewWidth;
#endif
    KSHORT	nRestWidth;
public:
            SwBreakPortion( const SwLinePortion &rPortion );
    // liefert 0 zurueck, wenn keine Nutzdaten enthalten sind.
    virtual SwLinePortion *Compress();
    virtual void Paint( const SwTxtPaintInfo &rInf ) const{DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    USHORT GetRestWidth() const { return nRestWidth; }

    // Accessibility: pass information about this portion to the PortionHandler

    OUTPUT_OPERATOR
};

/*************************************************************************
 *						class SwKernPortion
 *************************************************************************/

class SwKernPortion : public SwLinePortion
{
    short nKern;
    sal_Bool bBackground;

#ifdef VERTICAL_LAYOUT
    sal_Bool bGridKern;
#endif

public:

#ifdef VERTICAL_LAYOUT
    // This constructor automatically appends the portion to rPortion
    // bBG indicates, that the background of the kerning portion has to
    // be painted, e.g., if the portion if positioned between to fields.
    // bGridKern indicates, that the kerning portion is used to provide
    // additional space in grid mode.
    SwKernPortion( SwLinePortion &rPortion, short nKrn,
                   sal_Bool bBG = sal_False, sal_Bool bGridKern = sal_False );

    // This constructor only sets the height and ascent to the values
    // of rPortion. It is only used for kerning portions for grid mode
#else
    SwKernPortion( SwLinePortion &rPortion, short nKrn,
                   sal_Bool bBG = sal_False );
#endif

    virtual void FormatEOL( SwTxtFormatInfo &rInf );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *						class SwArrowPortion
 *************************************************************************/

class SwArrowPortion : public SwLinePortion
{
    Point aPos;
    sal_Bool bLeft;
public:
            SwArrowPortion( const SwLinePortion &rPortion );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual SwLinePortion *Compress();
#ifndef VERTICAL_LAYOUT
#endif
    inline sal_Bool IsLeft() const { return bLeft; }
    inline const Point& GetPos() const { return aPos; }
    OUTPUT_OPERATOR
};

/*************************************************************************
 *						class SwHangingPortion
 * The characters which are forbidden at the start of a line like the dot and
 * other punctuation marks are allowed to display in the margin of the page
 * by a user option.
 * The SwHangingPortion is the corresponding textportion to do that.
 *************************************************************************/

class SwHangingPortion : public SwTxtPortion
{
    KSHORT nInnerWidth;
public:
    inline SwHangingPortion( SwPosSize aSize ) : nInnerWidth( aSize.Width() )
        { SetWhichPor( POR_HNG );  SetLen( 1 ); Height( aSize.Height() ); }

    inline KSHORT GetInnerWidth() const { return nInnerWidth; }
};


/*************************************************************************
 *					inline - Implementations
 *************************************************************************/

CLASSIO( SwBreakPortion )
CLASSIO( SwEndPortion )
CLASSIO( SwKernPortion )
CLASSIO( SwArrowPortion )

} //namespace binfilter
#endif
