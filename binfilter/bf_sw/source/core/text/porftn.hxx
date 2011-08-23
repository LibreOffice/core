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
#ifndef _PORFTN_HXX
#define _PORFTN_HXX

#include "porfld.hxx"
namespace binfilter {

class SwTxtFrm;
class SwTxtFtn;

/*************************************************************************
 *						class SwFtnPortion
 *************************************************************************/

class SwFtnPortion : public SwFldPortion
{
    SwTxtFrm *pFrm; 		// um im Dtor RemoveFtn rufen zu koennen.
    SwTxtFtn *pFtn;
    KSHORT	nOrigHeight;
public:
    SwFtnPortion( const XubString &rExpand, SwTxtFrm *pFrm, SwTxtFtn *pFtn,
                  KSHORT nOrig = KSHRT_MAX );
    inline KSHORT& Orig() {	return nOrigHeight; }

    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );

    const SwTxtFtn* GetTxtFtn() const { return pFtn; };
    OUTPUT_OPERATOR
};

/*************************************************************************
 *						class SwFtnNumPortion
 *************************************************************************/

class SwFtnNumPortion : public SwNumberPortion
{
public:
    inline SwFtnNumPortion( const XubString &rExpand, SwFont *pFnt )
         : SwNumberPortion( rExpand, pFnt, sal_True, sal_False, 0 )
         { SetWhichPor( POR_FTNNUM ); }
    sal_Bool DiffFont( SwFont* pFont );
    OUTPUT_OPERATOR
};

/*************************************************************************
 *						class SwQuoVadisPortion
 *************************************************************************/

class SwQuoVadisPortion : public SwFldPortion
{
public:
    SwQuoVadisPortion( const XubString &rExp, const XubString& rStr ): SwFldPortion( rExp ){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 			SwQuoVadisPortion( const XubString &rExp, const XubString& rStr );

    // Felder-Cloner fuer SplitGlue

    // Accessibility: pass information about this portion to the PortionHandler
    OUTPUT_OPERATOR
};

/*************************************************************************
 *						class SwErgoSumPortion
 *************************************************************************/

class SwErgoSumPortion : public SwFldPortion
{
public:
SwErgoSumPortion( const XubString &rExp, const XubString& rStr ):SwFldPortion( rExp ){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 			SwErgoSumPortion( const XubString &rExp, const XubString& rStr );
    OUTPUT_OPERATOR
};

CLASSIO( SwFtnPortion )
CLASSIO( SwFtnNumPortion )
CLASSIO( SwQuoVadisPortion )
CLASSIO( SwErgoSumPortion )


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
