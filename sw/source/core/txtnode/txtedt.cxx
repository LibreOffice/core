/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtedt.cxx,v $
 *
 *  $Revision: 1.87 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-02 09:46:50 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

// So kann man die Linguistik-Statistik ( (Tmp-Path)\swlingu.stk ) aktivieren:
//#define LINGU_STATISTIK
#ifdef LINGU_STATISTIK
    #include <stdio.h>          // in SwLinguStatistik::DTOR
    #include <stdlib.h>         // getenv()
    #include <time.h>           // clock()
    #include <tools/stream.hxx>
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen wg. Application
#include <vcl/svapp.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SVX_SPLWRAP_HXX
#include <svx/splwrap.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif
#ifndef SVX_HANGUL_HANJA_CONVERSION_HXX
#include <svx/hangulhanja.hxx>
#endif
#include <SwSmartTagMgr.hxx>
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HDL
#include <com/sun/star/i18n/WordType.hdl>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _DLELSTNR_HXX_
#include <dlelstnr.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SPLARGS_HXX
#include <splargs.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _ACMPLWRD_HXX
#include <acmplwrd.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>      // GetDoc()
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _TXTFLD_HXX
#include <txtfld.hxx>
#endif
#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _WRONG_HXX
#include <wrong.hxx>
#endif
#ifndef _TXTTYPES_HXX
#include <txttypes.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _CRSTATE_HXX
#include <crstate.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _TXATRITR_HXX
#include <txatritr.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>      // SwRedline
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>       // SwRedlineTbl
#endif
#ifndef _SCRIPTINFO_HXX
#include <scriptinfo.hxx>
#endif
#ifndef _DOCSTAT_HXX //autogen
#include <docstat.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _UNOTEXTMARKUP_HXX
#include <unotextmarkup.hxx>
#endif
#ifndef _TXTATR_HXX //autogen
#include <txtatr.hxx>
#endif
#include <fmtautofmt.hxx>
#include <istyleaccess.hxx>

#include <unomid.h>

using rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::smarttags;

// Wir ersparen uns in Hyphenate ein GetFrm()
// Achtung: in edlingu.cxx stehen die Variablen!
extern const SwTxtNode *pLinguNode;
extern       SwTxtFrm  *pLinguFrm;

bool lcl_IsSkippableWhiteSpace( xub_Unicode cCh )
{
    return 0x3000 == cCh ||
           ' ' == cCh ||
           '\t' == cCh ||
           0x0a == cCh;
}

/*
 * This has basically the same function as SwScriptInfo::MaskHiddenRanges,
 * only for deleted redlines
 */

USHORT lcl_MaskRedlines( const SwTxtNode& rNode, XubString& rText,
                         const xub_StrLen nStt, const xub_StrLen nEnd,
                         const xub_Unicode cChar )
{
    USHORT nNumOfMaskedRedlines = 0;

    const SwDoc& rDoc = *rNode.GetDoc();
    USHORT nAct = rDoc.GetRedlinePos( rNode, USHRT_MAX );

    for ( ; nAct < rDoc.GetRedlineTbl().Count(); nAct++ )
    {
        const SwRedline* pRed = rDoc.GetRedlineTbl()[ nAct ];

        if ( pRed->Start()->nNode > rNode.GetIndex() )
            break;

        if( nsRedlineType_t::REDLINE_DELETE == pRed->GetType() )
        {
            xub_StrLen nRedlineEnd;
            xub_StrLen nRedlineStart;

            pRed->CalcStartEnd( rNode.GetIndex(), nRedlineStart, nRedlineEnd );

            if ( nRedlineEnd < nStt || nRedlineStart > nEnd )
                continue;

            while ( nRedlineStart < nRedlineEnd && nRedlineStart < nEnd )
            {
                if ( nRedlineStart >= nStt && nRedlineStart < nEnd )
                {
                    rText.SetChar( nRedlineStart, cChar );
                    ++nNumOfMaskedRedlines;
                }
                ++nRedlineStart;
            }
        }
    }

    return nNumOfMaskedRedlines;
}

/*
 * Used for spell checking. Deleted redlines and hidden characters are masked
 */

USHORT lcl_MaskRedlinesAndHiddenText( const SwTxtNode& rNode, XubString& rText,
                                      const xub_StrLen nStt, const xub_StrLen nEnd,
                                      const xub_Unicode cChar = CH_TXTATR_INWORD,
                                      bool bCheckShowHiddenChar = true )
{
    USHORT nRedlinesMasked = 0;
    USHORT nHiddenCharsMasked = 0;

    const SwDoc& rDoc = *rNode.GetDoc();
    const bool bShowChg = 0 != IDocumentRedlineAccess::IsShowChanges( rDoc.GetRedlineMode() );

    // If called from word count or from spell checking, deleted redlines
    // should be masked:
    if ( bShowChg )
    {
        nRedlinesMasked = lcl_MaskRedlines( rNode, rText, nStt, nEnd, cChar );
    }

    const bool bHideHidden = !SW_MOD()->GetViewOption(rDoc.get(IDocumentSettingAccess::HTML_MODE))->IsShowHiddenChar();

    // If called from word count, we want to mask the hidden ranges even
    // if they are visible:
    if ( !bCheckShowHiddenChar || bHideHidden )
    {
        nHiddenCharsMasked =
            SwScriptInfo::MaskHiddenRanges( rNode, rText, nStt, nEnd, cChar );
    }

    return nRedlinesMasked + nHiddenCharsMasked;
}

/*
 * Used for spell checking. Calculates a rectangle for repaint.
 */

static SwRect lcl_CalculateRepaintRect( SwTxtFrm& rTxtFrm, xub_StrLen nChgStart, xub_StrLen nChgEnd )
{
    SwRect aRect;

    SwTxtNode *pNode = rTxtFrm.GetTxtNode();

    SwNodeIndex aNdIdx( *pNode );
    SwPosition aPos( aNdIdx, SwIndex( pNode, nChgEnd ) );
    SwCrsrMoveState aTmpState( MV_NONE );
    aTmpState.b2Lines = sal_True;
    rTxtFrm.GetCharRect( aRect, aPos, &aTmpState );
    // information about end of repaint area
    Sw2LinesPos* pEnd2Pos = aTmpState.p2Lines;

    const SwTxtFrm *pEndFrm = &rTxtFrm;

    while( pEndFrm->HasFollow() &&
           nChgEnd >= pEndFrm->GetFollow()->GetOfst() )
        pEndFrm = pEndFrm->GetFollow();

    if ( pEnd2Pos )
    {
        // we are inside a special portion, take left border
        SWRECTFN( pEndFrm )
        (aRect.*fnRect->fnSetTop)( (pEnd2Pos->aLine.*fnRect->fnGetTop)() );
        if ( pEndFrm->IsRightToLeft() )
            (aRect.*fnRect->fnSetLeft)( (pEnd2Pos->aPortion.*fnRect->fnGetLeft)() );
        else
            (aRect.*fnRect->fnSetLeft)( (pEnd2Pos->aPortion.*fnRect->fnGetRight)() );
        (aRect.*fnRect->fnSetWidth)( 1 );
        (aRect.*fnRect->fnSetHeight)( (pEnd2Pos->aLine.*fnRect->fnGetHeight)() );
        delete pEnd2Pos;
    }

    aTmpState.p2Lines = NULL;
    SwRect aTmp;
    aPos = SwPosition( aNdIdx, SwIndex( pNode, nChgStart ) );
    rTxtFrm.GetCharRect( aTmp, aPos, &aTmpState );

    // i63141: GetCharRect(..) could cause a formatting,
    // during the formatting SwTxtFrms could be joined, deleted, created...
    // => we have to reinit pStartFrm and pEndFrm after the formatting
    const SwTxtFrm* pStartFrm = &rTxtFrm;
    while( pStartFrm->HasFollow() &&
           nChgStart >= pStartFrm->GetFollow()->GetOfst() )
        pStartFrm = pStartFrm->GetFollow();
    pEndFrm = pStartFrm;
    while( pEndFrm->HasFollow() &&
           nChgEnd >= pEndFrm->GetFollow()->GetOfst() )
        pEndFrm = pEndFrm->GetFollow();

    // information about start of repaint area
    Sw2LinesPos* pSt2Pos = aTmpState.p2Lines;
    if ( pSt2Pos )
    {
        // we are inside a special portion, take right border
        SWRECTFN( pStartFrm )
        (aTmp.*fnRect->fnSetTop)( (pSt2Pos->aLine.*fnRect->fnGetTop)() );
        if ( pStartFrm->IsRightToLeft() )
            (aTmp.*fnRect->fnSetLeft)( (pSt2Pos->aPortion.*fnRect->fnGetRight)() );
        else
            (aTmp.*fnRect->fnSetLeft)( (pSt2Pos->aPortion.*fnRect->fnGetLeft)() );
        (aTmp.*fnRect->fnSetWidth)( 1 );
        (aTmp.*fnRect->fnSetHeight)( (pSt2Pos->aLine.*fnRect->fnGetHeight)() );
        delete pSt2Pos;
    }

    BOOL bSameFrame = TRUE;

    if( rTxtFrm.HasFollow() )
    {
        if( pEndFrm != pStartFrm )
        {
            bSameFrame = FALSE;
            SwRect aStFrm( pStartFrm->PaintArea() );
            {
                SWRECTFN( pStartFrm )
                (aTmp.*fnRect->fnSetLeft)( (aStFrm.*fnRect->fnGetLeft)() );
                (aTmp.*fnRect->fnSetRight)( (aStFrm.*fnRect->fnGetRight)() );
                (aTmp.*fnRect->fnSetBottom)( (aStFrm.*fnRect->fnGetBottom)() );
            }
            aStFrm = pEndFrm->PaintArea();
            {
                SWRECTFN( pEndFrm )
                (aRect.*fnRect->fnSetTop)( (aStFrm.*fnRect->fnGetTop)() );
                (aRect.*fnRect->fnSetLeft)( (aStFrm.*fnRect->fnGetLeft)() );
                (aRect.*fnRect->fnSetRight)( (aStFrm.*fnRect->fnGetRight)() );
            }
            aRect.Union( aTmp );
            while( TRUE )
            {
                pStartFrm = pStartFrm->GetFollow();
                if( pStartFrm == pEndFrm )
                    break;
                aRect.Union( pStartFrm->PaintArea() );
            }
        }
    }
    if( bSameFrame )
    {
        SWRECTFN( pStartFrm )
        if( (aTmp.*fnRect->fnGetTop)() == (aRect.*fnRect->fnGetTop)() )
            (aRect.*fnRect->fnSetLeft)( (aTmp.*fnRect->fnGetLeft)() );
        else
        {
            SwRect aStFrm( pStartFrm->PaintArea() );
            (aRect.*fnRect->fnSetLeft)( (aStFrm.*fnRect->fnGetLeft)() );
            (aRect.*fnRect->fnSetRight)( (aStFrm.*fnRect->fnGetRight)() );
            (aRect.*fnRect->fnSetTop)( (aTmp.*fnRect->fnGetTop)() );
        }

        if( aTmp.Height() > aRect.Height() )
            aRect.Height( aTmp.Height() );
    }

    return aRect;
}

