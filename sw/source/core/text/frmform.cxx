/*************************************************************************
 *
 *  $RCSfile: frmform.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2000-10-16 13:11:59 $
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

#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif

#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>      // ChangeFtnRef
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>        // MakeFrm()
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>     // SwDrawContact
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>      // SwVirtFlyDrawObj
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _FTNFRM_HXX
#include <ftnfrm.hxx>       // SwFtnFrm
#endif
#ifndef _TXTFTN_HXX
#include <txtftn.hxx>
#endif
#ifndef _FMTFTN_HXX
#include <fmtftn.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>      // SwViewOptions
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>       // ViewShell
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FLYFRMS_HXX
#include <flyfrms.hxx>
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _TXTCFG_HXX
#include <txtcfg.hxx>
#endif
#ifndef _ITRFORM2_HXX
#include <itrform2.hxx>     // SwTxtFormatter
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>       // SwTxtFrm
#endif
#ifndef _WIDORP_HXX
#include <widorp.hxx>       // Widows and Orphans
#endif
#ifndef _TXTCACHE_HXX
#include <txtcache.hxx>
#endif
#ifndef _PORRST_HXX
#include <porrst.hxx>       // SwEmptyPortion
#endif
#ifndef _BLINK_HXX
#include <blink.hxx>        // pBlink
#endif
#ifndef _PORFLD_HXX
#include <porfld.hxx>       // SwFldPortion
#endif
#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>      // SwSectionFrm
#endif
#ifndef _PORMULTI_HXX
#include <pormulti.hxx>     // SwMultiPortion
#endif

#ifndef PRODUCT
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>       // SwFrmFmt
#endif
#endif

extern FASTBOOL IsInProgress( const SwFlyFrm *pFly );

class FormatLevel
{
    static MSHORT nLevel;
public:
    inline FormatLevel()  { ++nLevel; }
    inline ~FormatLevel() { --nLevel; }
    inline MSHORT GetLevel() const { return nLevel; }
    static sal_Bool LastLevel() { return 10 < nLevel; }
};
MSHORT FormatLevel::nLevel = 0;

/*************************************************************************
 *                          ValidateTxt/Frm()
 *************************************************************************/

void ValidateTxt( SwFrm *pFrm )     // Freund vom Frame
{
    if ( pFrm->Frm().Width() == pFrm->GetUpper()->Prt().Width() )
        pFrm->bValidSize = sal_True;
/*
    pFrm->bValidPrtArea = sal_True;
    //Die Position validieren um nicht unnoetige (Test-)Moves zu provozieren.
    //Dabei darf allerdings nicht eine tatsaechlich falsche Coordinate
    //validiert werden.
    if ( !pFrm->bValidPos )
    {
        //Leider muessen wir dazu die korrekte Position berechnen.
        Point aOld( pFrm->Frm().Pos() );
        pFrm->MakePos();
        if ( aOld != pFrm->Pos() )
        {
            pFrm->Frm().Pos( aOld );
            pFrm->bValidPos = sal_False;
        }
    }
*/
}

void SwTxtFrm::ValidateFrm()
{
    // Umgebung validieren, um Oszillationen zu verhindern.

    if ( !IsInFly() )
    {   //Innerhalb eines Flys nur this validieren, der Rest sollte eigentlich
        //nur fuer Fussnoten notwendig sein und die gibt es innerhalb von
        //Flys nicht. Fix fuer 5544
        SwSectionFrm* pSct = FindSctFrm();
        if( pSct )
        {
            if( !pSct->IsColLocked() )
                pSct->ColLock();
            else
                pSct = NULL;
        }

        SwFrm *pUp = GetUpper();
        pUp->Calc();
        if( pSct )
            pSct->ColUnlock();
    }
    ValidateTxt( this );

    //MA: mindestens das MustFit-Flag muessen wir retten!
    ASSERT( HasPara(), "ResetPreps(), missing ParaPortion." );
    SwParaPortion *pPara = GetPara();
    const sal_Bool bMustFit = pPara->IsPrepMustFit();
    ResetPreps();
    pPara->SetPrepMustFit( bMustFit );
}

/*************************************************************************
 *                          ValidateBodyFrm()
 *************************************************************************/

// nach einem RemoveFtn muss der BodyFrm und alle innenliegenden kalkuliert
// werden, damit die DeadLine richtig sitzt.
// Erst wird nach aussen hin gesucht, beim Rueckweg werden alle kalkuliert.

void _ValidateBodyFrm( SwFrm *pFrm )
{
    if( pFrm )
    {
        if( !pFrm->IsBodyFrm() && pFrm->GetUpper() )
            _ValidateBodyFrm( pFrm->GetUpper() );
        if( !pFrm->IsSctFrm() )
            pFrm->Calc();
        else
        {
            sal_Bool bOld = ((SwSectionFrm*)pFrm)->IsCntntLocked();
            ((SwSectionFrm*)pFrm)->SetCntntLock( sal_True );
            pFrm->Calc();
            if( !bOld )
                ((SwSectionFrm*)pFrm)->SetCntntLock( sal_False );
        }
    }
}

void SwTxtFrm::ValidateBodyFrm()
{
     //siehe Kommtar in ValidateFrm()
    if ( !IsInFly() && !( IsInSct() && FindSctFrm()->Lower()->IsColumnFrm() ) )
        _ValidateBodyFrm( GetUpper() );
}

/*************************************************************************
 *                      SwTxtFrm::FindBodyFrm()
 *************************************************************************/

sal_Bool SwTxtFrm::_GetDropRect( SwRect &rRect ) const
{
    ASSERT( HasPara(), "SwTxtFrm::_GetDropRect: try again next year." );
    SwTxtSizeInfo aInf( (SwTxtFrm*)this );
    SwTxtMargin aLine( (SwTxtFrm*)this, &aInf );
    if( aLine.GetDropLines() )
    {
        rRect.Top( aLine.Y() );
        rRect.Left( aLine.GetLineStart() );
        rRect.Height( aLine.GetDropHeight() );
        rRect.Width( aLine.GetDropLeft() );
        return sal_True;
    }
    return sal_False;
}

/*************************************************************************
 *                      SwTxtFrm::FindBodyFrm()
 *************************************************************************/

const SwBodyFrm *SwTxtFrm::FindBodyFrm() const
{
    if ( IsInDocBody() )
    {
        const SwFrm *pFrm = GetUpper();
        while( pFrm && !pFrm->IsBodyFrm() )
            pFrm = pFrm->GetUpper();
        return (const SwBodyFrm*)pFrm;
    }
    return 0;
}

/*************************************************************************
 *                      SwTxtFrm::CalcFollow()
 *************************************************************************/

