/*************************************************************************
 *
 *  $RCSfile: pormulti.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ama $ $Date: 2000-11-09 11:41:42 $
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

#ifndef _PORMULTI_HXX
#define _PORMULTI_HXX

#include "porlay.hxx"
#include "porexp.hxx"

class SwTxtFormatInfo;
class SwFldPortion;
class SwTxtCursor;
class SwLineLayout;
class SwBlankPortion;
class SwTxtPaintInfo;

/*-----------------25.10.00 16:19-------------------
 * A two-line-portion (SwMultiPortion) could have surrounding brackets,
 * in this case the structur SwBracket will be used.
 * --------------------------------------------------*/

struct SwBracket
{
    KSHORT nAscent;         // Ascent of the brackets
    KSHORT nHeight;         // Height of them
    KSHORT nPreWidth;       // Width of the opening bracket
    KSHORT nPostWidth;      // Width of the closing bracket
    sal_Unicode cPre;       // Opening character, e.g. '('
    sal_Unicode cPost;      // Closing character, e.g. ')'
};

/*-----------------16.10.00 12:45-------------------
 * The SwMultiPortion is line portion inside a line portion,
 * it's a group of portions,
 * e.g. a double line portion in a line
 * or phonetics (ruby)
 * or combined characters
 * or a rotated portion.
 * --------------------------------------------------*/

class SwMultiPortion : public SwLinePortion
{
    SwLineLayout aRoot;     // One or more lines
    SwFldPortion *pFldRest; // Field rest from the previous line
    sal_Bool bTab1      :1; // First line tabulator
    sal_Bool bTab2      :1; // Second line includes tabulator
    sal_Bool bDouble    :1; // Double line
    sal_Bool bRuby      :1; // Phonetics
    sal_Bool bTop       :1; // Phonetic position
    sal_Bool bFormatted :1; // Already formatted
    sal_Bool bFollowFld :1; // Field follow inside
protected:
    SwMultiPortion( xub_StrLen nEnd ) : pFldRest( 0 ), bTab1( sal_False ),
        bTab2( sal_False ), bDouble( sal_False ), bRuby( sal_False ),
        bFormatted( sal_False ), bFollowFld( sal_False )
        { SetWhichPor( POR_MULTI ); SetLen( nEnd ); }
    inline void SetDouble() { bDouble = sal_True; }
    inline void SetRuby() { bRuby = sal_True; }
    inline void SetTop( sal_Bool bNew ) { bTop = bNew; }
    inline void SetTab1( sal_Bool bNew ) { bTab1 = bNew; }
    inline void SetTab2( sal_Bool bNew ) { bTab2 = bNew; }
    inline sal_Bool GetTab1() const { return bTab1; }
    inline sal_Bool GetTab2() const { return bTab2; }
public:
    ~SwMultiPortion();
    const SwLineLayout& GetRoot() const { return aRoot; }
    SwLineLayout& GetRoot() { return aRoot; }
    SwFldPortion* GetFldRest() { return pFldRest; }
    void SetFldRest( SwFldPortion* pNew ) { pFldRest = pNew; }

    inline sal_Bool HasTabulator() const { return bTab1 || bTab2; }
    inline sal_Bool IsFormatted() const { return bFormatted; }
    inline void SetFormatted() { bFormatted = sal_True; }
    inline sal_Bool IsFollowFld() const { return bFollowFld; }
    inline void SetFollowFld() { bFollowFld = sal_True; }
    inline sal_Bool IsDouble() const { return bDouble; }
    inline sal_Bool IsRuby() const { return bRuby; }
    inline sal_Bool OnTop() const { return bTop; }
    void ActualizeTabulator();

    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual long CalcSpacing( short nSpaceAdd, const SwTxtSizeInfo &rInf ) const;

    // Summarize the internal lines to calculate the (external) size
    virtual void CalcSize( SwTxtFormatter& rLine );

    inline sal_Bool ChgSpaceAdd( SwLineLayout* pCurr, short nSpaceAdd );
    inline sal_Bool HasBrackets() const;
    OUTPUT_OPERATOR
};