/*
 * Used for automatic styles. Used during RstAttr.
 */

static bool lcl_HaveCommonAttributes( IStyleAccess& rStyleAccess,
                                      const SfxItemSet* pSet1,
                                      USHORT nWhichId,
                                      const SfxItemSet& rSet2,
                                      boost::shared_ptr<SfxItemSet>& pStyleHandle )
{
    bool bRet = false;

    SfxItemSet* pNewSet = 0;

    if ( !pSet1 )
    {
        ASSERT( nWhichId, "lcl_HaveCommonAttributes not used correctly" )
        if ( SFX_ITEM_SET == rSet2.GetItemState( nWhichId, FALSE ) )
        {
            pNewSet = rSet2.Clone( TRUE );
            pNewSet->ClearItem( nWhichId );
        }
    }
    else if ( pSet1->Count() )
    {
        SfxItemIter aIter( *pSet1 );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        while( TRUE )
        {
            if ( SFX_ITEM_SET == rSet2.GetItemState( pItem->Which(), FALSE ) )
            {
                if ( !pNewSet )
                    pNewSet = rSet2.Clone( TRUE );
                pNewSet->ClearItem( pItem->Which() );
            }

            if( aIter.IsAtEnd() )
                break;

            pItem = aIter.NextItem();
        }
    }

    if ( pNewSet )
    {
        if ( pNewSet->Count() )
            pStyleHandle = rStyleAccess.getAutomaticStyle( *pNewSet, IStyleAccess::AUTO_STYLE_CHAR );
        delete pNewSet;
        bRet = true;
    }

    return bRet;
}

/*
 * Ein Zeichen wurde eingefuegt.
 */

SwTxtNode& SwTxtNode::Insert( xub_Unicode c, const SwIndex &rIdx )
{
    xub_StrLen nOrigLen = aText.Len();

    ASSERT( rIdx <= nOrigLen, "Array ueberindiziert." );
    ASSERT( nOrigLen < STRING_LEN, "USHRT_MAX ueberschritten." );

    if( nOrigLen == aText.Insert( c, rIdx.GetIndex() ).Len() )
        return *this;

    Update(rIdx,1);

    // leere Hints und Feldattribute an rIdx.GetIndex suchen
    if( pSwpHints )
    {
        USHORT* pEndIdx;
        for( USHORT i=0; i < pSwpHints->Count() &&
                rIdx >= *(*pSwpHints)[i]->GetStart(); ++i)
        {
            SwTxtAttr *pHt = pSwpHints->GetHt(i);
            if( 0 != ( pEndIdx = pHt->GetEnd()) )
            {
                // leere Hints an rIdx.GetIndex ?
                BOOL bEmpty = *pEndIdx == *pHt->GetStart()
                            && rIdx == *pHt->GetStart();

                if( bEmpty )
                {
                    pSwpHints->DeleteAtPos(i);
                    if( bEmpty )
                        *pHt->GetStart() -= 1;
                    else
                        *pEndIdx -= 1;
                    Insert(pHt);
                }
            }
        }
        if ( pSwpHints->CanBeDeleted() )
            DELETEZ( pSwpHints );
    }
    // den Frames Bescheid sagen
    SwInsChr aHint( rIdx.GetIndex()-1 );
    SwModify::Modify( 0, &aHint );
    return *this;
}

inline BOOL InRange(xub_StrLen nIdx, xub_StrLen nStart, xub_StrLen nEnd) {
    return ((nIdx >=nStart) && (nIdx <= nEnd));
}

/*
 * void SwTxtNode::RstAttr(const SwIndex &rIdx, USHORT nLen)
 *
 * loescht alle Attribute ab der Position rIdx ueber eine Laenge
 * von nLen.
 */

/* 5 Faelle:
 * 1) Das Attribut liegt vollstaendig im Bereich:
 *    -> loeschen
 * 2) Das Attributende liegt im Bereich:
 *    -> Loeschen, mit neuem Ende einfuegen
 * 3) Der Attributanfang liegt im Bereich:
 *    -> Loeschen, mit neuem Anfang einfuegen
 * 4) Das Attrib umfasst den Bereich:
 *       Aufsplitten, d.h.
 *    -> Loeschen, mit alten Anfang und Anfang des Bereiches einfuegen
 *    -> Neues Attribut mit Ende des Bereiches und altem Ende einfuegen
 * 5) Das Attribut liegt ausserhalb des Bereiches
 *     -> nichts tun.
 */

