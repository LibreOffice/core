/*************************************************************************
 *
 *  $RCSfile: porfld.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2000-09-28 14:07:00 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_SCRIPTTYPE_HDL_
#include <com/sun/star/text/ScriptType.hdl>
#endif

#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif

#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>  // SwViewOptions
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _TXTCFG_HXX
#include <txtcfg.hxx>
#endif
#ifndef _PORLAY_HXX
#include <porlay.hxx>
#endif
#ifndef _PORFLD_HXX
#include <porfld.hxx>
#endif
#ifndef _INFTXT_HXX
#include <inftxt.hxx>
#endif
#ifndef _BLINK_HXX
#include <blink.hxx>    // pBlink
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>  // DrawGraphic
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#include "breakit.hxx"
#include "porrst.hxx"

using namespace ::com::sun::star::text;

/*************************************************************************
 *                      class SwFldPortion
 *************************************************************************/

SwLinePortion *SwFldPortion::Compress()
{ return (GetLen() || aExpand.Len() || SwLinePortion::Compress()) ? this : 0; }

SwFldPortion *SwFldPortion::Clone( const XubString &rExpand ) const
{
    SwFont *pNewFnt;
    if( 0 != ( pNewFnt = pFnt ) )
        pNewFnt = new SwFont( *pFnt );
    return new SwFldPortion( rExpand, pNewFnt );
}

void SwFldPortion::TakeNextOffset( const SwFldPortion* pFld )
{
    ASSERT( pFld, "TakeNextOffset: Missing Source" );
    nNextOffset = pFld->GetNextOffset();
    aExpand.Erase( 0, nNextOffset );
    bFollow = sal_True;
}

SwFldPortion::SwFldPortion( const XubString &rExpand, SwFont *pFnt )
    : aExpand(rExpand), pFnt(pFnt), nViewWidth(0), nNextOffset(0),
      bFollow( sal_False ), bHasFollow( sal_False )
{
    SetWhichPor( POR_FLD );
}

SwFldPortion::~SwFldPortion()
{
    delete pFnt;
    if( pBlink )
        pBlink->Delete( this );
}

/*************************************************************************
 *               virtual SwFldPortion::GetViewWidth()
 *************************************************************************/

KSHORT SwFldPortion::GetViewWidth( const SwTxtSizeInfo &rInf ) const
{
    // Wir stehen zwar im const, aber nViewWidth sollte erst im letzten
    // Moment errechnet werden:
    SwFldPortion* pThis = (SwFldPortion*)this;
    if( !Width() && rInf.OnWin() && rInf.GetOpt().IsField() )
    {
        if( !nViewWidth )
            pThis->nViewWidth = rInf.GetTxtSize( ' ' ).Width();
    }
    else
        pThis->nViewWidth = 0;
    return nViewWidth;
}

/*************************************************************************
 *                 virtual SwFldPortion::Format()
 *************************************************************************/

// 8653: in keinem Fall nur SetLen(0);

/*************************************************************************
 *   Hilfsklasse SwFldSlot
 **************************************************************************/

class SwFldSlot
{
    const XubString *pOldTxt;
    XubString aTxt;
    xub_StrLen nIdx;
    xub_StrLen nLen;
    sal_Bool bOn;
    SwTxtSizeInfo *pInf;
public:
    SwFldSlot( const SwTxtSizeInfo *pNew, const SwFldPortion *pPor );
    ~SwFldSlot();
};

SwFldSlot::SwFldSlot( const SwTxtSizeInfo *pNew, const SwFldPortion *pPor )
{
    bOn = pPor->GetExpTxt( *pNew, aTxt );

    // Der Text wird ausgetauscht...
    if( bOn )
    {
        pInf = (SwTxtSizeInfo*)pNew;
        nIdx = pInf->GetIdx();
        nLen = pInf->GetLen();
        pOldTxt = &(pInf->GetTxt());
        pInf->SetLen( aTxt.Len() );
        if( pPor->IsFollow() )
            pInf->SetIdx( 0 );
        else
        {
            XubString aTmp( aTxt );
            aTxt = *pOldTxt;
            aTxt.Erase( nIdx, 1 );
            aTxt.Insert( aTmp, nIdx );
        }
        pInf->SetTxt( aTxt );
    }
}

