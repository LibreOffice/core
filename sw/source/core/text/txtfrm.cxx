/*************************************************************************
 *
 *  $RCSfile: txtfrm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2000-10-13 08:59:05 $
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

#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif

#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>      // GetDoc()
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>  // InvalidateSpelling
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>   // ViewShell
#endif
#ifndef _PAM_HXX
#include <pam.hxx>      // SwPosition
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>        // SwTxtNode
#endif
#ifndef _TXTATR_HXX
#include <txtatr.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>        // SwInsChr
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _DBG_LAY_HXX
#include <dbg_lay.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif
#ifndef _FMTLINE_HXX
#include <fmtline.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>       // SwTxtFrm
#endif
#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>      // SwSectFrm
#endif
#ifndef _TXTCFG_HXX
#include <txtcfg.hxx>       // DBG_LOOP
#endif
#ifndef _ITRFORM2_HXX
#include <itrform2.hxx>       // Iteratoren
#endif
#ifndef _WIDORP_HXX
#include <widorp.hxx>       // SwFrmBreak
#endif
#ifndef _TXTCACHE_HXX
#include <txtcache.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>       // GetLineSpace benutzt SwFonts
#endif
#ifndef _FNTCACHE_HXX
#include <fntcache.hxx>     // GetLineSpace benutzt pLastFont
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _WRONG_HXX
#include <wrong.hxx>        // SwWrongList
#endif
#ifndef _LINEINFO_HXX
#include <lineinfo.hxx>
#endif

#ifdef DEBUG
#ifndef _TXTPAINT_HXX
#include <txtpaint.hxx>     // DbgRect
#endif
extern const sal_Char *GetPrepName( const enum PrepareHint ePrep );
#endif

TYPEINIT1( SwTxtFrm, SwCntntFrm );


/*************************************************************************
 *                      SwTxtFrm::Init()
 *************************************************************************/

void SwTxtFrm::Init()
{
    ASSERT( !IsLocked(), "+SwTxtFrm::Init: this ist locked." );
    if( !IsLocked() )
    {
        ClearPara();
        ResetBlinkPor();
        //Die Flags direkt setzen um ResetPreps und damit ein unnuetzes GetPara
        //einzusparen.
        // Nicht bOrphan, bLocked oder bWait auf sal_False setzen !
        // bOrphan = bFlag7 = bFlag8 = sal_False;
    }
}

/*************************************************************************
|*  SwTxtFrm::CTORen/DTOR
|*************************************************************************/

void SwTxtFrm::InitCtor()
{
    nCacheIdx = MSHRT_MAX;
    nOfst = 0;
    nAllLines = 0;
    nThisLines = 0;
    nType = FRM_TXT;
    bLocked = bFormatted = bWidow = bUndersized = bJustWidow =
        bEmpty = bInFtnConnect = bFtn = bRepaint = bBlinkPor =
        bFieldFollow = bHasAnimation = sal_False;
}


SwTxtFrm::SwTxtFrm(SwTxtNode * const pNode)
    : SwCntntFrm(pNode)
{
    InitCtor();
}

const XubString& SwTxtFrm::GetTxt() const
{
    return GetTxtNode()->GetTxt();
}

void SwTxtFrm::ResetPreps()
{
    if ( GetCacheIdx() != MSHRT_MAX )
    {
        SwParaPortion *pPara;
        if( 0 != (pPara = GetPara()) )
            pPara->ResetPreps();
    }
}

/*************************************************************************
 *                        SwTxtFrm::IsHiddenNow()
 *************************************************************************/
// liefert nur sal_True zurueck, wenn das Outputdevice ein Printer ist
// und bHidden gesetzt ist.

sal_Bool SwTxtFrm::IsHiddenNow() const
{
    if( !Frm().Width() && IsValid() && GetUpper()->IsValid() )
                                       //bei Stackueberlauf (StackHack) invalid!
    {
        ASSERT( Frm().Width(), "SwTxtFrm::IsHiddenNow: thin frame" );
        return sal_True;
    }

    if( !GetTxtNode()->IsVisible() )
    {
        const ViewShell *pVsh = GetShell();
        if ( !pVsh )
            return sal_False;
        return OUTDEV_PRINTER == pVsh->GetOut()->GetOutDevType() ||
               (!pVsh->GetViewOptions()->IsShowHiddenPara()      &&
                !pVsh->GetViewOptions()->IsFldName());
    }
    else
        return sal_False;
}


/*************************************************************************
 *                        SwTxtFrm::HideHidden()
 *************************************************************************/
// Entfernt die Anhaengsel des Textfrms wenn dieser hidden ist

void SwTxtFrm::HideHidden()
{
    ASSERT( IsHiddenNow(), "HideHidden on visible frame" );

    //Erst die Fussnoten
    const SwpHints *pHints = GetTxtNode()->GetpSwpHints();
    if( pHints )
    {
        const MSHORT nSize = pHints->Count();
        const xub_StrLen nEnd = GetFollow() ? GetFollow()->GetOfst():STRING_LEN;
        SwPageFrm *pPage = 0;
        for( MSHORT i = 0; i < nSize; ++i )
        {
            const SwTxtAttr *pHt = (*pHints)[i];
            if ( pHt->Which() == RES_TXTATR_FTN )
            {
                const xub_StrLen nIdx = *pHt->GetStart();
                if ( nEnd < nIdx )
                    break;
                if( GetOfst() <= nIdx )
                {
                    if( !pPage )
                        pPage = FindPageFrm();
                    pPage->RemoveFtn( this, (SwTxtFtn*)pHt );
                }
            }
        }
    }
    //Dann die zeichengebundenen Rahmen
    if ( GetDrawObjs() )
    {
        for ( int i = GetDrawObjs()->Count()-1; i >= 0; --i )
        {
            SdrObject *pObj = (*GetDrawObjs())[i];
            SwFlyFrm *pFly;
            if ( pObj->IsWriterFlyFrame() &&
                 (pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm())->IsFlyInCntFrm())
            {
                pFly->GetAnchor()->RemoveFly( pFly );
                delete pFly;
            }
        }
    }
    //Die Formatinfos sind jetzt obsolete
    ClearPara();
}


/*************************************************************************
 *                      SwTxtFrm::FindBrk()
 *
 * Liefert die erste Trennmoeglichkeit in der aktuellen Zeile zurueck.
 * Die Methode wird in SwTxtFrm::Format() benutzt, um festzustellen, ob
 * die Vorgaengerzeile mitformatiert werden muss.
 * nFound ist <= nEndLine.
 *************************************************************************/

xub_StrLen SwTxtFrm::FindBrk( const XubString &rTxt,
                          const xub_StrLen nStart, const xub_StrLen nEnd ) const
{
    xub_StrLen nFound = nStart;
    const xub_StrLen nEndLine = Min( nEnd, rTxt.Len() );

    // Wir ueberlesen erst alle Blanks am Anfang der Zeile (vgl. Bug 2235).
    while( nFound <= nEndLine && ' ' == rTxt.GetChar( nFound ) )
         ++nFound;

    // Eine knifflige Sache mit den TxtAttr-Dummy-Zeichen (hier "$"):
    // "Dr.$Meyer" am Anfang der zweiten Zeile. Dahinter ein Blank eingegeben
    // und das Wort rutscht nicht in die erste Zeile, obwohl es ginge.
    // Aus diesem Grund nehmen wir das Dummy-Zeichen noch mit.
    while( nFound <= nEndLine && ' ' != rTxt.GetChar( nFound ) )
        ++nFound;

    return nFound;
}