void SwTxtNode::RstAttr(const SwIndex &rIdx, xub_StrLen nLen, USHORT nWhich,
                        const SfxItemSet* pSet, BOOL bInclRefToxMark )
{
    // Attribute?
    if ( !GetpSwpHints() )
        return;

    USHORT i = 0;
    xub_StrLen nStt = rIdx.GetIndex();
    xub_StrLen nEnd = nStt + nLen;
    xub_StrLen *pAttrEnd;
    xub_StrLen nAttrStart;
    SwTxtAttr *pHt;

    BOOL    bChanged = FALSE;

    // nMin und nMax werden invers auf das Maximum bzw. Minimum gesetzt.
    xub_StrLen nMin = aText.Len();
    xub_StrLen nMax = nStt;

    const BOOL bNoLen = !nMin;

    // We have to remember the "new" attributes, which have
    // been introduced by splitting surrounding attributes (case 4).
    // They may not be forgotten inside the "Forget" function
    //std::vector< const SwTxtAttr* > aNewAttributes;

    // durch das Attribute-Array, bis der Anfang des Geltungsbereiches
    // des Attributs hinter dem Bereich liegt
    while( (i < pSwpHints->Count()) &&
                ((( nAttrStart = *(*pSwpHints)[i]->GetStart()) < nEnd )
                    || nLen==0) )
    {
        pHt = pSwpHints->GetHt(i);

        // Attribute ohne Ende bleiben drin!
        if ( 0 == (pAttrEnd=pHt->GetEnd()) )
        {
            i++;
            continue;
        }

        // Default behavior is to process all attributes:
        bool bSkipAttr = false;;
        boost::shared_ptr<SfxItemSet> pStyleHandle;

        // 1. case: We want to reset only the attributes listed in pSet:
        if ( pSet )
        {
            bSkipAttr = SFX_ITEM_SET != pSet->GetItemState( pHt->Which(), FALSE );
            if ( bSkipAttr && RES_TXTATR_AUTOFMT == pHt->Which() )
            {
                // if the current attribute is an autostyle, we have to check if the autostyle
                // and pSet have any attributes in common. If so, pStyleHandle will contain
                // a handle to AutoStyle / pSet:
                bSkipAttr = !lcl_HaveCommonAttributes( getIDocumentStyleAccess(), pSet, 0, *static_cast<const SwFmtAutoFmt&>(pHt->GetAttr()).GetStyleHandle(), pStyleHandle );
            }
        }
        else if ( nWhich )
        {
            // 2. case: We want to reset only the attributes with WhichId nWhich:
            bSkipAttr = nWhich != pHt->Which();
            if ( bSkipAttr && RES_TXTATR_AUTOFMT == pHt->Which() )
            {
                bSkipAttr = !lcl_HaveCommonAttributes( getIDocumentStyleAccess(), 0, nWhich, *static_cast<const SwFmtAutoFmt&>(pHt->GetAttr()).GetStyleHandle(), pStyleHandle );
            }
        }
        else if ( !bInclRefToxMark )
        {
            // 3. case: Reset all attributes except from ref/toxmarks:
            bSkipAttr = RES_TXTATR_REFMARK == pHt->Which() ||
                        RES_TXTATR_TOXMARK == pHt->Which();
        }

        if( bSkipAttr )

        {
            // Es sollen nur Attribute mit nWhich beachtet werden
            i++;
            continue;
        }


        if( nStt <= nAttrStart )          // Faelle: 1,3,5
        {
            if( nEnd > nAttrStart
                || ( nEnd == *pAttrEnd && nEnd==nAttrStart ) )
            {
                // Faelle: 1,3
                if ( nMin > nAttrStart )
                    nMin = nAttrStart;
                if ( nMax < *pAttrEnd )
                    nMax = *pAttrEnd;
                // Falls wir nur ein nichtaufgespanntes Attribut entfernen,
                // tun wir mal so, als ob sich nichts geaendert hat.
                bChanged = bChanged || nEnd > nAttrStart || bNoLen;
                if( *pAttrEnd <= nEnd )     // Fall: 1
                {
                    const xub_StrLen nAttrEnd = *pAttrEnd;

                    pSwpHints->DeleteAtPos(i);
                    DestroyAttr( pHt );

                    if ( pStyleHandle.get() )
                    {
                        SwTxtAttr* pNew = MakeTxtAttr( *pStyleHandle, nAttrStart, nAttrEnd );
                        Insert( pNew, nsSetAttrMode::SETATTR_NOHINTADJUST );
                    }

                    // falls das letzte Attribut ein Field ist, loescht
                    // dieses das HintsArray !!!
                    if( !pSwpHints )
                        break;

                    //JP 26.11.96:
                    // beim DeleteAtPos wird ein Resort ausgefuehrt!!
                    // darum muessen wir wieder bei 0 anfangen!!!
                    // ueber den Fall 3 koennen Attribute nach hinten
                    // verschoben worden sein; damit stimmt jetzt das i
                    // nicht mehr!!!
                    i = 0;

                    continue;
                }
                else                        // Fall: 3
                {
                    pSwpHints->NoteInHistory( pHt );
                    *pHt->GetStart() = nEnd;
                    pSwpHints->NoteInHistory( pHt, TRUE );

                    if ( pStyleHandle.get() && nAttrStart < nEnd )
                    {
                        SwTxtAttr* pNew = MakeTxtAttr( *pStyleHandle, nAttrStart, nEnd );
                        Insert( pNew, nsSetAttrMode::SETATTR_NOHINTADJUST );
                    }

                    bChanged = TRUE;
                }
            }
        }
        else                                // Faelle: 2,4,5
            if( *pAttrEnd > nStt )     // Faelle: 2,4
            {
                if( *pAttrEnd < nEnd )      // Fall: 2
                {
                    if ( nMin > nAttrStart )
                        nMin = nAttrStart;
                    if ( nMax < *pAttrEnd )
                        nMax = *pAttrEnd;
                    bChanged = TRUE;

                    const xub_StrLen nAttrEnd = *pAttrEnd;

                    pSwpHints->NoteInHistory( pHt );
                    *pAttrEnd = nStt;
                    pSwpHints->NoteInHistory( pHt, TRUE );

                    if ( pStyleHandle.get() )
                    {
                        SwTxtAttr* pNew = MakeTxtAttr( *pStyleHandle, static_cast< xub_StrLen >(nStart), nAttrEnd );
                        Insert( pNew, nsSetAttrMode::SETATTR_NOHINTADJUST );
                    }
                }
                else if( nLen )             // Fall: 4
                {       // bei Lange 0 werden beide Hints vom Insert(Ht)
                        // wieder zu einem zusammengezogen !!!!
                    if ( nMin > nAttrStart )
                        nMin = nAttrStart;
                    if ( nMax < *pAttrEnd )
                        nMax = *pAttrEnd;
                    bChanged = TRUE;
                    xub_StrLen nTmpEnd = *pAttrEnd;
                    pSwpHints->NoteInHistory( pHt );
                    *pAttrEnd = nStt;
                    pSwpHints->NoteInHistory( pHt, TRUE );

                    if ( pStyleHandle.get() && nStart < nEnd )
                    {
                        SwTxtAttr* pNew = MakeTxtAttr( *pStyleHandle, static_cast< xub_StrLen >(nStart), nEnd );
                        Insert( pNew, nsSetAttrMode::SETATTR_NOHINTADJUST );
                    }

                    if( nEnd < nTmpEnd )
                    {
                        SwTxtAttr* pNew = MakeTxtAttr( pHt->GetAttr(), nEnd, nTmpEnd );
                        if ( pNew )
                        {
                            SwTxtCharFmt* pCharFmt = dynamic_cast<SwTxtCharFmt*>(pHt);
                            if ( pCharFmt )
                                static_cast<SwTxtCharFmt*>(pNew)->SetSortNumber( pCharFmt->GetSortNumber() );

                            Insert( pNew, nsSetAttrMode::SETATTR_NOHINTADJUST );
                        }


                        // jetzt kein i+1, weil das eingefuegte Attribut
                        // ein anderes auf die Position geschoben hat !
                        continue;
                    }
                }
            }
        ++i;
    }

    if ( pSwpHints && pSwpHints->CanBeDeleted() )
        DELETEZ( pSwpHints );
    if(bChanged)
    {
        if ( pSwpHints )
        {
            ((SwpHintsArr*)pSwpHints)->Resort();
        }
        //TxtFrm's reagieren auf aHint, andere auf aNew
        SwUpdateAttr aHint( nMin, nMax, 0 );
        SwModify::Modify( 0, &aHint );
        SwFmtChg aNew( GetFmtColl() );
        SwModify::Modify( 0, &aNew );
    }
}



/*************************************************************************
 *                SwTxtNode::GetCurWord()
 *
 * Aktuelles Wort zurueckliefern:
 * Wir suchen immer von links nach rechts, es wird also das Wort
 * vor nPos gesucht. Es sei denn, wir befinden uns am Anfang des
 * Absatzes, dann wird das erste Wort zurueckgeliefert.
 * Wenn dieses erste Wort nur aus Whitespaces besteht, returnen wir
 * einen leeren String.
 *************************************************************************/



XubString SwTxtNode::GetCurWord( xub_StrLen nPos ) const
{
    ASSERT( nPos<=aText.Len() , "SwTxtNode::GetCurWord: Pos hinter String?");
    if (!aText.Len())
        return aText;

    Boundary aBndry;
    const uno::Reference< XBreakIterator > &rxBreak = pBreakIt->xBreak;
    if (rxBreak.is())
    {
        sal_Int16 nWordType = WordType::DICTIONARY_WORD;
        lang::Locale aLocale( pBreakIt->GetLocale( GetLang( nPos ) ) );
#ifdef DEBUG
        BOOL bBegin = rxBreak->isBeginWord( aText, nPos, aLocale, nWordType );
        BOOL bEnd   = rxBreak->isEndWord  ( aText, nPos, aLocale, nWordType );
        (void)bBegin;
        (void)bEnd;
#endif
        aBndry = rxBreak->getWordBoundary( aText, nPos, aLocale, nWordType, TRUE );

        // if no word was found use previous word (if any)
        if (aBndry.startPos == aBndry.endPos)
            aBndry = rxBreak->previousWord( aText, nPos, aLocale, nWordType );
    }

    // check if word was found and if it uses a symbol font, if so
    // enforce returning an empty string
    if (aBndry.endPos != aBndry.startPos && IsSymbol( (xub_StrLen)aBndry.startPos ))
        aBndry.endPos = aBndry.startPos;

    return aText.Copy( static_cast<xub_StrLen>(aBndry.startPos),
                       static_cast<xub_StrLen>(aBndry.endPos - aBndry.startPos) );
}

SwScanner::SwScanner( const SwTxtNode& rNd, const String& rTxt, const LanguageType* pLang,
                      const ModelToViewHelper::ConversionMap* pConvMap,
                      USHORT nType, xub_StrLen nStart, xub_StrLen nEnde, BOOL bClp )
    : rNode( rNd ), rText( rTxt), pLanguage( pLang ), pConversionMap( pConvMap ), nLen( 0 ), nWordType( nType ), bClip( bClp )
{
    ASSERT( rText.Len(), "SwScanner: EmptyString" );
    nStartPos = nBegin = nStart;
    nEndPos = nEnde;

    if ( pLanguage )
    {
        aCurrLang = *pLanguage;
    }
    else
    {
        ModelToViewHelper::ModelPosition aModelBeginPos = ModelToViewHelper::ConvertToModelPosition( pConversionMap, nBegin );
        const xub_StrLen nModelBeginPos = (xub_StrLen)aModelBeginPos.mnPos;
        aCurrLang = rNd.GetLang( nModelBeginPos );
    }
}

