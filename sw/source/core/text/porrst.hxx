/*************************************************************************
 *
 *  $RCSfile: porrst.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2000-11-21 11:14:20 $
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
#ifndef _PORRST_HXX
#define _PORRST_HXX

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#include "porlay.hxx"
#include "porexp.hxx"

class SwTxtFormatInfo;

/*************************************************************************
 *                      class SwTmpEndPortion
 *************************************************************************/

class SwTmpEndPortion : public SwLinePortion
{
public:
            SwTmpEndPortion( const SwLinePortion &rPortion );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwBreakPortion
 *************************************************************************/

class SwBreakPortion : public SwLinePortion
{
    void CalcViewWidth( const SwTxtSizeInfo &rInf );
protected:
    KSHORT  nViewWidth;
    KSHORT  nRestWidth;
public:
            SwBreakPortion( const SwLinePortion &rPortion );
    // liefert 0 zurueck, wenn keine Nutzdaten enthalten sind.
    virtual SwLinePortion *Compress();
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const;
    virtual xub_StrLen GetCrsrOfst( const MSHORT nOfst ) const;
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwKernPortion
 *************************************************************************/

class SwKernPortion : public SwLinePortion
{
    short nKern;
public:
            SwKernPortion( SwLinePortion &rPortion, short nKrn );
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwArrowPortion
 *************************************************************************/

class SwArrowPortion : public SwLinePortion
{
    Point aPos;
    sal_Bool bLeft;
public:
            SwArrowPortion( const SwLinePortion &rPortion );
            SwArrowPortion( const SwTxtPaintInfo &rInf );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual SwLinePortion *Compress();
    void PaintIt( OutputDevice *pOut ) const;
    inline sal_Bool IsLeft() const { return bLeft; }
    inline const Point& GetPos() const { return aPos; }
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwHangingPortion
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
 *                  inline - Implementations
 *************************************************************************/

CLASSIO( SwBreakPortion )
CLASSIO( SwEndPortion )
CLASSIO( SwKernPortion )
CLASSIO( SwArrowPortion )

#endif
