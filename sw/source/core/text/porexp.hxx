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

#ifndef _POREXP_HXX
#define _POREXP_HXX

#include "portxt.hxx"

/*************************************************************************
 *                      class SwExpandPortion
 *************************************************************************/

class SwExpandPortion : public SwTxtPortion
{
public:
    inline  SwExpandPortion() { SetWhichPor( POR_EXP ); }
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual xub_StrLen GetCrsrOfst( const MSHORT nOfst ) const;
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};


/*************************************************************************
 *                      class SwBlankPortion
 *************************************************************************/

class SwBlankPortion : public SwExpandPortion
{
    xub_Unicode cChar;
    sal_Bool bMulti;        // For multiportion brackets
public:
    inline  SwBlankPortion( xub_Unicode cCh, sal_Bool bMult = sal_False )
        : cChar( cCh ), bMulti( bMult )
        { cChar = cCh; SetLen(1); SetWhichPor( POR_BLANK ); }

    sal_Bool IsMulti() const { return bMulti; }
    void SetMulti( sal_Bool bNew ) { bMulti = bNew; }

    virtual SwLinePortion *Compress();
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    MSHORT MayUnderFlow( const SwTxtFormatInfo &rInf, xub_StrLen nIdx,
        sal_Bool bUnderFlow ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwPostItsPortion
 *************************************************************************/

class SwPostItsPortion : public SwExpandPortion
{
    KSHORT  nViewWidth;
    sal_Bool    bScript;
public:
            SwPostItsPortion( sal_Bool bScrpt );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const;
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    sal_Bool IsScript() const { return bScript; }
    OUTPUT_OPERATOR
};


CLASSIO( SwExpandPortion )
CLASSIO( SwBlankPortion )
CLASSIO( SwPostItsPortion )


#endif
