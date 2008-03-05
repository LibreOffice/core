/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: porftn.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:07:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _PORFTN_HXX
#define _PORFTN_HXX

#include "porfld.hxx"

class SwTxtFrm;
class SwTxtFtn;

/*************************************************************************
 *                      class SwFtnPortion
 *************************************************************************/

class SwFtnPortion : public SwFldPortion
{
    SwTxtFrm *pFrm;         // um im Dtor RemoveFtn rufen zu koennen.
    SwTxtFtn *pFtn;
    KSHORT  nOrigHeight;
public:
    SwFtnPortion( const XubString &rExpand, SwTxtFrm *pFrm, SwTxtFtn *pFtn,
                  KSHORT nOrig = KSHRT_MAX );
    inline KSHORT& Orig() { return nOrigHeight; }

    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );

    const SwTxtFtn* GetTxtFtn() const { return pFtn; };
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwFtnNumPortion
 *************************************************************************/

class SwFtnNumPortion : public SwNumberPortion
{
public:
    inline SwFtnNumPortion( const XubString &rExpand, SwFont *pFntL )
         // --> OD 2008-01-23 #newlistlevelattrs#
         : SwNumberPortion( rExpand, pFntL, sal_True, sal_False, 0, false )
         // <--
         { SetWhichPor( POR_FTNNUM ); }

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwQuoVadisPortion
 *************************************************************************/

class SwQuoVadisPortion : public SwFldPortion
{
    XubString   aErgo;
public:
            SwQuoVadisPortion( const XubString &rExp, const XubString& rStr );
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;

    inline void SetNumber( const XubString& rStr ) { aErgo = rStr; }
    inline const XubString &GetQuoTxt() const { return aExpand; }
    inline const XubString &GetContTxt() const { return aErgo; }

    // Felder-Cloner fuer SplitGlue
    virtual SwFldPortion *Clone( const XubString &rExpand ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwErgoSumPortion
 *************************************************************************/

class SwErgoSumPortion : public SwFldPortion
{
public:
            SwErgoSumPortion( const XubString &rExp, const XubString& rStr );
    virtual xub_StrLen GetCrsrOfst( const KSHORT nOfst ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );

    // Felder-Cloner fuer SplitGlue
    virtual SwFldPortion *Clone( const XubString &rExpand ) const;
    OUTPUT_OPERATOR
};

CLASSIO( SwFtnPortion )
CLASSIO( SwFtnNumPortion )
CLASSIO( SwQuoVadisPortion )
CLASSIO( SwErgoSumPortion )


#endif
