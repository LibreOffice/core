/*************************************************************************
 *
 *  $RCSfile: porfld.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: kz $ $Date: 2003-10-15 09:56:35 $
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


#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif

#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif
#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
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
#ifndef _SW_PORTIONHANDLER_HXX
#include <SwPortionHandler.hxx>
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
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _PORRST_HXX
#include <porrst.hxx>
#endif
#ifndef _PORFTN_HXX
#include <porftn.hxx>   // SwFtnPortion
#endif
#ifndef _ACCESSIBILITYOPTIONS_HXX
#include <accessibilityoptions.hxx>
#endif

using namespace ::com::sun::star;

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
    SwFldPortion* pClone = new SwFldPortion( rExpand, pNewFnt );
    pClone->SetNextOffset( nNextOffset );
    return pClone;
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

SwFldPortion::SwFldPortion( const SwFldPortion& rFld )
    : aExpand( rFld.GetExp() ),
      bCenter( rFld.IsCenter() ),
      bFollow( rFld.IsFollow() ),
      bHasFollow( rFld.HasFollow() ),
      bHide( rFld.IsHide() ),
      bLeft( rFld.IsLeft() ),
      nNextOffset( rFld.GetNextOffset() )
{
    if ( rFld.HasFont() )
        pFnt = new SwFont( *rFld.GetFont() );
    else
        pFnt = 0;

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
    if( !Width() && rInf.OnWin() && !rInf.GetOpt().IsPagePreview() &&
            !rInf.GetOpt().IsReadonly() && SwViewOption::IsFieldShadings() )
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
    SwTxtFormatInfo *pInf;
public:
    SwFldSlot( const SwTxtFormatInfo* pNew, const SwFldPortion *pPor );
    ~SwFldSlot();
};

SwFldSlot::SwFldSlot( const SwTxtFormatInfo* pNew, const SwFldPortion *pPor )
{
    bOn = pPor->GetExpTxt( *pNew, aTxt );

    // Der Text wird ausgetauscht...
    if( bOn )
    {
        pInf = (SwTxtFormatInfo*)pNew;
        nIdx = pInf->GetIdx();
        nLen = pInf->GetLen();
        pOldTxt = &(pInf->GetTxt());
        pInf->SetLen( aTxt.Len() );
        if( pPor->IsFollow() )
        {
            pInf->SetFakeLineStart( nIdx > pInf->GetLineStart() );
            pInf->SetIdx( 0 );
        }
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
        pInf->SetFakeLineStart( sal_False );
    }
}

BYTE SwFldPortion::ScriptChange( const SwTxtSizeInfo &rInf, xub_StrLen& rFull )
{
    BYTE nRet = 0;
    const String& rTxt = rInf.GetTxt();
    rFull += rInf.GetIdx();
    if( rFull > rTxt.Len() )
        rFull = rTxt.Len();
    if( rFull && pBreakIt->xBreak.is() )
    {
        BYTE nActual = pFnt ? pFnt->GetActual() : rInf.GetFont()->GetActual();
        xub_StrLen nChg = rInf.GetIdx();
        USHORT nScript;
        {
            nScript = i18n::ScriptType::LATIN;
            if( nActual )
                nScript = nActual == SW_CJK ? i18n::ScriptType::ASIAN
                                            : i18n::ScriptType::COMPLEX;
            nChg = (xub_StrLen)pBreakIt->xBreak->endOfScript(rTxt,nChg,nScript);
        }
        if( rFull > nChg )
        {
            nRet = nActual;
            nScript = pBreakIt->xBreak->getScriptType( rTxt, nChg );
            if( i18n::ScriptType::ASIAN == nScript )
                nRet += SW_CJK;
            else if( i18n::ScriptType::COMPLEX == nScript )
                nRet += SW_CTL;
            rFull = nChg;
        }
    }
    rFull -= rInf.GetIdx();
    return nRet;
}

void SwFldPortion::CheckScript( const SwTxtSizeInfo &rInf )
{
    String aTxt;
    if( GetExpTxt( rInf, aTxt ) && aTxt.Len() && pBreakIt->xBreak.is() )
    {
        BYTE nActual = pFnt ? pFnt->GetActual() : rInf.GetFont()->GetActual();
        USHORT nScript;
        {
            nScript = pBreakIt->xBreak->getScriptType( aTxt, 0 );
            xub_StrLen nChg = 0;
            USHORT nCnt = 0;
            if( i18n::ScriptType::WEAK == nScript )
            {
                nChg =(xub_StrLen)pBreakIt->xBreak->endOfScript(aTxt,0,nScript);
                if( nChg < aTxt.Len() )
                    nScript = pBreakIt->xBreak->getScriptType( aTxt, nChg );
            }
        }
        BYTE nTmp;
        switch ( nScript ) {
            case i18n::ScriptType::LATIN : nTmp = SW_LATIN; break;
            case i18n::ScriptType::ASIAN : nTmp = SW_CJK; break;
            case i18n::ScriptType::COMPLEX : nTmp = SW_CTL; break;
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
        SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
        aLayoutModeModifier.SetAuto();

        const xub_StrLen nOldFullLen = rInf.GetLen();
        const MSHORT nFollow = IsFollow() ? 0 : 1;
        xub_StrLen nFullLen;

        // Look for portion breaks (special characters like tab, break...)
        nFullLen = rInf.ScanPortionEnd( rInf.GetIdx(),
                   rInf.GetIdx() + nOldFullLen ) - rInf.GetIdx();
        if( nFullLen && CH_BREAK == aExpand.GetChar( nFullLen - 1 ) )
            --nFullLen;

        if ( STRING_LEN != rInf.GetUnderScorePos() &&
             rInf.GetUnderScorePos() > rInf.GetIdx() )
             rInf.SetUnderScorePos( rInf.GetIdx() );

        // field portion has to break if script changes
        BYTE nScriptChg = ScriptChange( rInf, nFullLen );
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
            if( IsFollow() )
                rInf.SetLineStart( 0 );
            rInf.SetNotEOL( nFullLen == nOldFullLen && nTxtRest > nFollow );

            // the height depending on the fields font is set,
            // this is required for SwTxtGuess::Guess
            Height( rInf.GetTxtHeight() );
            // If a kerning portion is inserted after our field portion,
            // the ascent and height must be known
            SetAscent( rInf.GetAscent() );
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

            if ( IsQuoVadisPortion() )
                nNextOfst += ((SwQuoVadisPortion*)this)->GetContTxt().Len();

            XubString aNew( aExpand, nNextOfst, STRING_LEN );
            aExpand.Erase( nNextOfst, STRING_LEN );

            // These characters should not be contained in the follow
            // field portion. They are handled via the HookChar mechanism.
            switch( aNew.GetChar( 0 ))
            {
                case CH_BREAK  : bFull = sal_True;
                            // kein break;
                case ' ' :
                case CH_TAB    :
                case CHAR_HARDHYPHEN:               // non-breaking hyphen
                case CHAR_SOFTHYPHEN:
                case CHAR_HARDBLANK:
                {
                    aNew.Erase( 0, 1 );
                    ++nNextOfst;
                    break;
                }
                default: ;
            }

            // Even if there is no more text left for a follow field,
            // we have to build a follow field portion (without font),
            // otherwise the HookChar mechanism would not work.
            SwFldPortion *pFld = Clone( aNew );
            if( aNew.Len() && !pFld->GetFont() )
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
        }
    }

    if( bEOL && rInf.GetLast() && !rInf.GetUnderFlow() )
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
    if( !rTxt.Len() && rInf.OnWin() &&
        !rInf.GetOpt().IsPagePreview() && !rInf.GetOpt().IsReadonly() &&
            SwViewOption::IsFieldShadings() &&
            !HasFollow() )
        rTxt = ' ';
    return sal_True;
}

/*************************************************************************
 *              virtual SwFldPortion::HandlePortion()
 *************************************************************************/

void SwFldPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), aExpand, GetWhichPor() );
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
    // a numbering portion can be contained in a rotated portion!!!
    nFixWidth = rInf.IsMulti() ? Height() : Width();
    rInf.SetNumDone( !rInf.GetRest() );
    if( rInf.IsNumDone() )
    {
//        SetAscent( rInf.GetAscent() );
        ASSERT( Height() && nAscent, "NumberPortions without Height | Ascent" )

        long nDiff = rInf.Left() - rInf.First() + rInf.ForcedLeftMargin();
        // Ein Vorschlag von Juergen und Volkmar:
        // Der Textteil hinter der Numerierung sollte immer
        // mindestens beim linken Rand beginnen.
        if( nDiff < 0 )
            nDiff = 0;
        else if ( nDiff > rInf.X() )
            nDiff -= rInf.X();
        else
            nDiff = 0;

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

        // A numbering portion can be inside a SwRotatedPortion. Then the
        // Height has to be changed
        if ( rInf.IsMulti() )
        {
            if ( Height() < nDiff )
                Height( KSHORT( nDiff ) );
        }
        else if( Width() < nDiff )
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

    // This caused trouble with flys anchored as characters.
    // If one of these is numbered but does not fit to the line,
    // it calls this function, causing a loop because both the number
    // portion and the fly portion go to the next line
//    SetHide( sal_True );
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

    // calculate the width of the number portion, including follows
    const KSHORT nOldWidth = Width();
    USHORT nSumWidth = 0;
    USHORT nOffset = 0;

    const SwLinePortion* pTmp = this;
    while ( pTmp && pTmp->InNumberGrp() )
    {
        nSumWidth += pTmp->Width();
        if ( ((SwNumberPortion*)pTmp)->HasFollow() )
            pTmp = pTmp->GetPortion();
        else
        {
            nOffset = pTmp->Width() - ((SwNumberPortion*)pTmp)->nFixWidth;
            break;
        }
    }

    // The master portion takes care for painting the background of the
    // follow field portions
    if ( ! IsFollow() )
    {
        SwLinePortion *pThis = (SwLinePortion*)this;
        pThis->Width( nSumWidth );
        rInf.DrawViewOpt( *this, POR_NUMBER );
        pThis->Width( nOldWidth );
    }

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

        if( nFixWidth == Width() && ! HasFollow() )
            SwExpandPortion::Paint( rInf );
        else
        {
            // logisches const: Width wird wieder zurueckgesetzt
            SwLinePortion *pThis = (SwLinePortion*)this;
            bPaintSpace = bPaintSpace && nFixWidth < nOldWidth;
            KSHORT nSpaceOffs = nFixWidth;
            pThis->Width( nFixWidth );

            if( ( IsLeft() && ! rInf.GetTxtFrm()->IsRightToLeft() ) ||
                ( ! IsLeft() && ! IsCenter() && rInf.GetTxtFrm()->IsRightToLeft() ) )
                SwExpandPortion::Paint( rInf );
            else
            {
                SwTxtPaintInfo aInf( rInf );
                if( nOffset < nMinDist )
                    nOffset = 0;
                else
                {
                    if( IsCenter() )
                    {
                        nOffset /= 2;
                        if( nOffset < nMinDist )
                            nOffset = 2 * nOffset - nMinDist;
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
    Height( KSHORT(nGrfHeight) );
    bNoPaint = sal_False;
}

SwGrfNumPortion::~SwGrfNumPortion()
{
    if ( IsAnimated() )
        ( (Graphic*) pBrush->GetGraphic() )->StopAnimation( 0, nId );
    delete pBrush;
}

void SwGrfNumPortion::StopAnimation( OutputDevice* pOut )
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
        Width( rInf.Width() - (KSHORT)rInf.X() );
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


    const sal_Bool bLeft = ( IsLeft() && ! rInf.GetTxtFrm()->IsRightToLeft() ) ||
                           ( ! IsLeft() && ! IsCenter() && rInf.GetTxtFrm()->IsRightToLeft() );


    if( nFixWidth < Width() && !bLeft )
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
            ViewShell* pViewShell = 0;
            if( OUTDEV_VIRDEV == rInf.GetOut()->GetOutDevType() )
            {
                ( (Graphic*) pBrush->GetGraphic() )->StopAnimation(0,nId);
                rInf.GetTxtFrm()->GetShell()->InvalidateWindows( aTmp );
            }
            // first check accessibility options before starting animation
            else if ( 0 != (pViewShell = rInf.GetTxtFrm()->GetShell()) &&
                     ! pViewShell->GetAccessibilityOptions()->IsStopAnimatedGraphics() &&
                      ! pViewShell->IsPreView() )
            {
                ( (Graphic*) pBrush->GetGraphic() )->StartAnimation(
                    (OutputDevice*)rInf.GetOut(), aPos, aSize, nId );
            }
            else
                bDraw = sal_True;
        }
        if( bDraw )
            ( (Graphic*) pBrush->GetGraphic() )->StopAnimation( 0, nId );
    }

    SwRect aRepaint( rInf.GetPaintRect() );
    const SwTxtFrm& rFrm = *rInf.GetTxtFrm();
    if( rFrm.IsVertical() )
    {
        rFrm.SwitchHorizontalToVertical( aTmp );
        rFrm.SwitchHorizontalToVertical( aRepaint );
    }

#ifdef BIDI
    if( rFrm.IsRightToLeft() )
    {
        rFrm.SwitchLTRtoRTL( aTmp );
        rFrm.SwitchLTRtoRTL( aRepaint );
    }
#endif

    if( bDraw && aTmp.HasArea() )
        DrawGraphic( pBrush, (OutputDevice*)rInf.GetOut(),
            aTmp, aRepaint, bReplace ? GRFNUM_REPLACE : GRFNUM_YES );
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

void SwTxtFrm::StopAnimation( OutputDevice* pOut )
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

/*************************************************************************
 * SwCombinedPortion::SwCombinedPortion(..)
 * initializes the script array and clears the width array
 *************************************************************************/

SwCombinedPortion::SwCombinedPortion( const XubString &rTxt )
     : SwFldPortion( rTxt )
{
    SetLen(1);
    SetWhichPor( POR_COMBINED );
    if( aExpand.Len() > 6 )
        aExpand.Erase( 6 );
    // Initialization of the scripttype array,
    // the arrays of width and position are filled by the format function
    if( pBreakIt->xBreak.is() )
    {
        BYTE nScr = SW_SCRIPTS;
        for( USHORT i = 0; i < rTxt.Len(); ++i )
        {
            USHORT nScript = pBreakIt->xBreak->getScriptType( rTxt, i );
            switch ( nScript ) {
                case i18n::ScriptType::LATIN : nScr = SW_LATIN; break;
                case i18n::ScriptType::ASIAN : nScr = SW_CJK; break;
                case i18n::ScriptType::COMPLEX : nScr = SW_CTL; break;
            }
            aScrType[i] = nScr;
        }
    }
    else
    {
        for( USHORT i = 0; i < 6; aScrType[i++] = 0 )
            ; // nothing
    }
    memset( &aWidth, 0, sizeof(aWidth) );
}

/*************************************************************************
 * SwCombinedPortion::Paint(..)
 *************************************************************************/

void SwCombinedPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    ASSERT( GetLen() <= 1, "SwFldPortion::Paint: rest-portion polution?" );
    if( Width() )
    {
        rInf.DrawBackBrush( *this );
        rInf.DrawViewOpt( *this, POR_FLD );

        // do we have to repaint a post it portion?
        if( rInf.OnWin() && pPortion && !pPortion->Width() )
            pPortion->PrePaint( rInf, this );

        USHORT nCount = aExpand.Len();
        if( !nCount )
            return;
        ASSERT( nCount < 7, "Too much combined characters" );

        // the first character of the second row
        USHORT nTop = ( nCount + 1 ) / 2;

        SwFont aTmpFont( *rInf.GetFont() );
        aTmpFont.SetProportion( nProportion );  // a smaller font
        SwFontSave aFontSave( rInf, &aTmpFont );

        USHORT i = 0;
        Point aOldPos = rInf.GetPos();
        Point aOutPos( aOldPos.X(), aOldPos.Y() - nUpPos );// Y of the first row
        while( i < nCount )
        {
            if( i == nTop ) // change the row
                aOutPos.Y() = aOldPos.Y() + nLowPos;    // Y of the second row
            aOutPos.X() = aOldPos.X() + aPos[i];        // X position
            const BYTE nAct = aScrType[i];              // script type
            aTmpFont.SetActual( nAct );
            // if there're more than 4 characters to display, we choose fonts
            // with 2/3 of the original font width.
            if( aWidth[ nAct ] )
            {
                Size aTmpSz = aTmpFont.GetSize( nAct );
                if( aTmpSz.Width() != aWidth[ nAct ] )
                {
                    aTmpSz.Width() = aWidth[ nAct ];
                    aTmpFont.SetSize( aTmpSz, nAct );
                }
            }
            ((SwTxtPaintInfo&)rInf).SetPos( aOutPos );
            rInf.DrawText( aExpand, *this, i, 1 );
            ++i;
        }
        // rInf is const, so we have to take back our manipulations
        ((SwTxtPaintInfo&)rInf).SetPos( aOldPos );
    }
}

/*************************************************************************
 * SwCombinedPortion::Format(..)
 *************************************************************************/

sal_Bool SwCombinedPortion::Format( SwTxtFormatInfo &rInf )
{
    USHORT nCount = aExpand.Len();
    if( !nCount )
    {
        Width( 0 );
        return sal_False;
    }

    ASSERT( nCount < 7, "Too much combined characters" );
    // If there are leading "weak"-scripttyped characters in this portion,
    // they get the actual scripttype.
    USHORT i = 0;
    while( i < nCount && SW_SCRIPTS == aScrType[i] )
        aScrType[i++] = rInf.GetFont()->GetActual();
    if( nCount > 4 )
    {
        // more than four? Ok, then we need the 2/3 font width
        i = 0;
        while( i < aExpand.Len() )
        {
            ASSERT( aScrType[i] < SW_SCRIPTS, "Combined: Script fault" );
            if( !aWidth[ aScrType[i] ] )
            {
                rInf.GetOut()->SetFont( rInf.GetFont()->GetFnt( aScrType[i] ) );
                aWidth[ aScrType[i] ] = 2*
                            rInf.GetOut()->GetFontMetric().GetSize().Width()/3;
            }
            ++i;
        }
    }

    USHORT nTop = ( nCount + 1 ) / 2; // the first character of the second line
    ViewShell *pSh = rInf.GetTxtFrm()->GetShell();
    SwFont aTmpFont( *rInf.GetFont() );
    SwFontSave aFontSave( rInf, &aTmpFont );
    nProportion = 55;
    // In nMainAscent/Descent we store the ascent and descent
    // of the original surrounding font
    USHORT nMaxDescent, nMaxAscent, nMaxWidth;
    USHORT nMainDescent = rInf.GetFont()->GetHeight( pSh, *rInf.GetOut() );
    const USHORT nMainAscent = rInf.GetFont()->GetAscent( pSh, *rInf.GetOut() );
    nMainDescent -= nMainAscent;
    // we start with a 50% font, but if we notice that the combined portion
    // becomes bigger than the surrounding font, we check 45% and maybe 40%.
    do
    {
        nProportion -= 5;
        aTmpFont.SetProportion( nProportion );
        i = 0;
        memset( &aPos, 0, sizeof(aPos) );
        nMaxDescent = 0;
        nMaxAscent = 0;
        nMaxWidth = 0;
        nUpPos = nLowPos = 0;

        // Now we get the width of all characters.
        // The ascent and the width of the first line are stored in the
        // ascent member of the portion, the descent in nLowPos.
        // The ascent, descent and width of the second line are stored in the
        // local nMaxAscent, nMaxDescent and nMaxWidth variables.
        while( i < nCount )
        {
            BYTE nScrp = aScrType[i];
            aTmpFont.SetActual( nScrp );
            if( aWidth[ nScrp ] )
            {
                Size aFontSize( aTmpFont.GetSize( nScrp ) );
                aFontSize.Width() = aWidth[ nScrp ];
                aTmpFont.SetSize( aFontSize, nScrp );
            }

            SwDrawTextInfo aDrawInf( pSh, *rInf.GetOut(), 0, aExpand, i, 1 );
            Size aSize = aTmpFont._GetTxtSize( aDrawInf );
            USHORT nAsc = aTmpFont.GetAscent( pSh, *rInf.GetOut() );
            aPos[ i ] = (USHORT)aSize.Width();
            if( i == nTop ) // enter the second line
            {
                nLowPos = nMaxDescent;
                Height( nMaxDescent + nMaxAscent );
                Width( nMaxWidth );
                SetAscent( nMaxAscent );
                nMaxAscent = 0;
                nMaxDescent = 0;
                nMaxWidth = 0;
            }
            nMaxWidth += aPos[ i++ ];
            if( nAsc > nMaxAscent )
                nMaxAscent = nAsc;
            if( aSize.Height() - nAsc > nMaxDescent )
                nMaxDescent = aSize.Height() - nAsc;
        }
        // for one or two characters we double the width of the portion
        if( nCount < 3 )
        {
            nMaxWidth *= 2;
            Width( 2*Width() );
            if( nCount < 2 )
            {
                Height( nMaxAscent + nMaxDescent );
                nLowPos = nMaxDescent;
            }
        }
        Height( Height() + nMaxDescent + nMaxAscent );
        nUpPos = nMaxAscent;
        SetAscent( Height() - nMaxDescent - nLowPos );
    } while( nProportion > 40 && ( GetAscent() > nMainAscent ||
                                    Height() - GetAscent() > nMainDescent ) );
    // if the combined portion is smaller than the surrounding text,
    // the portion grows. This looks better, if there's a character background.
    if( GetAscent() < nMainAscent )
    {
        Height( Height() + nMainAscent - GetAscent() );
        SetAscent( nMainAscent );
    }
    if( Height() < nMainAscent + nMainDescent )
        Height( nMainAscent + nMainDescent );

    // We calculate the x positions of the characters in both lines..
    USHORT nTopDiff = 0;
    USHORT nBotDiff = 0;
    if( nMaxWidth > Width() )
    {
        nTopDiff = ( nMaxWidth - Width() ) / 2;
        Width( nMaxWidth );
    }
    else
        nBotDiff = ( Width() - nMaxWidth ) / 2;
    switch( nTop)
    {
        case 3: aPos[1] = aPos[0] + nTopDiff;  // no break
        case 2: aPos[nTop-1] = Width() - aPos[nTop-1];
    }
    aPos[0] = 0;
    switch( nCount )
    {
        case 5: aPos[4] = aPos[3] + nBotDiff;   // no break
        case 3: aPos[nTop] = nBotDiff;          break;
        case 6: aPos[4] = aPos[3] + nBotDiff;   // no break
        case 4: aPos[nTop] = 0;                 // no break
        case 2: aPos[nCount-1] = Width() - aPos[nCount-1];
    }

    // Does the combined portion fit the line?
    const sal_Bool bFull = rInf.Width() < rInf.X() + Width();
    if( bFull )
    {
        if( rInf.GetLineStart() == rInf.GetIdx() && (!rInf.GetLast()->InFldGrp()
            || !((SwFldPortion*)rInf.GetLast())->IsFollow() ) )
            Width( (USHORT)( rInf.Width() - rInf.X() ) );
        else
        {
            Truncate();
            Width( 0 );
            SetLen( 0 );
            if( rInf.GetLast() )
                rInf.GetLast()->FormatEOL( rInf );
        }
    }
    return bFull;
}

/*************************************************************************
 * SwCombinedPortion::GetViewWidth(..)
 *************************************************************************/

KSHORT SwCombinedPortion::GetViewWidth( const SwTxtSizeInfo &rInf ) const
{
    if( !GetLen() ) // for the dummy part at the end of the line, where
        return 0;   // the combined portion doesn't fit.
    return SwFldPortion::GetViewWidth( rInf );
}