SwFldSlot::~SwFldSlot()
{
    if( bOn )
    {
        pInf->SetTxt( *pOldTxt );
        pInf->SetIdx( nIdx );
        pInf->SetLen( nLen );
    }
}

#ifdef DEBUG
USHORT lcl_Script( const xub_Unicode aChar )
{
    USHORT nRet = ScriptType::WEAK;
    if( 'A' <= aChar && aChar <= 'Z' )
        nRet = ScriptType::LATIN;
    else if( 'a' <= aChar && aChar <= 'z' )
        nRet = ScriptType::ASIAN;
    return nRet;
}
#endif

BOOL SwFldPortion::ScriptChange( const SwTxtSizeInfo &rInf, xub_StrLen& rFull )
{
    BOOL bRet = FALSE;
    const String& rTxt = rInf.GetTxt();
    rFull += rInf.GetIdx();
#ifdef DEBUG
    static BOOL bTest = FALSE;
#endif
    if( rFull > rTxt.Len() )
        rFull = rTxt.Len();
    if( rFull && pBreakIt->xBreak.is() )
    {
        BYTE nActual = pFnt ? pFnt->GetActual() : rInf.GetFont()->GetActual();
        xub_StrLen nChg = rInf.GetIdx();
#ifdef DEBUG
        if( bTest )
        {
            while( ++nChg < rFull )
            {
                USHORT nScript = lcl_Script( rTxt.GetChar( nChg ) );
                BYTE nScr = nActual;
                switch ( nScript ) {
                    case ScriptType::LATIN : nScr = SW_LATIN; break;
                    case ScriptType::ASIAN : nScr = SW_CJK; break;
                    case ScriptType::COMPLEX : nScr = SW_CTL; break;
                }
                if( nActual != nScr )
                    break;
            }
        }
        else
#endif
        {
            USHORT nScript = ScriptType::LATIN;
            if( nActual )
                nScript = nActual==SW_CJK ? ScriptType::ASIAN : ScriptType::COMPLEX;
            nChg = pBreakIt->xBreak->endOfScript( rTxt, nChg, nScript );
        }
        if( rFull > nChg )
        {
            bRet = TRUE;
            rFull = nChg;
        }
    }
    rFull -= rInf.GetIdx();
    return bRet;
}

void SwFldPortion::CheckScript( const SwTxtSizeInfo &rInf )
{
    String aTxt;
    if( GetExpTxt( rInf, aTxt ) && aTxt.Len() && pBreakIt->xBreak.is() )
    {
        BYTE nActual = pFnt ? pFnt->GetActual() : rInf.GetFont()->GetActual();
        USHORT nScript;
#ifdef DEBUG
        static BOOL bTest = FALSE;
        if( bTest )
        {
            nScript = lcl_Script( aTxt.GetChar(0) );
            xub_StrLen nChg = 0;
            USHORT nCnt = 0;
            if( ScriptType::WEAK == nScript )
            {
                while( ++nChg < aTxt.Len() &&
                        ScriptType::WEAK == lcl_Script( aTxt.GetChar( nChg ) ) )
                    ;
                if( nChg < aTxt.Len() )
                    nScript = lcl_Script( aTxt.GetChar( nChg ) );
            }
        }
        else
#endif
        {
            nScript = pBreakIt->xBreak->getScriptType( aTxt, 0 );
            xub_StrLen nChg = 0;
            USHORT nCnt = 0;
            if( ScriptType::WEAK == nScript )
            {
                nChg = pBreakIt->xBreak->endOfScript( aTxt, 0, nScript );
                if( nChg < aTxt.Len() )
                    nScript = pBreakIt->xBreak->getScriptType( aTxt, nChg );
            }
        }
        BYTE nTmp;
        switch ( nScript ) {
            case ScriptType::LATIN : nTmp = SW_LATIN; break;
            case ScriptType::ASIAN : nTmp = SW_CJK; break;
            case ScriptType::COMPLEX : nTmp = SW_CTL; break;
            default: nTmp = nActual;
        }
        if( nTmp != nActual )
        {
            if( !pFnt )
                pFnt = new SwFont( *rInf.GetFont() );
            pFnt->SetActual( nTmp );
        }
    }
}