BOOL SwScanner::NextWord()
{
    nBegin = nBegin + nLen;
    Boundary aBound;

    CharClass& rCC = GetAppCharClass();
    lang::Locale aOldLocale = rCC.getLocale();

    while ( true )
    {
        // skip non-letter characters:
        while ( nBegin < rText.Len() )
        {
            if ( !lcl_IsSkippableWhiteSpace( rText.GetChar( nBegin ) ) )
            {
                if ( !pLanguage )
                {
                    const USHORT nNextScriptType = pBreakIt->xBreak->getScriptType( rText, nBegin );
                    ModelToViewHelper::ModelPosition aModelBeginPos = ModelToViewHelper::ConvertToModelPosition( pConversionMap, nBegin );
                    const xub_StrLen nBeginModelPos = (xub_StrLen)aModelBeginPos.mnPos;
                    aCurrLang = rNode.GetLang( nBeginModelPos, 1, nNextScriptType );
                }

                if ( nWordType != i18n::WordType::WORD_COUNT )
                {
                    rCC.setLocale( pBreakIt->GetLocale( aCurrLang ) );
                    if ( rCC.isLetterNumeric( rText.GetChar( nBegin ) ) )
                        break;
                }
                else
                    break;
            }
            ++nBegin;
        }

        if ( nBegin >= rText.Len() || nBegin >= nEndPos )
            return FALSE;

        // get the word boundaries
        aBound = pBreakIt->xBreak->getWordBoundary( rText, nBegin,
                pBreakIt->GetLocale( aCurrLang ), nWordType, sal_True );

        //no word boundaries could be found
        if(aBound.endPos == aBound.startPos)
            return FALSE;

        //if a word before is found it has to be searched for the next
        if(aBound.endPos == nBegin)
            ++nBegin;
        else
            break;
    } // end while( true )

    rCC.setLocale( aOldLocale );

    // we have to differenciate between these cases:
    if ( aBound.startPos <= nBegin )
    {
        ASSERT( aBound.endPos >= nBegin, "Unexpected aBound result" )

        // restrict boundaries to script boundaries and nEndPos
        const USHORT nCurrScript =
                pBreakIt->xBreak->getScriptType( rText, nBegin );

        XubString aTmpWord = rText.Copy( nBegin, static_cast<xub_StrLen>(aBound.endPos - nBegin) );
        const sal_Int32 nScriptEnd = nBegin +
            pBreakIt->xBreak->endOfScript( aTmpWord, 0, nCurrScript );
        const sal_Int32 nEnd = Min( aBound.endPos, nScriptEnd );

        // restrict word start to last script change position
        sal_Int32 nScriptBegin = 0;
        if ( aBound.startPos < nBegin )
        {
            // search from nBegin backwards until the next script change
            aTmpWord = rText.Copy( static_cast<xub_StrLen>(aBound.startPos),
                                   static_cast<xub_StrLen>(nBegin - aBound.startPos + 1) );
            nScriptBegin = aBound.startPos +
                pBreakIt->xBreak->beginOfScript( aTmpWord, nBegin - aBound.startPos,
                                                nCurrScript );
        }

        nBegin = (xub_StrLen)Max( aBound.startPos, nScriptBegin );
        nLen = (xub_StrLen)(nEnd - nBegin);
    }
    else
    {
        const USHORT nCurrScript =
                pBreakIt->xBreak->getScriptType( rText, aBound.startPos );
        XubString aTmpWord = rText.Copy( static_cast<xub_StrLen>(aBound.startPos),
                                         static_cast<xub_StrLen>(aBound.endPos - aBound.startPos) );
        const sal_Int32 nScriptEnd = aBound.startPos +
            pBreakIt->xBreak->endOfScript( aTmpWord, 0, nCurrScript );
        const sal_Int32 nEnd = Min( aBound.endPos, nScriptEnd );
        nBegin = (xub_StrLen)aBound.startPos;
        nLen = (xub_StrLen)(nEnd - nBegin);
    }

    // optionally clip the result of getWordBoundaries:
    if ( bClip )
    {
        aBound.startPos = Max( (xub_StrLen)aBound.startPos, nStartPos );
        aBound.endPos = Min( (xub_StrLen)aBound.endPos, nEndPos );
        nBegin = (xub_StrLen)aBound.startPos;
        nLen = (xub_StrLen)(aBound.endPos - nBegin);
    }

    if( ! nLen )
        return FALSE;

    aWord = rText.Copy( nBegin, nLen );

    return TRUE;
}


USHORT SwTxtNode::Spell(SwSpellArgs* pArgs)
{
    // Die Aehnlichkeiten zu SwTxtFrm::_AutoSpell sind beabsichtigt ...
    // ACHTUNG: Ev. Bugs in beiden Routinen fixen!

    uno::Reference<beans::XPropertySet> xProp( GetLinguPropertySet() );

    xub_StrLen nBegin, nEnd;

    // modify string according to redline information and hidden text
    const XubString aOldTxt( aText );
    const bool bRestoreString =
            lcl_MaskRedlinesAndHiddenText( *this, aText, 0, aText.Len() ) > 0;

    if ( pArgs->pStartNode != this )
        nBegin = 0;
    else
        nBegin = pArgs->pStartIdx->GetIndex();

    if ( pArgs->pEndNode != this )
        nEnd = aText.Len();
    else
        nEnd = pArgs->pEndIdx->GetIndex();

    pArgs->xSpellAlt = NULL;

    // 4 cases:
    //
    // 1. IsWrongDirty = 0 and GetWrong = 0
    //      Everything is checked and correct
    // 2. IsWrongDirty = 0 and GetWrong = 1
    //      Everything is checked and errors are identified in the wrong list
    // 3. IsWrongDirty = 1 and GetWrong = 0
    //      Nothing has been checked
    // 4. IsWrongDirty = 1 and GetWrong = 1
    //      Text has been checked but there is an invalid range in the wrong list
    //
    // Nothing has to be done for case 1.
    if( ( IsWrongDirty() || GetWrong() ) && aText.Len() )
    {
        if( nBegin > aText.Len() )
            nBegin = aText.Len();
        if( nEnd > aText.Len() )
            nEnd = aText.Len();
        //
        if(!IsWrongDirty())
        {
            xub_StrLen nTemp = GetWrong()->NextWrong( nBegin );
            if(nTemp > nEnd)
            {
                // reset original text
                if ( bRestoreString )
                    aText = aOldTxt;
                return 0;
            }
            if(nTemp > nBegin)
                nBegin = nTemp;

        }

        // In case 2. we pass the wrong list to the scanned, because only
        // the words in the wrong list have to be checked
        SwScanner aScanner( *this, aText, 0, 0,
                            WordType::DICTIONARY_WORD,
                            nBegin, nEnd );
        while( !pArgs->xSpellAlt.is() && aScanner.NextWord() )
        {
            const XubString& rWord = aScanner.GetWord();

            // get next language for next word, consider language attributes
            // within the word
            LanguageType eActLang = aScanner.GetCurrentLanguage();

            if( rWord.Len() > 0 && LANGUAGE_NONE != eActLang )
            {
                if (pArgs->xSpeller.is())
                {
                    SvxSpellWrapper::CheckSpellLang( pArgs->xSpeller, eActLang );
                    pArgs->xSpellAlt = pArgs->xSpeller->spell( rWord, eActLang,
                                            Sequence< PropertyValue >() );
                }
                if( (pArgs->xSpellAlt).is() )
                {
                    if( IsSymbol( aScanner.GetBegin() ) )
                    {
                        pArgs->xSpellAlt = NULL;
                    }
                    else
                    {
                        // make sure the selection build later from the
                        // data below does not include footnotes and other
                        // "in word" character to the left and right in order
                        // to preserve those. Therefore count those "in words"
                        // in order to modify the selection accordingly.
                        const sal_Unicode* pChar = rWord.GetBuffer();
                        xub_StrLen nLeft = 0;
                        while (pChar && *pChar++ == CH_TXTATR_INWORD)
                            ++nLeft;
                        pChar = rWord.Len() ? rWord.GetBuffer() + rWord.Len() - 1 : 0;
                        xub_StrLen nRight = 0;
                        while (pChar && *pChar-- == CH_TXTATR_INWORD)
                            ++nRight;

                        pArgs->pStartNode = this;
                        pArgs->pEndNode = this;
                        pArgs->pStartIdx->Assign(this, aScanner.GetEnd() - nRight );
                        pArgs->pEndIdx->Assign(this, aScanner.GetBegin() + nLeft );
                    }
                }
            }
        }
    }

    // reset original text
    if ( bRestoreString )
        aText = aOldTxt;

    return pArgs->xSpellAlt.is() ? 1 : 0;
}


void SwTxtNode::SetLanguageAndFont( const SwPaM &rPaM,
    LanguageType nLang, USHORT nLangWhichId,
    const Font *pFont,  USHORT nFontWhichId )
{
    sal_uInt16 aRanges[] = {
            nLangWhichId, nLangWhichId,
            nFontWhichId, nFontWhichId,
            0, 0, 0 };
    if (!pFont)
        aRanges[2] = aRanges[3] = 0;    // clear entries with font WhichId

    SwEditShell *pEditShell = GetDoc()->GetEditShell();
    SfxItemSet aSet( pEditShell->GetAttrPool(), aRanges );
    aSet.Put( SvxLanguageItem( nLang, nLangWhichId ) );

    DBG_ASSERT( pFont, "target font missing?" );
    if (pFont)
    {
        SvxFontItem aFontItem = (SvxFontItem&) aSet.Get( nFontWhichId );
        aFontItem.GetFamilyName()   = pFont->GetName();
        aFontItem.GetFamily()       = pFont->GetFamily();
        aFontItem.GetStyleName()    = pFont->GetStyleName();
        aFontItem.GetPitch()        = pFont->GetPitch();
        aFontItem.GetCharSet()      = pFont->GetCharSet();
        aSet.Put( aFontItem );
    }

    GetDoc()->Insert( rPaM, aSet, 0 );
    // SetAttr( aSet );    <- Does not set language attribute of empty paragraphs correctly,
    //                     <- because since there is no selection the flag to garbage
    //                     <- collect all attributes is set, and therefore attributes spanned
    //                     <- over empty selection are removed.

}


