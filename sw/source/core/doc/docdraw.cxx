/*************************************************************************
 *
 *  $RCSfile: docdraw.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:46:37 $
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

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
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

#ifndef _SVDETC_HXX
#include <svx/svdetc.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
#endif

// OD 26.06.2003 #108784#
#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif
// OD 2004-04-01 #i26791#
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;


SV_IMPL_VARARR_SORT( _ZSortFlys, _ZSortFly )

/*************************************************************************
|*
|*  SwDoc::GroupSelection / SwDoc::UnGroupSelection
|*
|*  Ersterstellung      JP 21.08.95
|*  Letzte Aenderung    JP 21.08.95
|*
|*************************************************************************/
// OD 2004-04-01 #i26791# - local method to determine positioning and
// alignment attributes for a drawing object, which is newly connected to
// the layout. Used for a newly formed group object <SwDoc::GroupSelection(..)>
// and the members of a destroyed group <SwDoc::UnGroupSelection(..)>
void lcl_AdjustPositioningAttr( SwFrmFmt* _pFrmFmt,
                                const SdrObject& _rSdrObj )
{
    const SwContact* pContact = GetUserCall( &_rSdrObj );
    ASSERT( pContact, "<lcl_AdjustPositioningAttr(..)> - missing contact object." );

    // determine position of new group object relative to its anchor frame position
    SwTwips nHoriRelPos = 0;
    SwTwips nVertRelPos = 0;
    {
        const SwFrm* pAnchorFrm = pContact->GetAnchoredObj( &_rSdrObj )->GetAnchorFrm();
        ASSERT( !pAnchorFrm ||
                !pAnchorFrm->IsTxtFrm() ||
                !static_cast<const SwTxtFrm*>(pAnchorFrm)->IsFollow(),
                "<lcl_AdjustPositioningAttr(..)> - anchor frame is a follow. Please inform OD." );
        bool bVert;
        bool bR2L;
        if ( pAnchorFrm )
        {
            bVert = pAnchorFrm->IsVertical();
            bR2L = pAnchorFrm->IsRightToLeft();
        }
        else
        {
            // If no anchor frame exist - e.g. because no layout exists - the
            // default layout direction is taken.
            const SvxFrameDirectionItem* pDirItem =
                static_cast<const SvxFrameDirectionItem*>(&(_pFrmFmt->GetAttrSet().GetPool()->GetDefaultItem( RES_FRAMEDIR )));
            switch ( pDirItem->GetValue() )
            {
                case FRMDIR_VERT_TOP_LEFT:
                {
                    // vertical from left-to-right - not supported yet
                    bVert = true;
                    bR2L = true;
                    ASSERT( false,
                            "<lcl_AdjustPositioningAttr(..)> - vertical from left-to-right not supported." );
                }
                break;
                case FRMDIR_VERT_TOP_RIGHT:
                {
                    // vertical from right-to-left
                    bVert = true;
                    bR2L = false;
                }
                break;
                case FRMDIR_HORI_RIGHT_TOP:
                {
                    // horizontal from right-to-left
                    bVert = false;
                    bR2L = true;
                }
                break;
                case FRMDIR_HORI_LEFT_TOP:
                {
                    // horizontal from left-to-right
                    bVert = false;
                    bR2L = false;
                }
                break;
            }

        }
        Point aAnchorPos = _rSdrObj.GetAnchorPos();
        // use geometry of drawing object
        SwRect aObjRect = _rSdrObj.GetSnapRect();
        if ( bVert )
        {
            nHoriRelPos = aObjRect.Top() - aAnchorPos.Y();
            nVertRelPos = aAnchorPos.X() - aObjRect.Right();
        }
        else if ( bR2L )
        {
            nHoriRelPos = aAnchorPos.X() - aObjRect.Right();
            nVertRelPos = aObjRect.Top() - aAnchorPos.Y();
        }
        else
        {
            nHoriRelPos = aObjRect.Left() - aAnchorPos.X();
            nVertRelPos = aObjRect.Top() - aAnchorPos.Y();
        }
    }

    _pFrmFmt->SetAttr( SwFmtHoriOrient( nHoriRelPos, HORI_NONE, FRAME ) );
    _pFrmFmt->SetAttr( SwFmtVertOrient( nVertRelPos, VERT_NONE, FRAME ) );
}