class SwDoubleLinePortion : public SwMultiPortion
{
    SwBracket* pBracket;    // Surrounding brackets
    SwTwips nLineDiff;      // Difference of the width of the both lines
    xub_StrLen nBlank1;     // Number of blanks in the first line
    xub_StrLen nBlank2;     // Number of blanks in the second line
public:
    SwDoubleLinePortion( xub_StrLen nEnd ) : SwMultiPortion( nEnd ),
        pBracket( 0 ) { SetDouble(); }
    SwDoubleLinePortion( SwDoubleLinePortion& rDouble, xub_StrLen nEnd );
    ~SwDoubleLinePortion();

    inline SwBracket* GetBrackets() const { return pBracket; }
    void SetBrackets( sal_Unicode cPre, sal_Unicode cPost );
    inline void SetBrackets( const SwDoubleLinePortion& rDouble )
        { SetBrackets( rDouble.pBracket->cPre, rDouble.pBracket->cPost ); }
    void PaintBracket( SwTxtPaintInfo& rInf, short nSpc, sal_Bool bOpen ) const;
    void FormatBrackets( SwTxtFormatInfo &rInf, SwTwips& nMaxWidth );
    inline KSHORT PreWidth() const { return pBracket->nPreWidth; };
    inline KSHORT PostWidth() const { return pBracket->nPostWidth; }
    inline void ClearBrackets(){ pBracket->nPreWidth = pBracket->nPostWidth=0; }
    inline KSHORT BracketWidth(){ return PreWidth() + PostWidth(); }

    void CalcBlanks( SwTxtFormatInfo &rInf );
    sal_Bool ChangeSpaceAdd( SwLineLayout* pCurr, short nSpaceAdd );
    static void ResetSpaceAdd( SwLineLayout* pCurr );
    inline SwTwips GetLineDiff() const { return nLineDiff; }
    inline xub_StrLen GetSpaceCnt() const
        { return ( nLineDiff < 0 ) ? nBlank2 : nBlank1; }
    inline xub_StrLen GetSmallerSpaceCnt() const
        { return ( nLineDiff < 0 ) ? nBlank1 : nBlank2; }
    inline xub_StrLen GetBlank1() const { return nBlank1; }
    inline xub_StrLen GetBlank2() const { return nBlank2; }

    virtual long CalcSpacing( short nSpaceAdd, const SwTxtSizeInfo &rInf ) const;
};

class SwRubyPortion : public SwMultiPortion
{
    xub_StrLen nRubyOffset;
    USHORT nAdjustment;
    void _Adjust( SwTxtFormatInfo &rInf);
public:
    SwRubyPortion( xub_StrLen nEnd, USHORT nAdj, USHORT nPos, xub_StrLen nOfst )
        : SwMultiPortion( nEnd ), nRubyOffset( nOfst ), nAdjustment( nAdj )
        { SetRuby(); SetTop(!nPos); }
    SwRubyPortion( const SwTxtAttr& rAttr, const SwFont& rFnt, xub_StrLen nEnd,
        xub_StrLen nOffs = 0 );
    void CalcRubyOffset();
    inline void Adjust( SwTxtFormatInfo &rInf )
        { if(nAdjustment && GetRoot().GetNext()) _Adjust(rInf); }
    inline USHORT GetAdjustment() const { return nAdjustment; }
    inline xub_StrLen GetRubyOffset() const { return nRubyOffset; }
};


// For cursor travelling in multiportions

class SwTxtCursorSave
{
    SwTxtCursor* pTxtCrsr;
    SwLineLayout* pCurr;
    SwTwips nWidth;
    xub_StrLen nStart;
    sal_Bool bSpaceChg;
public:
    SwTxtCursorSave( SwTxtCursor* pTxtCursor, SwMultiPortion* pMulti,
        SwTwips nY, xub_StrLen nCurrStart, short nSpaceAdd );
    ~SwTxtCursorSave();
};

/*************************************************************************
 *                  inline - Implementations
 *************************************************************************/

inline sal_Bool SwMultiPortion::ChgSpaceAdd(SwLineLayout* pCurr,short nSpaceAdd)
    { return IsDouble() ? ((SwDoubleLinePortion*)this)->ChangeSpaceAdd( pCurr,
                            nSpaceAdd ) : sal_False; }

inline sal_Bool SwMultiPortion::HasBrackets() const
    { return IsDouble() ? 0 != ((SwDoubleLinePortion*)this)->GetBrackets()
      : sal_False; }

CLASSIO( SwMultiPortion )

#endif