sal_Bool SwTxtFrm::CalcFollow( const xub_StrLen nTxtOfst )
{
    ASSERT( HasFollow(), "CalcFollow: missing Follow." );

    SwTxtFrm *pFollow = GetFollow();

    SwParaPortion *pPara = GetPara();
    sal_Bool bFollowFld = pPara ? pPara->IsFollowField() : sal_False;

    if( !pFollow->GetOfst() || pFollow->GetOfst() != nTxtOfst ||
        bFollowFld || pFollow->IsFieldFollow() || !pFollow->Prt().Height() )
    {
#ifndef PRODUCT
        const SwFrm *pOldUp = GetUpper();
#endif

        SwTwips nOldBottom = GetUpper()->Frm().Bottom();
        SwTwips nMyPos = Frm().Top();

        const SwPageFrm *pPage = 0;
        sal_Bool  bOldInvaCntnt,
              bOldInvaLayout;
        if ( !IsInFly() && GetNext() )
        {
            pPage = FindPageFrm();
            //Minimieren - sprich ggf. zuruecksetzen - der Invalidierungen s.u.
            bOldInvaCntnt  = pPage->IsInvalidCntnt();
            bOldInvaLayout = pPage->IsInvalidLayout();
        }

        pFollow->_SetOfst( nTxtOfst );
        pFollow->SetFieldFollow( bFollowFld );
        if( HasFtn() || pFollow->HasFtn() )
        {
            ValidateFrm();
            ValidateBodyFrm();
            if( pPara )
            {
                *(pPara->GetReformat()) = SwCharRange();
                *(pPara->GetDelta()) = 0;
            }
        }

        //Der Fussnotenbereich darf sich keinesfalls vergrossern.
        SwSaveFtnHeight aSave( FindFtnBossFrm( sal_True ), LONG_MAX );

        ((SwTxtFrm*)pFollow)->CalcFtnFlag();
        if ( !pFollow->GetNext() && !pFollow->HasFtn() )
            nOldBottom = LONG_MAX;

        while( sal_True )
        {
            if( !FormatLevel::LastLevel() )
            {
                // Weenn der Follow in einem spaltigen Bereich oder einem
                // spaltigen Rahmen steckt, muss zunaechst dieser kalkuliert
                // werden, da das FormatWidthCols() nicht funktioniert, wenn
                // es aus dem MakeAll des _gelockten_ Follows heraus gerufen
                // wird.
                SwSectionFrm* pSct = pFollow->FindSctFrm();
                if( pSct && !pSct->IsAnLower( this ) )
                {
                    if( pSct->GetFollow() )
                        pSct->SimpleFormat();
                    else if( !pSct->Frm().Height() )
                        break;
                }
                pFollow->Calc();

                // Der Follow merkt anhand seiner Frm().Height(), dass was schief
                // gelaufen ist.
                ASSERT( !pFollow->GetPrev(), "SwTxtFrm::CalcFollow: cheesy follow" );
                if( pFollow->GetPrev() )
                {
                    pFollow->Prepare( PREP_CLEAR );
                    pFollow->Calc();
                    ASSERT( !pFollow->GetPrev(), "SwTxtFrm::CalcFollow: very cheesy follow" );
                }

                //Sicherstellen, dass der Follow gepaintet wird.
                pFollow->SetCompletePaint();
            }

            pPara = GetPara();
            //Solange der Follow wg. Orphans Zeilen angefordert, bekommt er
            //diese und wird erneut formatiert, falls moeglich.
            if( pPara && pPara->IsPrepWidows() )
                CalcPreps();
            else
                break;
        }

        if( HasFtn() || pFollow->HasFtn() )
        {
            ValidateBodyFrm();
            ValidateFrm();
            if( pPara )
            {
                *(pPara->GetReformat()) = SwCharRange();
                *(pPara->GetDelta()) = 0;
            }
        }

        if ( pPage )
        {
            if ( !bOldInvaCntnt )
                pPage->ValidateCntnt();
            if ( !bOldInvaLayout && !IsInSct() )
                pPage->ValidateLayout();
        }

#ifndef PRODUCT
        ASSERT( pOldUp == GetUpper(), "SwTxtFrm::CalcFollow: heavy follow" );
#endif

        if( nOldBottom < GetUpper()->Frm().Bottom()  && !GetUpper()->IsSctFrm()
            && GetUpper()->Frm().Bottom() - nOldBottom != Frm().Top() - nMyPos )
            return sal_True;
    }
    return sal_False;
}

/*************************************************************************
 *                      SwTxtFrm::AdjustFrm()
 *************************************************************************/

void SwTxtFrm::AdjustFrm( const SwTwips nChgHght, sal_Bool bHasToFit )
{
    if( IsUndersized() )
    {
        if( GetOfst() && !IsFollow() ) // ein gescrollter Absatz (undersized)
            return;
        SetUndersized( nChgHght == 0 || bHasToFit );
    }
    // Die Size-Variable des Frames wird durch Grow inkrementiert
    // oder durch Shrink dekrementiert. Wenn die Groesse
    // unveraendert ist, soll nichts passieren!
    if( nChgHght >= 0)
    {
        if( nChgHght && !bHasToFit )
        {
            if( IsInFtn() && !IsInSct() )
            {
                SwTwips nReal = Grow( nChgHght, pHeight, sal_True );
                if( nReal < nChgHght )
                {
                    SwTwips nBot = Frm().Top() + Frm().Height() + nChgHght
                                   - nReal;
                    SwFrm* pCont = FindFtnFrm()->GetUpper();
                    if( nBot > pCont->Frm().Top() + pCont->Frm().Height() )
                    {
                        Frm().SSize().Height() += nChgHght;
                        Prt().SSize().Height() += nChgHght;
                        return;
                    }
                }
            }

            Grow( nChgHght, pHeight );

            if ( IsInFly() )
            {
                //MA 06. May. 93: Wenn einer der Upper ein Fly ist, so ist es
                //sehr wahrscheinlich, dass dieser Fly durch das Grow seine
                //Position veraendert - also muss auch meine Position korrigiert
                //werden (sonst ist die Pruefung s.u. nicht aussagekraeftig).
                //Die Vorgaenger muessen berechnet werden, damit die Position
                //korrekt berechnet werden kann.
                if ( GetPrev() )
                {
                    SwFrm *pPre = GetUpper()->Lower();
                    do
                    {   pPre->Calc();
                        pPre = pPre->GetNext();
                    } while ( pPre && pPre != this );
                }
                const Point aOldPos( Frm().Pos() );
                MakePos();
                if ( aOldPos != Frm().Pos() )
                    CalcFlys( sal_True );   //#43679# Fly in Fly in ...
            }
        }
        // Ein Grow() wird von der Layout-Seite immer akzeptiert,
        // also auch, wenn die FixSize des umgebenden Layoutframes
        // dies nicht zulassen sollte. Wir ueberpruefen diesen
        // Fall und korrigieren die Werte.
        // MA 06. May. 93: Der Frm darf allerdings auch im Notfall nicht
        // weiter geschrumpft werden als es seine Groesse zulaesst.
        SwTwips nRstHeight = GetUpper()->Frm().Top()
                            + GetUpper()->Prt().Top()
                            + GetUpper()->Prt().Height()
                            - Frm().Top();
        //In Tabellenzellen kann ich mir evtl. noch ein wenig dazuholen, weil
        //durch eine vertikale Ausrichtung auch oben noch Raum sein kann.
        if ( IsInTab() )
        {
            long nAdd = GetUpper()->Lower()->Frm().Top() -
                            (GetUpper()->Frm().Top() + GetUpper()->Prt().Top());
            nRstHeight += nAdd;
        }

/* ------------------------------------
 * #50964#: nRstHeight < 0 bedeutet, dass der TxtFrm komplett ausserhalb seines
 * Upper liegt. Dies kann passieren, wenn er innerhalb eines FlyAtCntFrm liegt, der
 * durch das Grow() die Seite gewechselt hat. In so einem Fall ist es falsch, der
 * folgenden Grow-Versuch durchzufuehren. Im Bugfall fuehrte dies sogar zur
 * Endlosschleife.
 * -----------------------------------*/
        if( nRstHeight < Frm().Height() )
        {
            //Kann sein, dass ich die richtige Grosse habe, der Upper aber zu
            //klein ist und der Upper noch Platz schaffen kann.
            if( ( nRstHeight > 0 || ( IsInFtn() && IsInSct() ) ) && !bHasToFit )
                nRstHeight += GetUpper()->Grow( Frm().Height()-nRstHeight, pHeight );
            // In spaltigen Bereichen wollen wir moeglichst nicht zu gross werden, damit
            // nicht ueber GetNextSctLeaf weitere Bereiche angelegt werden. Stattdessen
            // schrumpfen wir und notieren bUndersized, damit FormatWidthCols die richtige
            // Spaltengroesse ermitteln kann.
            if ( nRstHeight < Frm().Height() )
            {
                if( bHasToFit || !IsMoveable() ||
                    ( IsInSct() && !FindSctFrm()->MoveAllowed(this) ) )
                {
                    SetUndersized( sal_True );
                    Shrink( Min( (Frm().Height() - nRstHeight), Prt().Height()),
                                  pHeight );
                }
                else
                    SetUndersized( sal_False );
            }
        }
    }
    else if ( nChgHght )
        Shrink( -nChgHght, pHeight );
}

/*************************************************************************
 *                      SwTxtFrm::AdjustFollow()
 *************************************************************************/

/* AdjustFollow erwartet folgende Situation:
 * Der SwTxtIter steht am unteren Ende des Masters, der Offset wird
 * im Follow eingestellt.
 * nOffset haelt den Offset im Textstring, ab dem der Master abschliesst
 * und der Follow beginnt. Wenn er 0 ist, wird der FolgeFrame geloescht.
 */