/*************************************************************************
 *                      SwTxtFrm::IsIdxInside()
 *************************************************************************/

sal_Bool SwTxtFrm::IsIdxInside( const xub_StrLen nPos, const xub_StrLen nLen ) const
{
    if( GetOfst() > nPos + nLen ) // d.h., der Bereich liegt komplett vor uns.
        return sal_False;

    if( !GetFollow() )         // der Bereich liegt nicht komplett vor uns,
        return sal_True;           // nach uns kommt niemand mehr.

    const xub_StrLen nMax = GetFollow()->GetOfst();

    // der Bereich liegt nicht komplett hinter uns bzw.
    // unser Text ist geloescht worden.
    if( nMax > nPos || nMax > GetTxt().Len() )
        return sal_True;

    // WICHTIG: Nun kann es noch sein, dass das erste Wort unseres Follows
    // in uns hochrutschen koennte:
    const xub_StrLen nNewPos = FindBrk( GetTxt(), nMax, nPos + 1 );
    return nNewPos >= nPos;
}

/*************************************************************************
 *                      SwTxtFrm::InvalidateRange()
 *************************************************************************/
inline void SwTxtFrm::InvalidateRange(const SwCharRange &aRange, const long nD)
{
    if ( IsIdxInside( aRange.Start(), aRange.Len() ) )
        _InvalidateRange( aRange, nD );
}

/*************************************************************************
 *                      SwTxtFrm::_InvalidateRange()
 *************************************************************************/

void SwTxtFrm::_InvalidateRange( const SwCharRange &aRange, const long nD)
{
    if ( !HasPara() )
    {   InvalidateSize();
        return;
    }

    SetWidow( sal_False );
    SwParaPortion *pPara = GetPara();

    sal_Bool bInv = sal_False;
    if( 0 != nD )
    {
        //Auf nDelta werden die Differenzen zwischen alter und
        //neuer Zeilenlaenge aufaddiert, deshalb ist es negativ,
        //wenn Zeichen eingefuegt wurden, positiv, wenn Zeichen
        //geloescht wurden.
        *(pPara->GetDelta()) += nD;
        bInv = sal_True;
    }
    SwCharRange &rReformat = *(pPara->GetReformat());
    if(aRange != rReformat) {
        if( STRING_LEN == rReformat.Len() )
            rReformat = aRange;
        else
            rReformat += aRange;
        bInv = sal_True;
    }
    if(bInv)
    {
        if( GetFollow() )
            ((SwTxtFrm*)GetFollow())->InvalidateRange( aRange, nD );
        InvalidateSize();
    }
}

/*************************************************************************
 *                      SwTxtFrm::CalcLineSpace()
 *************************************************************************/

void SwTxtFrm::CalcLineSpace()
{
    if( IsLocked() || !HasPara() )
        return;

    SwParaPortion *pPara;
    if( GetDrawObjs() ||
        GetTxtNode()->GetSwAttrSet().GetLRSpace().IsAutoFirst() ||
        ( pPara = GetPara() )->IsFixLineHeight() )
    {
        Init();
        return;
    }

    Size aNewSize( Prt().SSize() );
    SwTxtFormatInfo aInf( this );
    SwTxtFormatter aLine( this, &aInf );
    if( aLine.GetDropLines() )
    {
        Init();
        return;
    }

    aLine.Top();
    aLine.RecalcRealHeight();

    aNewSize.Height() = (aLine.Y() - Frm().Top()) + aLine.GetLineHeight();

    SwTwips nDelta = aNewSize.Height() - Prt().Height();
    // 4291: Unterlauf bei Flys
    if( aInf.GetTxtFly()->IsOn() )
    {
        SwRect aFrm( Frm() );
        if( nDelta < 0 )
            aFrm.Height( Prt().Height() );
        else
            aFrm.Height( aNewSize.Height() );
        if( aInf.GetTxtFly()->Relax( aFrm ) )
        {
            Init();
            return;
        }
    }

    if( nDelta )
    {
        SwTxtFrmBreak aBreak( this );
        if( GetFollow() || aBreak.IsBreakNow( aLine ) )
        {
            // Wenn es einen Follow() gibt, oder wenn wir an dieser
            // Stelle aufbrechen muessen, so wird neu formatiert.
            Init();
        }
        else
        {
            // Alles nimmt seinen gewohnten Gang ...
            pPara->SetPrepAdjust();
            pPara->SetPrep();
#ifdef USED
            if (nDelta > 0)
                Grow(nDelta,pHeight);
            else
                Shrink(-nDelta,pHeight);
            ASSERT( GetPara(), "+SwTxtFrm::CalcLineSpace: missing format information" );
            if( pPara )
                pPara->GetRepaint()->SSize( Prt().SSize() );
#endif
        }
    }
}

/*************************************************************************
 *                      SwTxtFrm::Modify()
 *************************************************************************/

#define SET_WRONG( nPos, nCnt, fnFunc )\
    if( GetTxtNode()->GetWrong() && !IsFollow() )\
        GetTxtNode()->GetWrong()->fnFunc( nPos, nCnt );\
    GetNode()->SetWrongDirty( sal_True );\
    GetNode()->SetAutoCompleteWordDirty(  sal_True );\
    SwPageFrm *pPage = FindPageFrm();\
    if( pPage ) {  \
        pPage->InvalidateSpelling(); \
        pPage->InvalidateAutoCompleteWords(); \
    }

void lcl_ModifyOfst( SwTxtFrm* pFrm, xub_StrLen nPos, xub_StrLen nLen )
{
    if( nLen < 0 )
        nPos -= nLen;
    while( pFrm && pFrm->GetOfst() <= nPos )
        pFrm = pFrm->GetFollow();
    while( pFrm )
    {
        pFrm->ManipOfst( pFrm->GetOfst() + nLen );
        pFrm = pFrm->GetFollow();
    }
}