sal_Bool SwFldPortion::Format( SwTxtFormatInfo &rInf )
{
    // Scope wegen aDiffTxt::DTOR!
    xub_StrLen nRest;
    sal_Bool bFull;
    sal_Bool bEOL = sal_False;
    long nTxtRest = rInf.GetTxt().Len() - rInf.GetIdx();
    {
        SwFldSlot aDiffTxt( &rInf, this );
        const xub_StrLen nOldFullLen = rInf.GetLen();
        const MSHORT nFollow = IsFollow() ? 0 : 1;
        xub_StrLen nFullLen;
        // In Numerierungen kennen wir keine Platzhalter, sondern
        // nur "normale" Zeichen.
        if( InNumberGrp() )
            nFullLen = nOldFullLen;
        else
        {
            nFullLen = rInf.ScanPortionEnd( rInf.GetIdx() + nOldFullLen,
                                            IsFollow() ) - rInf.GetIdx();
            if( nFullLen && CH_BREAK == aExpand.GetChar( nFullLen - 1 ) )
                --nFullLen;
        }
        BOOL bScriptChg = ScriptChange( rInf, nFullLen );
        rInf.SetLen( nFullLen );
        if( pFnt )
            pFnt->GoMagic( rInf.GetVsh(), pFnt->GetActual() );

        SwFontSave aSave( rInf, pFnt );

        // 8674: Laenge muss 0 sein, bei bFull nach Format ist die Laenge
        // gesetzt und wird in nRest uebertragen. Ansonsten bleibt die
        // Laenge erhalten und wuerde auch in nRest einfliessen!
        SetLen(0);

        // So komisch es aussieht, die Abfrage auf GetLen() muss wegen der
        // ExpandPortions _hinter_ aDiffTxt (vgl. SoftHyphs)
        // sal_False returnen wegen SetFull ...
        if( !nFullLen )
        {
            // nicht Init(), weil wir Hoehe und Ascent brauchen
            Width(0);
            bFull = rInf.Width() <= rInf.GetPos().X();
        }
        else
        {
            xub_StrLen nOldLineStart = rInf.GetLineStart();
            rInf.SetLineStart( 0 );
            rInf.SetNotEOL( nFullLen == nOldFullLen && nTxtRest > nFollow );
            bFull = SwTxtPortion::Format( rInf );
            rInf.SetNotEOL( sal_False );
            rInf.SetLineStart( nOldLineStart );
        }
        xub_StrLen nTmpLen = GetLen();
        bEOL = !nTmpLen && nFollow && bFull;
        nRest = nOldFullLen - nTmpLen;

        // Das Zeichen wird in der ersten Portion gehalten.
        // Unbedingt nach Format!
        SetLen( nFollow );

        if( nRest )
        {
            // aExpand ist noch nicht gekuerzt worden, der neue Ofst
            // ergibt sich durch nRest.
            xub_StrLen nNextOfst = aExpand.Len() - nRest;
            XubString aNew( aExpand, nNextOfst, STRING_LEN );
            aExpand.Erase( nNextOfst, STRING_LEN );

            // Trailingspace et al. !
            switch( aNew.GetChar( 0 ))
            {
                case CH_BREAK  : bFull = sal_True;
                            // kein break;
                case ' ' :
                case CH_TAB    :
                {
                    aNew.Erase( 0, 1 );
                    ++nNextOfst;
                    break;
                }
                default: ;
            }

            if( aNew.Len() )
            {
                // sal_True, weil es ein FollowFeld ist
                // SwFont *pFont = new SwFont( rInf.GetFont()->GetFnt() );
                SwFldPortion *pFld = Clone( aNew );
                if( !pFld->GetFont() )
                {
                    SwFont *pNewFnt = new SwFont( *rInf.GetFont() );
                    pFld->SetFont( pNewFnt );
                }
                pFld->SetFollow( sal_True );
                SetHasFollow( sal_True );
                // In nNextOffset steht bei einem neuangelegten Feld zunaechst
                // der Offset, an dem es selbst im Originalstring beginnt.
                // Wenn beim Formatieren ein FollowFeld angelegt wird, wird
                // der Offset dieses FollowFelds in nNextOffset festgehalten.
                nNextOffset += nNextOfst;
                pFld->SetNextOffset( nNextOffset );
                rInf.SetRest( pFld );
                if( bScriptChg )
                    new SwKernPortion( *this, 284 );
            }
        }
        // 7634 mit ASSERT: bad ascent
        if( pFnt && nTmpLen )
            SetAscent( rInf.GetAscent() );
    }

    if( bFull && !nRest )
    {
        // 8788: BreakCut bei Feldern schleift
        // vgl. BreakCut, Sonderfall Nr.2: Zeichen breiter als Zeile
        // vgl. 5057 und 6721: Zeichen wird abgeschnitten.
        if( 1 != GetLen() || Width() != rInf.RealWidth() )
        {
            aExpand.Erase();
            SetLen(0);
            // 7925: Wenn das Feld komplett auf die naechste Zeile muss
            // dann wird in BreakUnderflow() ein FormatEOL() auf die
            // TxtPortion gerufen. Der Text rInf.pTxt ist allerdings noch
            // manipuliert, hier aber nicht.
            bEOL = sal_True;
        }
    }
    if( bEOL && rInf.GetLast() )
        rInf.GetLast()->FormatEOL( rInf );
    return bFull;
}