void SwTxtFrm::_AdjustFollow( SwTxtFormatter &rLine,
                             const xub_StrLen nOffset, const xub_StrLen nEnd,
                             const sal_uInt8 nMode )
{
    // Wir haben den Rest der Textmasse: alle Follows loeschen
    // Sonderfall sind DummyPortions()
    if( HasFollow() && !(nMode & 1) && nOffset == nEnd )
    {
        while( GetFollow() )
        {
            if( ((SwTxtFrm*)GetFollow())->IsLocked() )
            {
                ASSERT( sal_False, "+SwTxtFrm::JoinFrm: Follow ist locked." );
                return;
            }
            JoinFrm();
        }
        return;
    }

    // Tanz auf dem Vulkan: Wir formatieren eben schnell noch einmal
    // die letzte Zeile fuer das QuoVadis-Geraffel. Selbstverstaendlich
    // kann sich dadurch auch der Offset verschieben:
    const xub_StrLen nNewOfst = ( IsInFtn() && ( !GetIndNext() || HasFollow() ) ) ?
                            rLine.FormatQuoVadis(nOffset) : nOffset;

    if( !(nMode & 1) )
    {
        // Wir klauen unseren Follows Textmasse, dabei kann es passieren,
        // dass wir einige Follows Joinen muessen.
        while( GetFollow() && GetFollow()->GetFollow() &&
               nNewOfst >= GetFollow()->GetFollow()->GetOfst() )
        {
            DBG_LOOP;
            JoinFrm();
        }
    }

    // Der Ofst hat sich verschoben.
    if( GetFollow() )
    {
#ifdef DEBUG
        static sal_Bool bTest = sal_False;
        if( !bTest || ( nMode & 1 ) )
#endif
        if ( nMode )
            GetFollow()->ManipOfst( 0 );

        if ( CalcFollow( nNewOfst ) )   // CalcFollow erst zum Schluss, dort erfolgt ein SetOfst
            rLine.SetOnceMore( sal_True );
    }
}

/*************************************************************************
 *                      SwTxtFrm::JoinFrm()
 *************************************************************************/

SwCntntFrm *SwTxtFrm::JoinFrm()
{
    ASSERT( GetFollow(), "+SwTxtFrm::JoinFrm: no follow" );
    SwTxtFrm  *pFoll = GetFollow();

    SwTxtFrm *pNxt = pFoll->GetFollow();

    // Alle Fussnoten des zu zerstoerenden Follows werden auf uns
    // umgehaengt.
    xub_StrLen nStart = pFoll->GetOfst();
    if ( pFoll->HasFtn() )
    {
        const SwpHints *pHints = pFoll->GetTxtNode()->GetpSwpHints();
        if( pHints )
        {
            SwFtnBossFrm *pFtnBoss = 0;
            SwFtnBossFrm *pEndBoss = 0;
            for( MSHORT i = 0; i < pHints->Count(); ++i )
            {
                const SwTxtAttr *pHt = (*pHints)[i];
                if( RES_TXTATR_FTN==pHt->Which() && *pHt->GetStart()>=nStart )
                {
                    if( pHt->GetFtn().IsEndNote() )
                    {
                        if( !pEndBoss )
                            pEndBoss = pFoll->FindFtnBossFrm();
                        pEndBoss->ChangeFtnRef( pFoll, (SwTxtFtn*)pHt, this );
                    }
                    else
                    {
                        if( !pFtnBoss )
                            pFtnBoss = pFoll->FindFtnBossFrm( sal_True );
                        pFtnBoss->ChangeFtnRef( pFoll, (SwTxtFtn*)pHt, this );
                    }
                    SetFtn( sal_True );
                }
            }
        }
    }

#ifndef PRODUCT
    else
    {
        pFoll->CalcFtnFlag();
        ASSERT( !pFoll->HasFtn(), "Missing FtnFlag." );
    }
#endif

    pFoll->MoveFlyInCnt( this, nStart, STRING_LEN );
    pFoll->Cut();
    delete pFoll;
    pFollow = pNxt;
    return pNxt;
}

/*************************************************************************
 *                      SwTxtFrm::SplitFrm()
 *************************************************************************/

SwCntntFrm *SwTxtFrm::SplitFrm( const xub_StrLen nTxtPos )
{
    // Durch das Paste wird ein Modify() an mich verschickt.
    // Damit meine Daten nicht verschwinden, locke ich mich.
    SwTxtFrmLocker aLock( this );
    SwTxtFrm *pNew = (SwTxtFrm *)(GetTxtNode()->MakeFrm());
    pNew->bIsFollow = sal_True;

    pNew->SetFollow( GetFollow() );
    SetFollow( pNew );

    pNew->Paste( GetUpper(), GetNext() );

    // Wenn durch unsere Aktionen Fussnoten in pNew landen,
    // so muessen sie umgemeldet werden.
    if ( HasFtn() )
    {
        const SwpHints *pHints = GetTxtNode()->GetpSwpHints();
        if( pHints )
        {
            SwFtnBossFrm *pFtnBoss = 0;
            SwFtnBossFrm *pEndBoss = 0;
            for( MSHORT i = 0; i < pHints->Count(); ++i )
            {
                const SwTxtAttr *pHt = (*pHints)[i];
                if( RES_TXTATR_FTN==pHt->Which() && *pHt->GetStart()>=nTxtPos )
                {
                    if( pHt->GetFtn().IsEndNote() )
                    {
                        if( !pEndBoss )
                            pEndBoss = FindFtnBossFrm();
                        pEndBoss->ChangeFtnRef( this, (SwTxtFtn*)pHt, pNew );
                    }
                    else
                    {
                        if( !pFtnBoss )
                            pFtnBoss = FindFtnBossFrm( sal_True );
                        pFtnBoss->ChangeFtnRef( this, (SwTxtFtn*)pHt, pNew );
                    }
                    pNew->SetFtn( sal_True );
                }
            }
        }
    }

#ifndef PRODUCT
    else
    {
        CalcFtnFlag( nTxtPos-1 );
        ASSERT( !HasFtn(), "Missing FtnFlag." );
    }
#endif

    MoveFlyInCnt( pNew, nTxtPos, STRING_LEN );

    // Kein SetOfst oder CalcFollow, weil gleich ohnehin ein AdjustFollow folgt.
#ifdef USED
    CalcFollow( nNewOfst );
#endif

    pNew->ManipOfst( nTxtPos );
    return pNew;
}


/*************************************************************************
 *                      virtual SwTxtFrm::SetOfst()
 *************************************************************************/

void SwTxtFrm::_SetOfst( const xub_StrLen nNewOfst )
{
#ifdef DBGTXT
    // Es gibt tatsaechlich einen Sonderfall, in dem ein SetOfst(0)
    // zulaessig ist: bug 3496
    ASSERT( nNewOfst, "!SwTxtFrm::SetOfst: missing JoinFrm()." );
#endif

    // Die Invalidierung unseres Follows ist nicht noetig.
    // Wir sind ein Follow, werden gleich formatiert und
    // rufen von dort aus das SetOfst() !
    nOfst = nNewOfst;
    SwParaPortion *pPara = GetPara();
    if( pPara )
    {
        SwCharRange &rReformat = *(pPara->GetReformat());
        rReformat.Start() = 0;
        rReformat.Len() = GetTxt().Len();
        *(pPara->GetDelta()) = rReformat.Len();
    }
    InvalidateSize();
}

/*************************************************************************
 *                      SwTxtFrm::CalcPreps
 *************************************************************************/

