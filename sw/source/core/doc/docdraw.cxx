/*************************************************************************
 *
 *  $RCSfile: docdraw.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
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

#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#ifndef _SVX_FLDITEM_HXX //autogen
#define ITEMID_FIELD EE_FEATURE_FIELD
#include <svx/flditem.hxx>
#endif
#ifndef _MyEDITENG_HXX //autogen
#include <svx/editeng.hxx>
#endif
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif

#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#ifndef _SVDOMEAS_HXX
#include <svx/svdfield.hxx>
#endif
#ifndef _SVDPOOL_HXX //autogen
#include <svx/svdpool.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _VIEWIMP_HXX //autogen
#include <viewimp.hxx>
#endif
#ifndef _SWHINTS_HXX
#include <swhints.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>  //Damit der RootDtor gerufen wird.
#endif
#ifndef _FRAME_HXX
#include <frame.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>           // fuer MakeDrawView
#endif
#ifndef _DRAWDOC_HXX
#include <drawdoc.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>           // fuer die UndoIds
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _DVIEW_HXX
#include <dview.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif

using namespace ::com::sun::star;


SV_IMPL_VARARR_SORT( _ZSortFlys, _ZSortFly )

/*************************************************************************
|*
|*  SwDoc::GroupSelection / SwDoc::UnGroupSelection
|*
|*  Ersterstellung      JP 21.08.95
|*  Letzte Aenderung    JP 21.08.95
|*
|*************************************************************************/

SwDrawContact* SwDoc::GroupSelection( SdrView& rDrawView )
{
    const SdrMarkList &rMrkList = rDrawView.GetMarkList();
    SwDrawFrmFmt *pFmt;
    SdrObject *pObj = rMrkList.GetMark( 0 )->GetObj();
    BOOL bNoGroup = ( 0 == pObj->GetUpGroup() );
    if( bNoGroup )
    {
        //Ankerattribut aufheben.
        SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
        const SwFmtAnchor aAnch( pContact->GetFmt()->GetAnchor() );
        //Ankerpos des ersten, um die Objekte zu synchronisieren.
        Point aAnchPos( pObj->GetAnchorPos() );

        SwUndoDrawGroup* pUndo = !DoesUndo() ? 0 : new SwUndoDrawGroup(
                                                (USHORT)rMrkList.GetMarkCount() );

        //ContactObjekte und Formate vernichten.
        for( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            pObj = rMrkList.GetMark( i )->GetObj();
            SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);

            //Ankerpos und Relpos synchronisieren, damit die Position der
            //Objekte erhalten bleibt.
            const Point aAbs( pContact->GetAnchor()->Frm().Pos() +
                                pObj->GetRelativePos() );

            pFmt = (SwDrawFrmFmt*)pContact->GetFmt();
            //loescht sich selbst!
            pContact->Changed(*pObj, SDRUSERCALL_DELETE, pObj->GetBoundRect() );
            pObj->SetUserCall( 0 );

            if( pUndo )
                pUndo->AddObj( i, pFmt, pObj );
            else
                DelFrmFmt( pFmt );

            pObj->NbcSetRelativePos( aAbs - aAnchPos );
            pObj->NbcSetAnchorPos( aAnchPos );
        }

        pFmt = MakeDrawFrmFmt( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( "DrawObject" )),
                                GetDfltFrmFmt() );
        pFmt->SetAttr( aAnch );

        if( pUndo )
        {
            pUndo->SetGroupFmt( pFmt );
            ClearRedo();
            AppendUndo( pUndo );
        }
    }
    else if( DoesUndo() )
        ClearRedo();

    rDrawView.GroupMarked();
    ASSERT( rMrkList.GetMarkCount() == 1, "GroupMarked more or none groups." );
    SwDrawContact *pNewContact = 0;
    if( bNoGroup )
    {
        pNewContact = new SwDrawContact( pFmt, rMrkList.GetMark( 0 )->GetObj() );
        pNewContact->ConnectToLayout();
    }
    return pNewContact;
}