SwDrawContact* SwDoc::GroupSelection( SdrView& rDrawView )
{
    // OD 30.06.2003 #108784# - replace marked 'virtual' drawing objects by
    // the corresponding 'master' drawing objects.
    SwDrawView::ReplaceMarkedDrawVirtObjs( rDrawView );

    const SdrMarkList &rMrkList = rDrawView.GetMarkedObjectList();
    SwDrawFrmFmt *pFmt = 0L;
    SdrObject *pObj = rMrkList.GetMark( 0 )->GetObj();
    BOOL bNoGroup = ( 0 == pObj->GetUpGroup() );
    SwDrawContact* pNewContact = 0;
    if( bNoGroup )
    {
        //Ankerattribut aufheben.
        SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
        const SwFmtAnchor aAnch( pContact->GetFmt()->GetAnchor() );

        SwUndoDrawGroup* pUndo = !DoesUndo() ? 0 : new SwUndoDrawGroup(
                                                (USHORT)rMrkList.GetMarkCount() );

        //ContactObjekte und Formate vernichten.
        for( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            pObj = rMrkList.GetMark( i )->GetObj();
            SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);

            pFmt = (SwDrawFrmFmt*)pContact->GetFmt();
            //loescht sich selbst!
            pContact->Changed(*pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
            pObj->SetUserCall( 0 );

            if( pUndo )
                pUndo->AddObj( i, pFmt, pObj );
            else
                DelFrmFmt( pFmt );
        }

        pFmt = MakeDrawFrmFmt( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( "DrawObject" )),
                                GetDfltFrmFmt() );
        pFmt->SetAttr( aAnch );

        rDrawView.GroupMarked();
        ASSERT( rMrkList.GetMarkCount() == 1, "GroupMarked more or none groups." );

        SdrObject* pNewGroupObj = rMrkList.GetMark( 0 )->GetObj();
        pNewContact = new SwDrawContact( pFmt, pNewGroupObj );
        pNewContact->ConnectToLayout();
        // OD 2004-04-01 #i26791# - Adjust positioning and alignment attributes.
        lcl_AdjustPositioningAttr( pFmt, *pNewGroupObj );

        if( pUndo )
        {
            pUndo->SetGroupFmt( pFmt );
            ClearRedo();
            AppendUndo( pUndo );
        }
    }
    else
    {
        if ( DoesUndo() )
            ClearRedo();

        rDrawView.GroupMarked();
        ASSERT( rMrkList.GetMarkCount() == 1, "GroupMarked more or none groups." );
    }

    return pNewContact;
}