sal_Bool SwTxtFrm::CalcPreps()
{
    SwParaPortion *pPara = GetPara();
    if ( !pPara )
        return sal_False;
    sal_Bool bPrep = pPara->IsPrep();
    sal_Bool bPrepWidows = pPara->IsPrepWidows();
    sal_Bool bPrepAdjust = pPara->IsPrepAdjust();
    sal_Bool bPrepMustFit = pPara->IsPrepMustFit();
    ResetPreps();

    sal_Bool bRet = sal_False;
    if( bPrep && !pPara->GetReformat()->Len() )
    {
        // PREP_WIDOWS bedeutet, dass im Follow die Orphans-Regel
        // zuschlug.
        // Es kann in unguenstigen Faellen vorkommen, dass auch ein
        // PrepAdjust vorliegt (3680)!
        if( bPrepWidows )
        {
            if( !GetFollow() )
            {
                ASSERT( GetFollow(), "+SwTxtFrm::CalcPreps: no credits" );
                return sal_False;
            }

            // Wir muessen uns auf zwei Faelle einstellen:
            // Wir konnten dem Follow noch ein paar Zeilen abgeben,
            // -> dann muessen wir schrumpfen
            // oder wir muessen auf die naechste Seite
            // -> dann lassen wir unseren Frame zu gross werden.

            SwTwips nChgHeight = GetParHeight();
            if( nChgHeight >= Prt().Height() )
            {
                if( bPrepMustFit )
                {
                    GetFollow()->SetJustWidow( sal_True );
                    GetFollow()->Prepare( PREP_CLEAR );
                }
                else
                {
                    SwTwips nTmp  = LONG_MAX - (Frm().Top()+10000);
                    SwTwips nDiff = nTmp - Frm().Height();
                    Frm().Height( nTmp );
                    Prt().Height( Prt().Height() + nDiff );
                    SetWidow( sal_True );
                }
            }
            else
            {
                ASSERT( nChgHeight < Prt().Height(),
                        "+SwTxtFrm::CalcPrep: wanna shrink" );
                nChgHeight = Prt().Height() - nChgHeight;
                GetFollow()->SetJustWidow( sal_True );
                GetFollow()->Prepare( PREP_CLEAR );
                Shrink( nChgHeight, pHeight );
                SwRect &rRepaint = *(pPara->GetRepaint());
                rRepaint.Chg( Frm().Pos() + Prt().Pos(), Prt().SSize() );
                // 6792: Rrand < LRand und Repaint
                if( 0 >= rRepaint.Width() )
                    rRepaint.Width(1);
            }
            bRet = sal_True;
        }

        else if ( bPrepAdjust )
        {
            if ( HasFtn() )
            {
                if( !CalcPrepFtnAdjust() )
                {
                    if( bPrepMustFit )
                    {
                        SwTxtLineAccess aAccess( this );
                        aAccess.GetPara()->SetPrepMustFit( sal_True );
                    }
                    return sal_False;
                }
            }

            SwTxtFormatInfo aInf( this );
            SwTxtFormatter aLine( this, &aInf );

            WidowsAndOrphans aFrmBreak( this );
            // Egal was die Attribute meinen, bei MustFit wird
            // der Absatz im Notfall trotzdem gesplittet...
            if( bPrepMustFit )
            {
                aFrmBreak.SetKeep( sal_False );
                aFrmBreak.ClrOrphLines();
            }
            // Bevor wir FormatAdjust aufrufen muessen wir dafuer
            // sorgen, dass die Zeilen, die unten raushaengen
            // auch tatsaechlich abgeschnitten werden.
            sal_Bool bBreak = aFrmBreak.IsBreakNow( aLine );
            bRet = sal_True;
            while( !bBreak && aLine.Next() )
                bBreak = aFrmBreak.IsBreakNow( aLine );
            if( bBreak )
            {
                // Es gibt Komplikationen: wenn TruncLines gerufen wird,
                // veraendern sich ploetzlich die Bedingungen in
                // IsInside, so dass IsBreakNow andere Ergebnisse
                // liefern kann. Aus diesem Grund wird rFrmBreak bekannt
                // gegeben, dass da wo rLine steht, das Ende erreicht
                // ist. Mal sehen, ob's klappt ...
                aLine.TruncLines();
                aFrmBreak.SetRstHeight( aLine );
                FormatAdjust( aLine, aFrmBreak, aInf.GetTxt().Len(), aInf.IsStop() );
            }
            else
            {
                if( !GetFollow() )
                    FormatAdjust( aLine, aFrmBreak,
                                  aInf.GetTxt().Len(), aInf.IsStop() );
                else if ( !aFrmBreak.IsKeepAlways() )
                {
                    // Siehe Bug: 2320
                    // Vor dem Master wird eine Zeile geloescht, der Follow
                    // koennte eine Zeile abgeben.
                    const SwCharRange aFollowRg( GetFollow()->GetOfst(), 1 );
                    *(pPara->GetReformat()) += aFollowRg;
                    // Es soll weitergehen!
                    bRet = sal_False;
                }
            }

            // Eine letzte Ueberpruefung, falls das FormatAdjust() nichts
            // brachte, muessen wir amputieren.
            if( bPrepMustFit )
            {
                const SwTwips nMust = GetUpper()->Frm().Top()
                    + GetUpper()->Prt().Top() + GetUpper()->Prt().Height();
                const SwTwips nIs   = Frm().Top() + Frm().Height();
                if( nIs > nMust )
                {
                    Shrink( nIs - nMust, pHeight );
                    if( Prt().Height() < 0 )
                        Prt().Height( 0 );
                    SetUndersized( sal_True );
                }
            }
        }
    }
    pPara->SetPrepMustFit( bPrepMustFit );
    return bRet;
}

/*************************************************************************
 *                      SwTxtFrm::FormatAdjust()
 *************************************************************************/

// Hier werden die Fussnoten und "als Zeichen"-gebundenen Objekte umgehaengt
#define CHG_OFFSET( pFrm, nNew )\
    {\
        if( pFrm->GetOfst() < nNew )\
            pFrm->MoveFlyInCnt( this, 0, nNew );\
        else if( pFrm->GetOfst() > nNew )\
            MoveFlyInCnt( pFrm, nNew, STRING_LEN );\
    }

void SwTxtFrm::FormatAdjust( SwTxtFormatter &rLine,
                             WidowsAndOrphans &rFrmBreak,
                             const xub_StrLen nStrLen, const sal_Bool bDummy )
{
    SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();

    xub_StrLen nEnd = rLine.GetStart();

    // Wir muessen fuer eindeutige Verhaeltnisse sorgen
    // rFrmBreak.SetRstHeight( rLine );

    // rLine.GetStart(): die letzte Zeile von rLine,
    // ist bereits die Zeile, die nicht
    // mehr passte. Ihr Anfang ist das Ende des Masters.
    // @@@if( !GetFollow() && nEnd < nStrLen )
    // (nEnd < nStrLen || rFrmBreak.IsBreakNow(rLine));

    sal_Bool bHasToFit = pPara->IsPrepMustFit();

    // Das StopFlag wird durch Fussnoten gesetzt,
    // die auf die naechste Seite wollen.
    sal_uInt8 nNew = ( !GetFollow() && nEnd < nStrLen &&
        ( rLine.IsStop() || ( bHasToFit ?
        ( rLine.GetLineNr() > 1 && !rFrmBreak.IsInside( rLine ) )
        : rFrmBreak.IsBreakNow( rLine ) ) ) ) ? 1 : 0;
    if( nNew )
        SplitFrm( nEnd );

    const SwFrm *pBodyFrm = (const SwFrm*)(FindBodyFrm());
    const long nBodyHeight = pBodyFrm ? pBodyFrm->Frm().Height() : 0;

    // Wenn die aktuellen Werte berechnet wurden, anzeigen, dass
    // sie jetzt gueltig sind.
    *(pPara->GetReformat()) = SwCharRange();
    sal_Bool bDelta = *pPara->GetDelta() != 0;
    *(pPara->GetDelta()) = 0;

    if( rLine.IsStop() )
    {
        rLine.TruncLines( sal_True );
        nNew = 1;
    }

    // FindBreak schneidet die letzte Zeile ab.
    if( !rFrmBreak.FindBreak( this, rLine, bHasToFit ) )
    {
        // Wenn wir bis zum Ende durchformatiert haben, wird nEnd auf das Ende
        // gesetzt. In AdjustFollow wird dadurch ggf. JoinFrm() ausgefuehrt.
        // Ansonsten ist nEnd das Ende der letzten Zeile im Master.
        xub_StrLen nOld = nEnd;
        nEnd = rLine.GetEnd();
        if( GetFollow() )
        {
            if( nNew && nOld < nEnd )
                RemoveFtn( nOld, nEnd - nOld );
            CHG_OFFSET( GetFollow(), nEnd )
            if( !bDelta )
                GetFollow()->ManipOfst( nEnd );
        }
    }
    else
    {   // Wenn wir Zeilen abgeben, darf kein Join auf den Folows gerufen werden,
        // im Gegenteil, es muss ggf. sogar ein Follow erzeugt werden.
        // Dies muss auch geschehen, wenn die Textmasse komplett im Master
        // bleibt, denn es könnte ja ein harter Zeilenumbruch noch eine weitere
        // Zeile (ohne Textmassse) notwendig machen!
        nEnd = rLine.GetEnd();
        if( GetFollow() )
        {
            if( GetFollow()->GetOfst() != nEnd || GetFollow()->IsFieldFollow() )
                nNew |= 3;
            CHG_OFFSET( GetFollow(), nEnd )
            GetFollow()->ManipOfst( nEnd );
        }
        else
        {
            SplitFrm( nEnd );
            nNew |= 3;
        }
        // Wenn sich die Resthoehe geaendert hat, z.B. durch RemoveFtn()
        // dann muessen wir auffuellen, um Oszillationen zu vermeiden!
        if( bDummy && pBodyFrm && nBodyHeight < pBodyFrm->Frm().Height() )
            rLine.MakeDummyLine();
    }

    // In AdjustFrm() stellen wir uns selbst per Grow/Shrink ein,
    // in AdjustFollow() stellen wir unseren FolgeFrame ein.

    const SwTwips nDocPrtTop = Frm().Top() + Prt().Top();
    const SwTwips nOldHeight = Prt().SSize().Height();
    const SwTwips nChg = rLine.CalcBottomLine() - nDocPrtTop - nOldHeight;

    AdjustFrm( nChg, bHasToFit );

    if( HasFollow() || IsInFtn() )
        _AdjustFollow( rLine, nEnd, nStrLen, nNew );
    pPara->SetPrepMustFit( sal_False );
}