USHORT SwTxtNode::Convert( SwConversionArgs &rArgs )
{
    // get range of text within node to be converted
    // (either all the text or the the text within the selection
    // when the conversion was started)
    xub_StrLen nTextBegin, nTextEnd;
    //
    if ( rArgs.pStartNode != this )
    {
        nTextBegin = 0;
    }
    else
        nTextBegin = rArgs.pStartIdx->GetIndex();
    if (nTextBegin > aText.Len())
        nTextBegin = aText.Len();
    //
    if ( rArgs.pEndNode != this )
        nTextEnd = aText.Len();
    else
        nTextEnd = rArgs.pEndIdx->GetIndex();
    if (nTextEnd > aText.Len())
        nTextEnd = aText.Len();

    rArgs.aConvText = rtl::OUString();

    // modify string according to redline information and hidden text
    const XubString aOldTxt( aText );
    const bool bRestoreString =
            lcl_MaskRedlinesAndHiddenText( *this, aText, 0, aText.Len() ) > 0;

    sal_Bool    bFound  = sal_False;
    xub_StrLen  nBegin  = nTextBegin;
    xub_StrLen  nLen = 0;
    LanguageType nLangFound = LANGUAGE_NONE;
    if (!aText.Len())
    {
        if (rArgs.bAllowImplicitChangesForNotConvertibleText)
        {
            // create SwPaM with mark & point spanning empty paragraph
            //SwPaM aCurPaM( *this, *this, nBegin, nBegin + nLen ); <-- wrong c-tor, does sth different
            SwPaM aCurPaM( *this, 0 );

            SetLanguageAndFont( aCurPaM,
                    rArgs.nConvTargetLang, RES_CHRATR_CJK_LANGUAGE,
                    rArgs.pTargetFont, RES_CHRATR_CJK_FONT );
        }
    }
    else
    {
        SwLanguageIterator aIter( *this, nBegin );

        // find non zero length text portion of appropriate language
        do {
            nLangFound = aIter.GetLanguage();
            sal_Bool bLangOk =  (nLangFound == rArgs.nConvSrcLang) ||
                                (svx::HangulHanjaConversion::IsChinese( nLangFound ) &&
                                 svx::HangulHanjaConversion::IsChinese( rArgs.nConvSrcLang ));

            xub_StrLen nChPos = aIter.GetChgPos();
            // the position at the end of the paragraph returns -1
            // which becomes 65535 when converted to xub_StrLen,
            // and thus must be cut to the end of the actual string.
            if (nChPos == (xub_StrLen) -1)
                nChPos = aText.Len();

            nLen = nChPos - nBegin;
            bFound = bLangOk && nLen > 0;
            if (!bFound)
            {
                // create SwPaM with mark & point spanning the attributed text
                //SwPaM aCurPaM( *this, *this, nBegin, nBegin + nLen ); <-- wrong c-tor, does sth different
                SwPaM aCurPaM( *this, nBegin );
                aCurPaM.SetMark();
                aCurPaM.GetPoint()->nContent = nBegin + nLen;

                // check script type of selected text
                SwEditShell *pEditShell = GetDoc()->GetEditShell();
                pEditShell->Push();             // save current cursor on stack
                pEditShell->SetSelection( aCurPaM );
                sal_Bool bIsAsianScript = (SCRIPTTYPE_ASIAN == pEditShell->GetScriptType());
                pEditShell->Pop( sal_False );   // restore cursor from stack

                if (!bIsAsianScript && rArgs.bAllowImplicitChangesForNotConvertibleText)
                {
                    SetLanguageAndFont( aCurPaM,
                            rArgs.nConvTargetLang, RES_CHRATR_CJK_LANGUAGE,
                            rArgs.pTargetFont, RES_CHRATR_CJK_FONT );
                }
                nBegin = nChPos;    // start of next language portion
            }
        } while (!bFound && aIter.Next());  /* loop while nothing was found and still sth is left to be searched */
    }

    // keep resulting text within selection / range of text to be converted
    if (nBegin < nTextBegin)
        nBegin = nTextBegin;
    if (nBegin + nLen > nTextEnd)
        nLen = nTextEnd - nBegin;
    sal_Bool bInSelection = nBegin < nTextEnd;

    if (bFound && bInSelection)     // convertible text found within selection/range?
    {
        const XubString aTxtPortion = aText.Copy( nBegin, nLen );
        DBG_ASSERT( aText.Len() > 0, "convertible text portion missing!" );
        rArgs.aConvText     = aText.Copy( nBegin, nLen );
        rArgs.nConvTextLang = nLangFound;

        // position where to start looking in next iteration (after current ends)
        rArgs.pStartNode = this;
        rArgs.pStartIdx->Assign(this, nBegin + nLen );
        // end position (when we have travelled over the whole document)
        rArgs.pEndNode = this;
        rArgs.pEndIdx->Assign(this, nBegin );
    }

    // restore original text
    if ( bRestoreString )
        aText = aOldTxt;

    return rArgs.aConvText.getLength() ? 1 : 0;
}

// Die Aehnlichkeiten zu SwTxtNode::Spell sind beabsichtigt ...
// ACHTUNG: Ev. Bugs in beiden Routinen fixen!
SwRect SwTxtFrm::_AutoSpell( const SwCntntNode* pActNode, const SwViewOption& rViewOpt, xub_StrLen nActPos )
{
    SwRect aRect;
#if OSL_DEBUG_LEVEL > 1
    static BOOL bStop = FALSE;
    if ( bStop )
        return aRect;
#endif
    // Die Aehnlichkeiten zu SwTxtNode::Spell sind beabsichtigt ...
    // ACHTUNG: Ev. Bugs in beiden Routinen fixen!
    SwTxtNode *pNode = GetTxtNode();
    if( pNode != pActNode || !nActPos )
        nActPos = STRING_LEN;

    SwAutoCompleteWord& rACW = SwDoc::GetAutoCompleteWords();

    // modify string according to redline information and hidden text
    const XubString aOldTxt( pNode->aText );
    const bool bRestoreString =
            lcl_MaskRedlinesAndHiddenText( *pNode, pNode->aText, 0, pNode->aText.Len() ) > 0;

    // a change of data indicates that at least one word has been modified
    const sal_Bool bRedlineChg = ( pNode->aText.GetBuffer() != aOldTxt.GetBuffer() );

    xub_StrLen nBegin = 0;
    xub_StrLen nEnd = pNode->aText.Len();
    xub_StrLen nInsertPos = 0;
    xub_StrLen nChgStart = STRING_LEN;
    xub_StrLen nChgEnd = 0;
    xub_StrLen nInvStart = STRING_LEN;
    xub_StrLen nInvEnd = 0;

    const sal_Bool bAddAutoCmpl = pNode->IsAutoCompleteWordDirty() &&
                                  rViewOpt.IsAutoCompleteWords();

    if( pNode->GetWrong() )
    {
        nBegin = pNode->GetWrong()->GetBeginInv();
        if( STRING_LEN != nBegin )
        {
            nEnd = pNode->GetWrong()->GetEndInv();
            if ( nEnd > pNode->aText.Len() )
                nEnd = pNode->aText.Len();
        }

        // get word around nBegin, we start at nBegin - 1
        if ( STRING_LEN != nBegin )
        {
            if ( nBegin )
                --nBegin;

            LanguageType eActLang = pNode->GetLang( nBegin );
            Boundary aBound = pBreakIt->xBreak->getWordBoundary( pNode->aText, nBegin,
                            pBreakIt->GetLocale( eActLang ), WordType::DICTIONARY_WORD, TRUE );
            nBegin = xub_StrLen(aBound.startPos);
        }

        // get the position in the wrong list
        nInsertPos = pNode->GetWrong()->GetWrongPos( nBegin );

        // sometimes we have to skip one entry
        if( nInsertPos < pNode->GetWrong()->Count() &&
            nBegin == pNode->GetWrong()->Pos( nInsertPos ) +
                      pNode->GetWrong()->Len( nInsertPos ) )
                nInsertPos++;
    }

    BOOL bFresh = nBegin < nEnd;

    if( nBegin < nEnd )
    {
        //! register listener to LinguServiceEvents now in order to get
        //! notified about relevant changes in the future
        SwModule *pModule = SW_MOD();
        if (!pModule->GetLngSvcEvtListener().is())
            pModule->CreateLngSvcEvtListener();

        uno::Reference< XSpellChecker1 > xSpell( ::GetSpellChecker() );
        SwDoc* pDoc = pNode->GetDoc();

        SwScanner aScanner( *pNode, pNode->aText, 0, 0, WordType::DICTIONARY_WORD,
                            nBegin, nEnd);

        while( aScanner.NextWord() )
        {
            const XubString& rWord = aScanner.GetWord();
            nBegin = aScanner.GetBegin();
            xub_StrLen nLen = aScanner.GetLen();

            // get next language for next word, consider language attributes
            // within the word
            LanguageType eActLang = aScanner.GetCurrentLanguage();

            BOOL bSpell = TRUE;
            bSpell = xSpell.is() ? xSpell->hasLanguage( eActLang ) : FALSE;
            if( bSpell && rWord.Len() > 0 )
            {
                // check for: bAlter => xHyphWord.is()
                DBG_ASSERT(!bSpell || xSpell.is(), "NULL pointer");

                if( !xSpell->isValid( rWord, eActLang, Sequence< PropertyValue >() ) )
                {
                    xub_StrLen nSmartTagStt = nBegin;
                    xub_StrLen nDummy = 1;
                    if ( !pNode->GetSmartTags() || !pNode->GetSmartTags()->InWrongWord( nSmartTagStt, nDummy ) )
                    {
                        if( !pNode->GetWrong() )
                        {
                            pNode->SetWrong( new SwWrongList() );
                            pNode->GetWrong()->SetInvalid( 0, nEnd );
                        }
                        if( pNode->GetWrong()->Fresh( nChgStart, nChgEnd,
                            nBegin, nLen, nInsertPos, nActPos ) )
                            pNode->GetWrong()->Insert( rtl::OUString(), 0, nBegin, nLen, nInsertPos++ );
                        else
                        {
                            nInvStart = nBegin;
                            nInvEnd = nBegin + nLen;
                        }
                    }
                }
                else if( bAddAutoCmpl && rACW.GetMinWordLen() <= rWord.Len() )
                {
                    if ( bRedlineChg )
                    {
                        XubString rNewWord( rWord );
                        rACW.InsertWord( rNewWord, *pDoc );
                    }
                    else
                        rACW.InsertWord( rWord, *pDoc );
                }
            }
        }
    }

    // reset original text
    // i63141 before calling GetCharRect(..) with formatting!
    if ( bRestoreString )
        pNode->aText = aOldTxt;
    if( pNode->GetWrong() )
    {
        if( bFresh )
            pNode->GetWrong()->Fresh( nChgStart, nChgEnd,
                                      nEnd, 0, nInsertPos, nActPos );

        //
        // Calculate repaint area:
        //
        if( nChgStart < nChgEnd && !rViewOpt.IsHideSpell() )
        {
            aRect = lcl_CalculateRepaintRect( *this, nChgStart, nChgEnd );
        }

        pNode->GetWrong()->SetInvalid( nInvStart, nInvEnd );
        pNode->SetWrongDirty( STRING_LEN != pNode->GetWrong()->GetBeginInv() );
        if( !pNode->GetWrong()->Count() && ! pNode->IsWrongDirty() )
            pNode->SetWrong( NULL );
    }
    else
        pNode->SetWrongDirty( false );

    if( bAddAutoCmpl )
        pNode->SetAutoCompleteWordDirty( false );

    return aRect;
}