void SwDoc::UnGroupSelection( SdrView& rDrawView )
{
    int bUndo = DoesUndo();
    if( bUndo )
        ClearRedo();

    // OD 30.06.2003 #108784# - replace marked 'virtual' drawing objects by
    // the corresponding 'master' drawing objects.
    SwDrawView::ReplaceMarkedDrawVirtObjs( rDrawView );

    const SdrMarkList &rMrkList = rDrawView.GetMarkedObjectList();
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
                        SdrObject* pSubObj = pLst->GetObj( i2 );
                        SwDrawFrmFmt *pFmt = MakeDrawFrmFmt( sDrwFmtNm,
                                                            GetDfltFrmFmt() );
                        pFmt->SetAttr( aAnch );
                        SwDrawContact* pContact = new SwDrawContact( pFmt, pSubObj );
                        pContact->ConnectToLayout();
                        // OD 2004-04-07 #i26791# - Adjust positioning and
                        // alignment attributes.
                        lcl_AdjustPositioningAttr( pFmt, *pSubObj );

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
    const SdrMarkList &rMrkList = rDrawView.GetMarkedObjectList();
    if( rMrkList.GetMarkCount() )
    {
        StartUndo();
        USHORT i;
        FASTBOOL bDelMarked = TRUE;

        if( 1 == rMrkList.GetMarkCount() )
        {
            SdrObject *pObj = rMrkList.GetMark( 0 )->GetObj();
            if( pObj->ISA(SwVirtFlyDrawObj) )
            {
                SwFlyFrmFmt* pFrmFmt = (SwFlyFrmFmt*)
                    ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->GetFmt();
                if( pFrmFmt )
                {
                    DelLayoutFmt( pFrmFmt );
                    bDelMarked = FALSE;
                }
            }
        }

        for( i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetObj();
            if( !pObj->ISA(SwVirtFlyDrawObj) )
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
                    const SdrMark& rMark = *rMrkList.GetMark( i );
                    pObj = rMark.GetObj();
                    SwDrawContact *pContact = (SwDrawContact*)pObj->GetUserCall();
                    if( pContact ) // natuerlich nicht bei gruppierten Objekten
                    {
                        SwDrawFrmFmt *pFmt = (SwDrawFrmFmt*)pContact->GetFmt();
                        // OD 18.06.2003 #108784# - before delete of selection
                        // is performed, marked <SwDrawVirtObj>-objects have to
                        // be replaced by its reference objects.
                        // Thus, assert, if a <SwDrawVirt>-object is found in the mark list.
                        if ( pObj->ISA(SwDrawVirtObj) )
                        {
                            ASSERT( false,
                                    "<SwDrawVirtObj> is still marked for delete. application will crash!" );
                        }
                        //loescht sich selbst!
                        pContact->Changed(*pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
                        pObj->SetUserCall( 0 );

                        if( pUndo )
                            pUndo->AddObj( i, pFmt, rMark );
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
    // #i11176#
    // This also needs to work when no layout exists. Thus, for
    // FlyFrames an alternative method is used now in that case.
    SwClientIter aIter( (SwFmt&)*pFmt );

    if( RES_FLYFRMFMT == pFmt->Which() )
    {
        if( pFmt->GetDoc()->GetRootFrm() )
        {
            // Schauen, ob es ein SdrObject dafuer gibt
            if( aIter.First( TYPE( SwFlyFrm) ) )
                nOrdNum = ((SwFlyFrm*)aIter())->GetVirtDrawObj()->GetOrdNum();
        }
        else
        {
            // Schauen, ob es ein SdrObject dafuer gibt
            if( aIter.First( TYPE(SwFlyDrawContact) ) )
                nOrdNum = ((SwFlyDrawContact*)aIter())->GetMaster()->GetOrdNum();
        }
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

/*************************************************************************/
// Wird auch vom Sw3-Reader gerufen, wenn ein Fehler beim Einlesen
// des Drawing Layers auftrat. In diesem Fall wird der Layer komplett
// neu aufgebaut.

// #75371#
#ifndef _SXENDITM_HXX
#include <svx/sxenditm.hxx>
#endif

void SwDoc::InitDrawModel()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDoc::InitDrawModel" );

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
    // #75371# change DefaultItems for the SdrEdgeObj distance items
    // to TWIPS.
    if(pSdrPool)
    {
        const long nDefEdgeDist = ((500 * 72) / 127); // 1/100th mm in twips
        pSdrPool->SetPoolDefaultItem(SdrEdgeNode1HorzDistItem(nDefEdgeDist));
        pSdrPool->SetPoolDefaultItem(SdrEdgeNode1VertDistItem(nDefEdgeDist));
        pSdrPool->SetPoolDefaultItem(SdrEdgeNode2HorzDistItem(nDefEdgeDist));
        pSdrPool->SetPoolDefaultItem(SdrEdgeNode2VertDistItem(nDefEdgeDist));
    }
    SfxItemPool *pEEgPool = EditEngine::CreatePool( FALSE );
    pSdrPool->SetSecondaryPool( pEEgPool );
     if ( !aAttrPool.GetFrozenIdRanges () )
        aAttrPool.FreezeIdRanges();
    else
        pSdrPool->FreezeIdRanges();

    // SJ: #95129# set FontHeight pool defaults without changing static SdrEngineDefaults
     aAttrPool.SetPoolDefaultItem(SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT ));

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "before create DrawDocument" );
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

    // OD 25.06.2003 #108784# - add invisible layers corresponding to the
    // visible ones.
    {
        sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("InvisibleHell" ));
        nInvisibleHell   = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

        sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("InvisibleHeaven" ));
        nInvisibleHeaven = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

        sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("InvisibleControls" ));
        nInvisibleControls = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();
    }

    pDrawModel->InsertPage( pDrawModel->AllocPage( FALSE ) );
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "after create DrawDocument" );

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "before create Spellchecker/Hyphenator" );
    SdrOutliner& rOutliner = pDrawModel->GetDrawOutliner();
    uno::Reference< XSpellChecker1 > xSpell = ::GetSpellChecker();
    rOutliner.SetSpeller( xSpell );
    uno::Reference<XHyphenator> xHyphenator( ::GetHyphenator() );
    rOutliner.SetHyphenator( xHyphenator );
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "after create Spellchecker/Hyphenator" );

    SetCalcFieldValueHdl(&rOutliner);
    SetCalcFieldValueHdl(&pDrawModel->GetHitTestOutliner());

    //JP 16.07.98: Bug 50193 - Linkmanager am Model setzen, damit
    //          dort ggfs. verlinkte Grafiken eingefuegt werden koennen
    //JP 28.01.99: der WinWord Import benoetigt ihn auch
    pDrawModel->SetLinkManager( &GetLinkManager() );
    pDrawModel->SetAddExtLeading( IsAddExtLeading() );

    OutputDevice* pRefDev = _GetRefDev();
    if ( pRefDev )
        pDrawModel->SetRefDevice( pRefDev );

    pDrawModel->SetNotifyUndoActionHdl( LINK( this, SwDoc, AddDrawUndo ));
    if ( pLayout )
    {
        pLayout->SetDrawPage( pDrawModel->GetPage( 0 ) );
        pLayout->GetDrawPage()->SetSize( pLayout->Frm().SSize() );
    }
}