/*************************************************************************
 *                      SwTxtFrm::FormatLine()
 *************************************************************************/

// bPrev zeigt an, ob Reformat.Start() wegen Prev() vorgezogen wurde.
// Man weiss sonst nicht, ob man Repaint weiter einschraenken kann oder nicht.


sal_Bool SwTxtFrm::FormatLine( SwTxtFormatter &rLine, const sal_Bool bPrev )
{
    SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();
    // Nach rLine.FormatLine() haelt nStart den neuen Wert,
    // waehrend in pOldStart der alte Offset gepflegt wird.
    // Ueber diesen Weg soll das nDelta ersetzt werden.
    // *pOldStart += rLine.GetCurr()->GetLen();
    const SwLineLayout *pOldCur = rLine.GetCurr();
    const xub_StrLen nOldLen    = pOldCur->GetLen();
    const KSHORT nOldAscent = pOldCur->GetAscent();
    const KSHORT nOldHeight = pOldCur->Height();
    const KSHORT nOldWidth  = pOldCur->Width();
    const sal_Bool bOldHyph = pOldCur->IsEndHyph();
    SwTwips nOldTop = 0;
    SwTwips nOldBottom;
    if( rLine.GetCurr()->IsClipping() )
        rLine.CalcUnclipped( nOldTop, nOldBottom );

    const xub_StrLen nNewStart = rLine.FormatLine( rLine.GetStart() );

    ASSERT( Frm().Pos() + Prt().Pos() == rLine.GetFirstPos(),
            "SwTxtFrm::FormatLine: frame leaves orbit." );
    ASSERT( rLine.GetCurr()->Height(),
            "SwTxtFrm::FormatLine: line height is zero" );

    // Das aktuelle Zeilenumbruchobjekt.
    const SwLineLayout *pNew = rLine.GetCurr();

    sal_Bool bUnChg = nOldLen == pNew->GetLen() &&
                  bOldHyph == pNew->IsEndHyph();
    if ( bUnChg && !bPrev )
    {
        // 6672: Toleranz von SLOPPY_TWIPS (5 Twips); vgl. 6922
        const KSHORT nWidthDiff = nOldWidth > pNew->Width()
                                ? nOldWidth - pNew->Width()
                                : pNew->Width() - nOldWidth;
        bUnChg = nOldHeight == pNew->Height() &&
                 nOldAscent == pNew->GetAscent() &&
                 nWidthDiff <= SLOPPY_TWIPS;
    }

    // rRepaint wird berechnet:
    const SwTwips nBottom = rLine.Y() + rLine.GetLineHeight();
    SwRepaint &rRepaint = *(pPara->GetRepaint());
    if( bUnChg && rRepaint.Top() == rLine.Y()
               && (bPrev || nNewStart <= pPara->GetReformat()->Start())
               && ( nNewStart < GetTxtNode()->GetTxt().Len() ) )
    {
        rRepaint.Top( nBottom );
        rRepaint.Height( 0 );
    }
    else
    {
        if( nOldTop )
        {
            if( nOldTop < rRepaint.Top() )
                rRepaint.Top( nOldTop );
            if( !rLine.IsUnclipped() || nOldBottom > rRepaint.Bottom() )
            {
                rRepaint.Bottom( nOldBottom - 1 );
                rLine.SetUnclipped( sal_True );
            }
        }
        if( rLine.GetCurr()->IsClipping() && rLine.IsFlyInCntBase() )
        {
            SwTwips nTmpTop, nTmpBottom;
            rLine.CalcUnclipped( nTmpTop, nTmpBottom );
            if( nTmpTop < rRepaint.Top() )
                rRepaint.Top( nTmpTop );
            if( !rLine.IsUnclipped() || nTmpBottom > rRepaint.Bottom() )
            {
                rRepaint.Bottom( nTmpBottom - 1 );
                rLine.SetUnclipped( sal_True );
            }
        }
        else
        {
            if( !rLine.IsUnclipped() || nBottom > rRepaint.Bottom() )
            {
                rRepaint.Bottom( nBottom - 1 );
                rLine.SetUnclipped( sal_False );
            }
        }
        if ( rRepaint.GetOfst() )
        {
            SwTwips nRght = Max( nOldWidth, pNew->Width() );
            ViewShell *pSh = GetShell();
            const SwViewOption *pOpt = pSh ? pSh->GetViewOptions() : 0;
            if( pOpt && (pOpt->IsParagraph() || pOpt->IsLineBreak()) )
                nRght += ( Max( nOldAscent, pNew->GetAscent() ) );
            else
                nRght += ( Max( nOldAscent, pNew->GetAscent() ) / 4);
            rRepaint.SetRightOfst( rLine.GetLeftMargin() + nRght );
        }
    }
    if( !bUnChg )
        rLine.SetChanges();

    // Die gute, alte nDelta-Berechnung:
    *(pPara->GetDelta()) -= long(pNew->GetLen()) - long(nOldLen);

    // Stop!
    if( rLine.IsStop() )
        return sal_False;

    // Unbedingt noch eine Zeile
    if( rLine.IsNewLine() )
        return sal_True;

    // bis zum Ende des Strings ?
    if( nNewStart >= GetTxtNode()->GetTxt().Len() )
        return sal_False;

    if( rLine.GetInfo().IsShift() )
        return sal_True;

    // Ende des Reformats erreicht ?
    const xub_StrLen nEnd = pPara->GetReformat()->Start() +
                        pPara->GetReformat()->Len();

    if( nNewStart <= nEnd )
        return sal_True;

    return 0 != *(pPara->GetDelta());

// Dieser Bereich ist so sensibel, da behalten wir mal die alte Version:
#ifdef USED
    // nDelta abgearbeitet ?
    if( 0 == *(pPara->GetDelta()) )
        return sal_False;

    // Wenn die Zeilen ausgeglichen sind, ist alles ok.
    if( bUnChg )
        return sal_False;

    return sal_True;
#endif
}

/*************************************************************************
 *                      SwTxtFrm::_Format()
 *************************************************************************/