/** Function: SmartTagScan

    Function scans words in current text and checks them in the
    smarttag libraries. If the check returns true to bounds of the
    recognized words are stored into a list which is used later for drawing
    the underline.

    @param SwCntntNode* pActNode

    @param xub_StrLen nActPos

    @return SwRect: Repaint area
*/
SwRect SwTxtFrm::SmartTagScan( SwCntntNode* /*pActNode*/, xub_StrLen /*nActPos*/ )
{
    SwRect aRet;
    SwTxtNode *pNode = GetTxtNode();
    const rtl::OUString& rText = pNode->GetTxt();

    // Iterate over language portions
    SmartTagMgr& rSmartTagMgr = SwSmartTagMgr::Get();

    SwWrongList* pSmartTagList = pNode->GetSmartTags();

    xub_StrLen nBegin = 0;
    xub_StrLen nEnd = static_cast< xub_StrLen >(rText.getLength());

    if ( pSmartTagList )
    {
        if ( pSmartTagList->GetBeginInv() != STRING_LEN )
        {
            nBegin = pSmartTagList->GetBeginInv();
            nEnd = Min( pSmartTagList->GetEndInv(), (xub_StrLen)rText.getLength() );

            if ( nBegin < nEnd )
            {
                const LanguageType aCurrLang = pNode->GetLang( nBegin );
                const com::sun::star::lang::Locale aCurrLocale = pBreakIt->GetLocale( aCurrLang );
                nBegin = static_cast< xub_StrLen >(pBreakIt->xBreak->beginOfSentence( rText, nBegin, aCurrLocale ));
                nEnd = static_cast< xub_StrLen >(Min( rText.getLength(), pBreakIt->xBreak->endOfSentence( rText, nEnd, aCurrLocale ) ));
            }
        }
    }

    const USHORT nNumberOfEntries = pSmartTagList ? pSmartTagList->Count() : 0;
    USHORT nNumberOfRemovedEntries = 0;
    USHORT nNumberOfInsertedEntries = 0;

    // clear smart tag list between nBegin and nEnd:
    if ( 0 != nNumberOfEntries )
    {
        xub_StrLen nChgStart = STRING_LEN;
        xub_StrLen nChgEnd = 0;
        const USHORT nCurrentIndex = pSmartTagList->GetWrongPos( nBegin );
        pSmartTagList->Fresh( nChgStart, nChgEnd, nBegin, nEnd - nBegin, nCurrentIndex, STRING_LEN );
        nNumberOfRemovedEntries = nNumberOfEntries - pSmartTagList->Count();
    }

    if ( nBegin < nEnd )
    {
        // Expand the string:
        rtl::OUString aExpandText;
        const ModelToViewHelper::ConversionMap* pConversionMap =
                pNode->BuildConversionMap( aExpandText );

        // Ownership ov ConversionMap is passed to SwXTextMarkup object!
        Reference< com::sun::star::text::XTextMarkup > xTextMarkup =
             new SwXTextMarkup( *pNode, pConversionMap );

        Reference< ::com::sun::star::frame::XController > xController = pNode->GetDoc()->GetDocShell()->GetController();

        xub_StrLen nLangBegin = nBegin;
        xub_StrLen nLangEnd = nEnd;

        // smart tag recognization has to be done for each language portion:
        SwLanguageIterator aIter( *pNode, nLangBegin );

        do
        {
            const LanguageType nLang = aIter.GetLanguage();
            const com::sun::star::lang::Locale aLocale = pBreakIt->GetLocale( nLang );
            nLangEnd = Min( nEnd, aIter.GetChgPos() );

            const sal_uInt32 nExpandBegin = ModelToViewHelper::ConvertToViewPosition( pConversionMap, nLangBegin );
            const sal_uInt32 nExpandEnd   = ModelToViewHelper::ConvertToViewPosition( pConversionMap, nLangEnd );

            rSmartTagMgr.Recognize( aExpandText, xTextMarkup, xController, aLocale, nExpandBegin, nExpandEnd - nExpandBegin );

            nLangBegin = nLangEnd;
        }
        while ( aIter.Next() && nLangEnd < nEnd );

        pSmartTagList = pNode->GetSmartTags();

        const USHORT nNumberOfEntriesAfterRecognize = pSmartTagList ? pSmartTagList->Count() : 0;
        nNumberOfInsertedEntries = nNumberOfEntriesAfterRecognize - ( nNumberOfEntries - nNumberOfRemovedEntries );
    }

    if( pSmartTagList )
    {
        //
        // Update WrongList stuff
        //
        pSmartTagList->SetInvalid( STRING_LEN, 0 );
        pNode->SetSmartTagDirty( STRING_LEN != pSmartTagList->GetBeginInv() );

        if( !pSmartTagList->Count() && !pNode->IsSmartTagDirty() )
            pNode->SetSmartTags( NULL );

        //
        // Calculate repaint area:
        //
#if OSL_DEBUG_LEVEL > 1
        const USHORT nNumberOfEntriesAfterRecognize2 = pSmartTagList->Count();
        (void) nNumberOfEntriesAfterRecognize2;
#endif
        if ( nBegin < nEnd && ( 0 != nNumberOfRemovedEntries ||
                                0 != nNumberOfInsertedEntries ) )
        {
            aRet = lcl_CalculateRepaintRect( *this, nBegin, nEnd );
        }
    }
    else
        pNode->SetSmartTagDirty( false );

    return aRet;
}


// Wird vom CollectAutoCmplWords gerufen
void SwTxtFrm::CollectAutoCmplWrds( SwCntntNode* pActNode, xub_StrLen nActPos )
{
    SwTxtNode *pNode = GetTxtNode();
    if( pNode != pActNode || !nActPos )
        nActPos = STRING_LEN;

    SwDoc* pDoc = pNode->GetDoc();
    SwAutoCompleteWord& rACW = SwDoc::GetAutoCompleteWords();

    xub_StrLen nBegin = 0;
    xub_StrLen nEnd = pNode->aText.Len();
    xub_StrLen nLen;
    BOOL bACWDirty = FALSE, bAnyWrd = FALSE;


    if( nBegin < nEnd )
    {
        USHORT nCnt = 200;
        SwScanner aScanner( *pNode, pNode->aText, 0, 0, WordType::DICTIONARY_WORD,
                            nBegin, nEnd );
        while( aScanner.NextWord() )
        {
            nBegin = aScanner.GetBegin();
            nLen = aScanner.GetLen();
            if( rACW.GetMinWordLen() <= nLen )
            {
                const XubString& rWord = aScanner.GetWord();

                if( nActPos < nBegin || ( nBegin + nLen ) < nActPos )
                {
                    if( rACW.GetMinWordLen() <= rWord.Len() )
                        rACW.InsertWord( rWord, *pDoc );
                    bAnyWrd = TRUE;
                }
                else
                    bACWDirty = TRUE;
            }
            if( !--nCnt )
            {
                if ( Application::AnyInput( INPUT_ANY ) )
                    return;
                nCnt = 100;
            }
        }
    }

    if( bAnyWrd && !bACWDirty )
        pNode->SetAutoCompleteWordDirty( FALSE );
}