void SwTxtFrm::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
{
    const MSHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;

    //Wuensche die FrmAttribute betreffen werden von der Basisklasse
    //verarbeitet.
    if( IsInRange( aFrmFmtSetRange, nWhich ) || RES_FMT_CHG == nWhich )
    {
        SwCntntFrm::Modify( pOld, pNew );
        if( nWhich == RES_FMT_CHG && GetShell() )
        {
            // Collection hat sich geaendert
            Prepare( PREP_CLEAR );
            _InvalidatePrt();
            SET_WRONG( 0, STRING_LEN, Invalidate );
            InvalidateLineNum();
        }
        return;
    }

    // Im gelockten Zustand werden keine Bestellungen angenommen.
    if( IsLocked() )
        return;

    // Dies spart Stack, man muss nur aufpassen,
    // dass sie Variablen gesetzt werden.
    xub_StrLen nPos, nLen;
    sal_Bool bSetFldsDirty = sal_False;
    sal_Bool bRecalcFtnFlag = sal_False;

    switch( nWhich )
    {
        case RES_LINENUMBER:
        {
            InvalidateLineNum();
        }
        break;
        case RES_INS_CHR:
        {
            nPos = ((SwInsChr*)pNew)->nPos;
            InvalidateRange( SwCharRange( nPos, 1 ), 1 );
            SET_WRONG( nPos, 1, Move )
            bSetFldsDirty = sal_True;
            if( HasFollow() )
                lcl_ModifyOfst( this, nPos, 1 );
        }
        break;
        case RES_INS_TXT:
        {
            nPos = ((SwInsTxt*)pNew)->nPos;
            nLen = ((SwInsTxt*)pNew)->nLen;
            if( IsIdxInside( nPos, nLen ) )
            {
                if( !nLen )
                {
                    // 6969: Aktualisierung der NumPortions auch bei leeren Zeilen!
                    if( nPos )
                        InvalidateSize();
                    else
                        Prepare( PREP_CLEAR );
                }
                else
                    _InvalidateRange( SwCharRange( nPos, nLen ), nLen );
            }
            SET_WRONG( nPos, nLen, Move )
            bSetFldsDirty = sal_True;
            if( HasFollow() )
                lcl_ModifyOfst( this, nPos, nLen );
        }
        break;
        case RES_DEL_CHR:
        {
            nPos = ((SwDelChr*)pNew)->nPos;
            InvalidateRange( SwCharRange( nPos, 1 ), -1 );
            SET_WRONG( nPos, -1, Move )
            bSetFldsDirty = bRecalcFtnFlag = sal_True;
            if( HasFollow() )
                lcl_ModifyOfst( this, nPos, -1 );
        }
        break;
        case RES_DEL_TXT:
        {
            nPos = ((SwDelTxt*)pNew)->nStart;
            nLen = ((SwDelTxt*)pNew)->nLen;
            long m = nLen;
            m *= -1;
            if( IsIdxInside( nPos, nLen ) )
            {
                if( !nLen )
                    InvalidateSize();
                else
                    InvalidateRange( SwCharRange( nPos, 1 ), m );
            }
            SET_WRONG( nPos, m, Move )
            bSetFldsDirty = bRecalcFtnFlag = sal_True;
            if( HasFollow() )
                lcl_ModifyOfst( this, nPos, nLen );
        }
        break;
        case RES_UPDATE_ATTR:
        {
            nPos = ((SwUpdateAttr*)pNew)->nStart;
            nLen = ((SwUpdateAttr*)pNew)->nEnd - nPos;
            if( IsIdxInside( nPos, nLen ) )
            {
                // Es muss in jedem Fall neu formatiert werden,
                // auch wenn der invalidierte Bereich null ist.
                // Beispiel: leere Zeile, 14Pt einstellen !
                // if( !nLen ) nLen = 1;

                // 6680: FtnNummern muessen formatiert werden.
                if( !nLen )
                    nLen = 1;

                _InvalidateRange( SwCharRange( nPos, nLen) );
                MSHORT nTmp = ((SwUpdateAttr*)pNew)->nWhichAttr;
                if( !nTmp || RES_CHRATR_LANGUAGE == nTmp ||
                    RES_TXTATR_CHARFMT == nTmp )
                    SET_WRONG( nPos, nPos + nLen, Invalidate );
            }
        }
        break;
        case RES_OBJECTDYING:
        break;

        case RES_PARATR_LINESPACING:
            {
                CalcLineSpace();
                InvalidateSize();
                _InvalidatePrt();
                if( IsInSct() && !GetPrev() )
                {
                    SwSectionFrm *pSect = FindSctFrm();
                    if( pSect->ContainsAny() == this )
                        pSect->InvalidatePrt();
                }
                SwFrm* pNxt;
                if ( 0 != ( pNxt = GetIndNext() ) )
                {
                    pNxt->_InvalidatePrt();
                    if ( pNxt->IsLayoutFrm() )
                        pNxt->InvalidatePage();
                }
                SetCompletePaint();
            }
            break;
        case RES_TXTATR_FIELD:
        {
            nPos = *((SwFmtFld*)pNew)->GetTxtFld()->GetStart();
            if( IsIdxInside( nPos, 1 ) )
            {
                if( pNew == pOld )
                {
                    // Nur repainten
                    // opt: invalidate aufs Window ?
                    InvalidatePage();
                    SetCompletePaint();
                }
                else
                    _InvalidateRange( SwCharRange( nPos, 1 ) );
            }
            bSetFldsDirty = sal_True;
        }
        break;
        case RES_TXTATR_FTN :
        {
            nPos = *((SwFmtFtn*)pNew)->GetTxtFtn()->GetStart();
            if( IsInFtn() || IsIdxInside( nPos, 1 ) )
                Prepare( PREP_FTN, ((SwFmtFtn*)pNew)->GetTxtFtn() );
            break;
        }

        case RES_ATTRSET_CHG:
        {
            InvalidateLineNum();

            SwAttrSet& rNewSet = *((SwAttrSetChg*)pNew)->GetChgSet();
            const SfxPoolItem* pItem;
            int nClear = 0;
            MSHORT nCount = rNewSet.Count();

            if( SFX_ITEM_SET == rNewSet.GetItemState( RES_TXTATR_FTN,
                sal_False, &pItem ))
            {
                nPos = *((SwFmtFtn*)pItem)->GetTxtFtn()->GetStart();
                if( IsIdxInside( nPos, 1 ) )
                    Prepare( PREP_FTN, pNew );
                nClear = 0x01;
                --nCount;
            }

            if( SFX_ITEM_SET == rNewSet.GetItemState( RES_TXTATR_FIELD,
                sal_False, &pItem ))
            {
                nPos = *((SwFmtFld*)pItem)->GetTxtFld()->GetStart();
                if( IsIdxInside( nPos, 1 ) )
                {
                    const SfxPoolItem& rOldItem = ((SwAttrSetChg*)pOld)->
                                        GetChgSet()->Get( RES_TXTATR_FIELD );
                    if( pItem == &rOldItem )
                    {
                        // Nur repainten
                        // opt: invalidate aufs Window ?
                        InvalidatePage();
                        SetCompletePaint();
                    }
                    else
                        _InvalidateRange( SwCharRange( nPos, 1 ) );
                }
                nClear |= 0x02;
                --nCount;
            }
            sal_Bool bLineSpace = SFX_ITEM_SET == rNewSet.GetItemState(
                                            RES_PARATR_LINESPACING, sal_False ),
                     bRegister  = SFX_ITEM_SET == rNewSet.GetItemState(
                                            RES_PARATR_REGISTER, sal_False );
            if ( bLineSpace || bRegister )
            {
                Prepare( bRegister ? PREP_REGISTER : PREP_ADJUST_FRM );
                CalcLineSpace();
                InvalidateSize();
                _InvalidatePrt();
                SwFrm* pNxt;
                if ( 0 == ( pNxt = GetIndNext() ) &&
                     bLineSpace && IsInFtn() )
                    pNxt = FindNext();
                if( pNxt )
                {
                    pNxt->_InvalidatePrt();
                    if ( pNxt->IsLayoutFrm() )
                    {
                        if( pNxt->IsSctFrm() )
                        {
                            SwFrm* pCnt = ((SwSectionFrm*)pNxt)->ContainsAny();
                            if( pCnt )
                                pCnt->_InvalidatePrt();
                        }
                        pNxt->InvalidatePage();
                    }
                }

                SetCompletePaint();
                nClear |= 0x04;
                if ( bLineSpace )
                {
                    --nCount;
                    if( IsInSct() && !GetPrev() )
                    {
                        SwSectionFrm *pSect = FindSctFrm();
                        if( pSect->ContainsAny() == this )
                            pSect->InvalidatePrt();
                    }
                }
                if ( bRegister )
                    --nCount;
            }
            if ( SFX_ITEM_SET == rNewSet.GetItemState( RES_PARATR_SPLIT,
                                                       sal_False ))
            {
                if ( GetPrev() )
                    CheckKeep();
                Prepare( PREP_CLEAR );
                InvalidateSize();
                nClear |= 0x08;
                --nCount;
            }

            if ( SFX_ITEM_SET == rNewSet.GetItemState( RES_CHRATR_LANGUAGE,
                                                       sal_False ) ||
                 SFX_ITEM_SET == rNewSet.GetItemState( RES_TXTATR_CHARFMT,
                                                       sal_False ) )
                SET_WRONG( 0, STRING_LEN, Invalidate );

            if( nCount )
            {
                if( GetShell() )
                {
                    Prepare( PREP_CLEAR );
                    _InvalidatePrt();
                }

                if( nClear )
                {
                    SwAttrSetChg aOldSet( *(SwAttrSetChg*)pOld );
                    SwAttrSetChg aNewSet( *(SwAttrSetChg*)pNew );

                    if( 0x01 & nClear )
                    {
                        aOldSet.ClearItem( RES_TXTATR_FTN );
                        aNewSet.ClearItem( RES_TXTATR_FTN );
                    }
                    if( 0x02 & nClear )
                    {
                        aOldSet.ClearItem( RES_TXTATR_FIELD );
                        aNewSet.ClearItem( RES_TXTATR_FIELD );
                    }
                    if ( 0x04 & nClear )
                    {
                        if ( bLineSpace )
                        {
                            aOldSet.ClearItem( RES_PARATR_LINESPACING );
                            aNewSet.ClearItem( RES_PARATR_LINESPACING );
                        }
                        if ( bRegister )
                        {
                            aOldSet.ClearItem( RES_PARATR_REGISTER );
                            aNewSet.ClearItem( RES_PARATR_REGISTER );
                        }
                    }
                    if ( 0x08 & nClear )
                    {
                        aOldSet.ClearItem( RES_PARATR_SPLIT );
                        aNewSet.ClearItem( RES_PARATR_SPLIT );
                    }
                    SwCntntFrm::Modify( &aOldSet, &aNewSet );
                }
                else
                    SwCntntFrm::Modify( pOld, pNew );
            }
        }
        break;

/* Seit dem neuen Blocksatz muessen wir immer neu formatieren:
        case RES_PARATR_ADJUST:
        {
            if( GetShell() )
            {
                Prepare( PREP_CLEAR );
            }
            break;
        }
*/
        // 6870: SwDocPosUpdate auswerten.
        case RES_DOCPOS_UPDATE:
        {
            if( pOld && pNew )
            {
                const SwDocPosUpdate *pDocPos = (const SwDocPosUpdate*)pOld;
                if( pDocPos->nDocPos <= aFrm.Top() )
                {
                    const SwFmtFld *pFld = (const SwFmtFld *)pNew;
                    InvalidateRange(
                        SwCharRange( *pFld->GetTxtFld()->GetStart(), 1 ) );
                }
            }
            break;
        }
        case RES_PARATR_SPLIT:
            if ( GetPrev() )
                CheckKeep();
            Prepare( PREP_CLEAR );
            bSetFldsDirty = sal_True;
            break;

        default:
        {
            Prepare( PREP_CLEAR );
            _InvalidatePrt();
            if ( !nWhich )
            {
                //Wird z.B. bei HiddenPara mit 0 gerufen.
                SwFrm *pNxt;
                if ( 0 != (pNxt = FindNext()) )
                    pNxt->InvalidatePrt();
            }
        }
    } // switch

    if( bSetFldsDirty )
        GetNode()->GetDoc()->SetFieldsDirty( sal_True, GetNode(), 1 );

    if ( bRecalcFtnFlag )
        CalcFtnFlag();
}