void SwDoc::UnGroupSelection( SdrView& rDrawView )
{
    int bUndo = DoesUndo();
    if( bUndo )
        ClearRedo();
    const SdrMarkList &rMrkList = rDrawView.GetMarkList();
    if( rMrkList.GetMarkCount() )
    {
        SdrObject *pObj = rMrkList.GetMark( 0 )->GetObj();
        if( !pObj->GetUpGroup() )
        {
            String sDrwFmtNm( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM("DrawObject" )));
            for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
            {
                SdrObject *pObj = rMrkList.GetMark( i )->GetObj();
                if ( pObj->IsA( TYPE(SdrObjGroup) ) )
                {
                    SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
                    SwFmtAnchor aAnch( pContact->GetFmt()->GetAnchor() );
                    SdrObjList *pLst = ((SdrObjGroup*)pObj)->GetSubList();

                    SwUndoDrawUnGroup* pUndo = 0;
                    if( bUndo )
                    {
                        pUndo = new SwUndoDrawUnGroup( (SdrObjGroup*)pObj );
                        AppendUndo( pUndo );
                    }

                    for ( USHORT i2 = 0; i2 < pLst->GetObjCount(); ++i2 )
                    {
                        SdrObject *pSubObj = pLst->GetObj( i2 );
                        SwDrawFrmFmt *pFmt = MakeDrawFrmFmt( sDrwFmtNm,
                                                            GetDfltFrmFmt() );
                        pFmt->SetAttr( aAnch );
                        SwDrawContact *pContact = new SwDrawContact( pFmt, pSubObj );
                        pContact->ConnectToLayout();

                        if( bUndo )
                            pUndo->AddObj( i2, pFmt );
                    }
                }
            }
        }
    }
    rDrawView.UnGroupMarked();
}

/*************************************************************************
|*
|*  SwDoc::DeleteSelection()
|*
|*  Ersterstellung      MA 14. Nov. 95
|*  Letzte Aenderung    MA 14. Nov. 95
|*
|*************************************************************************/

BOOL SwDoc::DeleteSelection( SwDrawView& rDrawView )
{
    BOOL bCallBase = FALSE;
    const SdrMarkList &rMrkList = rDrawView.GetMarkList();
    if( rMrkList.GetMarkCount() )
    {
        StartUndo();
        USHORT i;
        FASTBOOL bDelMarked = TRUE;

        if( 1 == rMrkList.GetMarkCount() )
        {
            SdrObject *pObj = rMrkList.GetMark( 0 )->GetObj();
            if( pObj->IsWriterFlyFrame() )
            {
                SwFlyFrmFmt* pFrmFmt = (SwFlyFrmFmt*)
                    ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->GetFmt();
                if( pFrmFmt )
                {
#if 0
// JP 28.09.98: erstmal wuerde ich NEIN sagen.
// muss das sein ????
                    // ggfs. die CrsrPosition umsetzen
                    SwCrsrShell* pCShell = PTR_CAST( SwCrsrShell,
                                            rDrawView.Imp().GetShell() );
                    if( pCShell )
                    {
                        SwRect& rChrRect = (SwRect&)pCShell->GetCharRect();
                        SwFlyFrm* pFly = pFrmFmt->GetFrm( &rChrRect.Pos(), FALSE );

                        if( pFly && pFly->IsFlyInCntFrm() )
                        {
                            rChrRect = pFly->Frm();
                            pCShell->GetCrsrDocPos() = rChrRect.Pos();
                        }
                    }
// muss das sein ????
#endif
                    DelLayoutFmt( pFrmFmt );
                    bDelMarked = FALSE;
                }
            }
        }

        for( i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetObj();
            if( !pObj->IsWriterFlyFrame() )
            {
                SwDrawContact *pC = (SwDrawContact*)GetUserCall(pObj);
                SwDrawFrmFmt *pFrmFmt = (SwDrawFrmFmt*)pC->GetFmt();
                if( pFrmFmt &&
                    FLY_IN_CNTNT == pFrmFmt->GetAnchor().GetAnchorId() )
                {
                    rDrawView.MarkObj( pObj, rDrawView.Imp().GetPageView(), TRUE );
                    --i;
                    DelLayoutFmt( pFrmFmt );
                }
            }
        }

        if( rMrkList.GetMarkCount() && bDelMarked )
        {
            SdrObject *pObj = rMrkList.GetMark( 0 )->GetObj();
            if( !pObj->GetUpGroup() )
            {
                SwUndoDrawDelete* pUndo = !DoesUndo() ? 0
                            : new SwUndoDrawDelete( (USHORT)rMrkList.GetMarkCount() );

                //ContactObjekte vernichten, Formate sicherstellen.
                for( i = 0; i < rMrkList.GetMarkCount(); ++i )
                {
                    pObj = rMrkList.GetMark( i )->GetObj();
                    SwDrawContact *pContact = (SwDrawContact*)pObj->GetUserCall();
                    if( pContact ) // natuerlich nicht bei gruppierten Objekten
                    {
                        SwDrawFrmFmt *pFmt = (SwDrawFrmFmt*)pContact->GetFmt();
                        //loescht sich selbst!
                        pContact->Changed(*pObj, SDRUSERCALL_DELETE, pObj->GetBoundRect() );
                        pObj->SetUserCall( 0 );

                        if( pUndo )
                            pUndo->AddObj( i, pFmt, pObj );
                        else
                            DelFrmFmt( pFmt );
                    }
                }

                if( pUndo )
                    AppendUndo( pUndo );
            }
            bCallBase = TRUE;
        }
        SetModified();

        EndUndo();
    }

    return bCallBase;
}

