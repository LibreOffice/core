/*************************************************************************
 *
 *  $RCSfile: porftn.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

class SwFtnPortion : public SwExpandPortion
{
    XubString   aExpand;
    SwTxtFrm *pFrm;         // um im Dtor RemoveFtn rufen zu koennen.
    SwTxtFtn *pFtn;
    KSHORT  nOrigHeight;
public:
            SwFtnPortion( const XubString &rExpand, SwTxtFrm *pFrm, SwTxtFtn *pFtn,
                          KSHORT nOrig = KSHRT_MAX );
    void ClearFtn();
    inline KSHORT& Orig() { return nOrigHeight; }

    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwFtnNumPortion
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

    // Felder-Cloner fuer SplitGlue
    virtual SwFldPortion *Clone( const XubString &rExpand ) const;

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