sal_Bool SwTxtFrm::GetInfo( SfxPoolItem &rHnt ) const
{
    if ( RES_VIRTPAGENUM_INFO == rHnt.Which() && IsInDocBody() )
    {
        SwVirtPageNumInfo &rInfo = (SwVirtPageNumInfo&)rHnt;
        const SwPageFrm *pPage = FindPageFrm();
        if ( pPage  )
        {
            if ( pPage == rInfo.GetOrigPage() && !GetPrev() )
            {
                //Das sollte er sein (kann allenfalls temporaer anders sein,
                //                    sollte uns das beunruhigen?)
                rInfo.SetInfo( pPage, this );
                return sal_False;
            }
            if ( pPage->GetPhyPageNum() < rInfo.GetOrigPage()->GetPhyPageNum() &&
                 (!rInfo.GetPage() || pPage->GetPhyPageNum() > rInfo.GetPage()->GetPhyPageNum()))
            {
                //Das koennte er sein.
                rInfo.SetInfo( pPage, this );
            }
        }
    }
    return sal_True;
}

/*************************************************************************
 *                      SwTxtFrm::PrepWidows()
 *************************************************************************/

void SwTxtFrm::PrepWidows( const MSHORT nNeed, sal_Bool bNotify )
{
    ASSERT(GetFollow() && nNeed, "+SwTxtFrm::Prepare: lost all friends");

    SwParaPortion *pPara = GetPara();
    if ( !pPara )
        return;
    pPara->SetPrepWidows( sal_True );

    // returnen oder nicht ist hier die Frage.
    // Ohne IsLocked() ist 5156 gefaehrlich,
    // ohne IsFollow() werden die Orphans unterdrueckt: 6968.
    // Abfrage auf IsLocked erst hier, weil das Flag gesetzt werden soll.
    if( IsLocked() && IsFollow() )
        return;

    MSHORT nHave = nNeed;

    // Wir geben ein paar Zeilen ab und schrumpfen im CalcPreps()
    SwTxtSizeInfo aInf( this );
    SwTxtMargin aLine( this, &aInf );
    aLine.Bottom();
    xub_StrLen nTmpLen = aLine.GetCurr()->GetLen();
    while( nHave && aLine.PrevLine() )
    {
        if( nTmpLen )
            --nHave;
        nTmpLen = aLine.GetCurr()->GetLen();
    }
    // In dieser Ecke tummelten sich einige Bugs: 7513, 7606.
    // Wenn feststeht, dass Zeilen abgegeben werden koennen,
    // muss der Master darueber hinaus die Widow-Regel ueberpruefen.
    if( !nHave )
    {
        sal_Bool bSplit;
        if( !IsFollow() )   //Nur ein Master entscheidet ueber Orphans
        {
            const WidowsAndOrphans aWidOrp( this );
            bSplit = ( aLine.GetLineNr() >= aWidOrp.GetOrphansLines() &&
                       aLine.GetLineNr() >= aLine.GetDropLines() );
        }
        else
            bSplit = sal_True;

        if( bSplit )
        {
            GetFollow()->SetOfst( aLine.GetEnd() );
            aLine.TruncLines( sal_True );
            if( pPara->IsFollowField() )
                GetFollow()->SetFieldFollow( sal_True );
        }
    }
    if ( bNotify )
    {
        _InvalidateSize();
        InvalidatePage();
    }
}

/*************************************************************************
 *                      SwTxtFrm::Prepare
 *************************************************************************/

