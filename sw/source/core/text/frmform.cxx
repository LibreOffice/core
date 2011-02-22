/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <hintids.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/hyznitem.hxx>
#include <pagefrm.hxx>      // ChangeFtnRef
#include <ndtxt.hxx>        // MakeFrm()
#include <dcontact.hxx>     // SwDrawContact
#include <dflyobj.hxx>      // SwVirtFlyDrawObj
#include <flyfrm.hxx>
#include <ftnfrm.hxx>       // SwFtnFrm
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <paratr.hxx>
#include <viewopt.hxx>      // SwViewOptions
#include <viewsh.hxx>       // ViewShell
#include <frmatr.hxx>
#include <pam.hxx>
#include <flyfrms.hxx>
#include <fmtanchr.hxx>
#include <txtcfg.hxx>
#include <itrform2.hxx>     // SwTxtFormatter
#include <widorp.hxx>       // Widows and Orphans
#include <txtcache.hxx>
#include <porrst.hxx>       // SwEmptyPortion
#include <blink.hxx>        // pBlink
#include <porfld.hxx>       // SwFldPortion
#include <sectfrm.hxx>      // SwSectionFrm
#include <pormulti.hxx>     // SwMultiPortion

#include <rootfrm.hxx>
#include <frmfmt.hxx>       // SwFrmFmt
// OD 2004-05-24 #i28701#
#include <sortedobjs.hxx>

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
    if ( ( ! pFrm->IsVertical() &&
             pFrm->Frm().Width() == pFrm->GetUpper()->Prt().Width() ) ||
         (   pFrm->IsVertical() &&
             pFrm->Frm().Height() == pFrm->GetUpper()->Prt().Height() ) )
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
    SWAP_IF_SWAPPED( this )

    if ( !IsInFly() && !IsInTab() )
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

    UNDO_SWAP( this )
}

/*************************************************************************
 *                          ValidateBodyFrm()
 *************************************************************************/

// nach einem RemoveFtn muss der BodyFrm und alle innenliegenden kalkuliert
// werden, damit die DeadLine richtig sitzt.
// Erst wird nach aussen hin gesucht, beim Rueckweg werden alle kalkuliert.

void _ValidateBodyFrm( SwFrm *pFrm )
{
    if( pFrm && !pFrm->IsCellFrm() )
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
    SWAP_IF_SWAPPED( this )

     //siehe Kommtar in ValidateFrm()
    if ( !IsInFly() && !IsInTab() &&
         !( IsInSct() && FindSctFrm()->Lower()->IsColumnFrm() ) )
        _ValidateBodyFrm( GetUpper() );

    UNDO_SWAP( this )
}

/*************************************************************************
 *                      SwTxtFrm::FindBodyFrm()
 *************************************************************************/

