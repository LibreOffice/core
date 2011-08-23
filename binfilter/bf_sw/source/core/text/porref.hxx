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
#ifndef _PORREF_HXX
#define _PORREF_HXX

#include "portxt.hxx"
namespace binfilter {

/*************************************************************************
 *                      class SwRefPortion
 *************************************************************************/

class SwRefPortion : public SwTxtPortion
{
public:
    inline	SwRefPortion(){	SetWhichPor( POR_REF ); }
//STRIP001 	virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwIsoRefPortion
 *************************************************************************/

class SwIsoRefPortion : public SwRefPortion
{
//STRIP001 	KSHORT nViewWidth;
//STRIP001 
public:
    SwIsoRefPortion(){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 //STRIP001 			SwIsoRefPortion();
//STRIP001 	virtual sal_Bool Format( SwTxtFormatInfo &rInf );
//STRIP001 	virtual void Paint( const SwTxtPaintInfo &rInf ) const;
//STRIP001 	virtual SwLinePortion *Compress();
//STRIP001 	virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const;

    // Accessibility: pass information about this portion to the PortionHandler
//STRIP001     virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *					inline - Implementations
 *************************************************************************/

CLASSIO( SwRefPortion )
//STRIP001 CLASSIO( SwIsoRefPortion )


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