void SwTxtFrm::_Format( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf,
                        const sal_Bool bAdjust )
{
    SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();
    rLine.SetUnclipped( sal_False );

    // Das war dem C30 zu kompliziert: aString( GetTxt() );
    const XubString &rString = GetTxtNode()->GetTxt();
    const xub_StrLen nStrLen = rString.Len();

    SwCharRange &rReformat = *(pPara->GetReformat());
    SwRepaint   &rRepaint = *(pPara->GetRepaint());
    SwRepaint *pFreeze = NULL;

    // Aus Performancegruenden wird in Init() rReformat auf STRING_LEN gesetzt.
    // Fuer diesen Fall wird rReformat angepasst.
    if( rReformat.Len() > nStrLen )
        rReformat.Len() = nStrLen;

    // Optimiert:
    xub_StrLen nEnd = rReformat.Start() + rReformat.Len();
    if( nEnd > nStrLen )
    {
        rReformat.Len() = nStrLen - rReformat.Start();
        nEnd = nStrLen;
    }

    SwTwips nOldBottom;
    if( GetOfst() && !IsFollow() )
    {
        rLine.Bottom();
        nOldBottom = rLine.Y();
        rLine.Top();
    }
    else
        nOldBottom = 0;
    rLine.CharToLine( rReformat.Start() );

    // Worte koennen durch Fortfall oder Einfuegen eines Space
    // auf die Zeile vor der editierten hinausgezogen werden,
    // deshalb muss diese ebenfalls formatiert werden.
    // Optimierung: Wenn rReformat erst hinter dem ersten Wort der
    // Zeile beginnt, so kann diese Zeile die vorige nicht mehr beeinflussen.
    // AMA: Leider doch, Textgroessenaenderungen + FlyFrames, die Rueckwirkung
    // kann im Extremfall mehrere Zeilen (Frames!!!) betreffen!

    sal_Bool bPrev = rLine.GetPrev() &&
                     ( FindBrk( rString, rLine.GetStart(),
                                rReformat.Start() + 1 ) >= rReformat.Start() ||
                       rLine.GetCurr()->IsRest() );
    if( bPrev )
    {
        while( rLine.Prev() )
            if( rLine.GetCurr()->GetLen() && !rLine.GetCurr()->IsRest() )
            {
                if( !rLine.GetStart() )
                    rLine.Top(); // damit NumDone nicht durcheinander kommt
                break;
            }
        xub_StrLen nNew = rLine.GetStart() + rLine.GetLength();
        if( nNew )
        {
            --nNew;
            if( CH_BREAK == rString.GetChar( nNew ) )
            {
                ++nNew;
                rLine.Next();
                bPrev = sal_False;
            }
        }
        rReformat.Len()  += rReformat.Start() - nNew;
        rReformat.Start() = nNew;
    }

    rRepaint.SetOfst( 0 );
    rRepaint.SetRightOfst( 0 );
    rRepaint.Chg( Frm().Pos() + Prt().Pos(), Prt().SSize() );
    rRepaint.Top( rLine.Y() );
    // 6792: Rrand < LRand und Repaint
    if( 0 >= rRepaint.Width() )
        rRepaint.Width(1);
    WidowsAndOrphans aFrmBreak( this, rInf.IsTest() ? 1 : 0 );

    // rLine steht jetzt auf der ersten Zeile, die formatiert werden
    // muss. Das Flag bFirst sorgt dafuer, dass nicht Next() gerufen wird.
    // Das ganze sieht verdreht aus, aber es muss sichergestellt werden,
    // dass bei IsBreakNow rLine auf der Zeile zum stehen kommt, die
    // nicht mehr passt.
    sal_Bool bFirst  = sal_True;
    sal_Bool bFormat = sal_True;

    // 5383: Das CharToLine() kann uns auch in den roten Bereich fuehren.
    // In diesem Fall muessen wir zurueckwandern, bis die Zeile, die
    // nicht mehr passt in rLine eingestellt ist. Ansonsten geht Textmasse
    // verloren, weil der Ofst im Follow falsch eingestellt wird.

    sal_Bool bBreak = ( !pPara->IsPrepMustFit() || rLine.GetLineNr() > 1 )
                    && aFrmBreak.IsBreakNow( rLine );
    if( bBreak )
    {
        sal_Bool bPrevDone = 0 != rLine.Prev();
        while( bPrevDone && aFrmBreak.IsBreakNow(rLine) )
            bPrevDone = 0 != rLine.Prev();
        if( bPrevDone )
        {
            aFrmBreak.SetKeep( sal_False );
            rLine.Next();
        }
        rLine.TruncLines();

        // auf Nummer sicher:
        bBreak = aFrmBreak.IsBreakNow(rLine) &&
                  ( !pPara->IsPrepMustFit() || rLine.GetLineNr() > 1 );
    }

 /* Bedeutung der folgenden Flags:
    Ist das Watch(End/Mid)Hyph-Flag gesetzt, so muss formatiert werden, wenn
    eine Trennung am (Zeilenende/Fly) vorliegt, sofern MaxHyph erreicht ist.
    Das Jump(End/Mid)Flag bedeutet, dass die naechste Zeile, bei der keine
    Trennung (Zeilenende/Fly) vorliegt, formatiert werden muss, da jetzt
    umgebrochen werden koennte, was vorher moeglicherweise durch MaxHyph
    verboten war.
    Watch(End/Mid)Hyph wird gesetzt, wenn die letzte formatierte Zeile eine
    Trennstelle erhalten hat, vorher aber keine hatte,
    Jump(End/Mid)Hyph, wenn eine Trennstelle verschwindet.
 */
    sal_Bool bJumpEndHyph  = sal_False,
         bWatchEndHyph = sal_False,
         bJumpMidHyph  = sal_False,
         bWatchMidHyph = sal_False;

    const SwAttrSet& rAttrSet = GetTxtNode()->GetSwAttrSet();
    sal_Bool bMaxHyph = ( 0 !=
        ( rInf.MaxHyph() = rAttrSet.GetHyphenZone().GetMaxHyphens() ) );
    if ( bMaxHyph )
        rLine.InitCntHyph();

    if( IsFollow() && IsFieldFollow() && rLine.GetStart() == GetOfst() )
    {
        SwFldPortion* pRest = rLine.GetFieldRest( rInf );
        if( pRest )
        {
            SwTxtFrm *pMaster = FindMaster();
            ASSERT( pMaster, "SwTxtFrm::Format: homeless follow" );
            const SwFldPortion *pFld = pMaster->GetRestPortion();
            if( pFld )
            {
                pRest->TakeNextOffset( pFld );
                xub_StrLen nEndOf;
                // If we get a field portion rest in a multi-line part of the
                // text, we have to create the surrounding multi-portion, too.
                if( GetOfst() && 0 < (nEndOf = rInf.EndOfMulti( GetOfst()-1) ) )
                {
                    SwMultiPortion* pTmp = new SwMultiPortion( nEndOf );
                    pTmp->SetFldRest( pRest );
                    rInf.SetRest( pTmp );
                }
                else
                    rInf.SetRest( pRest );
            }
            else
                delete pRest;
        }
        else
            SetFieldFollow( sal_False );
    }

    /* Zum Abbruchkriterium:
     * Um zu erkennen, dass eine Zeile nicht mehr auf die Seite passt,
     * muss sie formatiert werden. Dieser Ueberhang wird z.B. in AdjustFollow
     * wieder entfernt.
     * Eine weitere Komplikation: wenn wir der Master sind, so muessen
     * wir die Zeilen durchgehen, da es ja sein kann, dass eine Zeile
     * vom Follow in den Master rutschen kann.
     */
    do
    {
        DBG_LOOP;
        if( bFirst )
            bFirst = sal_False;
        else
        {
            if ( bMaxHyph )
            {
                if ( rLine.GetCurr()->IsEndHyph() )
                    rLine.CntEndHyph()++;
                else
                    rLine.CntEndHyph() = 0;
                if ( rLine.GetCurr()->IsMidHyph() )
                    rLine.CntMidHyph()++;
                else
                    rLine.CntMidHyph() = 0;
            }
            if( !rLine.Next() )
            {
                if( !bFormat )
                    rLine.MakeRestPortion();
                rLine.Insert( new SwLineLayout() );
                rLine.Next();
                bFormat = sal_True;
            }
        }
        if ( !bFormat && bMaxHyph &&
              (bWatchEndHyph || bJumpEndHyph || bWatchMidHyph || bJumpMidHyph) )
        {
            if ( rLine.GetCurr()->IsEndHyph() )
            {
                if ( bWatchEndHyph )
                    bFormat = ( rLine.CntEndHyph() == rInf.MaxHyph() );
            }
            else
            {
                bFormat = bJumpEndHyph;
                bWatchEndHyph = sal_False;
                bJumpEndHyph = sal_False;
            }
            if ( rLine.GetCurr()->IsMidHyph() )
            {
                if ( bWatchMidHyph && !bFormat )
                    bFormat = ( rLine.CntEndHyph() == rInf.MaxHyph() );
            }
            else
            {
                bFormat = bFormat || bJumpMidHyph;
                bWatchMidHyph = sal_False;
                bJumpMidHyph = sal_False;
            }
        }
        if( bFormat )
        {
            sal_Bool bOldEndHyph = rLine.GetCurr()->IsEndHyph();
            sal_Bool bOldMidHyph = rLine.GetCurr()->IsMidHyph();
            bFormat = FormatLine( rLine, bPrev );
            //9334: Es kann nur ein bPrev geben... (???)
            bPrev = sal_False;
            if ( bMaxHyph )
            {
                if ( rLine.GetCurr()->IsEndHyph() != bOldEndHyph )
                {
                    bWatchEndHyph = !bOldEndHyph;
                    bJumpEndHyph = bOldEndHyph;
                }
                if ( rLine.GetCurr()->IsMidHyph() != bOldMidHyph )
                {
                    bWatchMidHyph = !bOldMidHyph;
                    bJumpMidHyph = bOldMidHyph;
                }
            }
        }

        if( !rInf.IsNewLine() )
        {
            if( !bFormat )
                 bFormat = 0 != rInf.GetRest();
            if( rInf.IsStop() || rInf.GetIdx() >= nStrLen )
                break;
            if( !bFormat && ( !bMaxHyph || ( !bWatchEndHyph &&
                    !bJumpEndHyph && !bWatchMidHyph && !bJumpMidHyph ) ) )
            {
                if( GetFollow() )
                {
                    while( rLine.Next() )
                        ; //Nothing
                    pFreeze = new SwRepaint( rRepaint ); // to minimize painting
                }
                else
                    break;
            }
        }
        bBreak = aFrmBreak.IsBreakNow(rLine);
    }while( !bBreak );

    if( pFreeze )
    {
        rRepaint = *pFreeze;
        delete pFreeze;
    }

    if( !rLine.IsStop() )
    {
        // Wurde aller Text formatiert und gibt es noch weitere
        // Zeilenobjekte, dann sind diese jetzt ueberfluessig,
        // weil der Text kuerzer geworden ist.
        if( rLine.GetStart() + rLine.GetLength() >= nStrLen &&
            rLine.GetCurr()->GetNext() )
        {
            rLine.TruncLines();
            rLine.SetTruncLines( sal_True );
        }
    }

    if( !rInf.IsTest() )
    {
        // Bei OnceMore lohnt sich kein FormatAdjust
        if( bAdjust || !rLine.GetDropFmt() || !rLine.CalcOnceMore() )
            FormatAdjust( rLine, aFrmBreak, nStrLen, rInf.IsStop() );
        if( rRepaint.HasArea() )
            SetRepaint();
        rLine.SetTruncLines( sal_False );
        if( nOldBottom )                    // Bei "gescollten" Absaetzen wird
        {                                   // noch ueberprueft, ob durch Schrumpfen
            rLine.Bottom();                 // das Scrolling ueberfluessig wurde.
            SwTwips nNewBottom = rLine.Y();
            if( nNewBottom < nOldBottom )
                _SetOfst( 0 );
        }
    }
}