sal_Bool lcl_ErgoVadis( SwTxtFrm* pFrm, xub_StrLen &rPos, const PrepareHint ePrep )
{
    const SwFtnInfo &rFtnInfo = pFrm->GetNode()->GetDoc()->GetFtnInfo();
    if( ePrep == PREP_ERGOSUM )
    {
        if( !rFtnInfo.aErgoSum.Len() )
            return sal_False;;
        rPos = pFrm->GetOfst();
    }
    else
    {
        if( !rFtnInfo.aQuoVadis.Len() )
            return sal_False;
        if( pFrm->HasFollow() )
            rPos = pFrm->GetFollow()->GetOfst();
        else
            rPos = pFrm->GetTxt().Len();
        if( rPos )
            --rPos; // unser letztes Zeichen
    }
    return sal_True;
}

void SwTxtFrm::Prepare( const PrepareHint ePrep, const void* pVoid,
                        sal_Bool bNotify )
{
#ifdef DEBUG
    const SwTwips nDbgY = Frm().Top();
#endif

    if ( IsEmpty() )
    {
        switch ( ePrep )
        {
            case PREP_BOSS_CHGD:
            case PREP_WIDOWS_ORPHANS:
            case PREP_WIDOWS:
            case PREP_FTN_GONE :    return;

            case PREP_POS_CHGD :
            {   // Auch in (spaltigen) Bereichen ist ein InvalidateSize notwendig,
                // damit formatiert wird und ggf. das bUndersized gesetzt wird.
                if( IsInFly() || IsInSct() )
                {
                    SwTwips nTmpBottom = GetUpper()->Frm().Top() +
                        GetUpper()->Prt().Bottom();
                    if( nTmpBottom < Frm().Bottom() )
                        break;
                }
                // Gibt es ueberhaupt Flys auf der Seite ?
                SwTxtFly aTxtFly( this );
                if( aTxtFly.IsOn() )
                {
                    // Ueberlappt irgendein Fly ?
                    aTxtFly.Relax();
                    if ( aTxtFly.IsOn() || IsUndersized() )
                        break;
                }
                if( GetTxtNode()->GetSwAttrSet().GetRegister().GetValue())
                    break;
                return;
            }
        }
    }

    if( !HasPara() && PREP_MUST_FIT != ePrep )
    {
        ASSERT( !IsLocked(), "SwTxtFrm::Prepare: three of a perfect pair" );
        if ( bNotify )
            InvalidateSize();
        else
            _InvalidateSize();
        return;
    }

    //Objekt mit Locking aus dem Cache holen.
    SwTxtLineAccess aAccess( this );
    SwParaPortion *pPara = aAccess.GetPara();

    switch( ePrep )
    {
        case PREP_MOVEFTN :     Frm().Height(0);
                                Prt().Height(0);
                                _InvalidatePrt();
                                _InvalidateSize();
                                // KEIN break
        case PREP_ADJUST_FRM :  pPara->SetPrepAdjust( sal_True );
                                if( IsFtnNumFrm() != pPara->IsFtnNum() ||
                                    IsUndersized() )
                                {
                                    InvalidateRange( SwCharRange( 0, 1 ), 1);
                                    if( GetOfst() && !IsFollow() )
                                        _SetOfst( 0 );
                                }
                                break;
        case PREP_MUST_FIT :        pPara->SetPrepMustFit( sal_True );
            /* no break here */
        case PREP_WIDOWS_ORPHANS :  pPara->SetPrepAdjust( sal_True );
                                    break;

        case PREP_WIDOWS :
            // MustFit ist staerker als alles anderes
            if( pPara->IsPrepMustFit() )
                return;
            // Siehe Kommentar in WidowsAndOrphans::FindOrphans und CalcPreps()
            PrepWidows( *(const MSHORT *)pVoid, bNotify );
            break;

        case PREP_FTN :
        {
            SwTxtFtn *pFtn = (SwTxtFtn *)pVoid;
            if( IsInFtn() )
            {
                // Bin ich der erste TxtFrm einer Fussnote ?
                if( !GetPrev() )
                    // Wir sind also ein TxtFrm der Fussnote, die
                    // die Fussnotenzahl zur Anzeige bringen muss.
                    // Oder den ErgoSum-Text...
                    InvalidateRange( SwCharRange( 0, 1 ), 1);

                if( !GetNext() )
                {
                    // Wir sind der letzte Ftn, jetzt muessten die
                    // QuoVadis-Texte geupdated werden.
                    const SwFtnInfo &rFtnInfo = GetNode()->GetDoc()->GetFtnInfo();
                    if( !pPara->UpdateQuoVadis( rFtnInfo.aQuoVadis ) )
                    {
                        xub_StrLen nPos = pPara->GetParLen();
                        if( nPos )
                            --nPos;
                        InvalidateRange( SwCharRange( nPos, 1 ), 1);
                    }
                }
            }
            else
            {
                // Wir sind also der TxtFrm _mit_ der Fussnote
                const xub_StrLen nPos = *pFtn->GetStart();
                InvalidateRange( SwCharRange( nPos, 1 ), 1);
            }
            break;
        }
        case PREP_BOSS_CHGD :
        {
            if( HasFollow() )
            {
                xub_StrLen nNxtOfst = GetFollow()->GetOfst();
                if( nNxtOfst )
                    --nNxtOfst;
                InvalidateRange( SwCharRange( nNxtOfst, 1 ), 1);
            }
            if( IsInFtn() )
            {
                xub_StrLen nPos;
                if( lcl_ErgoVadis( this, nPos, PREP_QUOVADIS ) )
                    InvalidateRange( SwCharRange( nPos, 1 ), 0 );
                if( lcl_ErgoVadis( this, nPos, PREP_ERGOSUM ) )
                    InvalidateRange( SwCharRange( nPos, 1 ), 0 );
            }
            // 4739: Wenn wir ein Seitennummernfeld besitzen, muessen wir
            // die Stellen invalidieren.
            SwpHints *pHints = GetTxtNode()->GetpSwpHints();
            if( pHints )
            {
                const MSHORT nSize = pHints->Count();
                const xub_StrLen nEnd = GetFollow() ?
                                    GetFollow()->GetOfst() : STRING_LEN;
                for( MSHORT i = 0; i < nSize; ++i )
                {
                    const SwTxtAttr *pHt = (*pHints)[i];
                    const xub_StrLen nStart = *pHt->GetStart();
                    if( nStart >= GetOfst() )
                    {
                        if( nStart >= nEnd )
                            i = nSize;          // fuehrt das Ende herbei
                        else
                        {
                // 4029: wenn wir zurueckfliessen und eine Ftn besitzen, so
                // fliesst die Ftn in jedem Fall auch mit. Damit sie nicht im
                // Weg steht, schicken wir uns ein ADJUST_FRM.
                // pVoid != 0 bedeutet MoveBwd()
                            const MSHORT nWhich = pHt->Which();
                            if( RES_TXTATR_FIELD == nWhich ||
                                (HasFtn() && pVoid && RES_TXTATR_FTN == nWhich))
                            InvalidateRange( SwCharRange( nStart, 1 ), 1 );
                        }
                    }
                }
            }
            // A new boss, a new chance for growing
            if( IsUndersized() )
            {
                _InvalidateSize();
                InvalidateRange( SwCharRange( GetOfst(), 1 ), 1);
            }
            break;
        }

        case PREP_POS_CHGD :
        {
            // Falls wir mit niemandem ueberlappen:
            // Ueberlappte irgendein Fly _vor_ der Positionsaenderung ?
            sal_Bool bFormat = pPara->HasFly();
            if( !bFormat )
            {
                if( IsInFly() )
                {
                    SwTwips nTmpBottom = GetUpper()->Frm().Top() +
                        GetUpper()->Prt().Bottom();
                    if( nTmpBottom < Frm().Bottom() )
                        bFormat = sal_True;
                }
                if( !bFormat )
                {
                    if ( GetDrawObjs() )
                    {
                        MSHORT nCnt = GetDrawObjs()->Count();
                        for ( MSHORT i = 0; i < nCnt; ++i )
                        {
                            SdrObject *pO = (*GetDrawObjs())[i];
                            if ( pO->IsWriterFlyFrame() )
                            {
                                SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                                if( pFly->IsAutoPos() )
                                {
                                    bFormat = sal_True;
                                    break;
                                }
                            }
                        }
                    }
                    if( !bFormat )
                    {
                        // Gibt es ueberhaupt Flys auf der Seite ?
                        SwTxtFly aTxtFly( this );
                        if( aTxtFly.IsOn() )
                        {
                            // Ueberlappt irgendein Fly ?
                            aTxtFly.Relax();
                            bFormat = aTxtFly.IsOn() || IsUndersized();
                        }
                    }
                }
            }

            if( bFormat )
            {
                if( !IsLocked() )
                {
                    if( pPara->GetRepaint()->HasArea() )
                        SetCompletePaint();
                    Init();
                    pPara = 0;
                    _InvalidateSize();
                }
            }
            else
            {
                if( GetTxtNode()->GetSwAttrSet().GetRegister().GetValue() )
                    Prepare( PREP_REGISTER, 0, bNotify );
                // Durch Positionsverschiebungen mit Ftns muessen die
                // Frames neu adjustiert werden.
                else if( HasFtn() )
                {
                    Prepare( PREP_ADJUST_FRM, 0, bNotify );
                    _InvalidateSize();
                }
                else
                    return;     // damit kein SetPrep() erfolgt.
            }
            break;
        }
        case PREP_REGISTER:
            if( GetTxtNode()->GetSwAttrSet().GetRegister().GetValue() )
            {
                pPara->SetPrepAdjust( sal_True );
                CalcLineSpace();
                InvalidateSize();
                _InvalidatePrt();
                SwFrm* pNxt;
                if ( 0 != ( pNxt = GetIndNext() ) )
                {
                    pNxt->_InvalidatePrt();
                    if ( pNxt->IsLayoutFrm() )
                        pNxt->InvalidatePage();
                }
                SetCompletePaint();
            }
            break;
        case PREP_FTN_GONE :
            {
                // Wenn ein Follow uns ruft, weil eine Fussnote geloescht wird, muss unsere
                // letzte Zeile formatiert werden, damit ggf. die erste Zeile des Follows
                // hochrutschen kann, die extra auf die naechste Seite gerutscht war, um mit
                // der Fussnote zusammen zu sein, insbesondere bei spaltigen Bereichen.
                ASSERT( GetFollow(), "PREP_FTN_GONE darf nur vom Follow gerufen werden" );
                xub_StrLen nPos = GetFollow()->GetOfst();
                if( IsFollow() && GetOfst() == nPos )       // falls wir gar keine Textmasse besitzen,
                    FindMaster()->Prepare( PREP_FTN_GONE ); // rufen wir das Prepare unseres Masters
                if( nPos )
                    --nPos; // das Zeichen vor unserem Follow
                InvalidateRange( SwCharRange( nPos, 1 ), 0 );
                return;
            }
        case PREP_ERGOSUM:
        case PREP_QUOVADIS:
            {
                xub_StrLen nPos;
                if( lcl_ErgoVadis( this, nPos, ePrep ) )
                    InvalidateRange( SwCharRange( nPos, 1 ), 0 );
            }
            break;
        case PREP_FLY_ATTR_CHG:
        {
            if( pVoid )
            {
                xub_StrLen nWhere = CalcFlyPos( (SwFrmFmt*)pVoid );
                ASSERT( STRING_LEN != nWhere, "Prepare: Why me?" );
                InvalidateRange( SwCharRange( nWhere, 1 ) );
                return;
            }
            // else ... Laufe in den Default-Switch
        }
        case PREP_CLEAR:
        default:
        {
            if( IsLocked() )
            {
                if( PREP_FLY_ARRIVE == ePrep )
                {
                    xub_StrLen nLen = ( GetFollow() ? GetFollow()->GetOfst() :
                                      STRING_LEN ) - GetOfst();
                    InvalidateRange( SwCharRange( GetOfst(), nLen ), 0 );
                }
            }
            else
            {
                if( pPara->GetRepaint()->HasArea() )
                    SetCompletePaint();
                Init();
                pPara = 0;
                if( GetOfst() && !IsFollow() )
                    _SetOfst( 0 );
                if ( bNotify )
                    InvalidateSize();
                else
                    _InvalidateSize();
            }
            return;     // damit kein SetPrep() erfolgt.
        }
    }
    if( pPara )
        pPara->SetPrep( sal_True );
}

