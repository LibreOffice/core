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
#ifndef _PORHYPH_HXX
#define _PORHYPH_HXX

#include "porexp.hxx"
namespace binfilter {

/*************************************************************************
 *                      class SwHyphPortion
 *************************************************************************/

class SwHyphPortion : public SwExpandPortion
{
public:
    inline	SwHyphPortion( ) { SetWhichPor( POR_HYPH ); }
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );

    // Accessibility: pass information about this portion to the PortionHandler

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwHyphStrPortion
 *************************************************************************/

class SwHyphStrPortion : public SwHyphPortion
{
    XubString aExpand;
public:
    inline  SwHyphStrPortion( const XubString &rStr );

    // Accessibility: pass information about this portion to the PortionHandler

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwSoftHyphPortion
 *************************************************************************/

class SwSoftHyphPortion : public SwHyphPortion
{
    sal_Bool    bExpand;
    KSHORT  nViewWidth;
    KSHORT	nHyphWidth;

public:
    SwSoftHyphPortion();
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual SwLinePortion *Compress();
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
    inline void SetExpand( const sal_Bool bNew ) { bExpand = bNew; }
    sal_Bool IsExpand() const { return bExpand; }


    // Accessibility: pass information about this portion to the PortionHandler

    OUTPUT_OPERATOR
};

/*************************************************************************
 *						class SwSoftHyphStrPortion
 *************************************************************************/

CLASSIO( SwHyphPortion )
CLASSIO( SwHyphStrPortion )
CLASSIO( SwSoftHyphPortion )


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