sal_Bool SwTxtFrm::_GetDropRect( SwRect &rRect ) const
{
    SWAP_IF_NOT_SWAPPED( this )

    ASSERT( HasPara(), "SwTxtFrm::_GetDropRect: try again next year." );
    SwTxtSizeInfo aInf( (SwTxtFrm*)this );
    SwTxtMargin aLine( (SwTxtFrm*)this, &aInf );
    if( aLine.GetDropLines() )
    {
        rRect.Top( aLine.Y() );
        rRect.Left( aLine.GetLineStart() );
        rRect.Height( aLine.GetDropHeight() );
        rRect.Width( aLine.GetDropLeft() );

        if ( IsRightToLeft() )
            SwitchLTRtoRTL( rRect );

        if ( IsVertical() )
            SwitchHorizontalToVertical( rRect );
        UNDO_SWAP( this )
        return sal_True;
    }

    UNDO_SWAP( this )

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
    SWAP_IF_SWAPPED( this )

    ASSERT( HasFollow(), "CalcFollow: missing Follow." );

    SwTxtFrm* pMyFollow = GetFollow();

    SwParaPortion *pPara = GetPara();
    sal_Bool bFollowFld = pPara ? pPara->IsFollowField() : sal_False;

    if( !pMyFollow->GetOfst() || pMyFollow->GetOfst() != nTxtOfst ||
        bFollowFld || pMyFollow->IsFieldFollow() ||
        ( pMyFollow->IsVertical() && !pMyFollow->Prt().Width() ) ||
        ( ! pMyFollow->IsVertical() && !pMyFollow->Prt().Height() ) )
    {
#ifdef DBG_UTIL
        const SwFrm *pOldUp = GetUpper();
#endif

        SWRECTFN ( this )
        SwTwips nOldBottom = (GetUpper()->Frm().*fnRect->fnGetBottom)();
        SwTwips nMyPos = (Frm().*fnRect->fnGetTop)();

        const SwPageFrm *pPage = 0;
        sal_Bool  bOldInvaCntnt = sal_True;
        if ( !IsInFly() && GetNext() )
        {
            pPage = FindPageFrm();
            //Minimieren - sprich ggf. zuruecksetzen - der Invalidierungen s.u.
            bOldInvaCntnt  = pPage->IsInvalidCntnt();
        }

        pMyFollow->_SetOfst( nTxtOfst );
        pMyFollow->SetFieldFollow( bFollowFld );
        if( HasFtn() || pMyFollow->HasFtn() )
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

        pMyFollow->CalcFtnFlag();
        if ( !pMyFollow->GetNext() && !pMyFollow->HasFtn() )
            nOldBottom = bVert ? 0 : LONG_MAX;

        while( sal_True )
        {
            if( !FormatLevel::LastLevel() )
            {
                // Weenn der Follow in einem spaltigen Bereich oder einem
                // spaltigen Rahmen steckt, muss zunaechst dieser kalkuliert
                // werden, da das FormatWidthCols() nicht funktioniert, wenn
                // es aus dem MakeAll des _gelockten_ Follows heraus gerufen
                // wird.
                SwSectionFrm* pSct = pMyFollow->FindSctFrm();
                if( pSct && !pSct->IsAnLower( this ) )
                {
                    if( pSct->GetFollow() )
                        pSct->SimpleFormat();
                    else if( ( pSct->IsVertical() && !pSct->Frm().Width() ) ||
                             ( ! pSct->IsVertical() && !pSct->Frm().Height() ) )
                        break;
                }
                // OD 14.03.2003 #i11760# - intrinsic format of follow is controlled.
                if ( FollowFormatAllowed() )
                {
                    // OD 14.03.2003 #i11760# - no nested format of follows, if
                    // text frame is contained in a column frame.
                    // Thus, forbid intrinsic format of follow.
                    {
                        bool bIsFollowInColumn = false;
                        SwFrm* pFollowUpper = pMyFollow->GetUpper();
                        while ( pFollowUpper )
                        {
                            if ( pFollowUpper->IsColumnFrm() )
                            {
                                bIsFollowInColumn = true;
                                break;
                            }
                            if ( pFollowUpper->IsPageFrm() ||
                                 pFollowUpper->IsFlyFrm() )
                            {
                                break;
                            }
                            pFollowUpper = pFollowUpper->GetUpper();
                        }
                        if ( bIsFollowInColumn )
                        {
                            pMyFollow->ForbidFollowFormat();
                        }
                    }

                    pMyFollow->Calc();
                    // Der Follow merkt anhand seiner Frm().Height(), dass was schief
                    // gelaufen ist.
                    ASSERT( !pMyFollow->GetPrev(), "SwTxtFrm::CalcFollow: cheesy follow" );
                    if( pMyFollow->GetPrev() )
                    {
                        pMyFollow->Prepare( PREP_CLEAR );
                        pMyFollow->Calc();
                        ASSERT( !pMyFollow->GetPrev(), "SwTxtFrm::CalcFollow: very cheesy follow" );
                    }

                    // OD 14.03.2003 #i11760# - reset control flag for follow format.
                    pMyFollow->AllowFollowFormat();
                }

                //Sicherstellen, dass der Follow gepaintet wird.
                pMyFollow->SetCompletePaint();
            }

            pPara = GetPara();
            //Solange der Follow wg. Orphans Zeilen angefordert, bekommt er
            //diese und wird erneut formatiert, falls moeglich.
            if( pPara && pPara->IsPrepWidows() )
                CalcPreps();
            else
                break;
        }

        if( HasFtn() || pMyFollow->HasFtn() )
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
        }

#ifdef DBG_UTIL
        ASSERT( pOldUp == GetUpper(), "SwTxtFrm::CalcFollow: heavy follow" );
#endif

        const long nRemaining =
                 - (GetUpper()->Frm().*fnRect->fnBottomDist)( nOldBottom );
        if (  nRemaining > 0 && !GetUpper()->IsSctFrm() &&
              nRemaining != ( bVert ?
                              nMyPos - Frm().Right() :
                              Frm().Top() - nMyPos ) )
        {
            UNDO_SWAP( this )
            return sal_True;
        }
    }

    UNDO_SWAP( this )

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

    // AdjustFrm is called with a swapped frame during
    // formatting but the frame is not swapped during FormatEmpty
    SWAP_IF_SWAPPED( this )
    SWRECTFN ( this )

    // Die Size-Variable des Frames wird durch Grow inkrementiert
    // oder durch Shrink dekrementiert. Wenn die Groesse
    // unveraendert ist, soll nichts passieren!
    if( nChgHght >= 0)
    {
        SwTwips nChgHeight = nChgHght;
        if( nChgHght && !bHasToFit )
        {
            if( IsInFtn() && !IsInSct() )
            {
                SwTwips nReal = Grow( nChgHght, sal_True );
                if( nReal < nChgHght )
                {
                    SwTwips nBot = (*fnRect->fnYInc)( (Frm().*fnRect->fnGetBottom)(),
                                                      nChgHght - nReal );
                    SwFrm* pCont = FindFtnFrm()->GetUpper();

                    if( (pCont->Frm().*fnRect->fnBottomDist)( nBot ) > 0 )
                    {
                        (Frm().*fnRect->fnAddBottom)( nChgHght );
                        if( bVert )
                            Prt().SSize().Width() += nChgHght;
                        else
                        Prt().SSize().Height() += nChgHght;
                        UNDO_SWAP( this )
                        return;
                    }
                }
            }

            Grow( nChgHght );

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
                {
                    // OD 2004-07-01 #i28701# - use new method <SwFrm::InvalidateObjs(..)>
                    // No format is performed for the floating screen objects.
                    InvalidateObjs( true );
                }
            }
            nChgHeight = 0;
        }
        // Ein Grow() wird von der Layout-Seite immer akzeptiert,
        // also auch, wenn die FixSize des umgebenden Layoutframes
        // dies nicht zulassen sollte. Wir ueberpruefen diesen
        // Fall und korrigieren die Werte.
        // MA 06. May. 93: Der Frm darf allerdings auch im Notfall nicht
        // weiter geschrumpft werden als es seine Groesse zulaesst.
        SwTwips nRstHeight;
        if ( IsVertical() )
        {
            ASSERT( ! IsSwapped(),"Swapped frame while calculating nRstHeight" );

            //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
            if ( IsVertLR() )
                    nRstHeight = GetUpper()->Frm().Left()
                               + GetUpper()->Prt().Left()
                               + GetUpper()->Prt().Width()
                               - Frm().Left();
            else
                nRstHeight = Frm().Left() + Frm().Width() -
                            ( GetUpper()->Frm().Left() + GetUpper()->Prt().Left() );
         }
        else
            nRstHeight = GetUpper()->Frm().Top()
                       + GetUpper()->Prt().Top()
                       + GetUpper()->Prt().Height()
                       - Frm().Top();

        //In Tabellenzellen kann ich mir evtl. noch ein wenig dazuholen, weil
        //durch eine vertikale Ausrichtung auch oben noch Raum sein kann.
        // --> OD 2004-11-25 #115759# - assure, that first lower in upper
        // is the current one or is valid.
        if ( IsInTab() &&
             ( GetUpper()->Lower() == this ||
               GetUpper()->Lower()->IsValid() ) )
        // <--
        {
            long nAdd = (*fnRect->fnYDiff)( (GetUpper()->Lower()->Frm().*fnRect->fnGetTop)(),
                                            (GetUpper()->*fnRect->fnGetPrtTop)() );
            ASSERT( nAdd >= 0, "Ey" );
            nRstHeight += nAdd;
        }