/* -----------------11.02.99 17:56-------------------
 * Kleine Hilfsklasse mit folgender Funktion:
 * Sie soll eine Probeformatierung vorbereiten.
 * Der Frame wird in Groesse und Position angepasst, sein SwParaPortion zur Seite
 * gestellt und eine neue erzeugt, dazu wird formatiert mit gesetztem bTestFormat.
 * Im Dtor wird der TxtFrm wieder in seinen alten Zustand zurueckversetzt.
 *
 * --------------------------------------------------*/

class SwTestFormat
{
    SwTxtFrm *pFrm;
    SwParaPortion *pOldPara;
    SwRect aOldFrm, aOldPrt;
public:
    SwTestFormat( SwTxtFrm* pTxtFrm, const SwFrm* pPrv, SwTwips nMaxHeight );
    ~SwTestFormat();
};

SwTestFormat::SwTestFormat( SwTxtFrm* pTxtFrm, const SwFrm* pPre, SwTwips nMaxHeight )
    : pFrm( pTxtFrm )
{
    aOldFrm = pFrm->Frm();
    aOldPrt = pFrm->Prt();
    SwTwips nLower = aOldFrm.Height() - aOldPrt.Height() - aOldPrt.Top();
    pFrm->Frm() = pFrm->GetUpper()->Prt();
    pFrm->Frm() += pFrm->GetUpper()->Frm().Pos();
    if( pFrm->GetPrev() )
        pFrm->Frm().Top( pFrm->GetPrev()->Frm().Bottom() );
    pFrm->Frm().Height( nMaxHeight );
    SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm );
    const SwBorderAttrs &rAttrs = *aAccess.Get();
    pFrm->Prt().Pos().X() = rAttrs.CalcLeft( pFrm );
    if( pPre )
    {
        SwTwips nUpper = pFrm->CalcUpperSpace( &rAttrs, pPre );
        pFrm->Prt().Pos().Y() = nUpper;
    }
    pFrm->Prt().Height( Max( 0L , pFrm->Frm().Height() - pFrm->Prt().Top() - nLower ) );
    pFrm->Prt().Width( pFrm->Frm().Width() - (rAttrs.CalcLeft( pFrm ) + rAttrs.CalcRight()) );
    pOldPara = pFrm->HasPara() ? pFrm->GetPara() : NULL;
    pFrm->SetPara( new SwParaPortion(), sal_False );
    SwTxtFormatInfo aInf( pFrm, sal_False, sal_True, sal_True );
    SwTxtFormatter  aLine( pFrm, &aInf );
    pFrm->_Format( aLine, aInf );
}