/*************************************************************************
 *               virtual SwFldPortion::Paint()
 *************************************************************************/

void SwFldPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    SwFontSave aSave( rInf, pFnt );

    ASSERT( GetLen() <= 1, "SwFldPortion::Paint: rest-portion polution?" );
    if( Width() )
    {
        // Dies ist eine freizuegige Auslegung der Hintergrundbelegung ...
        rInf.DrawViewOpt( *this, POR_FLD );
        SwExpandPortion::Paint( rInf );
    }
}

/*************************************************************************
 *              virtual SwFldPortion::GetExpTxt()
 *************************************************************************/

sal_Bool SwFldPortion::GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const
{
    rTxt = aExpand;
    if( !rTxt.Len() && rInf.OnWin() && rInf.GetOpt().IsField() && !HasFollow() )
        rTxt = ' ';
    return sal_True;
}

/*************************************************************************
 *                virtual SwFldPortion::GetTxtSize()
 *************************************************************************/

SwPosSize SwFldPortion::GetTxtSize( const SwTxtSizeInfo &rInf ) const
{
    SwFontSave aSave( rInf, pFnt );
    SwPosSize aSize( SwExpandPortion::GetTxtSize( rInf ) );
    return aSize;
}

/*************************************************************************
 *                      class SwHiddenPortion
 *************************************************************************/

SwFldPortion *SwHiddenPortion::Clone(const XubString &rExpand ) const
{
    SwFont *pNewFnt;
    if( 0 != ( pNewFnt = pFnt ) )
        pNewFnt = new SwFont( *pFnt );
    return new SwHiddenPortion( rExpand, pNewFnt );
}

/*************************************************************************
 *               virtual SwHiddenPortion::Paint()
 *************************************************************************/

void SwHiddenPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( Width() )
    {
        SwFontSave aSave( rInf, pFnt );
        rInf.DrawViewOpt( *this, POR_HIDDEN );
        SwExpandPortion::Paint( rInf );
    }
}

/*************************************************************************
 *              virtual SwHiddenPortion::GetExpTxt()
 *************************************************************************/

sal_Bool SwHiddenPortion::GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const
{
    // Nicht auf IsHidden() abfragen !
    return SwFldPortion::GetExpTxt( rInf, rTxt );
}

/*************************************************************************
 *                      class SwNumberPortion
 *************************************************************************/