/** method to notify drawing page view about the invisible layers

    OD 26.06.2003 #108784#

    @author OD
*/
void SwDoc::NotifyInvisibleLayers( SdrPageView& _rSdrPageView )
{
    String sLayerNm;
    sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("InvisibleHell" ));
    _rSdrPageView.SetLayerVisible( sLayerNm, FALSE );

    sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("InvisibleHeaven" ));
    _rSdrPageView.SetLayerVisible( sLayerNm, FALSE );

    sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("InvisibleControls" ));
    _rSdrPageView.SetLayerVisible( sLayerNm, FALSE );
}

/** method to determine, if a layer ID belongs to the visible ones.

    OD 25.06.2003 #108784#
    Note: If given layer ID is unknown, method asserts and returns <false>.

    @author OD
*/
bool SwDoc::IsVisibleLayerId( const SdrLayerID& _nLayerId ) const
{
    bool bRetVal;

    if ( _nLayerId == GetHeavenId() ||
         _nLayerId == GetHellId() ||
         _nLayerId == GetControlsId() )
    {
        bRetVal = true;
    }
    else if ( _nLayerId == GetInvisibleHeavenId() ||
              _nLayerId == GetInvisibleHellId() ||
              _nLayerId == GetInvisibleControlsId() )
    {
        bRetVal = false;
    }
    else
    {
        ASSERT( false, "<SwDoc::IsVisibleLayerId(..)> - unknown layer ID." );
        bRetVal = false;
    }

    return bRetVal;
}