/*************************************************************************
 *                      SwTxtNode::Hyphenate
 *************************************************************************/
// Findet den TxtFrm und sucht dessen CalcHyph

BOOL SwTxtNode::Hyphenate( SwInterHyphInfo &rHyphInf )
{
    // Abkuerzung: am Absatz ist keine Sprache eingestellt:
    if( LANGUAGE_NONE == USHORT( GetSwAttrSet().GetLanguage().GetLanguage() ) &&
        USHRT_MAX == GetLang( 0, aText.Len() ) )
    {
        if( !rHyphInf.IsCheck() )
            rHyphInf.SetNoLang( TRUE );
        return FALSE;
    }

    if( pLinguNode != this )
    {
        pLinguNode = this;
        pLinguFrm = (SwTxtFrm*)GetFrm( (Point*)(rHyphInf.GetCrsrPos()) );
    }
    SwTxtFrm *pFrm = pLinguFrm;
    if( pFrm )
        pFrm = &(pFrm->GetFrmAtOfst( rHyphInf.nStart ));
    else
    {
        // 4935: Seit der Trennung ueber Sonderbereiche sind Faelle
        // moeglich, in denen kein Frame zum Node vorliegt.
        // Also kein ASSERT!
#if OSL_DEBUG_LEVEL > 1
        ASSERT( pFrm, "!SwTxtNode::Hyphenate: can't find any frame" );
#endif
        return FALSE;
    }

    while( pFrm )
    {
        if( pFrm->Hyphenate( rHyphInf ) )
        {
            // Das Layout ist nicht robust gegen "Direktformatierung"
            // (7821, 7662, 7408); vgl. layact.cxx,
            // SwLayAction::_TurboAction(), if( !pCnt->IsValid() ...
            pFrm->SetCompletePaint();
            return TRUE;
        }
        pFrm = (SwTxtFrm*)(pFrm->GetFollow());
        if( pFrm )
        {
            rHyphInf.nLen = rHyphInf.nLen - (pFrm->GetOfst() - rHyphInf.nStart);
            rHyphInf.nStart = pFrm->GetOfst();
        }
    }
    return FALSE;
}

#ifdef LINGU_STATISTIK

// globale Variable
SwLinguStatistik aSwLinguStat;


void SwLinguStatistik::Flush()
{
    if ( !nWords )
        return ;

    static char *pLogName = 0;
    const BOOL bFirstOpen = pLogName ? FALSE : TRUE;
    if( bFirstOpen )
    {
        char *pPath = getenv( "TEMP" );
        char *pName = "swlingu.stk";
        if( !pPath )
            pLogName = pName;
        else
        {
            const int nLen = strlen(pPath);
            // fuer dieses new wird es kein delete geben.
            pLogName = new char[nLen + strlen(pName) + 3];
            if(nLen && (pPath[nLen-1] == '\\') || (pPath[nLen-1] == '/'))
                snprintf( pLogName, sizeof(pLogName), "%s%s", pPath, pName );
            else
                snprintf( pLogName, sizeof(pLogName), "%s/%s", pPath, pName );
        }
    }
    SvFileStream aStream( String::CreateFromAscii(pLogName), (bFirstOpen
                                        ? STREAM_WRITE | STREAM_TRUNC
                                        : STREAM_WRITE ));

    if( !aStream.GetError() )
    {
        if ( bFirstOpen )
            aStream << "\nLinguistik-Statistik\n";
        aStream << endl << ++nFlushCnt << ". Messung\n";
        aStream << "Rechtschreibung\n";
        aStream << "gepruefte Worte: \t" << nWords << endl;
        aStream << "als fehlerhaft erkannt:\t" << nWrong << endl;
        aStream << "Alternativvorschlaege:\t" << nAlter << endl;
        if ( nWrong )
            aStream << "Durchschnitt:\t\t" << nAlter*1.0 / nWrong << endl;
        aStream << "Dauer (msec):\t\t" << nSpellTime << endl;
        aStream << "\nThesaurus\n";
        aStream << "Synonyme gesamt:\t" << nSynonym << endl;
        if ( nSynonym )
            aStream << "Synonym-Durchschnitt:\t" <<
                            nSynonym*1.0 / ( nWords - nNoSynonym ) << endl;
        aStream << "ohne Synonyme:\t\t" << nNoSynonym << endl;
        aStream << "Bedeutungen gesamt:\t" << nSynonym << endl;
        aStream << "keine Bedeutungen:\t"<< nNoSynonym << endl;
        aStream << "Dauer (msec):\t\t" << nTheTime << endl;
        aStream << "\nHyphenator\n";
        aStream << "Trennstellen gesamt:\t" << nHyphens << endl;
        if ( nHyphens )
            aStream << "Hyphen-Durchschnitt:\t" <<
                    nHyphens*1.0 / ( nWords - nNoHyph - nHyphErr ) << endl;
        aStream << "keine Trennstellen:\t" << nNoHyph << endl;
        aStream << "Trennung verweigert:\t" << nHyphErr << endl;
        aStream << "Dauer (msec):\t\t" << nHyphTime << endl;
        aStream << "---------------------------------------------\n";
    }
    nWords = nWrong = nAlter = nSynonym = nNoSynonym =
    nHyphens = nNoHyph = nHyphErr = nSpellTime = nTheTime =
    nHyphTime = 0;
    //pThes = NULL;
}

#endif

// change text to Upper/Lower/Hiragana/Katagana/...
void SwTxtNode::TransliterateText( utl::TransliterationWrapper& rTrans,
        xub_StrLen nStt, xub_StrLen nEnd, SwUndoTransliterate* pUndo )
{
    if( nStt < nEnd )
    {
        SwLanguageIterator* pIter;
        if( rTrans.needLanguageForTheMode() )
            pIter = new SwLanguageIterator( *this, nStt );
        else
            pIter = 0;

        xub_StrLen nEndPos;
        sal_uInt16 nLang;
        do {
            if( pIter )
            {
                nLang = pIter->GetLanguage();
                nEndPos = pIter->GetChgPos();
                if( nEndPos > nEnd )
                    nEndPos = nEnd;
            }
            else
            {
                nLang = LANGUAGE_SYSTEM;
                nEndPos = nEnd;
            }
            xub_StrLen nLen = nEndPos - nStt;

            Sequence <sal_Int32> aOffsets;
            String sChgd( rTrans.transliterate( aText, nLang, nStt, nLen,
                                                    &aOffsets ));
            if( !aText.Equals( sChgd, nStt, nLen ) )
            {
                if( pUndo )
                    pUndo->AddChanges( *this, nStt, nLen, aOffsets );
                ReplaceTextOnly( nStt, nLen, sChgd, aOffsets );
            }
            nStt = nEndPos;
        } while( nEndPos < nEnd && pIter && pIter->Next() );
        delete pIter;
    }
}


void SwTxtNode::ReplaceTextOnly( xub_StrLen nPos, xub_StrLen nLen,
                                const XubString& rText,
                                const Sequence<sal_Int32>& rOffsets )
{
    aText.Replace( nPos, nLen, rText );

    xub_StrLen nTLen = rText.Len();
    const sal_Int32* pOffsets = rOffsets.getConstArray();
    // now look for no 1-1 mapping -> move the indizies!
    xub_StrLen nI, nMyOff;
    for( nI = 0, nMyOff = nPos; nI < nTLen; ++nI, ++nMyOff )
    {
        xub_StrLen nOff = (xub_StrLen)pOffsets[ nI ];
        if( nOff < nMyOff )
        {
            // something is inserted
            xub_StrLen nCnt = 1;
            while( nI + nCnt < nTLen && nOff == pOffsets[ nI + nCnt ] )
                ++nCnt;

            Update( SwIndex( this, nMyOff ), nCnt, FALSE );
            nMyOff = nOff;
            //nMyOff -= nCnt;
            nI += nCnt - 1;
        }
        else if( nOff > nMyOff )
        {
            // something is deleted
            Update( SwIndex( this, nMyOff+1 ), nOff - nMyOff, TRUE );
            nMyOff = nOff;
        }
    }
    if( nMyOff < nLen )
        // something is deleted at the end
        Update( SwIndex( this, nMyOff ), nLen - nMyOff, TRUE );

    // notify the layout!
    SwDelTxt aDelHint( nPos, nTLen );
    SwModify::Modify( 0, &aDelHint );

    SwInsTxt aHint( nPos, nTLen );
    SwModify::Modify( 0, &aHint );
}