SwNumberPortion::SwNumberPortion( const XubString &rExpand, SwFont *pFnt,
                    const sal_Bool bLft, const sal_Bool bCntr, const KSHORT nMinDst )
        : SwFldPortion( rExpand, pFnt ), nFixWidth(0), nMinDist( nMinDst )
{
    SetWhichPor( POR_NUMBER );
    SetLeft( bLft );
    SetHide( sal_False );
    SetCenter( bCntr );
}

xub_StrLen SwNumberPortion::GetCrsrOfst( const MSHORT ) const
{
    return 0;
}

SwFldPortion *SwNumberPortion::Clone( const XubString &rExpand ) const
{
    SwFont *pNewFnt;
    if( 0 != ( pNewFnt = pFnt ) )
        pNewFnt = new SwFont( *pFnt );
    return new SwNumberPortion( rExpand, pNewFnt, IsLeft(), IsCenter(),
        nMinDist );
}

/*************************************************************************
 *                 virtual SwNumberPortion::Format()
 *************************************************************************/

// 5010: Wir sind in der Lage, mehrzeilige NumFelder anzulegen!
// 3689: Fies ist, wenn man in der Dialogbox soviel Davor-Text
// eingibt, bis die Zeile ueberlaeuft.
// Man muss die Fly-Ausweichmanoever beachten!

sal_Bool SwNumberPortion::Format( SwTxtFormatInfo &rInf )
{
    SetHide( sal_False );
    const sal_Bool bFull = SwFldPortion::Format( rInf );
    SetLen( 0 );
    nFixWidth = Width();
    rInf.SetNumDone( !rInf.GetRest() );
    if( rInf.IsNumDone() )
    {
        SetAscent( rInf.GetAscent() );
        long nDiff = rInf.Left() - rInf.First() + rInf.ForcedLeftMargin();
        // Ein Vorschlag von Juergen und Volkmar:
        // Der Textteil hinter der Numerierung sollte immer
        // mindestens beim linken Rand beginnen.
        if( nDiff < 0 )
            nDiff = 0;
        else if ( nDiff > rInf.X() )
            nDiff -= rInf.X();
        if( nDiff < nFixWidth + nMinDist )
            nDiff = nFixWidth + nMinDist;
        // 2739: Numerierung weicht Fly aus, kein nDiff in der zweiten Runde
        // fieser Sonderfall: FlyFrm liegt in dem Bereich,
        // den wir uns gerade unter den Nagel reissen wollen.
        // Die NumberPortion wird als verborgen markiert.
        if( nDiff > rInf.Width() )
        {
            nDiff = rInf.Width();
            SetHide( sal_True );
        }

        if( Width() < nDiff )
            Width( KSHORT(nDiff) );
    }
    return bFull;
}

void SwNumberPortion::FormatEOL( SwTxtFormatInfo &rInf )
{
/*  Ein FormatEOL deutet daraufhin, dass der folgende Text
 *  nicht mit auf die Zeile passte. Damit die Numerierung mitwandert,
 *  wird diese NumberPortion verborgen.
 */
    SetHide( sal_True );
}

/*************************************************************************
 *               virtual SwNumberPortion::Paint()
 *************************************************************************/

void SwNumberPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
/*  Eine verborgene NumberPortion wird nicht angezeigt, es sei denn, es gibt
 *  Textportions in dieser Zeile oder es gibt ueberhaupt nur eine einzige Zeile.
 */
    if ( IsHide() && rInf.GetParaPortion() && rInf.GetParaPortion()->GetNext() )
    {
        SwLinePortion *pTmp = GetPortion();
        while ( pTmp && !pTmp->InTxtGrp() )
            pTmp = pTmp->GetPortion();
        if ( !pTmp )
            return;
    }

    // Dies ist eine freizuegige Auslegung der Hintergrundbelegung ...
    rInf.DrawViewOpt( *this, POR_NUMBER );

    if( aExpand.Len() )
    {
        const SwFont *pTmpFnt = rInf.GetFont();
        sal_Bool bPaintSpace = ( UNDERLINE_NONE != pTmpFnt->GetUnderline() ||
                             STRIKEOUT_NONE != pTmpFnt->GetStrikeout() ) &&
                            !pTmpFnt->IsWordLineMode();
        if( bPaintSpace && pFnt )
            bPaintSpace = ( UNDERLINE_NONE != pFnt->GetUnderline() ||
                             STRIKEOUT_NONE != pFnt->GetStrikeout() ) &&
                            !pFnt->IsWordLineMode();

        SwFontSave aSave( rInf, pFnt );

        if( nFixWidth == Width() )
            SwExpandPortion::Paint( rInf );
        else
        {
            // logisches const: Width wird wieder zurueckgesetzt
            SwLinePortion *pThis = (SwLinePortion*)this;
            const KSHORT nOldWidth = Width();
            bPaintSpace = bPaintSpace && nFixWidth < nOldWidth;
            KSHORT nSpaceOffs = nFixWidth;
            pThis->Width( nFixWidth );
            if( IsLeft() )
                SwExpandPortion::Paint( rInf );
            else
            {
                SwTxtPaintInfo aInf( rInf );
                KSHORT nOffset = nOldWidth - nFixWidth;
                if( nOffset < nMinDist )
                    nOffset = 0;
                else
                {
                    if( IsCenter() )
                    {
                        nOffset /= 2;
                        if( nOffset < nMinDist )
                            nOffset = nOldWidth - nFixWidth - nMinDist;
                    }
                    else
                        nOffset -= nMinDist;
                }
                aInf.X( aInf.X() + nOffset );
                SwExpandPortion::Paint( aInf );
                if( bPaintSpace )
                    nSpaceOffs += nOffset;
            }
            if( bPaintSpace && nOldWidth > nSpaceOffs )
            {
                SwTxtPaintInfo aInf( rInf );
static sal_Char __READONLY_DATA sDoubleSpace[] = "  ";
                aInf.X( aInf.X() + nSpaceOffs );
                pThis->Width( nOldWidth - nSpaceOffs + 12 );
                {
                    SwTxtSlotLen aDiffTxt( &aInf, this, sDoubleSpace );
                    aInf.DrawText( *this, aInf.GetLen(), sal_True );
                }
            }
            pThis->Width( nOldWidth );
        }
    }
}


/*************************************************************************
 *                      class SwBulletPortion
 *************************************************************************/

SwBulletPortion::SwBulletPortion( const xub_Unicode cBullet, SwFont *pFont,
                    const sal_Bool bLft, const sal_Bool bCntr, const KSHORT nMinDst )
    : SwNumberPortion( XubString( cBullet ), pFont, bLft, bCntr, nMinDst )
{
    SetWhichPor( POR_BULLET );
}

/*************************************************************************
 *                      class SwGrfNumPortion
 *************************************************************************/

#define GRFNUM_SECURE 10

SwGrfNumPortion::SwGrfNumPortion(
        SwFrm *pFrm,
        const SvxBrushItem* pGrfBrush,
        const SwFmtVertOrient* pGrfOrient, const Size& rGrfSize,
        const sal_Bool bLft, const sal_Bool bCntr, const KSHORT nMinDst ) :
    SwNumberPortion( aEmptyStr, NULL, bLft, bCntr, nMinDst ),
    pBrush( new SvxBrushItem() ), nId( 0 )
{
    SetWhichPor( POR_GRFNUM );
    SetAnimated( sal_False );
    bReplace = sal_False;
    if( pGrfBrush )
    {
        *pBrush = *pGrfBrush;
        SwDocShell *pSh = pFrm->GetShell()->GetDoc()->GetDocShell();
        const Graphic* pGraph = pGrfBrush->GetGraphic( pSh );
        if( pGraph )
            SetAnimated( pGraph->IsAnimated() );
        else
            bReplace = sal_True;
    }
    if( pGrfOrient )
    {
        nYPos = pGrfOrient->GetPos();
        eOrient = pGrfOrient->GetVertOrient();
    }
    else
    {
        nYPos = 0;
        eOrient = VERT_TOP;
    }
    Width( rGrfSize.Width() + 2 * GRFNUM_SECURE );
    nFixWidth = Width();
    nGrfHeight = rGrfSize.Height() + 2 * GRFNUM_SECURE;
    Height( nGrfHeight );
    bNoPaint = sal_False;
}