/*************************************************************************
 *                      SwTxtFrm::Format()
 *************************************************************************/

void SwTxtFrm::FormatOnceMore( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf )
{
    SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();
    if( !pPara )
        return;

    // ggf gegen pPara
    KSHORT nOld  = ((const SwTxtMargin&)rLine).GetDropHeight();
    sal_Bool bShrink = sal_False,
         bGrow   = sal_False,
         bGoOn   = rLine.IsOnceMore();
    sal_uInt8 nGo    = 0;
    while( bGoOn )
    {
#ifdef DBGTXT
        aDbstream << "OnceMore!" << endl;
#endif
        ++nGo;
        rInf.Init();
        rLine.Top();
        if( !rLine.GetDropFmt() )
            rLine.SetOnceMore( sal_False );
        SwCharRange aRange( 0, rInf.GetTxt().Len() );
        *(pPara->GetReformat()) = aRange;
        _Format( rLine, rInf );

        bGoOn = rLine.IsOnceMore();
        if( bGoOn )
        {
            const KSHORT nNew = ((const SwTxtMargin&)rLine).GetDropHeight();
            if( nOld == nNew )
                bGoOn = sal_False;
            else
            {
                if( nOld > nNew )
                    bShrink = sal_True;
                else
                    bGrow = sal_True;

                if( bShrink == bGrow || 5 < nGo )
                    bGoOn = sal_False;

                nOld = nNew;
            }

            // 6107: Wenn was schief ging, muss noch einmal formatiert werden.
            if( !bGoOn )
            {
                rInf.CtorInit( this );
                rLine.CtorInit( this, &rInf );
                rLine.SetDropLines( 1 );
                rLine.CalcDropHeight( 1 );
                SwCharRange aRange( 0, rInf.GetTxt().Len() );
                *(pPara->GetReformat()) = aRange;
                _Format( rLine, rInf, sal_True );
                // 8047: Wir painten alles...
                SetCompletePaint();
            }
        }
    }
}

/*************************************************************************
 *                      SwTxtFrm::_Format()
 *************************************************************************/


void SwTxtFrm::_Format( SwParaPortion *pPara )
{
    const xub_StrLen nStrLen = GetTxt().Len();

    // AMA: Wozu soll das gut sein? Scheint mir zuoft zu einem kompletten
    // Formatieren und Repainten zu fuehren???
//  if ( !(*pPara->GetDelta()) )
//      *(pPara->GetDelta()) = nStrLen;
//  else
    if ( !nStrLen )
    {
        // Leere Zeilen werden nicht lange gequaelt:
        // pPara wird blank geputzt
        // entspricht *pPara = SwParaPortion;
        sal_Bool bMustFit = pPara->IsPrepMustFit();
        pPara->Truncate();
        pPara->FormatReset();
        if( pBlink && pPara->IsBlinking() )
            pBlink->Delete( pPara );
        pPara->ResetFlags();
        pPara->SetPrepMustFit( bMustFit );
    }

    SwTxtFormatInfo aInf( this );
    SwTxtFormatter  aLine( this, &aInf );

    _Format( aLine, aInf );

    if( aLine.IsOnceMore() )
        FormatOnceMore( aLine, aInf );

    if( 1 < aLine.GetDropLines() )
    {
        if( SVX_ADJUST_LEFT != aLine.GetAdjust() &&
            SVX_ADJUST_BLOCK != aLine.GetAdjust() )
        {
            aLine.CalcDropAdjust();
            aLine.SetPaintDrop( sal_True );
        }

        if( aLine.IsPaintDrop() )
        {
            aLine.CalcDropRepaint();
            aLine.SetPaintDrop( sal_False );
        }
    }
}

/*************************************************************************
 *                      SwTxtFrm::Format()
 *************************************************************************/

/*
 * Format berechnet die Groesse des Textframes und ruft, wenn
 * diese feststeht, Shrink() oder Grow(), um die Framegroesse dem
 * evtl. veraenderten Platzbedarf anzupassen.
 */

