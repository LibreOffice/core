/*************************************************************************
 *
 *  $RCSfile: porfld.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ama $ $Date: 2000-09-29 13:54:47 $
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
#ifndef _PORFLD_HXX
#define _PORFLD_HXX

#include "swtypes.hxx"
#include "porexp.hxx"
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif

class SwFont;
class SvxBrushItem;
class SwFmtVertOrient;
class SwFrm;

/*************************************************************************
 *                      class SwFldPortion
 *************************************************************************/

class SwFldPortion : public SwExpandPortion
{
    friend class SwTxtFormatter;
protected:
    XubString  aExpand;            // das expandierte Feld
    SwFont  *pFnt;              // Fuer mehrzeilige Felder
    xub_StrLen nNextOffset;     // Offset des Follows im Originalstring
    KSHORT  nViewWidth;         // Screenbreite fuer leere Felder
    sal_Bool bFollow : 1;           // 2. oder weiterer Teil eines Feldes
    sal_Bool bLeft : 1;             // wird von SwNumberPortion benutzt
    sal_Bool bHide : 1;             // wird von SwNumberPortion benutzt
    sal_Bool bCenter : 1;           // wird von SwNumberPortion benutzt
    sal_Bool bHasFollow : 1;        // geht in der naechsten Zeile weiter
    sal_Bool bAnimated : 1;         // wird von SwGrfNumPortion benutzt
    sal_Bool bNoPaint : 1;          // wird von SwGrfNumPortion benutzt
    sal_Bool bReplace : 1;          // wird von SwGrfNumPortion benutzt

    inline void SetFont( SwFont *pNew ) { pFnt = pNew; }
    inline const SwFont *GetFont() const { return pFnt; }
    BYTE ScriptChange( const SwTxtSizeInfo &rInf, xub_StrLen& rFull );
public:
    SwFldPortion( const XubString &rExpand, SwFont *pFnt = 0 );
    ~SwFldPortion();

    void TakeNextOffset( const SwFldPortion* pFld );
    void CheckScript( const SwTxtSizeInfo &rInf );

    inline const XubString &GetExp() const { return aExpand; }
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;

    // leere Felder sind auch erlaubt
    virtual SwLinePortion *Compress();

    virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const;

    inline sal_Bool IsFollow() const { return bFollow; }
    inline void SetFollow( sal_Bool bNew ) { bFollow = bNew; }

    inline sal_Bool IsLeft() const { return bLeft; }
    inline void SetLeft( sal_Bool bNew ) { bLeft = bNew; }

    inline sal_Bool IsHide() const { return bHide; }
    inline void SetHide( sal_Bool bNew ) { bHide = bNew; }

    inline sal_Bool IsCenter() const { return bCenter; }
    inline void SetCenter( sal_Bool bNew ) { bCenter = bNew; }

    inline sal_Bool HasFollow() const { return bHasFollow; }
    inline void SetHasFollow( sal_Bool bNew ) { bHasFollow = bNew; }

    inline xub_StrLen GetNextOffset() const { return nNextOffset; }
    inline void SetNextOffset( xub_StrLen nNew ) { nNextOffset = nNew; }

    // Felder-Cloner fuer SplitGlue
    virtual SwFldPortion *Clone( const XubString &rExpand ) const;

    // Extra-GetTxtSize wegen pFnt
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwHiddenPortion
 *************************************************************************/
// Unterscheidung nur fuer's Painten/verstecken.

class SwHiddenPortion : public SwFldPortion
{
public:
    inline SwHiddenPortion( const XubString &rExpand, SwFont *pFnt = 0 )
         : SwFldPortion( rExpand, pFnt )
        { SetLen(1); SetWhichPor( POR_HIDDEN ); }
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;

    // Felder-Cloner fuer SplitGlue
    virtual SwFldPortion *Clone( const XubString &rExpand ) const;
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwNumberPortion
 *************************************************************************/

class SwNumberPortion : public SwFldPortion
{
protected:
    KSHORT  nFixWidth;      // vgl. Glues
    KSHORT  nMinDist;       // minimaler Abstand zum Text
public:
    SwNumberPortion( const XubString &rExpand, SwFont *pFnt,
        const sal_Bool bLeft, const sal_Bool bCenter, const KSHORT nMinDst );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual xub_StrLen GetCrsrOfst( const MSHORT nOfst ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );

    // Felder-Cloner fuer SplitGlue
    virtual SwFldPortion *Clone( const XubString &rExpand ) const;
    virtual void FormatEOL( SwTxtFormatInfo &rInf );

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwBulletPortion
 *************************************************************************/

class SwBulletPortion : public SwNumberPortion
{
public:
    SwBulletPortion( const xub_Unicode cCh, SwFont *pFnt, const sal_Bool bLeft,
                     const sal_Bool bCenter, const KSHORT nMinDst );
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwBmpBulletPortion
 *************************************************************************/

class SwGrfNumPortion : public SwNumberPortion
{
    SvxBrushItem* pBrush;
    long            nId;    //fuer StopAnimation
    SwTwips         nYPos;  //Enthaelt _immer_ die aktuelle RelPos.
    SwTwips         nGrfHeight;
    SwVertOrient    eOrient;
public:
    SwGrfNumPortion( SwFrm *pFrm, const SvxBrushItem* pGrfBrush,
        const SwFmtVertOrient* pGrfOrient, const Size& rGrfSize,
        const sal_Bool bLeft, const sal_Bool bCenter, const KSHORT nMinDst );
    ~SwGrfNumPortion();
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );

    void SetBase( long nLnAscent, long nLnDescent,
        long nFlyAscent, long nFlyDescent );

    void StopAnimation( OutputDevice *pOut );

    inline sal_Bool IsAnimated() const { return bAnimated; }
    inline void SetAnimated( sal_Bool bNew ) { bAnimated = bNew; }
    inline sal_Bool DontPaint() const { return bNoPaint; }
    inline void SetNoPaint( sal_Bool bNew ) { bNoPaint = bNew; }
    inline void SetRelPos( SwTwips nNew ) { nYPos = nNew; }
    inline void SetId( long nNew ) const
        { ((SwGrfNumPortion*)this)->nId = nNew; }
    inline SwTwips GetRelPos() const { return nYPos; }
    inline SwTwips GetGrfHeight() const { return nGrfHeight; }
    inline SwTwips GetId() const { return nId; }
    inline SwVertOrient GetOrient() const { return eOrient; }

    OUTPUT_OPERATOR
};

CLASSIO( SwHiddenPortion )
CLASSIO( SwNumberPortion )
CLASSIO( SwBulletPortion )
CLASSIO( SwGrfNumPortion )


#endif