SwGrfNumPortion::~SwGrfNumPortion()
{
    if ( IsAnimated() )
        ( (Graphic*) pBrush->GetGraphic() )->StopAnimation( 0, nId );
    delete pBrush;
}

void SwGrfNumPortion::StopAnimation( OutputDevice *pOut )
{
    if ( IsAnimated() )
        ( (Graphic*) pBrush->GetGraphic() )->StopAnimation( pOut, nId );
}

sal_Bool SwGrfNumPortion::Format( SwTxtFormatInfo &rInf )
{
    SetHide( sal_False );
    Width( nFixWidth );
    const sal_Bool bFull = rInf.Width() < rInf.X() + Width();
    const sal_Bool bFly = rInf.GetFly() ||
        ( rInf.GetLast() && rInf.GetLast()->IsFlyPortion() );
    SetAscent( GetRelPos() > 0 ? GetRelPos() : 0 );
    if( GetAscent() > Height() )
        Height( GetAscent() );

    if( bFull )
    {
        Width( rInf.Width() - rInf.X() );
        if( bFly )
        {
            SetLen( 0 );
            SetNoPaint( sal_True );
            rInf.SetNumDone( sal_False );
            return sal_True;
        }
    }
    rInf.SetNumDone( sal_True );
    long nDiff = rInf.Left() - rInf.First() + rInf.ForcedLeftMargin();
    // Ein Vorschlag von Juergen und Volkmar:
    // Der Textteil hinter der Numerierung sollte immer
    // mindestens beim linken Rand beginnen.
    if( nDiff < 0 )
        nDiff = 0;
    else if ( nDiff > rInf.X() )
        nDiff -= rInf.X();
    if( nDiff < nFixWidth + nMinDist )
        nDiff = nFixWidth + nMinDist;
    // 2739: Numerierung weicht Fly aus, kein nDiff in der zweiten Runde
    // fieser Sonderfall: FlyFrm liegt in dem Bereich,
    // den wir uns gerade unter den Nagel reissen wollen.
    // Die NumberPortion wird als verborgen markiert.
    if( nDiff > rInf.Width() )
    {
        nDiff = rInf.Width();
        if( bFly )
            SetHide( sal_True );
    }

    if( Width() < nDiff )
        Width( KSHORT(nDiff) );
    return bFull;
}

void SwGrfNumPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( DontPaint() )
        return;
/*  Eine verborgene NumberPortion wird nicht angezeigt, es sei denn, es gibt
 *  Textportions in dieser Zeile oder es gibt ueberhaupt nur eine einzige Zeile.
 */
    if ( IsHide() && rInf.GetParaPortion() && rInf.GetParaPortion()->GetNext() )
    {
        SwLinePortion *pTmp = GetPortion();
        while ( pTmp && !pTmp->InTxtGrp() )
            pTmp = pTmp->GetPortion();
        if ( !pTmp )
            return;
    }
    Point aPos( rInf.X() + GRFNUM_SECURE, rInf.Y() - GetRelPos() + GRFNUM_SECURE );
    long nTmpWidth = Max( (long)0, (long)(nFixWidth - 2 * GRFNUM_SECURE) );
    Size aSize( nTmpWidth, GetGrfHeight() - 2 * GRFNUM_SECURE );

    if( nFixWidth < Width() && !IsLeft() )
    {
        KSHORT nOffset = Width() - nFixWidth;
        if( nOffset < nMinDist )
            nOffset = 0;
        else
        {
            if( IsCenter() )
            {
                nOffset /= 2;
                if( nOffset < nMinDist )
                    nOffset = Width() - nFixWidth - nMinDist;
            }
            else
                nOffset -= nMinDist;
        }
        aPos.X() += nOffset;
    }

    if( bReplace )
    {
        KSHORT nTmpH = GetPortion() ? GetPortion()->GetAscent() : 120;
        aSize = Size( nTmpH, nTmpH );
        aPos.Y() = rInf.Y() - nTmpH;
    }
    SwRect aTmp( aPos, aSize );

    sal_Bool bDraw = sal_True;

    if ( IsAnimated() )
    {
        bDraw = !rInf.GetOpt().IsGraphic();
        if( !nId )
        {
            SetId( long( rInf.GetTxtFrm() ) );
            rInf.GetTxtFrm()->SetAnimation();
        }
        if( aTmp.IsOver( rInf.GetPaintRect() ) && !bDraw )
        {
            rInf.NoteAnimation();
            if( OUTDEV_VIRDEV == rInf.GetOut()->GetOutDevType() )
            {
                ( (Graphic*) pBrush->GetGraphic() )->StopAnimation(0,nId);
                rInf.GetTxtFrm()->GetShell()->InvalidateWindows( aTmp );
            }
            else
                ( (Graphic*) pBrush->GetGraphic() )->StartAnimation(
                    (OutputDevice*)rInf.GetOut(), aPos, aSize, nId );
        }
        if( bDraw )
            ( (Graphic*) pBrush->GetGraphic() )->StopAnimation( 0, nId );
    }

    if( bDraw && aTmp.HasArea() )
        DrawGraphic( pBrush, (OutputDevice*)rInf.GetOut(),
            aTmp, rInf.GetPaintRect(), bReplace ? GRFNUM_REPLACE : GRFNUM_YES );
}

