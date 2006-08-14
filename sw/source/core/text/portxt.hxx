/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: portxt.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 16:42:42 $
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
#ifndef _PORTXT_HXX
#define _PORTXT_HXX
#ifdef GCC
#include <sys/types.h>
#else
#include <new.h>    //fuer size_t, FIXEDMEM aus tools
#endif

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif

#include "porlin.hxx"

class SwTxtGuess;

/*************************************************************************
 *                      class SwTxtPortion
 *************************************************************************/

class SwTxtPortion : public SwLinePortion
{
    void BreakCut( SwTxtFormatInfo &rInf, const SwTxtGuess &rGuess );
    void BreakUnderflow( SwTxtFormatInfo &rInf );
    sal_Bool _Format( SwTxtFormatInfo &rInf );

public:
    inline SwTxtPortion(){ SetWhichPor( POR_TXT ); }
    SwTxtPortion( const SwLinePortion &rPortion );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
    virtual xub_StrLen GetCrsrOfst( const KSHORT nOfst ) const;
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual long CalcSpacing( long nSpaceAdd, const SwTxtSizeInfo &rInf ) const;

    // zaehlt die Spaces fuer Blocksatz
    xub_StrLen GetSpaceCnt( const SwTxtSizeInfo &rInf, xub_StrLen& rCnt ) const;

    sal_Bool CreateHyphen( SwTxtFormatInfo &rInf, SwTxtGuess &rGuess );

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
    DECL_FIXEDMEMPOOL_NEWDEL(SwTxtPortion)
};

/*************************************************************************
 *                      class SwHolePortion
 *************************************************************************/

class SwHolePortion : public SwLinePortion
{
    KSHORT nBlankWidth;
public:
            SwHolePortion( const SwTxtPortion &rPor );
    inline const KSHORT GetBlankWidth( ) const { return nBlankWidth; }
    inline void SetBlankWidth( const KSHORT nNew ) { nBlankWidth = nNew; }
    virtual SwLinePortion *Compress();
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
    DECL_FIXEDMEMPOOL_NEWDEL(SwHolePortion)
};

CLASSIO( SwTxtPortion )
CLASSIO( SwHolePortion )


#endif