/*************************************************************************
|*
|*  SwDoc::DeleteSelection()
|*
|*  Ersterstellung      JP 11.01.96
|*  Letzte Aenderung    JP 11.01.96
|*
|*************************************************************************/

_ZSortFly::_ZSortFly( const SwFrmFmt* pFrmFmt, const SwFmtAnchor* pFlyAn,
                      UINT32 nArrOrdNum )
    : pFmt( pFrmFmt ), pAnchor( pFlyAn ), nOrdNum( nArrOrdNum )
{
    if( pFmt->GetDoc()->GetRootFrm() )
    {
        SwClientIter aIter( (SwFmt&)*pFmt );
        if( RES_FLYFRMFMT == pFmt->Which() )
        {
            // Schauen, ob es ein SdrObject dafuer gibt
            if( aIter.First( TYPE( SwFlyFrm) ) )
                nOrdNum = ((SwFlyFrm*)aIter())->GetVirtDrawObj()->GetOrdNum();
        }
        else if( RES_DRAWFRMFMT == pFmt->Which() )
        {
            // Schauen, ob es ein SdrObject dafuer gibt
            if( aIter.First( TYPE(SwDrawContact) ) )
                nOrdNum = ((SwDrawContact*)aIter())->GetMaster()->GetOrdNum();
        }
        else
            ASSERT( !this, "was ist das fuer ein Format?" );
    }
}

/*************************************************************************/
// Wird auch vom Sw3-Reader gerufen, wenn ein Fehler beim Einlesen
// des Drawing Layers auftrat. In diesem Fall wird der Layer komplett
// neu aufgebaut.


void SwDoc::InitDrawModel()
{
    //!!Achtung im sw3-Reader (sw3imp.cxx) gibt es aehnlichen Code, der
    //mitgepfelgt werden muss.
    if ( pDrawModel )
        ReleaseDrawModel();

    //DrawPool und EditEnginePool anlegen, diese gehoeren uns und werden
    //dem Drawing nur mitgegeben. Im ReleaseDrawModel werden die Pools wieder
    //zerstoert.
    // 17.2.99: for Bug 73110 - for loading the drawing items. This must
    //                          be loaded without RefCounts!
    SfxItemPool *pSdrPool = new SdrItemPool( &aAttrPool, SDRATTR_START,
                                            SDRATTR_END, FALSE );
    SfxItemPool *pEEgPool = EditEngine::CreatePool( FALSE );
    pSdrPool->SetSecondaryPool( pEEgPool );
    aAttrPool.FreezeIdRanges();

    //Das SdrModel gehoert dem Dokument, wir haben immer zwei Layer und eine
    //Seite.
    pDrawModel = new SwDrawDocument( this );

    String sLayerNm;
    sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Hell" ));
    nHell   = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

    sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Heaven" ));
    nHeaven = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

    sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Controls" ));
    nControls = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

    pDrawModel->InsertPage( pDrawModel->AllocPage( FALSE ) );
    SdrOutliner& rOutliner = pDrawModel->GetDrawOutliner();
    uno::Reference< linguistic::XSpellChecker1> xSpell = OFF_APP()->GetSpellChecker();
    rOutliner.SetSpeller( xSpell );
    rOutliner.SetHyphenator( ::GetHyphenator() );
    const SfxPoolItem& rItem = GetDefault(RES_CHRATR_LANGUAGE);
    rOutliner.SetDefaultLanguage(((const SvxLanguageItem&)rItem).GetLanguage());

    SetCalcFieldValueHdl(&rOutliner);
    SetCalcFieldValueHdl(&pDrawModel->GetHitTestOutliner());

    //JP 16.07.98: Bug 50193 - Linkmanager am Model setzen, damit
    //          dort ggfs. verlinkte Grafiken eingefuegt werden koennen
    //JP 28.01.99: der WinWord Import benoetigt ihn auch
    pDrawModel->SetLinkManager( &GetLinkManager() );

    if( pPrt )
        pDrawModel->SetRefDevice( pPrt );
    pDrawModel->SetNotifyUndoActionHdl( LINK( this, SwDoc, AddDrawUndo ));
    if ( pLayout )
    {
        pLayout->SetDrawPage( pDrawModel->GetPage( 0 ) );
        pLayout->GetDrawPage()->SetSize( pLayout->Frm().SSize() );
    }
}