/* ------------------------------------
 * #50964#: nRstHeight < 0 bedeutet, dass der TxtFrm komplett ausserhalb seines
 * Upper liegt. Dies kann passieren, wenn er innerhalb eines FlyAtCntFrm liegt, der
 * durch das Grow() die Seite gewechselt hat. In so einem Fall ist es falsch, der
 * folgenden Grow-Versuch durchzufuehren. Im Bugfall fuehrte dies sogar zur
 * Endlosschleife.
 * -----------------------------------*/
        SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
        SwTwips nPrtHeight = (Prt().*fnRect->fnGetHeight)();

        if( nRstHeight < nFrmHeight )
        {
            //Kann sein, dass ich die richtige Grosse habe, der Upper aber zu
            //klein ist und der Upper noch Platz schaffen kann.
            if( ( nRstHeight >= 0 || ( IsInFtn() && IsInSct() ) ) && !bHasToFit )
                nRstHeight += GetUpper()->Grow( nFrmHeight - nRstHeight );
            // In spaltigen Bereichen wollen wir moeglichst nicht zu gross werden, damit
            // nicht ueber GetNextSctLeaf weitere Bereiche angelegt werden. Stattdessen
            // schrumpfen wir und notieren bUndersized, damit FormatWidthCols die richtige
            // Spaltengroesse ermitteln kann.
            if ( nRstHeight < nFrmHeight )
            {
                if( bHasToFit || !IsMoveable() ||
                    ( IsInSct() && !FindSctFrm()->MoveAllowed(this) ) )
                {
                    SetUndersized( sal_True );
                    Shrink( Min( ( nFrmHeight - nRstHeight), nPrtHeight ) );
                }
                else
                    SetUndersized( sal_False );
            }
        }
        else if( nChgHeight )
        {
            if( nRstHeight - nFrmHeight < nChgHeight )
                nChgHeight = nRstHeight - nFrmHeight;
            if( nChgHeight )
                Grow( nChgHeight );
        }
    }
    else
        Shrink( -nChgHght );

    UNDO_SWAP( this )
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
    SwFrmSwapper aSwapper( this, sal_False );

    // Wir haben den Rest der Textmasse: alle Follows loeschen
    // Sonderfall sind DummyPortions()
    // - special cases are controlled by parameter <nMode>.
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
#if OSL_DEBUG_LEVEL > 1
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
            for ( sal_uInt16 i = 0; i < pHints->Count(); ++i )
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

#ifdef DBG_UTIL
    else if ( pFoll->GetValidPrtAreaFlag() ||
              pFoll->GetValidSizeFlag() )
    {
        pFoll->CalcFtnFlag();
        ASSERT( !pFoll->HasFtn(), "Missing FtnFlag." );
    }