void SwTxtFrm::Format( const SwBorderAttrs * )
{
    DBG_LOOP;
#ifdef DEBUG
    const XubString aXXX = GetTxtNode()->GetTxt();
    const SwTwips nDbgY = Frm().Top();
    const SwPageFrm *pDbgPage = FindPageFrm();
    const MSHORT nDbgPageNr = pDbgPage->GetPhyPageNum();
    // Um zu gucken, ob es einen Ftn-Bereich gibt.
    const SwFrm *pDbgFtnCont = (const SwFrm*)(FindPageFrm()->FindFtnCont());

#ifndef PRODUCT
    // nStopAt laesst sich vom CV bearbeiten.
    static MSHORT nStopAt = 0;
    if( nStopAt == GetFrmId() )
    {
        int i = GetFrmId();
    }
#endif
#endif

#ifdef DEBUG_FTN
    //Fussnote darf nicht auf einer Seite vor ihrer Referenz stehen.
    if( IsInFtn() )
    {
        const SwFtnFrm *pFtn = (SwFtnFrm*)GetUpper();
        const SwPageFrm *pFtnPage = pFtn->GetRef()->FindPageFrm();
        const MSHORT nFtnPageNr = pFtnPage->GetPhyPageNum();
        if( !IsLocked() )
        {
            if( nFtnPageNr > nDbgPageNr )
            {
                SwTxtFrmLocker aLock(this);
                ASSERT( nFtnPageNr <= nDbgPageNr, "!Ftn steht vor der Referenz." );
                MSHORT i = 0;
            }
        }
    }
#endif

    MSHORT nRepeat = 0;
    do
    {
        // Vom Berichtsautopiloten oder ueber die BASIC-Schnittstelle kommen
        // gelegentlich TxtFrms mit einer Breite <=0.
        if( Prt().Width() <= 0 )
        {
            // Wenn MustFit gesetzt ist, schrumpfen wir ggf. auf die Unterkante
            // des Uppers, ansonsten nehmen wir einfach eine Standardgroesse
            // von 12 Pt. ein (240 Twip).
            SwTxtLineAccess aAccess( this );
            if( aAccess.GetPara()->IsPrepMustFit() )
            {
                const SwTwips nMust = GetUpper()->Frm().Top()
                                    + GetUpper()->Frm().Height();
                const SwTwips nIs   = Frm().Top() + Frm().Height();
                if( nIs > nMust )
                    Shrink( nIs - nMust, pHeight );
            }
            else if( 240 < Frm().Height() )
                Shrink( Frm().Height() - 240, pHeight );
            else if( 240 > Frm().Height() )
                Grow( 240 - Frm().Height(), pHeight );
            if( Prt().Top() > Frm().Height() )
                Prt().Top( Frm().Height() );
            if( Prt().Height() < 0 )
                Prt().Height( 0 );
            return;
        }
        sal_Bool bChkAtCnt = sal_False;
        const xub_StrLen nStrLen = GetTxtNode()->GetTxt().Len();
        if ( nStrLen || !FormatEmpty() )
        {

            SetEmpty( sal_False );
        // Um nicht durch verschachtelte Formats irritiert zu werden.
            FormatLevel aLevel;
            if( 12 == aLevel.GetLevel() )
                return;

            // Die Formatinformationen duerfen u.U. nicht veraendert werden.
            if( IsLocked() )
                return;

            // Waehrend wir formatieren, wollen wir nicht gestoert werden.
            SwTxtFrmLocker aLock(this);

#ifdef DEBUG
    //MA 25. Jan. 94 Das Flag stimmt sehr haufig beim Eintritt nicht. Das muss
    //             bei naechster Gelegenheit geprueft und gefixt werden.
            const sal_Bool bOldFtnFlag = HasFtn();
            CalcFtnFlag();
            if ( bOldFtnFlag != HasFtn() )
                {int bla = 5;}
#endif

            // 8708: Vorsicht, das Format() kann auch durch GetFormatted()
            // angestossen werden.
            if( IsHiddenNow() )
            {
                if( Prt().Height() )
                {
                    HideHidden();
                    Shrink( Prt().Height(), pHeight );
                }
                ChgThisLines();
                return;
            }

            SwTxtLineAccess aAccess( this );
            const sal_Bool bNew = !aAccess.SwTxtLineAccess::IsAvailable();

            if( CalcPreps() )
                ; // nothing
            // Wir returnen, wenn schon formatiert wurde, nicht aber, wenn
            // der TxtFrm gerade erzeugt wurde und ueberhaupt keine Format-
            // informationen vorliegen.
            else if( !bNew && !aAccess.GetPara()->GetReformat()->Len() )
            {
                if( GetTxtNode()->GetSwAttrSet().GetRegister().GetValue() )
                {
                    aAccess.GetPara()->SetPrepAdjust( sal_True );
                    aAccess.GetPara()->SetPrep( sal_True );
                    CalcPreps();
                }
                SetWidow( sal_False );
            }
            else if( GetOfst() && GetOfst() > GetTxtNode()->GetTxt().Len() )
            {
                SwTxtFrm *pMaster = FindMaster();
                ASSERT( pMaster, "SwTxtFrm::Format: homeless follow" );
                if( pMaster )
                    pMaster->Prepare( PREP_FOLLOW_FOLLOWS );
            }
            else
            {
                const sal_Bool bOrphan = IsWidow();
                _Format( aAccess.GetPara() );
                if( bOrphan )
                {
                    ValidateFrm();
                    SetWidow( sal_False );
                }
                bChkAtCnt = sal_True;
            }
            if( IsEmptyMaster() )
            {
                SwFrm* pPre = GetPrev();
                if( pPre && pPre->GetAttrSet()->GetKeep().GetValue() )
                    pPre->InvalidatePos();
            }
        }
        MSHORT nMaxRepeat = 2;
        if( bChkAtCnt && nRepeat < nMaxRepeat )
        {
            sal_Bool bRepeat = sal_False;
            MSHORT nRepAdd = 0;
            SwDrawObjs *pObjs;
            SwTxtFrm *pMaster = IsFollow() ? FindMaster() : this;
            if( pMaster )
            {
                if ( 0 != (pObjs = pMaster->GetDrawObjs()) )
                {
                    MSHORT nAutoCnt = 0;
                    for ( int i = 0; i < int(pObjs->Count()); ++i )
                    {
                        SdrObject *pO = (*pObjs)[MSHORT(i)];
                        if ( pO->IsWriterFlyFrame() )
                        {
                            SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                            if( pFly->IsAutoPos() && !::IsInProgress( pFly ) )
                            {
                                ++nAutoCnt;
                                ASSERT( pFly->IsFlyAtCntFrm(), "Not at content, but autopos.?" );
                                ((SwFlyAtCntFrm*)pFly)->CheckCharRect();
                                if( !pFly->IsValid() )
                                {
                                    SwTwips nOldTop = pFly->Frm().Top();
                                    pFly->Calc();
                                    bRepeat = sal_True;
                                    if( !nRepAdd && nOldTop >= pFly->Frm().Top() )
                                        nRepAdd = 1;
                                }
                            }
                        }
                    }
                    if( nAutoCnt > 11 )
                        nMaxRepeat = nAutoCnt/4;
                }
            }
            if( bRepeat )
                nRepeat += nRepAdd;
            else
                nRepeat = 0;
        }
        else
            nRepeat = 0;
    } while( nRepeat );

    ChgThisLines();

#ifdef DEBUG
    // Hier ein Instrumentarium, um ungewoehnlichen Master/Follow-Kombinationen,
    // insbesondere bei Fussnoten, auf die Schliche zu kommen
    if( IsFollow() || GetFollow() )
    {
        SwTxtFrm *pTmpFrm = IsFollow() ? FindMaster() : this;
        const SwPageFrm *pTmpPage = pTmpFrm->FindPageFrm();
        MSHORT nPgNr = pTmpPage->GetPhyPageNum();
        MSHORT nLast;
        MSHORT nDummy = 0; // nur zum Breakpoint setzen
        while( pTmpFrm->GetFollow() )
        {
            pTmpFrm = pTmpFrm->GetFollow();
            nLast = nPgNr;
            pTmpPage = pTmpFrm->FindPageFrm();
            nPgNr = pTmpPage->GetPhyPageNum();
            if( nLast > nPgNr )
                ++nDummy; // schon fast eine Assertion wert
            else if( nLast == nPgNr )
                ++nDummy; // bei Spalten voellig normal, aber sonst!?
            else if( nLast < nPgNr - 1 )
                ++nDummy; // kann schon mal temporaer vorkommen
        }
    }
#endif
}

/*************************************************************************
 *                      SwTxtFrm::FormatQuick()
 *************************************************************************/
// 6995:
// return sal_False: Prepare(), HasPara(), InvalidateRanges(),

sal_Bool SwTxtFrm::FormatQuick()
{
    DBG_LOOP;
#ifdef DEBUG
    const XubString aXXX = GetTxtNode()->GetTxt();
    const SwTwips nDbgY = Frm().Top();
#ifndef PRODUCT
    // nStopAt laesst sich vom CV bearbeiten.
    static MSHORT nStopAt = 0;
    if( nStopAt == GetFrmId() )
    {
        int i = GetFrmId();
    }
#endif
#endif

    if( IsEmpty() && FormatEmpty() )
        return sal_True;
    // Wir sind sehr waehlerisch:
    if( HasPara() || IsWidow() || IsLocked()
        || !GetValidSizeFlag() || (Prt().Height() && IsHiddenNow()) )
        return sal_False;

    SwTxtLineAccess aAccess( this );
    SwParaPortion *pPara = aAccess.GetPara();
    if( !pPara )
        return sal_False;

    SwTxtFrmLocker aLock(this);
    SwTxtFormatInfo aInf( this, sal_False, sal_True );
    if( 0 != aInf.MaxHyph() )   // 27483: MaxHyphen beachten!
        return sal_False;

    SwTxtFormatter  aLine( this, &aInf );

    // DropCaps sind zu kompliziert...
    if( aLine.GetDropFmt() )
        return sal_False;

    xub_StrLen nStart = GetOfst();
    const xub_StrLen nEnd = GetFollow()
                      ? GetFollow()->GetOfst() : aInf.GetTxt().Len();
    do
    {   DBG_LOOP;
        nStart = aLine.FormatLine( nStart );
        if( aInf.IsNewLine() || (!aInf.IsStop() && nStart < nEnd) )
            aLine.Insert( new SwLineLayout() );
    } while( aLine.Next() );

    // Last exit: die Hoehen muessen uebereinstimmen.
    Point aTopLeft( Frm().Pos() );
    aTopLeft += Prt().Pos();
    const SwTwips nNewHeight = aLine.Y() + aLine.GetLineHeight();
    const SwTwips nOldHeight = aTopLeft.Y() + Prt().Height();
    if( nNewHeight != nOldHeight && !IsUndersized() )
    {
#ifdef DEBUG
//  Achtung: Durch FormatLevel==12 kann diese Situation auftreten, don't panic!
//      ASSERT( nNewHeight == nOldHeight, "!FormatQuick: rosebud" );
#endif
        xub_StrLen nStrt = GetOfst();
        _InvalidateRange( SwCharRange( nStrt, nEnd - nStrt) );
        return sal_False;
    }

    if( pFollow && nStart != ((SwTxtFrm*)pFollow)->GetOfst() )
        return sal_False; // kann z.B. durch Orphans auftreten (35083,35081)

    // Geschafft, wir sind durch ...

    // Repaint setzen
    pPara->GetRepaint()->Pos( aTopLeft );
    pPara->GetRepaint()->SSize( Prt().SSize() );

    // Reformat loeschen
    *(pPara->GetReformat()) = SwCharRange();
    *(pPara->GetDelta()) = 0;

    return sal_True;
}