/** method to determine, if the corresponding visible layer ID for a invisible one.

    OD 25.06.2003 #108784#
    Note: If given layer ID is a visible one, method returns given layer ID.
    Note: If given layer ID is unknown, method returns given layer ID.

    @author OD
*/
SdrLayerID SwDoc::GetVisibleLayerIdByInvisibleOne( const SdrLayerID& _nInvisibleLayerId )
{
    SdrLayerID nVisibleLayerId;

    if ( _nInvisibleLayerId == GetInvisibleHeavenId() )
    {
        nVisibleLayerId = GetHeavenId();
    }
    else if ( _nInvisibleLayerId == GetInvisibleHellId() )
    {
        nVisibleLayerId = GetHellId();
    }
    else if ( _nInvisibleLayerId == GetInvisibleControlsId() )
    {
        nVisibleLayerId = GetControlsId();
    }
    else if ( _nInvisibleLayerId == GetHeavenId() ||
              _nInvisibleLayerId == GetHellId() ||
              _nInvisibleLayerId == GetControlsId() )
    {
        ASSERT( false, "<SwDoc::GetVisibleLayerIdByInvisibleOne(..)> - given layer ID already an invisible one." );
        nVisibleLayerId = _nInvisibleLayerId;
    }
    else
    {
        ASSERT( false, "<SwDoc::GetVisibleLayerIdByInvisibleOne(..)> - given layer ID is unknown." );
        nVisibleLayerId = _nInvisibleLayerId;
    }

    return nVisibleLayerId;
}

/** method to determine, if the corresponding invisible layer ID for a visible one.

    OD 25.06.2003 #108784#
    Note: If given layer ID is a invisible one, method returns given layer ID.
    Note: If given layer ID is unknown, method returns given layer ID.

    @author OD
*/
SdrLayerID SwDoc::GetInvisibleLayerIdByVisibleOne( const SdrLayerID& _nVisibleLayerId )
{
    SdrLayerID nInvisibleLayerId;

    if ( _nVisibleLayerId == GetHeavenId() )
    {
        nInvisibleLayerId = GetInvisibleHeavenId();
    }
    else if ( _nVisibleLayerId == GetHellId() )
    {
        nInvisibleLayerId = GetInvisibleHellId();
    }
    else if ( _nVisibleLayerId == GetControlsId() )
    {
        nInvisibleLayerId = GetInvisibleControlsId();
    }
    else if ( _nVisibleLayerId == GetInvisibleHeavenId() ||
              _nVisibleLayerId == GetInvisibleHellId() ||
              _nVisibleLayerId == GetInvisibleControlsId() )
    {
        ASSERT( false, "<SwDoc::GetInvisibleLayerIdByVisibleOne(..)> - given layer ID already an invisible one." );
        nInvisibleLayerId = _nVisibleLayerId;
    }
    else
    {
        ASSERT( false, "<SwDoc::GetInvisibleLayerIdByVisibleOne(..)> - given layer ID is unknown." );
        nInvisibleLayerId = _nVisibleLayerId;
    }

    return nInvisibleLayerId;
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

/*************************************************************************
*
* Am Outliner Link auf Methode fuer Felddarstellung in Editobjekten setzen
*
*************************************************************************/

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
                ((const SvxDateField*) pField)->GetFormatted(
                        *GetNumberFormatter( TRUE ), LANGUAGE_SYSTEM) );
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
        else if ( pField && pField->ISA(SvxExtTimeField))
        {
            /******************************************************************
            * Time-Field
            ******************************************************************/
            pInfo->SetRepresentation(
                ((const SvxExtTimeField*) pField)->GetFormatted(
                        *GetNumberFormatter( TRUE ), LANGUAGE_SYSTEM) );
        }
        else
        {
            DBG_ERROR("unbekannter Feldbefehl");
            pInfo->SetRepresentation( String( '?' ) );
        }
    }

    return(0);
}