void SwTxtNode::CountWords( SwDocStat& rStat,
                            xub_StrLen nStt, xub_StrLen nEnd ) const
{
    if( nStt < nEnd )
    {
        if ( !IsHidden() )
        {
            ++rStat.nPara;
            ULONG nTmpWords = 0;
            ULONG nTmpChars = 0;

            // Shortcut: Whole paragraph should be considered and cached values
            // are valid:
            if ( 0 == nStt && GetTxt().Len() == nEnd && !IsWordCountDirty() )
            {
                nTmpWords = GetParaNumberOfWords();
                nTmpChars = GetParaNumberOfChars();
            }
            else
            {
                String aOldStr( aText );
                String& rCastStr = (String&)aText;

                // fills the deleted redlines and hidden ranges with cChar:
                const xub_Unicode cChar(' ');
                const USHORT nNumOfMaskedChars =
                        lcl_MaskRedlinesAndHiddenText( *this, rCastStr, nStt, nEnd, cChar, false );

                // expand fields
                rtl::OUString aExpandText;
                const ModelToViewHelper::ConversionMap* pConversionMap =
                        BuildConversionMap( aExpandText );

                const sal_uInt32 nExpandBegin = ModelToViewHelper::ConvertToViewPosition( pConversionMap, nStt );
                const sal_uInt32 nExpandEnd   = ModelToViewHelper::ConvertToViewPosition( pConversionMap, nEnd );

                const bool bCount = aExpandText.getLength();

                // count words in 'regular' text:
                if( bCount && pBreakIt->xBreak.is() )
                {
                    SwScanner aScanner( *this, aExpandText, 0, pConversionMap,
                                        i18n::WordType::WORD_COUNT,
                                        (xub_StrLen)nExpandBegin, (xub_StrLen)nExpandEnd );

                    const rtl::OUString aBreakWord( CH_TXTATR_BREAKWORD );

                    while ( aScanner.NextWord() )
                    {
                        if ( aScanner.GetLen() > 1 ||
                             CH_TXTATR_BREAKWORD != aExpandText.match(aBreakWord, aScanner.GetBegin() ) )
                            ++nTmpWords;
                    }
                }

                ASSERT( aExpandText.getLength() >= nNumOfMaskedChars,
                        "More characters hidden that characters in string!" )
                nTmpChars = nExpandEnd - nExpandBegin - nNumOfMaskedChars;

                // count words in numbering string:
                if ( nStt == 0 && bCount )
                {
                    // add numbering label
                    rtl::OUString aNumString = GetNumString();
                    if ( aNumString.getLength() )
                    {
                        LanguageType aLanguage = GetLang( 0 );

                        SwScanner aScanner( *this, aNumString, &aLanguage, 0,
                                            i18n::WordType::WORD_COUNT,
                                            0, (xub_StrLen)aNumString.getLength() );

                        while ( aScanner.NextWord() )
                            ++nTmpWords;

                        nTmpChars += aNumString.getLength();
                    }
                    else if ( HasBullet() )
                    {
                        ++nTmpWords;
                        ++nTmpChars;
                    }
                }

                delete pConversionMap;

                rCastStr = aOldStr;

                // If the whole paragraph has been calculated, update cached
                // values:
                if ( 0 == nStt && GetTxt().Len() == nEnd )
                {
                    SetParaNumberOfWords( nTmpWords );
                    SetParaNumberOfChars( nTmpChars );
                    SetWordCountDirty( false );
                }
            }

            rStat.nWord += nTmpWords;
            rStat.nChar += nTmpChars;
        }
    }
}

//
// Paragraph statistics start
//
struct SwParaIdleData_Impl
{
    SwWrongList* pWrong;            // for spell checking
    SwWrongList* pGrammarCheck;     // for grammar checking /  proof reading
    SwWrongList* pSmartTags;
    ULONG nNumberOfWords;
    ULONG nNumberOfChars;
    bool bWordCountDirty        : 1;
    bool bWrongDirty            : 1;    // Ist das Wrong-Feld auf invalid?
    bool bGrammarCheckDirty     : 1;
    bool bSmartTagDirty         : 1;
    bool bAutoComplDirty        : 1;    // die ACompl-Liste muss angepasst werden

    SwParaIdleData_Impl() :
        pWrong              ( 0 ),
        pGrammarCheck       ( 0 ),
        pSmartTags          ( 0 ),
        nNumberOfWords      ( 0 ),
        nNumberOfChars      ( 0 ),
        bWordCountDirty     ( true ),
        bWrongDirty         ( true ),
        bGrammarCheckDirty  ( true ),
        bSmartTagDirty      ( true ),
        bAutoComplDirty     ( true ) {};
};

void SwTxtNode::InitSwParaStatistics( bool bNew )
{
    if ( bNew )
        pParaIdleData_Impl = new SwParaIdleData_Impl;
    else if ( pParaIdleData_Impl )
    {
        delete pParaIdleData_Impl->pWrong;
        delete pParaIdleData_Impl->pGrammarCheck;
        delete pParaIdleData_Impl->pSmartTags;
        delete pParaIdleData_Impl;
        pParaIdleData_Impl = 0;
    }
}

void SwTxtNode::SetWrong( SwWrongList* pNew, bool bDelete )
{
    if ( pParaIdleData_Impl )
    {
        if ( bDelete )
            delete pParaIdleData_Impl->pWrong;
        pParaIdleData_Impl->pWrong = pNew;
    }
}

SwWrongList* SwTxtNode::GetWrong()
{
    return pParaIdleData_Impl ? pParaIdleData_Impl->pWrong : 0;
}

void SwTxtNode::SetGrammarCheck( SwWrongList* pNew, bool bDelete )
{
    if ( pParaIdleData_Impl )
    {
        if ( bDelete )
            delete pParaIdleData_Impl->pGrammarCheck;
        pParaIdleData_Impl->pGrammarCheck = pNew;
    }
}

SwWrongList* SwTxtNode::GetGrammarCheck()
{
    return pParaIdleData_Impl ? pParaIdleData_Impl->pGrammarCheck : 0;
}

void SwTxtNode::SetSmartTags( SwWrongList* pNew, bool bDelete )
{
    ASSERT( !pNew || SwSmartTagMgr::Get().IsSmartTagsEnabled(),
            "Weird - we have a smart tag list without any recognizers?" )

    if ( pParaIdleData_Impl )
    {
        if ( bDelete )
            delete pParaIdleData_Impl->pSmartTags;
        pParaIdleData_Impl->pSmartTags = pNew;
    }
}

SwWrongList* SwTxtNode::GetSmartTags()
{
    return pParaIdleData_Impl ? pParaIdleData_Impl->pSmartTags : 0;
}

void SwTxtNode::SetParaNumberOfWords( ULONG nNew ) const
{
    if ( pParaIdleData_Impl )
        pParaIdleData_Impl->nNumberOfWords = nNew;
}
ULONG SwTxtNode::GetParaNumberOfWords() const
{
    return pParaIdleData_Impl ? pParaIdleData_Impl->nNumberOfWords : 0;
}
void SwTxtNode::SetParaNumberOfChars( ULONG nNew ) const
{
    if ( pParaIdleData_Impl )
        pParaIdleData_Impl->nNumberOfChars = nNew;
}
ULONG SwTxtNode::GetParaNumberOfChars() const
{
    return pParaIdleData_Impl ? pParaIdleData_Impl->nNumberOfChars : 0;
}
void SwTxtNode::SetWordCountDirty( bool bNew ) const
{
    if ( pParaIdleData_Impl )
        pParaIdleData_Impl->bWordCountDirty = bNew;
}
bool SwTxtNode::IsWordCountDirty() const
{
    return pParaIdleData_Impl ? pParaIdleData_Impl->bWordCountDirty : 0;
}
void SwTxtNode::SetWrongDirty( bool bNew ) const
{
    if ( pParaIdleData_Impl )
        pParaIdleData_Impl->bWrongDirty = bNew;
}
bool SwTxtNode::IsWrongDirty() const
{
    return pParaIdleData_Impl ? pParaIdleData_Impl->bWrongDirty : 0;
}
void SwTxtNode::SetGrammarCheckDirty( bool bNew ) const
{
    if ( pParaIdleData_Impl )
        pParaIdleData_Impl->bGrammarCheckDirty = bNew;
}
bool SwTxtNode::IsGrammarCheckDirty() const
{
    return pParaIdleData_Impl ? pParaIdleData_Impl->bGrammarCheckDirty : 0;
}
void SwTxtNode::SetSmartTagDirty( bool bNew ) const
{
    if ( pParaIdleData_Impl )
        pParaIdleData_Impl->bSmartTagDirty = bNew;
}
bool SwTxtNode::IsSmartTagDirty() const
{
    return pParaIdleData_Impl ? pParaIdleData_Impl->bSmartTagDirty : 0;
}
void SwTxtNode::SetAutoCompleteWordDirty( bool bNew ) const
{
    if ( pParaIdleData_Impl )
        pParaIdleData_Impl->bAutoComplDirty = bNew;
}
bool SwTxtNode::IsAutoCompleteWordDirty() const
{
    return pParaIdleData_Impl ? pParaIdleData_Impl->bAutoComplDirty : 0;
}
//
// Paragraph statistics end
//