/*************************************************************************/


void SwDoc::ReleaseDrawModel()
{
    if ( pDrawModel )
    {
        //!!Den code im sw3io fuer Einfuegen Dokument mitpflegen!!

        delete pDrawModel; pDrawModel = 0;
        SfxItemPool *pSdrPool = aAttrPool.GetSecondaryPool();

        ASSERT( pSdrPool, "missing Pool" );
        SfxItemPool *pEEgPool = pSdrPool->GetSecondaryPool();
        ASSERT( !pEEgPool->GetSecondaryPool(), "i don't accept additional pools");
        pSdrPool->Delete();                 //Erst die Items vernichten lassen,
                                            //dann erst die Verkettung loesen
        aAttrPool.SetSecondaryPool( 0 );    //Der ist ein muss!
        pSdrPool->SetSecondaryPool( 0 );    //Der ist sicherer
        delete pSdrPool;
        delete pEEgPool;
    }
}

/*************************************************************************/


SdrModel* SwDoc::_MakeDrawModel()
{
    ASSERT( !pDrawModel, "_MakeDrawModel: Why?" );
    InitDrawModel();
    if ( pLayout && pLayout->GetCurrShell() )
    {
        ViewShell* pTmp = pLayout->GetCurrShell();
        do
        {
            pTmp->MakeDrawView();
            pTmp = (ViewShell*) pTmp->GetNext();
        } while ( pTmp != pLayout->GetCurrShell() );

        //Broadcast, damit die FormShell mit der DrawView verbunden werden kann
        if( GetDocShell() )
        {
            SfxSimpleHint aHnt( SW_BROADCAST_DRAWVIEWS_CREATED );
            GetDocShell()->Broadcast( aHnt );
        }
    }
    return pDrawModel;
}

/*************************************************************************/

void SwDoc::DrawNotifyUndoHdl()
{
    pDrawModel->SetNotifyUndoActionHdl( Link() );
}

/*************************************************************************/

void SwDoc::DrawSetRefDevice()
{
    pDrawModel->SetRefDevice( pPrt );
}

/*************************************************************************/
/*
/* Am Outliner Link auf Methode fuer Felddarstellung in Editobjekten setzen
/*
/*************************************************************************/

void SwDoc::SetCalcFieldValueHdl(Outliner* pOutliner)
{
    pOutliner->SetCalcFieldValueHdl(LINK(this, SwDoc, CalcFieldValueHdl));
}

/*************************************************************************
|*
|* Felder bzw URLs im Outliner erkennen und Darstellung festlegen
|*
\************************************************************************/

IMPL_LINK(SwDoc, CalcFieldValueHdl, EditFieldInfo*, pInfo)
{
    if (pInfo)
    {
        const SvxFieldItem& rField = pInfo->GetField();
        const SvxFieldData* pField = rField.GetField();

        if (pField && pField->ISA(SvxDateField))
        {
            /******************************************************************
            * Date-Field
            ******************************************************************/
            pInfo->SetRepresentation(
                ((const SvxDateField*) pField)->GetFormatted(LANGUAGE_SYSTEM,
                                                             LANGUAGE_SYSTEM) );
        }
        else if (pField && pField->ISA(SvxURLField))
        {
            /******************************************************************
            * URL-Field
            ******************************************************************/

            switch ( ((const SvxURLField*) pField)->GetFormat() )
            {
                case SVXURLFORMAT_APPDEFAULT: //!!! einstellbar an App???
                case SVXURLFORMAT_REPR:
                {
                    pInfo->SetRepresentation(
                        ((const SvxURLField*)pField)->GetRepresentation());
                }
                break;

                case SVXURLFORMAT_URL:
                {
                    pInfo->SetRepresentation(
                        ((const SvxURLField*)pField)->GetURL());
                }
                break;
            }

            USHORT nChrFmt;

            if (IsVisitedURL(((const SvxURLField*)pField)->GetURL()))
                nChrFmt = RES_POOLCHR_INET_VISIT;
            else
                nChrFmt = RES_POOLCHR_INET_NORMAL;

            SwFmt *pFmt = GetCharFmtFromPool(nChrFmt);

            Color aColor(COL_LIGHTBLUE);
            if (pFmt)
                aColor = pFmt->GetColor().GetValue();

            pInfo->SetTxtColor(aColor);
        }
        else if (pField && pField->ISA(SdrMeasureField))
        {
            /******************************************************************
            * Measure-Field
            ******************************************************************/
            pInfo->ClearFldColor();
        }
        else
        {
            DBG_ERROR("unbekannter Feldbefehl");
            pInfo->SetRepresentation( String( '?' ) );
        }
    }

    return(0);
}