SwTestFormat::~SwTestFormat()
{
    pFrm->Frm() = aOldFrm;
    pFrm->Prt() = aOldPrt;
    pFrm->SetPara( pOldPara );
}

sal_Bool SwTxtFrm::TestFormat( const SwFrm* pPrv, SwTwips &rMaxHeight, sal_Bool &bSplit )
{
    PROTOCOL_ENTER( this, PROT_TESTFORMAT, 0, 0 )

    if( IsLocked() && GetUpper()->Prt().Width() <= 0 )
        return sal_False;

    SwTestFormat aSave( this, pPrv, rMaxHeight );

    return SwTxtFrm::WouldFit( rMaxHeight, bSplit );
}


/*************************************************************************
 *                      SwTxtFrm::WouldFit()
 *************************************************************************/

/* SwTxtFrm::WouldFit()
 * sal_True: wenn ich aufspalten kann.
 * Es soll und braucht nicht neu formatiert werden.
 * Wir gehen davon aus, dass bereits formatiert wurde und dass
 * die Formatierungsdaten noch aktuell sind.
 * Wir gehen davon aus, dass die Framebreiten des evtl. Masters und
 * Follows gleich sind. Deswegen wird kein FindBreak() mit FindOrphans()
 * gerufen.
 * Die benoetigte Hoehe wird von nMaxHeight abgezogen!
 */

sal_Bool SwTxtFrm::WouldFit( SwTwips &rMaxHeight, sal_Bool &bSplit )
{
    if( IsLocked() )
        return sal_False;

    //Kann gut sein, dass mir der IdleCollector mir die gecachten
    //Informationen entzogen hat.
    if( !IsEmpty() )
        GetFormatted();

    if ( IsEmpty() )
    {
        bSplit = sal_False;
        SwTwips nHeight = Prt().SSize().Height();
        if( rMaxHeight < nHeight )
            return sal_False;
        else
        {
            rMaxHeight -= nHeight;
            return sal_True;
        }
    }

    // In sehr unguenstigen Faellen kann GetPara immer noch 0 sein.
    // Dann returnen wir sal_True, um auf der neuen Seite noch einmal
    // anformatiert zu werden.
    ASSERT( HasPara() || IsHiddenNow(), "WouldFit: GetFormatted() and then !HasPara()" );
    if( !HasPara() || (!Frm().Height() && IsHiddenNow() ) )
        return sal_True;

    // Da das Orphan-Flag nur sehr fluechtig existiert, wird als zweite
    // Bedingung  ueberprueft, ob die Rahmengroesse durch CalcPreps
    // auf riesengross gesetzt wird, um ein MoveFwd zu erzwingen.
    if( IsWidow() || LONG_MAX - 20000 < Frm().Bottom() )
    {
        SetWidow(sal_False);
        if ( GetFollow() )
        {
            // Wenn wir hier durch eine Widow-Anforderung unseres Follows gelandet
            // sind, wird ueberprueft, ob es ueberhaupt einen Follow mit einer
            // echten Hoehe gibt, andernfalls (z.B. in neu angelegten SctFrms)
            // ignorieren wir das IsWidow() und pruefen doch noch, ob wir
            // genung Platz finden.
            if( LONG_MAX - 20000 >= Frm().Bottom() && !GetFollow()->Frm().Height() )
            {
                SwTxtFrm* pFoll = GetFollow()->GetFollow();
                while( pFoll && !pFoll->Frm().Height() )
                    pFoll = pFoll->GetFollow();
                if( pFoll )
                    return sal_False;
            }
            else
                return sal_False;
        }
    }

    SwTxtSizeInfo aInf( this );
    SwTxtMargin aLine( this, &aInf );

    WidowsAndOrphans aFrmBreak( this, rMaxHeight, bSplit );

    register sal_Bool bRet = sal_True;

    aLine.Bottom();
    // Ist Aufspalten ueberhaupt notwendig?
    if ( 0 != ( bSplit = !aFrmBreak.IsInside( aLine ) ) )
        bRet = !aFrmBreak.IsKeepAlways() && aFrmBreak.WouldFit( aLine, rMaxHeight );
    else
    {
        //Wir brauchen die Gesamthoehe inklusive der aktuellen Zeile
        aLine.Top();
        do
        {
            rMaxHeight -= aLine.GetLineHeight();
        } while ( aLine.Next() );
    }
    return bRet;
}


/*************************************************************************
 *                      SwTxtFrm::GetParHeight()
 *************************************************************************/

KSHORT SwTxtFrm::GetParHeight() const
{
    if( !HasPara() )
    {   // Fuer nichtleere Absaetze ist dies ein Sonderfall, da koennen wir
        // bei UnderSized ruhig nur 1 Twip mehr anfordern.
        KSHORT nRet = Prt().SSize().Height();
        if( IsUndersized() )
        {
            if( IsEmpty() )
                nRet = EmptyHeight();
            else
                ++nRet;
        }
        return nRet;
    }
    SwTxtFrm *pThis = (SwTxtFrm*)this;
    SwTxtSizeInfo aInf( pThis );
    SwTxtIter aLine( pThis, &aInf );
    KSHORT nHeight = aLine.GetLineHeight();
    if( GetOfst() && !IsFollow() )          // Ist dieser Absatz gescrollt? Dann ist unsere
        nHeight += aLine.GetLineHeight();   // bisherige Hoehe mind. eine Zeilenhoehe zu gering
    while( aLine.Next() )
        nHeight += aLine.GetLineHeight();
    return nHeight;
}


/*************************************************************************
 *                      SwTxtFrm::GetFormatted()
 *************************************************************************/

// returnt this _immer_ im formatierten Zustand!
SwTxtFrm *SwTxtFrm::GetFormatted()
{
    //Kann gut sein, dass mir der IdleCollector mir die gecachten
    //Informationen entzogen hat. Calc() ruft unser Format.
                      //Nicht bei leeren Absaetzen!
    if( !HasPara() && !(IsValid() && IsEmpty()) )
    {
        // Calc() muss gerufen werden, weil unsere Frameposition
        // nicht stimmen muss.
        const sal_Bool bFormat = GetValidSizeFlag();
        Calc();
        // Es kann durchaus sein, dass Calc() das Format()
        // nicht anstiess (weil wir einst vom Idle-Zerstoerer
        // aufgefordert wurden unsere Formatinformationen wegzuschmeissen).
        // 6995: Optimierung mit FormatQuick()
        if( bFormat && !FormatQuick() )
            Format();
    }
    return this;
}

/*************************************************************************
 *                      SwTxtFrm::CalcFitToContent()
 *************************************************************************/

KSHORT SwTxtFrm::CalcFitToContent( )
{
    sal_Bool bNoPara = !HasPara();
    if ( bNoPara )
    {
        SwParaPortion *pDummy = new SwParaPortion();
        SetPara( pDummy );
    }

    SwTxtFormatInfo aInf( this );
    aInf.Right( KSHRT_MAX );
    aInf.Width( KSHRT_MAX );
    aInf.RealWidth( KSHRT_MAX );
    aInf.SetIgnoreFly( sal_True );

    SwTxtFormatter  aLine( this, &aInf );
    SwTxtFormatInfo *pInf = &aInf;

    SwHookOut aHook( pInf );
    KSHORT nMax = aLine._CalcFitToContent( );
    if ( nMax )
        nMax -= KSHORT( GetLeftMargin() );

    if ( bNoPara )
        ClearPara( );  // Dummy-Paraportion wieder loeschen
    else
        SetPara( NULL ); // Die Einzeilen-Formatinformation wegwerfen!

    return nMax;
}

