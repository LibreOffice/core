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

#ifndef _POREXP_HXX
#define _POREXP_HXX

#include "portxt.hxx"
namespace binfilter {

/*************************************************************************
 *                      class SwExpandPortion
 *************************************************************************/

class SwExpandPortion : public SwTxtPortion
{
public:
    inline	SwExpandPortion() { SetWhichPor( POR_EXP ); }
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;

    // Accessibility: pass information about this portion to the PortionHandler

    OUTPUT_OPERATOR
};


/*************************************************************************
 *						class SwBlankPortion
 *************************************************************************/

class SwBlankPortion : public SwExpandPortion
{
    xub_Unicode cChar;
    BOOL bMulti;		// For multiportion brackets
public:
    inline	SwBlankPortion( xub_Unicode cCh, BOOL bMult = sal_False )
        : cChar( cCh ), bMulti( bMult )
        { cChar = cCh; SetLen(1); SetWhichPor( POR_BLANK ); }

    BOOL IsMulti() const { return bMulti; }
    void SetMulti( BOOL bNew ) { bMulti = bNew; }

    virtual SwLinePortion *Compress();
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    MSHORT MayUnderFlow( const SwTxtFormatInfo &rInf, xub_StrLen nIdx,
        sal_Bool bUnderFlow ) const;

    // Accessibility: pass information about this portion to the PortionHandler

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwPostItsPortion
 *************************************************************************/

class SwPostItsPortion : public SwExpandPortion
{
public:
    SwPostItsPortion( sal_Bool bScrpt ){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 //STRIP001 			SwPostItsPortion( sal_Bool bScrpt );
    OUTPUT_OPERATOR
};


CLASSIO( SwExpandPortion )
CLASSIO( SwBlankPortion )


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