#endif

    pFoll->MoveFlyInCnt( this, nStart, STRING_LEN );
    pFoll->SetFtn( sal_False );
    // --> OD 2005-12-01 #i27138#
    // notify accessibility paragraphs objects about changed CONTENT_FLOWS_FROM/_TO relation.
    // Relation CONTENT_FLOWS_FROM for current next paragraph will change
    // and relation CONTENT_FLOWS_TO for current previous paragraph, which
    // is <this>, will change.
    {
        ViewShell* pViewShell( pFoll->GetShell() );
        if ( pViewShell && pViewShell->GetLayout() &&
             pViewShell->GetLayout()->IsAnyShellAccessible() )
        {
            pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTxtFrm*>(pFoll->FindNextCnt( true )),
                            this );
        }
    }
    // <--
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
    SWAP_IF_SWAPPED( this )

    // Durch das Paste wird ein Modify() an mich verschickt.
    // Damit meine Daten nicht verschwinden, locke ich mich.
    SwTxtFrmLocker aLock( this );
    SwTxtFrm *pNew = (SwTxtFrm *)(GetTxtNode()->MakeFrm());
    pNew->bIsFollow = sal_True;

    pNew->SetFollow( GetFollow() );
    SetFollow( pNew );

    pNew->Paste( GetUpper(), GetNext() );
    // --> OD 2005-12-01 #i27138#
    // notify accessibility paragraphs objects about changed CONTENT_FLOWS_FROM/_TO relation.
    // Relation CONTENT_FLOWS_FROM for current next paragraph will change
    // and relation CONTENT_FLOWS_TO for current previous paragraph, which
    // is <this>, will change.
    {
        ViewShell* pViewShell( pNew->GetShell() );
        if ( pViewShell && pViewShell->GetLayout() &&
             pViewShell->GetLayout()->IsAnyShellAccessible() )
        {
            pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTxtFrm*>(pNew->FindNextCnt( true )),
                            this );
        }
    }
    // <--

    // Wenn durch unsere Aktionen Fussnoten in pNew landen,
    // so muessen sie umgemeldet werden.
    if ( HasFtn() )
    {
        const SwpHints *pHints = GetTxtNode()->GetpSwpHints();
        if( pHints )
        {
            SwFtnBossFrm *pFtnBoss = 0;
            SwFtnBossFrm *pEndBoss = 0;
            for ( sal_uInt16 i = 0; i < pHints->Count(); ++i )
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

#ifdef DBG_UTIL
    else
    {
        CalcFtnFlag( nTxtPos-1 );
        ASSERT( !HasFtn(), "Missing FtnFlag." );
    }
#endif

    MoveFlyInCnt( pNew, nTxtPos, STRING_LEN );

    // Kein SetOfst oder CalcFollow, weil gleich ohnehin ein AdjustFollow folgt.

    pNew->ManipOfst( nTxtPos );

    UNDO_SWAP( this )
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
    ASSERT( ! IsVertical() || ! IsSwapped(), "SwTxtFrm::CalcPreps with swapped frame" );
    SWRECTFN( this );

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
            if( nChgHeight >= (Prt().*fnRect->fnGetHeight)() )
            {
                if( bPrepMustFit )
                {
                    GetFollow()->SetJustWidow( sal_True );
                    GetFollow()->Prepare( PREP_CLEAR );
                }
                else if ( bVert )
                {
                    Frm().Width( Frm().Width() + Frm().Left() );
                    Prt().Width( Prt().Width() + Frm().Left() );
                    Frm().Left( 0 );
                    SetWidow( sal_True );
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
                ASSERT( nChgHeight < (Prt().*fnRect->fnGetHeight)(),
                        "+SwTxtFrm::CalcPrep: wanna shrink" );

                nChgHeight = (Prt().*fnRect->fnGetHeight)() - nChgHeight;

                GetFollow()->SetJustWidow( sal_True );
                GetFollow()->Prepare( PREP_CLEAR );
                Shrink( nChgHeight );
                SwRect &rRepaint = *(pPara->GetRepaint());

                if ( bVert )
                {
                    SwRect aRepaint( Frm().Pos() + Prt().Pos(), Prt().SSize() );
                    SwitchVerticalToHorizontal( aRepaint );
                    rRepaint.Chg( aRepaint.Pos(), aRepaint.SSize() );
                }
                else
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

            SWAP_IF_NOT_SWAPPED( this )

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
            // OD 2004-02-25 #i16128# - method renamed
            sal_Bool bBreak = aFrmBreak.IsBreakNowWidAndOrp( aLine );
            bRet = sal_True;
            while( !bBreak && aLine.Next() )
            {
                // OD 2004-02-25 #i16128# - method renamed
                bBreak = aFrmBreak.IsBreakNowWidAndOrp( aLine );
            }
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
                {
                    FormatAdjust( aLine, aFrmBreak,
                                  aInf.GetTxt().Len(), aInf.IsStop() );
                }
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

            UNDO_SWAP( this )
            // Eine letzte Ueberpruefung, falls das FormatAdjust() nichts
            // brachte, muessen wir amputieren.
            if( bPrepMustFit )
            {
                const SwTwips nMust = (GetUpper()->*fnRect->fnGetPrtBottom)();
                const SwTwips nIs   = (Frm().*fnRect->fnGetBottom)();

                if( bVert && nIs < nMust )
                {
                    Shrink( nMust - nIs );
                    if( Prt().Width() < 0 )
                        Prt().Width( 0 );
                    SetUndersized( sal_True );
                }
                else if ( ! bVert && nIs > nMust )
                {
                    Shrink( nIs - nMust );
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
                             const xub_StrLen nStrLen,
                             const sal_Bool bDummy )
{
    SWAP_IF_NOT_SWAPPED( this )

    SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();

    xub_StrLen nEnd = rLine.GetStart();

    sal_Bool bHasToFit = pPara->IsPrepMustFit();

    // Das StopFlag wird durch Fussnoten gesetzt,
    // die auf die naechste Seite wollen.
    // OD, FME 2004-03-03 - call base class method <SwTxtFrmBreak::IsBreakNow(..)>
    // instead of method <WidowsAndOrphans::IsBreakNow(..)> to get a break,
    // even if due to widow rule no enough lines exists.
    sal_uInt8 nNew = ( !GetFollow() &&
                       nEnd < nStrLen &&
                       ( rLine.IsStop() ||
                         ( bHasToFit
                           ? ( rLine.GetLineNr() > 1 &&
                               !rFrmBreak.IsInside( rLine ) )
                           : rFrmBreak.IsBreakNow( rLine ) ) ) )
                     ? 1 : 0;
    if ( nNew )
        SplitFrm( nEnd );

    const SwFrm *pBodyFrm = (const SwFrm*)(FindBodyFrm());

    const long nBodyHeight = pBodyFrm ? ( IsVertical() ?
                                          pBodyFrm->Frm().Width() :
                                          pBodyFrm->Frm().Height() ) : 0;

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
        // bleibt, denn es k???nnte ja ein harter Zeilenumbruch noch eine weitere
        // Zeile (ohne Textmassse) notwendig machen!
        nEnd = rLine.GetEnd();
        if( GetFollow() )
        {
            // OD 21.03.2003 #108121# - Another case for not joining the follow:
            // Text frame has no content, but a numbering. Then, do *not* join.
            // Example of this case: When an empty, but numbered paragraph
            // at the end of page is completely displaced by a fly frame.
            // Thus, the text frame introduced a follow by a
            // <SwTxtFrm::SplitFrm(..)> - see below. The follow then shows
            // the numbering and must stay.
            if ( GetFollow()->GetOfst() != nEnd ||
                 GetFollow()->IsFieldFollow() ||
                 ( nStrLen == 0 && GetTxtNode()->GetNumRule())
               )
            {
                nNew |= 3;
            }
            CHG_OFFSET( GetFollow(), nEnd )
            GetFollow()->ManipOfst( nEnd );
        }
        else
        {
            // OD 21.03.2003 #108121# - Only split frame, if the frame contains
            // content or contains no content, but has a numbering.
            if ( nStrLen > 0 ||
                 ( nStrLen == 0 && GetTxtNode()->GetNumRule())
               )
            {
                SplitFrm( nEnd );
                nNew |= 3;
            }
        }
        // Wenn sich die Resthoehe geaendert hat, z.B. durch RemoveFtn()
        // dann muessen wir auffuellen, um Oszillationen zu vermeiden!
        if( bDummy && pBodyFrm &&
           nBodyHeight < ( IsVertical() ?
                           pBodyFrm->Frm().Width() :
                           pBodyFrm->Frm().Height() ) )
            rLine.MakeDummyLine();
    }

    // In AdjustFrm() stellen wir uns selbst per Grow/Shrink ein,
    // in AdjustFollow() stellen wir unseren FolgeFrame ein.

    const SwTwips nDocPrtTop = Frm().Top() + Prt().Top();
    const SwTwips nOldHeight = Prt().SSize().Height();
    const SwTwips nChg = rLine.CalcBottomLine() - nDocPrtTop - nOldHeight;

    // Vertical Formatting:
    // The (rotated) repaint rectangle's x coordinate referes to the frame.
    // If the frame grows (or shirks) the repaint rectangle cannot simply
    // be rotated back after formatting, because we use the upper left point
    // of the frame for rotation. This point changes when growing/shrinking.

    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    if ( IsVertical() && !IsVertLR() && nChg )
    {
        SwRect &rRepaint = *(pPara->GetRepaint());
        rRepaint.Left( rRepaint.Left() - nChg );
        rRepaint.Width( rRepaint.Width() - nChg );
    }

    AdjustFrm( nChg, bHasToFit );

/*
    // FME 16.07.2003 #i16930# - removed this code because it did not
    // work correctly. In SwCntntFrm::MakeAll, the frame did not move to the
    // next page, instead the print area was recalculated and
    // Prepare( PREP_POS_CHGD, (const void*)&bFormatted, sal_False ) invalidated
    // the other flags => loop

    // OD 04.04.2003 #108446# - handle special case:
    // If text frame contains no content and just has split, because of a
    // line stop, it has to move forward. To force this forward move without
    // unnecessary formatting of its footnotes and its follow, especially in
    // columned sections, adjust frame height to zero (0) and do not perform
    // the intrinsic format of the follow.
    // The formating method <SwCntntFrm::MakeAll()> will initiate the move forward.
    sal_Bool bForcedNoIntrinsicFollowCalc = sal_False;
    if ( nEnd == 0 &&
         rLine.IsStop() && HasFollow() && nNew == 1
       )
    {
        AdjustFrm( -Frm().SSize().Height(), bHasToFit );
        Prt().Pos().Y() = 0;
        Prt().Height( Frm().Height() );
        if ( FollowFormatAllowed() )
        {
            bForcedNoIntrinsicFollowCalc = sal_True;
            ForbidFollowFormat();
        }
    }
    else
    {
        AdjustFrm( nChg, bHasToFit );
    }
 */

    if( HasFollow() || IsInFtn() )
        _AdjustFollow( rLine, nEnd, nStrLen, nNew );

    // FME 16.07.2003 #i16930# - removed this code because it did not work
    // correctly
    // OD 04.04.2003 #108446# - allow intrinsic format of follow, if above
    // special case has forbit it.
/*    if ( bForcedNoIntrinsicFollowCalc )
    {
        AllowFollowFormat();
    }
 */

    pPara->SetPrepMustFit( sal_False );

    UNDO_SWAP( this )
}

/*************************************************************************
 *                      SwTxtFrm::FormatLine()
 *************************************************************************/

// bPrev zeigt an, ob Reformat.Start() wegen Prev() vorgezogen wurde.
// Man weiss sonst nicht, ob man Repaint weiter einschraenken kann oder nicht.


sal_Bool SwTxtFrm::FormatLine( SwTxtFormatter &rLine, const sal_Bool bPrev )
{
    ASSERT( ! IsVertical() || IsSwapped(),
            "SwTxtFrm::FormatLine( rLine, bPrev) with unswapped frame" );
    SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();
    // Nach rLine.FormatLine() haelt nStart den neuen Wert,
    // waehrend in pOldStart der alte Offset gepflegt wird.
    // Ueber diesen Weg soll das nDelta ersetzt werden.
    // *pOldStart += rLine.GetCurr()->GetLen();
    const SwLineLayout *pOldCur = rLine.GetCurr();
    const xub_StrLen nOldLen    = pOldCur->GetLen();
    const KSHORT nOldAscent = pOldCur->GetAscent();
    const KSHORT nOldHeight = pOldCur->Height();
    const SwTwips nOldWidth = pOldCur->Width() + pOldCur->GetHangingMargin();
    const sal_Bool bOldHyph = pOldCur->IsEndHyph();
    SwTwips nOldTop = 0;
    SwTwips nOldBottom = 0;
    if( rLine.GetCurr()->IsClipping() )
        rLine.CalcUnclipped( nOldTop, nOldBottom );

    const xub_StrLen nNewStart = rLine.FormatLine( rLine.GetStart() );

    ASSERT( Frm().Pos().Y() + Prt().Pos().Y() == rLine.GetFirstPos(),
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
        const long nWidthDiff = nOldWidth > pNew->Width()
                                ? nOldWidth - pNew->Width()
                                : pNew->Width() - nOldWidth;

        // we only declare a line as unchanged, if its main values have not
        // changed and it is not the last line (!paragraph end symbol!)
        bUnChg = nOldHeight == pNew->Height() &&
                 nOldAscent == pNew->GetAscent() &&
                 nWidthDiff <= SLOPPY_TWIPS &&
                 pOldCur->GetNext();
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
        SwTwips nRght = Max( nOldWidth, pNew->Width() +
                             pNew->GetHangingMargin() );
        ViewShell *pSh = GetShell();
        const SwViewOption *pOpt = pSh ? pSh->GetViewOptions() : 0;
        if( pOpt && (pOpt->IsParagraph() || pOpt->IsLineBreak()) )
            nRght += ( Max( nOldAscent, pNew->GetAscent() ) );
        else
            nRght += ( Max( nOldAscent, pNew->GetAscent() ) / 4);
        nRght += rLine.GetLeftMargin();
        if( rRepaint.GetOfst() || rRepaint.GetRightOfst() < nRght )
            rRepaint.SetRightOfst( nRght );

        // Finally we enlarge the repaint rectangle if we found an underscore
        // within our line. 40 Twips should be enough
        const sal_Bool bHasUnderscore =
                ( rLine.GetInfo().GetUnderScorePos() < nNewStart );
        if ( bHasUnderscore || rLine.GetCurr()->HasUnderscore() )
            rRepaint.Bottom( rRepaint.Bottom() + 40 );

        ((SwLineLayout*)rLine.GetCurr())->SetUnderscore( bHasUnderscore );
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
}

/*************************************************************************
 *                      SwTxtFrm::_Format()
 *************************************************************************/

void SwTxtFrm::_Format( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf,
                        const sal_Bool bAdjust )
{
    ASSERT( ! IsVertical() || IsSwapped(),"SwTxtFrm::_Format with unswapped frame" );

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

    // --> FME 2005-04-18 #i46560#
    // FME: Yes, consider this case: (word ) has to go to the next line
    // because ) is a forbidden character at the beginning of a line although
    // (word would still fit on the previous line. Adding text right in front
    // of ) would not trigger a reformatting of the previous line. Adding 1
    // to the result of FindBrk() does not solve the problem in all cases,
    // nevertheless it should be sufficient.
    // <--
    sal_Bool bPrev = rLine.GetPrev() &&
                     ( FindBrk( rString, rLine.GetStart(), rReformat.Start() + 1 )
                       // --> FME 2005-04-18 #i46560#
                       + 1
                       // <--
                       >= rReformat.Start() ||
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
    if( pPara->IsMargin() )
        rRepaint.Width( rRepaint.Width() + pPara->GetHangingMargin() );
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

    // OD 2004-02-25 #i16128# - method renamed
    sal_Bool bBreak = ( !pPara->IsPrepMustFit() || rLine.GetLineNr() > 1 )
                    && aFrmBreak.IsBreakNowWidAndOrp( rLine );
    if( bBreak )
    {
        sal_Bool bPrevDone = 0 != rLine.Prev();
        // OD 2004-02-25 #i16128# - method renamed
        while( bPrevDone && aFrmBreak.IsBreakNowWidAndOrp(rLine) )
            bPrevDone = 0 != rLine.Prev();
        if( bPrevDone )
        {
            aFrmBreak.SetKeep( sal_False );
            rLine.Next();
        }
        rLine.TruncLines();

        // auf Nummer sicher:
        // OD 2004-02-25 #i16128# - method renamed
        bBreak = aFrmBreak.IsBreakNowWidAndOrp(rLine) &&
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
        const SwLineLayout* pLine;
        {
            SwTxtFrm *pMaster = FindMaster();
            ASSERT( pMaster, "SwTxtFrm::Format: homeless follow" );
            if( !pMaster->HasPara() )
                pMaster->GetFormatted();
            SwTxtSizeInfo aInf( pMaster );
            SwTxtIter aMasterLine( pMaster, &aInf );
            aMasterLine.Bottom();
            pLine = aMasterLine.GetCurr();
        }
        SwLinePortion* pRest =
            rLine.MakeRestPortion( pLine, GetOfst() );
        if( pRest )
            rInf.SetRest( pRest );
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
                {
                    SwLinePortion* pRest =
                        rLine.MakeRestPortion( rLine.GetCurr(), rLine.GetEnd() );
                    if( pRest )
                        rInf.SetRest( pRest );
                }
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
        // OD 2004-02-25 #i16128# - method renamed
        bBreak = aFrmBreak.IsBreakNowWidAndOrp(rLine);
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
        {
            FormatAdjust( rLine, aFrmBreak, nStrLen, rInf.IsStop() );
        }
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
    ASSERT( ! IsVertical() || IsSwapped(),
            "A frame is not swapped in SwTxtFrm::FormatOnceMore" );

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
                rInf.CtorInitTxtFormatInfo( this );
                rLine.CtorInitTxtFormatter( this, &rInf );
                rLine.SetDropLines( 1 );
                rLine.CalcDropHeight( 1 );
                SwCharRange aTmpRange( 0, rInf.GetTxt().Len() );
                *(pPara->GetReformat()) = aTmpRange;
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

        // delete pSpaceAdd und pKanaComp
        pPara->FinishSpaceAdd();
        pPara->FinishKanaComp();
        pPara->ResetFlags();
        pPara->SetPrepMustFit( bMustFit );
    }

    ASSERT( ! IsSwapped(), "A frame is swapped before _Format" );

    if ( IsVertical() )
        SwapWidthAndHeight();

    SwTxtFormatInfo aInf( this );
    SwTxtFormatter  aLine( this, &aInf );

    // OD 2004-01-15 #110582#
    HideAndShowObjects();

    _Format( aLine, aInf );

    if( aLine.IsOnceMore() )
        FormatOnceMore( aLine, aInf );

    if ( IsVertical() )
        SwapWidthAndHeight();

    ASSERT( ! IsSwapped(), "A frame is swapped after _Format" );

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
#if OSL_DEBUG_LEVEL > 1
    const XubString aXXX = GetTxtNode()->GetTxt();
    const SwTwips nDbgY = Frm().Top();
    (void)nDbgY;
    const SwPageFrm *pDbgPage = FindPageFrm();
    const MSHORT nDbgPageNr = pDbgPage->GetPhyPageNum();
    (void)nDbgPageNr;
    // Um zu gucken, ob es einen Ftn-Bereich gibt.
    const SwFrm *pDbgFtnCont = (const SwFrm*)(FindPageFrm()->FindFtnCont());
    (void)pDbgFtnCont;

#ifdef DBG_UTIL
    // nStopAt laesst sich vom CV bearbeiten.
    static MSHORT nStopAt = 0;
    if( nStopAt == GetFrmId() )
    {
        int i = GetFrmId();
        (void)i;
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

    SWRECTFN( this )

    // --> OD 2008-01-31 #newlistlevelattrs#
    CalcAdditionalFirstLineOffset();
    // <--

    // Vom Berichtsautopiloten oder ueber die BASIC-Schnittstelle kommen
    // gelegentlich TxtFrms mit einer Breite <=0.
    if( (Prt().*fnRect->fnGetWidth)() <= 0 )
    {
        // Wenn MustFit gesetzt ist, schrumpfen wir ggf. auf die Unterkante
        // des Uppers, ansonsten nehmen wir einfach eine Standardgroesse
        // von 12 Pt. ein (240 Twip).
        SwTxtLineAccess aAccess( this );
        long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
        if( aAccess.GetPara()->IsPrepMustFit() )
        {
            const SwTwips nLimit = (GetUpper()->*fnRect->fnGetPrtBottom)();
            const SwTwips nDiff = - (Frm().*fnRect->fnBottomDist)( nLimit );
            if( nDiff > 0 )
                Shrink( nDiff );
        }
        else if( 240 < nFrmHeight )
            Shrink( nFrmHeight - 240 );
        else if( 240 > nFrmHeight )
            Grow( 240 - nFrmHeight );
        nFrmHeight = (Frm().*fnRect->fnGetHeight)();

        long nTop = (this->*fnRect->fnGetTopMargin)();
        if( nTop > nFrmHeight )
            (this->*fnRect->fnSetYMargins)( nFrmHeight, 0 );
        else if( (Prt().*fnRect->fnGetHeight)() < 0 )
            (Prt().*fnRect->fnSetHeight)( 0 );
        return;
    }

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

        // 8708: Vorsicht, das Format() kann auch durch GetFormatted()
        // angestossen werden.
        if( IsHiddenNow() )
        {
            long nPrtHeight = (Prt().*fnRect->fnGetHeight)();
            if( nPrtHeight )
            {
                HideHidden();
                Shrink( nPrtHeight );
            }
            else
            {
                // OD 2004-01-20 #110582# - assure that objects anchored
                // at paragraph resp. at/as character inside paragraph
                // are hidden.
                HideAndShowObjects();
            }
            ChgThisLines();
            return;
        }

        // Waehrend wir formatieren, wollen wir nicht gestoert werden.
        SwTxtFrmLocker aLock(this);
        SwTxtLineAccess aAccess( this );
        const sal_Bool bNew = !aAccess.SwTxtLineAccess::IsAvailable();
        const sal_Bool bSetOfst = ( GetOfst() && GetOfst() > GetTxtNode()->GetTxt().Len() );

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
        else if( bSetOfst && IsFollow() )
        {
            SwTxtFrm *pMaster = FindMaster();
            ASSERT( pMaster, "SwTxtFrm::Format: homeless follow" );
            if( pMaster )
                pMaster->Prepare( PREP_FOLLOW_FOLLOWS );
            SwTwips nMaxY = (GetUpper()->*fnRect->fnGetPrtBottom)();
            if( (Frm().*fnRect->fnOverStep)( nMaxY  ) )
                (this->*fnRect->fnSetLimit)( nMaxY );
            else if( (Frm().*fnRect->fnBottomDist)( nMaxY  ) < 0 )
                (Frm().*fnRect->fnAddBottom)( -(Frm().*fnRect->fnGetHeight)() );
        }
        else
        {
            // bSetOfst here means that we have the "red arrow situation"
            if ( bSetOfst )
                _SetOfst( 0 );

            const sal_Bool bOrphan = IsWidow();
            const SwFtnBossFrm* pFtnBoss = HasFtn() ? FindFtnBossFrm() : 0;
            SwTwips nFtnHeight = 0;
            if( pFtnBoss )
            {
                const SwFtnContFrm* pCont = pFtnBoss->FindFtnCont();
                nFtnHeight = pCont ? (pCont->Frm().*fnRect->fnGetHeight)() : 0;
            }
            do
            {
                _Format( aAccess.GetPara() );
                if( pFtnBoss && nFtnHeight )
                {
                    const SwFtnContFrm* pCont = pFtnBoss->FindFtnCont();
                    SwTwips nNewHeight = pCont ? (pCont->Frm().*fnRect->fnGetHeight)() : 0;
                    // If we lost some footnotes, we may have more space
                    // for our main text, so we have to format again ...
                    if( nNewHeight < nFtnHeight )
                        nFtnHeight = nNewHeight;
                    else
                        break;
                }
                else
                    break;
            } while ( pFtnBoss );
            if( bOrphan )
            {
                ValidateFrm();
                SetWidow( sal_False );
            }
        }
        if( IsEmptyMaster() )
        {
            SwFrm* pPre = GetPrev();
            if( pPre &&
                // --> FME 2004-07-22 #i10826# It's the first, it cannot keep!
                pPre->GetIndPrev() &&
                // <--
                pPre->GetAttrSet()->GetKeep().GetValue() )
            {
                pPre->InvalidatePos();
            }
        }
    }

    ChgThisLines();

    // the PrepMustFit should not survive a Format operation
    SwParaPortion *pPara = GetPara();
    if ( pPara )
           pPara->SetPrepMustFit( sal_False );

#if OSL_DEBUG_LEVEL > 1
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

    CalcBaseOfstForFly();
    // OD 2004-03-17 #i11860#
    _CalcHeightOfLastLine();
}

/*************************************************************************
 *                      SwTxtFrm::FormatQuick()
 *
 * bForceQuickFormat is set if GetFormatted() has been called during the
 * painting process. Actually I cannot imagine a situation which requires
 * a full formatting of the paragraph during painting, on the other hand
 * a full formatting can cause the invalidation of other layout frames,
 * e.g., if there are footnotes in this paragraph, and invalid layout
 * frames will not calculated during the painting. So I actually want to
 * avoid a formatting during painting, but since I'm a coward, I'll only
 * force the quick formatting in the situation of issue i29062.
 *************************************************************************/

sal_Bool SwTxtFrm::FormatQuick( bool bForceQuickFormat )
{
    ASSERT( ! IsVertical() || ! IsSwapped(),
            "SwTxtFrm::FormatQuick with swapped frame" );

    DBG_LOOP;
#if OSL_DEBUG_LEVEL > 1
    const XubString aXXX = GetTxtNode()->GetTxt();
    const SwTwips nDbgY = Frm().Top();
    (void)nDbgY;
#ifdef DBG_UTIL
    // nStopAt laesst sich vom CV bearbeiten.
    static MSHORT nStopAt = 0;
    if( nStopAt == GetFrmId() )
    {
        int i = GetFrmId();
        (void)i;
    }
#endif
#endif

    if( IsEmpty() && FormatEmpty() )
        return sal_True;

    // Wir sind sehr waehlerisch:
    if( HasPara() || IsWidow() || IsLocked()
        || !GetValidSizeFlag() ||
        ( ( IsVertical() ? Prt().Width() : Prt().Height() ) && IsHiddenNow() ) )
        return sal_False;

    SwTxtLineAccess aAccess( this );
    SwParaPortion *pPara = aAccess.GetPara();
    if( !pPara )
        return sal_False;

    SwFrmSwapper aSwapper( this, sal_True );

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
    {
        //DBG_LOOP; shadows declaration above.
        //resolved into:
#if OSL_DEBUG_LEVEL > 1
#ifdef DBG_UTIL
        DbgLoop aDbgLoop2( (const void*) this );
#endif
#endif
        nStart = aLine.FormatLine( nStart );
        if( aInf.IsNewLine() || (!aInf.IsStop() && nStart < nEnd) )
            aLine.Insert( new SwLineLayout() );
    } while( aLine.Next() );

    // Last exit: die Hoehen muessen uebereinstimmen.
    Point aTopLeft( Frm().Pos() );
    aTopLeft += Prt().Pos();
    const SwTwips nNewHeight = aLine.Y() + aLine.GetLineHeight();
    const SwTwips nOldHeight = aTopLeft.Y() + Prt().Height();

    if( !bForceQuickFormat && nNewHeight != nOldHeight && !IsUndersized() )
    {
        // Achtung: Durch FormatLevel==12 kann diese Situation auftreten, don't panic!
        // ASSERT( nNewHeight == nOldHeight, "!FormatQuick: rosebud" );
        const xub_StrLen nStrt = GetOfst();
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