/*************************************************************************
 *                      SwTxtFrm::GetLineSpace()
 *************************************************************************/

KSHORT SwTxtFrm::GetLineSpace() const
{
    KSHORT nRet = 0;
    long nTmp;

    const SwAttrSet* pSet = GetAttrSet();
    const SvxLineSpacingItem &rSpace = pSet->GetLineSpacing();

    switch( rSpace.GetInterLineSpaceRule() )
    {
        case SVX_INTER_LINE_SPACE_PROP:
        {
            ViewShell* pVsh = (ViewShell*)GetShell();
            if ( !pVsh )
                break;
            OutputDevice *pOut = pVsh->GetOut();
            if( !pVsh->GetDoc()->IsBrowseMode() ||
                pVsh->GetViewOptions()->IsPrtFormat() )
            {
                OutputDevice* pPrt = GetTxtNode()->GetDoc()->GetPrt();
                if ( pPrt && ((Printer*)pPrt)->IsValid() )
                    pOut = pPrt;
            }
            SwFont aFont( pSet );
            // Wir muessen dafuer sorgen, dass am OutputDevice der Font
            // korrekt restauriert wird, sonst droht ein Last!=Owner.
            if ( pLastFont )
            {
                SwFntObj *pOldFont = pLastFont;
                pLastFont = NULL;
                aFont.SetFntChg( sal_True );
                aFont.ChgPhysFnt( pVsh, pOut );
                nRet = aFont.GetHeight( pVsh, pOut );
                pLastFont->Unlock();
                pLastFont = pOldFont;
                pLastFont->SetDevFont( pVsh, pOut );
            }
            else
            {
                Font aOldFont = pOut->GetFont();
                aFont.SetFntChg( sal_True );
                aFont.ChgPhysFnt( pVsh, pOut );
                nRet = aFont.GetHeight( pVsh, pOut );
                pLastFont->Unlock();
                pLastFont = NULL;
                pOut->SetFont( aOldFont );
            }
            nTmp = nRet;
            nTmp *= rSpace.GetPropLineSpace();
            nTmp /= 100;
            nTmp -= nRet;
            if ( nTmp > 0 )
                nRet = (KSHORT)nTmp;
            else
                nRet = 0;
        }
            break;
        case SVX_INTER_LINE_SPACE_FIX:
            if ( rSpace.GetInterLineSpace() > 0 )
                nRet = (KSHORT)rSpace.GetInterLineSpace();
    }
    return nRet;
}

/*************************************************************************
 *                      SwTxtFrm::FirstLineHeight()
 *************************************************************************/

KSHORT SwTxtFrm::FirstLineHeight() const
{
    if ( !HasPara() )
    {
        if( IsEmpty() && IsValid() )
            return Prt().Height();
        return KSHRT_MAX;
    }
    const SwParaPortion *pPara = GetPara();
    if ( !pPara )
        return KSHRT_MAX;

    return pPara->Height();
}

MSHORT SwTxtFrm::GetLineCount( xub_StrLen nPos )
{
    MSHORT nRet = 0;
    GetFormatted();
    if( HasPara() )
    {
        SwTxtFrm *pFrm = this;
        do
        {
            SwTxtSizeInfo aInf( pFrm );
            SwTxtMargin aLine( pFrm, &aInf );
            if( STRING_LEN == nPos )
                aLine.Bottom();
            else
                aLine.CharToLine( nPos );
            nRet += aLine.GetLineNr();
            pFrm = pFrm->GetFollow();
        } while ( pFrm && pFrm->GetOfst() <= nPos );
    }
    return nRet;
}

void SwTxtFrm::ChgThisLines()
{
    //not necassary to format here (GerFormatted etc.), because we have to come from there!

    ULONG nNew = 0;
    const SwLineNumberInfo &rInf = GetNode()->GetDoc()->GetLineNumberInfo();
    if ( GetTxt().Len() && HasPara() )
    {
        SwTxtSizeInfo aInf( this );
        SwTxtMargin aLine( this, &aInf );
        if ( rInf.IsCountBlankLines() )
        {
            aLine.Bottom();
            nNew = (ULONG)aLine.GetLineNr();
        }
        else
        {
            do
            {
                if( aLine.GetCurr()->HasCntnt() )
                    ++nNew;
            } while ( aLine.NextLine() );
        }
    }
    else if ( rInf.IsCountBlankLines() )
        nNew = 1;

    if ( nNew != nThisLines )
    {
        if ( GetAttrSet()->GetLineNumber().IsCount() )
        {
            nAllLines -= nThisLines;
            nThisLines = nNew;
            nAllLines  += nThisLines;
            SwFrm *pNxt = GetNextCntntFrm();
            while( pNxt && pNxt->IsInTab() )
            {
                if( 0 != (pNxt = pNxt->FindTabFrm()) )
                    pNxt = pNxt->FindNextCnt();
            }
            if( pNxt )
                pNxt->InvalidateLineNum();

            //Extend repaint to the bottom.
            if ( HasPara() )
            {
                SwRepaint *pRepaint = GetPara()->GetRepaint();
                pRepaint->Bottom( Max( pRepaint->Bottom(),
                                       Frm().Top()+Prt().Bottom()));
            }
        }
        else //Paragraphs which are not counted should not manipulate the AllLines.
            nThisLines = nNew;
    }
}


void SwTxtFrm::RecalcAllLines()
{
    ValidateLineNum();

    const SwAttrSet *pAttrSet = GetAttrSet();

    if ( !IsInTab() )
    {
        const ULONG nOld = GetAllLines();
        const SwFmtLineNumber &rLineNum = pAttrSet->GetLineNumber();
        ULONG nNewNum;

        if ( !IsFollow() && rLineNum.GetStartValue() && rLineNum.IsCount() )
            nNewNum = rLineNum.GetStartValue() - 1;
        //If it is a follow or not has not be considered if it is a restart at each page; the
        //restart should also take affekt at follows.
        else if ( pAttrSet->GetDoc()->GetLineNumberInfo().IsRestartEachPage() &&
                  FindPageFrm()->FindFirstBodyCntnt() == this )
        {
            nNewNum = 0;
        }
        else
        {
            SwCntntFrm *pPrv = GetPrevCntntFrm();
            while ( pPrv &&
                    (pPrv->IsInTab() || pPrv->IsInDocBody() != IsInDocBody()) )
                pPrv = pPrv->GetPrevCntntFrm();

            nNewNum = pPrv ? ((SwTxtFrm*)pPrv)->GetAllLines() : 0;
        }
        if ( rLineNum.IsCount() )
            nNewNum += GetThisLines();

        if ( nOld != nNewNum )
        {
            nAllLines = nNewNum;
            SwCntntFrm *pNxt = GetNextCntntFrm();
            while ( pNxt &&
                    (pNxt->IsInTab() || pNxt->IsInDocBody() != IsInDocBody()) )
                pNxt = pNxt->GetNextCntntFrm();
            if ( pNxt )
            {
                if ( pNxt->GetUpper() != GetUpper() )
                    pNxt->InvalidateLineNum();
                else
                    pNxt->_InvalidateLineNum();
            }
        }
    }
}