void SwGrfNumPortion::SetBase( long nLnAscent, long nLnDescent,
                               long nFlyAsc, long nFlyDesc )
{
    if ( GetOrient() != VERT_NONE )
    {
        SetRelPos( 0 );
        if ( GetOrient() == VERT_CENTER )
            SetRelPos( GetGrfHeight() / 2 );
        else if ( GetOrient() == VERT_TOP )
            SetRelPos( GetGrfHeight() - GRFNUM_SECURE );
        else if ( GetOrient() == VERT_BOTTOM )
            ;
        else if ( GetOrient() == VERT_CHAR_CENTER )
            SetRelPos( ( GetGrfHeight() + nLnAscent - nLnDescent ) / 2 );
        else if ( GetOrient() == VERT_CHAR_TOP )
            SetRelPos( nLnAscent );
        else if ( GetOrient() == VERT_CHAR_BOTTOM )
            SetRelPos( GetGrfHeight() - nLnDescent );
        else
        {
            if( GetGrfHeight() >= nFlyAsc + nFlyDesc )
            {
                // wenn ich genauso gross bin wie die Zeile, brauche ich mich
                // nicht an der Zeile nicht weiter ausrichten, ich lasse
                // dann auch den max. Ascent der Zeile unveraendert

                SetRelPos( nFlyAsc );
            }
            else if ( GetOrient() == VERT_LINE_CENTER )
                SetRelPos( ( GetGrfHeight() + nFlyAsc - nFlyDesc ) / 2 );
            else if ( GetOrient() == VERT_LINE_TOP )
                SetRelPos( nFlyAsc );
            else if ( GetOrient() == VERT_LINE_BOTTOM )
                SetRelPos( GetGrfHeight() - nFlyDesc );
        }
    }
}
void SwTxtFrm::StopAnimation( OutputDevice *pOut )
{
    ASSERT( HasAnimation(), "SwTxtFrm::StopAnimation: Which Animation?" );
    if( HasPara() )
    {
        SwLineLayout *pLine = GetPara();
        while( pLine )
        {
            SwLinePortion *pPor = pLine->GetPortion();
            while( pPor )
            {
                if( pPor->IsGrfNumPortion() )
                    ((SwGrfNumPortion*)pPor)->StopAnimation( pOut );
                // Die Numerierungsportion sitzt immer vor dem ersten Zeichen,
                // deshalb koennen wir abbrechen, sobald wir eine Portion mit
                // einer Laenge > 0 erreicht haben.
                pPor = pPor->GetLen() ? 0 : pPor->GetPortion();
            }
            pLine = pLine->GetLen() ? 0 : pLine->GetNext();
        }
    }
}


