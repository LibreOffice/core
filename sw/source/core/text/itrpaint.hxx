/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itrpaint.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:14:57 $
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
#ifndef _ITRPAINT_HXX
#define _ITRPAINT_HXX
#include "itrtxt.hxx"

class SwSaveClip;          // SwTxtPainter
class SwMultiPortion;

/*************************************************************************
 *                      class SwTxtPainter
 *************************************************************************/

class SwTxtPainter : public SwTxtCursor
{
    sal_Bool bPaintDrop;

    SwLinePortion *CalcPaintOfst( const SwRect &rPaint );
    void CheckSpecialUnderline( const SwLinePortion* pPor,
                                long nAdjustBaseLine = 0 );
protected:
    void CtorInitTxtPainter( SwTxtFrm *pFrm, SwTxtPaintInfo *pInf );
    inline SwTxtPainter() { }
public:
    inline SwTxtPainter( SwTxtFrm *pTxtFrm, SwTxtPaintInfo *pTxtPaintInf )
           { CtorInitTxtPainter( pTxtFrm, pTxtPaintInf ); }
    void DrawTextLine( const SwRect &rPaint, SwSaveClip &rClip,
                       const sal_Bool bUnderSz );
    void PaintDropPortion();
    // if PaintMultiPortion is called recursively, we have to pass the
    // surrounding SwBidiPortion
    void PaintMultiPortion( const SwRect &rPaint, SwMultiPortion& rMulti,
                            const SwMultiPortion* pEnvPor = 0 );
    inline void SetPaintDrop( const sal_Bool bNew ) { bPaintDrop = bNew; }
    inline sal_Bool IsPaintDrop() const { return bPaintDrop; }
    inline SwTxtPaintInfo &GetInfo()
        { return (SwTxtPaintInfo&)SwTxtIter::GetInfo(); }
    inline const SwTxtPaintInfo &GetInfo() const
        { return (const SwTxtPaintInfo&)SwTxtIter::GetInfo(); }
};



#endif
