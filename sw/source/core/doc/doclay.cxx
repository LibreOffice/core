/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <unotools/linguprops.hxx>
#include <unotools/lingucfg.hxx>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <hintids.hxx>
#include <com/sun/star/util/XCloseable.hpp>
#include <sfx2/progress.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/prntitem.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <svx/fmpage.hxx>
#include <editeng/frmdiritem.hxx>

#include <swmodule.hxx>
#include <modcfg.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <rtl/logfile.hxx>
#include <SwStyleNameMapper.hxx>
#include <fchrfmt.hxx>
#include <frmatr.hxx>
#include <txatbase.hxx>
#include <fmtfld.hxx>
#include <fmtornt.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include <fmtflcnt.hxx>
#include <fmtcnct.hxx>
#include <frmfmt.hxx>
#include <txtflcnt.hxx>
#include <docfld.hxx>       // for ExpressionFields
#include <pam.hxx>
#include <ndtxt.hxx>
#include <ndnotxt.hxx>
#include <ndole.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <flyfrm.hxx>
#include <fesh.hxx>
#include <docsh.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <swundo.hxx>
#include <flypos.hxx>
#include <UndoInsert.hxx>
#include <expfld.hxx>       // InsertLabel
#include <poolfmt.hxx>      // PoolTemplate Ids
#include <docary.hxx>
#include <swtable.hxx>
#include <tblsel.hxx>
#include <viewopt.hxx>
#include <fldupde.hxx>
#include <txtftn.hxx>
#include <ftnidx.hxx>
#include <ftninfo.hxx>
#include <pagedesc.hxx>
#include <PostItMgr.hxx>
#include <comcore.hrc>      // STR ResIds

#include <unoframe.hxx>

#include <sortedobjs.hxx>

#include <vector>

using namespace ::com::sun::star;
using ::rtl::OUString;

#define DEF_FLY_WIDTH    2268   // Default width for FlyFrms (2268 == 4cm)


static bool lcl_IsItemSet(const SwCntntNode & rNode, sal_uInt16 which)
{
    bool bResult = false;

    if (SFX_ITEM_SET == rNode.GetSwAttrSet().GetItemState(which))
        bResult = true;

    return bResult;
}

/*************************************************************************
|*
|*  SwDoc::MakeLayoutFmt()
|*
|*  Description: Create a new format whose settings fit to the Request by
|*  default.
|*  The format is put into the respective format array.
|*  If there already is a fitting format, it is returned instead.
|*************************************************************************/
SwFrmFmt *SwDoc::MakeLayoutFmt( RndStdIds eRequest, const SfxItemSet* pSet )
{
    SwFrmFmt *pFmt = 0;
    const sal_Bool bMod = IsModified();
    sal_Bool bHeader = sal_False;

    switch ( eRequest )
    {
    case RND_STD_HEADER:
    case RND_STD_HEADERL:
    case RND_STD_HEADERR:
        {
            bHeader = sal_True;
            // no break, we continue further down
        }
    case RND_STD_FOOTER:
    case RND_STD_FOOTERL:
    case RND_STD_FOOTERR:
        {
            pFmt = new SwFrmFmt( GetAttrPool(),
                                 (bHeader ? "Right header" : "Right footer"),
                                 GetDfltFrmFmt() );

            SwNodeIndex aTmpIdx( GetNodes().GetEndOfAutotext() );
            SwStartNode* pSttNd =
                GetNodes().MakeTextSection
                ( aTmpIdx,
                  bHeader ? SwHeaderStartNode : SwFooterStartNode,
                  GetTxtCollFromPool(static_cast<sal_uInt16>( bHeader
                                     ? ( eRequest == RND_STD_HEADERL
                                         ? RES_POOLCOLL_HEADERL
                                         : eRequest == RND_STD_HEADERR
                                         ? RES_POOLCOLL_HEADERR
                                         : RES_POOLCOLL_HEADER )
                                     : ( eRequest == RND_STD_FOOTERL
                                         ? RES_POOLCOLL_FOOTERL
                                         : eRequest == RND_STD_FOOTERR
                                         ? RES_POOLCOLL_FOOTERR
                                         : RES_POOLCOLL_FOOTER )
                                     ) ) );
            pFmt->SetFmtAttr( SwFmtCntnt( pSttNd ));

            if( pSet )      // Set a few more attributes
                pFmt->SetFmtAttr( *pSet );

            // Why set it back?  Doc has changed, or not?
            // In any case, wrong for the FlyFrames!
            if ( !bMod )
                ResetModified();
        }
        break;

    case RND_DRAW_OBJECT:
        {
            pFmt = MakeDrawFrmFmt( aEmptyStr, GetDfltFrmFmt() );
            if( pSet )      // Set a few more attributes
                pFmt->SetFmtAttr( *pSet );

            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(
                    new SwUndoInsLayFmt(pFmt, 0, 0));
            }
        }
        break;

#if OSL_DEBUG_LEVEL > 0
    case FLY_AT_PAGE:
    case FLY_AT_CHAR:
    case FLY_AT_FLY:
    case FLY_AT_PARA:
    case FLY_AS_CHAR:
        OSL_FAIL( "use new interface instead: SwDoc::MakeFlySection!" );
        break;
#endif

    default:
        OSL_ENSURE( !this,
                "LayoutFormat was requested with an invalid Request." );

    }
    return pFmt;
}

/*************************************************************************
|*
|*  SwDoc::DelLayoutFmt()
|*
|*  Description: Deletes the denoted format and its content.
|*
|*************************************************************************/
void SwDoc::DelLayoutFmt( SwFrmFmt *pFmt )
{
    // A chain of frames needs to be merged, if necessary,
    // so that the Frame's contents are adjusted accordingly before we destroy the Frames.
    const SwFmtChain &rChain = pFmt->GetChain();
    if ( rChain.GetPrev() )
    {
        SwFmtChain aChain( rChain.GetPrev()->GetChain() );
        aChain.SetNext( rChain.GetNext() );
        SetAttr( aChain, *rChain.GetPrev() );
    }
    if ( rChain.GetNext() )
    {
        SwFmtChain aChain( rChain.GetNext()->GetChain() );
        aChain.SetPrev( rChain.GetPrev() );
        SetAttr( aChain, *rChain.GetNext() );
    }

    const SwNodeIndex* pCntIdx = pFmt->GetCntnt().GetCntntIdx();
    if (pCntIdx && !GetIDocumentUndoRedo().DoesUndo())
    {
        // Disconnect if it's an OLE object
        SwOLENode* pOLENd = GetNodes()[ pCntIdx->GetIndex()+1 ]->GetOLENode();
        if( pOLENd && pOLENd->GetOLEObj().IsOleRef() )
        {

            // TODO: the old object closed the object and cleared all references to it, but didn't remove it from the container.
            // I have no idea, why, nobody could explain it - so I do my very best to mimic this behavior
            //uno::Reference < util::XCloseable > xClose( pOLENd->GetOLEObj().GetOleRef(), uno::UNO_QUERY );
            //if ( xClose.is() )
            {
                try
                {
                    pOLENd->GetOLEObj().GetOleRef()->changeState( embed::EmbedStates::LOADED );
                }
                catch ( uno::Exception& )
                {
                }
            }

        }
    }

    // Destroy Frames
    pFmt->DelFrms();

    // Only FlyFrames are undoable at first
    const sal_uInt16 nWh = pFmt->Which();
    if (GetIDocumentUndoRedo().DoesUndo() &&
        (RES_FLYFRMFMT == nWh || RES_DRAWFRMFMT == nWh))
    {
        GetIDocumentUndoRedo().AppendUndo( new SwUndoDelLayFmt( pFmt ));
    }
    else
    {
        // #i32089# - delete at-frame anchored objects
        if ( nWh == RES_FLYFRMFMT )
        {
            // determine frame formats of at-frame anchored objects
            const SwNodeIndex* pCntntIdx = pFmt->GetCntnt().GetCntntIdx();
            if ( pCntntIdx )
            {
                const SwFrmFmts* pTbl = pFmt->GetDoc()->GetSpzFrmFmts();
                if ( pTbl )
                {
                    std::vector<SwFrmFmt*> aToDeleteFrmFmts;
                    const sal_uLong nNodeIdxOfFlyFmt( pCntntIdx->GetIndex() );

                    for ( sal_uInt16 i = 0; i < pTbl->size(); ++i )
                    {
                        SwFrmFmt* pTmpFmt = (*pTbl)[i];
                        const SwFmtAnchor &rAnch = pTmpFmt->GetAnchor();
                        if ( rAnch.GetAnchorId() == FLY_AT_FLY &&
                             rAnch.GetCntntAnchor()->nNode.GetIndex() == nNodeIdxOfFlyFmt )
                        {
                            aToDeleteFrmFmts.push_back( pTmpFmt );
                        }
                    }

                    // delete found frame formats
                    while ( !aToDeleteFrmFmts.empty() )
                    {
                        SwFrmFmt* pTmpFmt = aToDeleteFrmFmts.back();
                        pFmt->GetDoc()->DelLayoutFmt( pTmpFmt );

                        aToDeleteFrmFmts.pop_back();
                    }
                }
            }
        }

        // Delete content
        if( pCntIdx )
        {
            SwNode *pNode = &pCntIdx->GetNode();
            ((SwFmtCntnt&)pFmt->GetFmtAttr( RES_CNTNT )).SetNewCntntIdx( 0 );
            DeleteSection( pNode );
        }

        // Delete the character for FlyFrames anchored as char (if necessary)
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
        if ((FLY_AS_CHAR == rAnchor.GetAnchorId()) && rAnchor.GetCntntAnchor())
        {
            const SwPosition* pPos = rAnchor.GetCntntAnchor();
            SwTxtNode *pTxtNd = pPos->nNode.GetNode().GetTxtNode();

            // attribute is still in text node, delete it
            if ( pTxtNd )
            {
                SwTxtFlyCnt* const pAttr = static_cast<SwTxtFlyCnt*>(
                    pTxtNd->GetTxtAttrForCharAt( pPos->nContent.GetIndex(),
                        RES_TXTATR_FLYCNT ));
                if ( pAttr && (pAttr->GetFlyCnt().GetFrmFmt() == pFmt) )
                {
                    // dont delete, set pointer to 0
                    const_cast<SwFmtFlyCnt&>(pAttr->GetFlyCnt()).SetFlyFmt();
                    SwIndex aIdx( pPos->nContent );
                    pTxtNd->EraseText( aIdx, 1 );
                }
            }
        }

        DelFrmFmt( pFmt );
    }
    SetModified();
}

/*************************************************************************
|*
|*  SwDoc::CopyLayoutFmt()
|*
|*  Copies the stated format (pSrc) to pDest and returns pDest.
|*  If there's no pDest, it is created.
|*  If the source format is located in another document, also copy correctly
|*  in this case.
|*  The Anchor attribute's position is always set to 0!
|*
|*************************************************************************/
SwFrmFmt *SwDoc::CopyLayoutFmt( const SwFrmFmt& rSource,
                                const SwFmtAnchor& rNewAnchor,
                                bool bSetTxtFlyAtt, bool bMakeFrms )
{
    const bool bFly = RES_FLYFRMFMT == rSource.Which();
    const bool bDraw = RES_DRAWFRMFMT == rSource.Which();
    OSL_ENSURE( bFly || bDraw, "this method only works for fly or draw" );

    SwDoc* pSrcDoc = (SwDoc*)rSource.GetDoc();

    // May we copy this object?
    // We may, unless it's 1) it's a control (and therfore a draw)
    //                     2) anchored in a header/footer
    //                     3) anchored (to paragraph?)
    bool bMayNotCopy = false;
    if( bDraw )
    {
        const SwDrawContact* pDrawContact =
            static_cast<const SwDrawContact*>( rSource.FindContactObj() );

        bMayNotCopy =
            ((FLY_AT_PARA == rNewAnchor.GetAnchorId()) ||
             (FLY_AT_FLY  == rNewAnchor.GetAnchorId()) ||
             (FLY_AT_CHAR == rNewAnchor.GetAnchorId())) &&
            rNewAnchor.GetCntntAnchor() &&
            IsInHeaderFooter( rNewAnchor.GetCntntAnchor()->nNode ) &&
            pDrawContact != NULL  &&
            pDrawContact->GetMaster() != NULL  &&
            CheckControlLayer( pDrawContact->GetMaster() );
    }

    // just return if we can't copy this
    if( bMayNotCopy )
        return NULL;

    SwFrmFmt* pDest = GetDfltFrmFmt();
    if( rSource.GetRegisteredIn() != pSrcDoc->GetDfltFrmFmt() )
        pDest = CopyFrmFmt( *(SwFrmFmt*)rSource.GetRegisteredIn() );
    if( bFly )
    {
        // #i11176#
        // To do a correct cloning concerning the ZOrder for all objects
        // it is necessary to actually create a draw object for fly frames, too.
        // These are then added to the DrawingLayer (which needs to exist).
        // Together with correct sorting of all drawinglayer based objects
        // before cloning ZOrder transfer works correctly then.
        SwFlyFrmFmt *pFormat = MakeFlyFrmFmt( rSource.GetName(), pDest );
        pDest = pFormat;

        SwXFrame::GetOrCreateSdrObject(pFormat);
    }
    else
        pDest = MakeDrawFrmFmt( aEmptyStr, pDest );

    // Copy all other or new attributes
    pDest->CopyAttrs( rSource );

    // Do not copy chains
    pDest->ResetFmtAttr( RES_CHAIN );

    if( bFly )
    {
        // Duplicate the content.
        const SwNode& rCSttNd = rSource.GetCntnt().GetCntntIdx()->GetNode();
        SwNodeRange aRg( rCSttNd, 1, *rCSttNd.EndOfSectionNode() );

        SwNodeIndex aIdx( GetNodes().GetEndOfAutotext() );
        SwStartNode* pSttNd = GetNodes().MakeEmptySection( aIdx, SwFlyStartNode );

        // Set the Anchor/CntntIndex first.
        // Within the copying part, we can access the values (DrawFmt in Headers and Footers)
        aIdx = *pSttNd;
        SwFmtCntnt aAttr( rSource.GetCntnt() );
        aAttr.SetNewCntntIdx( &aIdx );
        pDest->SetFmtAttr( aAttr );
        pDest->SetFmtAttr( rNewAnchor );

        if( !mbCopyIsMove || this != pSrcDoc )
        {
            if( mbInReading )
                pDest->SetName( aEmptyStr );
            else
            {
                // Test first if the name is already taken, if so generate a new one.
                sal_Int8 nNdTyp = aRg.aStart.GetNode().GetNodeType();

                String sOld( pDest->GetName() );
                pDest->SetName( aEmptyStr );
                if( FindFlyByName( sOld, nNdTyp ) )     // found one
                    switch( nNdTyp )
                    {
                    case ND_GRFNODE:    sOld = GetUniqueGrfName();      break;
                    case ND_OLENODE:    sOld = GetUniqueOLEName();      break;
                    default:            sOld = GetUniqueFrameName();    break;
                    }

                pDest->SetName( sOld );
            }
        }

        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo(new SwUndoInsLayFmt(pDest,0,0));
        }

        // Make sure that FlyFrames in FlyFrames are copied
        aIdx = *pSttNd->EndOfSectionNode();

        //fdo#36631 disable (scoped) any undo operations associated with the
        //contact object itself. They should be managed by SwUndoInsLayFmt.
        const ::sw::DrawUndoGuard drawUndoGuard(GetIDocumentUndoRedo());

        pSrcDoc->CopyWithFlyInFly( aRg, 0, aIdx, sal_False, sal_True, sal_True );
    }
    else
    {
        OSL_ENSURE( RES_DRAWFRMFMT == rSource.Which(), "Neither Fly nor Draw." );
        // #i52780# - Note: moving object to visible layer not needed.
        SwDrawContact* pSourceContact = (SwDrawContact *)rSource.FindContactObj();

        SwDrawContact* pContact = new SwDrawContact( (SwDrawFrmFmt*)pDest,
                                CloneSdrObj( *pSourceContact->GetMaster(),
                                        mbCopyIsMove && this == pSrcDoc ) );
        // #i49730# - notify draw frame format that position attributes are
        // already set, if the position attributes are already set at the
        // source draw frame format.
        if ( pDest->ISA(SwDrawFrmFmt) &&
             rSource.ISA(SwDrawFrmFmt) &&
             static_cast<const SwDrawFrmFmt&>(rSource).IsPosAttrSet() )
        {
            static_cast<SwDrawFrmFmt*>(pDest)->PosAttrSet();
        }

        if( pDest->GetAnchor() == rNewAnchor )
        {
            // Do *not* connect to layout, if a <MakeFrms> will not be called.
            if ( bMakeFrms )
            {
                pContact->ConnectToLayout( &rNewAnchor );
            }
        }
        else
            pDest->SetFmtAttr( rNewAnchor );

        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo(new SwUndoInsLayFmt(pDest,0,0));
        }
    }

    if (bSetTxtFlyAtt && (FLY_AS_CHAR == rNewAnchor.GetAnchorId()))
    {
        const SwPosition* pPos = rNewAnchor.GetCntntAnchor();
        SwFmtFlyCnt aFmt( pDest );
        pPos->nNode.GetNode().GetTxtNode()->InsertItem(
            aFmt, pPos->nContent.GetIndex(), 0 );
    }

    if( bMakeFrms )
        pDest->MakeFrms();

    return pDest;
}

SdrObject* SwDoc::CloneSdrObj( const SdrObject& rObj, sal_Bool bMoveWithinDoc,
                                sal_Bool bInsInPage )
{
    // #i52858# - method name changed
    SdrPage *pPg = GetOrCreateDrawModel()->GetPage( 0 );
    if( !pPg )
    {
        pPg = GetDrawModel()->AllocPage( sal_False );
        GetDrawModel()->InsertPage( pPg );
    }

    SdrObject *pObj = rObj.Clone();
    if( bMoveWithinDoc && FmFormInventor == pObj->GetObjInventor() )
    {
        // We need to preserve the Name for Controls
        uno::Reference< awt::XControlModel >  xModel = ((SdrUnoObj*)pObj)->GetUnoControlModel();
        uno::Any aVal;
        uno::Reference< beans::XPropertySet >  xSet(xModel, uno::UNO_QUERY);
        OUString sName(RTL_CONSTASCII_USTRINGPARAM("Name"));
        if( xSet.is() )
            aVal = xSet->getPropertyValue( sName );
        if( bInsInPage )
            pPg->InsertObject( pObj );
        if( xSet.is() )
            xSet->setPropertyValue( sName, aVal );
    }
    else if( bInsInPage )
        pPg->InsertObject( pObj );

    // For drawing objects: set layer of cloned object to invisible layer
    SdrLayerID nLayerIdForClone = rObj.GetLayer();
    if ( !pObj->ISA(SwFlyDrawObj) &&
         !pObj->ISA(SwVirtFlyDrawObj) &&
         !IS_TYPE(SdrObject,pObj) )
    {
        if ( IsVisibleLayerId( nLayerIdForClone ) )
        {
            nLayerIdForClone = GetInvisibleLayerIdByVisibleOne( nLayerIdForClone );
        }
    }
    pObj->SetLayer( nLayerIdForClone );


    return pObj;
}

SwFlyFrmFmt* SwDoc::_MakeFlySection( const SwPosition& rAnchPos,
                                    const SwCntntNode& rNode,
                                    RndStdIds eRequestId,
                                    const SfxItemSet* pFlySet,
                                    SwFrmFmt* pFrmFmt )
{
    if( !pFrmFmt )
        pFrmFmt = GetFrmFmtFromPool( RES_POOLFRM_FRAME );

    String sName;
    if( !mbInReading )
        switch( rNode.GetNodeType() )
        {
        case ND_GRFNODE:        sName = GetUniqueGrfName();     break;
        case ND_OLENODE:        sName = GetUniqueOLEName();     break;
        default:                sName = GetUniqueFrameName();       break;
        }
    SwFlyFrmFmt* pFmt = MakeFlyFrmFmt( sName, pFrmFmt );

    // Create content and connect to the format.
    // Create CntntNode and put it into the autotext selection.
    SwNodeRange aRange( GetNodes().GetEndOfAutotext(), -1,
                        GetNodes().GetEndOfAutotext() );
    GetNodes().SectionDown( &aRange, SwFlyStartNode );

    pFmt->SetFmtAttr( SwFmtCntnt( rNode.StartOfSectionNode() ));


    const SwFmtAnchor* pAnchor = 0;
    if( pFlySet )
    {
        pFlySet->GetItemState( RES_ANCHOR, sal_False,
                                (const SfxPoolItem**)&pAnchor );
        if( SFX_ITEM_SET == pFlySet->GetItemState( RES_CNTNT, sal_False ))
        {
            SfxItemSet aTmpSet( *pFlySet );
            aTmpSet.ClearItem( RES_CNTNT );
            pFmt->SetFmtAttr( aTmpSet );
        }
        else
            pFmt->SetFmtAttr( *pFlySet );
    }

    // Anchor not yet set?
    RndStdIds eAnchorId = pAnchor ? pAnchor->GetAnchorId()
                                  : pFmt->GetAnchor().GetAnchorId();
    // #i107811# Assure that at-page anchored fly frames have a page num or a
    // content anchor set.
    if ( !pAnchor ||
         ( FLY_AT_PAGE != pAnchor->GetAnchorId() &&
           !pAnchor->GetCntntAnchor() ) ||
         ( FLY_AT_PAGE == pAnchor->GetAnchorId() &&
           !pAnchor->GetCntntAnchor() &&
           pAnchor->GetPageNum() == 0 ) )
    {
        // set it again, needed for Undo
        SwFmtAnchor aAnch( pFmt->GetAnchor() );
        if (pAnchor && (FLY_AT_FLY == pAnchor->GetAnchorId()))
        {
            SwPosition aPos( *rAnchPos.nNode.GetNode().FindFlyStartNode() );
            aAnch.SetAnchor( &aPos );
            eAnchorId = FLY_AT_FLY;
        }
        else
        {
            if( eRequestId != aAnch.GetAnchorId() &&
                SFX_ITEM_SET != pFmt->GetItemState( RES_ANCHOR, sal_True ) )
            {
                aAnch.SetType( eRequestId );
            }

            eAnchorId = aAnch.GetAnchorId();
            if ( FLY_AT_PAGE != eAnchorId ||
                 ( FLY_AT_PAGE == eAnchorId &&
                   ( !pAnchor ||
                     aAnch.GetPageNum() == 0 ) ) )
            {
                aAnch.SetAnchor( &rAnchPos );
            }
        }
        pFmt->SetFmtAttr( aAnch );
    }
    else
        eAnchorId = pFmt->GetAnchor().GetAnchorId();

    if ( FLY_AS_CHAR == eAnchorId )
    {
        xub_StrLen nStt = rAnchPos.nContent.GetIndex();
        SwTxtNode * pTxtNode = rAnchPos.nNode.GetNode().GetTxtNode();

        OSL_ENSURE(pTxtNode!= 0, "There should be a SwTxtNode!");

        if (pTxtNode != NULL)
        {
            SwFmtFlyCnt aFmt( pFmt );
            pTxtNode->InsertItem( aFmt, nStt, nStt );
        }
    }

    if( SFX_ITEM_SET != pFmt->GetAttrSet().GetItemState( RES_FRM_SIZE ))
    {
        SwFmtFrmSize aFmtSize( ATT_VAR_SIZE, 0, DEF_FLY_WIDTH );
        const SwNoTxtNode* pNoTxtNode = rNode.GetNoTxtNode();
        if( pNoTxtNode )
        {
            // Set size
             Size aSize( pNoTxtNode->GetTwipSize() );
            if( MINFLY > aSize.Width() )
                aSize.Width() = DEF_FLY_WIDTH;
            aFmtSize.SetWidth( aSize.Width() );
            if( aSize.Height() )
            {
                aFmtSize.SetHeight( aSize.Height() );
                aFmtSize.SetHeightSizeType( ATT_FIX_SIZE );
            }
        }
        pFmt->SetFmtAttr( aFmtSize );
    }

    // Set up frames
    if( GetCurrentViewShell() )
        pFmt->MakeFrms();           // ???  //swmod 071108//swmod 071225

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        sal_uLong nNodeIdx = rAnchPos.nNode.GetIndex();
        xub_StrLen nCntIdx = rAnchPos.nContent.GetIndex();
        GetIDocumentUndoRedo().AppendUndo(
            new SwUndoInsLayFmt( pFmt, nNodeIdx, nCntIdx ));
    }

    SetModified();
    return pFmt;
}

SwFlyFrmFmt* SwDoc::MakeFlySection( RndStdIds eAnchorType,
                                    const SwPosition* pAnchorPos,
                                    const SfxItemSet* pFlySet,
                                    SwFrmFmt* pFrmFmt, sal_Bool bCalledFromShell )
{
    SwFlyFrmFmt* pFmt = 0;
    sal_Bool bCallMake = sal_True;
    if ( !pAnchorPos && (FLY_AT_PAGE != eAnchorType) )
    {
        const SwFmtAnchor* pAnch;
        if( (pFlySet && SFX_ITEM_SET == pFlySet->GetItemState(
                RES_ANCHOR, sal_False, (const SfxPoolItem**)&pAnch )) ||
            ( pFrmFmt && SFX_ITEM_SET == pFrmFmt->GetItemState(
                RES_ANCHOR, sal_True, (const SfxPoolItem**)&pAnch )) )
        {
            if ( (FLY_AT_PAGE != pAnch->GetAnchorId()) )
            {
                pAnchorPos = pAnch->GetCntntAnchor();
                if (pAnchorPos)
                {
                    bCallMake = sal_False;
                }
            }
        }
    }

    if( bCallMake )
    {
        if( !pFrmFmt )
            pFrmFmt = GetFrmFmtFromPool( RES_POOLFRM_FRAME );

        sal_uInt16 nCollId = static_cast<sal_uInt16>(
            get(IDocumentSettingAccess::HTML_MODE) ? RES_POOLCOLL_TEXT : RES_POOLCOLL_FRAME );

        /* If there is no adjust item in the paragraph style for the content node of the new fly section
           propagate an existing adjust item at the anchor to the new content node. */
        SwCntntNode * pNewTxtNd = GetNodes().MakeTxtNode
            (SwNodeIndex( GetNodes().GetEndOfAutotext()),
             GetTxtCollFromPool( nCollId ));
        SwCntntNode * pAnchorNode = pAnchorPos->nNode.GetNode().GetCntntNode();

        const SfxPoolItem * pItem = NULL;

        if (bCalledFromShell && !lcl_IsItemSet(*pNewTxtNd, RES_PARATR_ADJUST) &&
            SFX_ITEM_SET == pAnchorNode->GetSwAttrSet().
            GetItemState(RES_PARATR_ADJUST, sal_True, &pItem))
            static_cast<SwCntntNode *>(pNewTxtNd)->SetAttr(*pItem);

         pFmt = _MakeFlySection( *pAnchorPos, *pNewTxtNd,
                                eAnchorType, pFlySet, pFrmFmt );
    }
    return pFmt;
}

SwFlyFrmFmt* SwDoc::MakeFlyAndMove( const SwPaM& rPam, const SfxItemSet& rSet,
                                    const SwSelBoxes* pSelBoxes,
                                    SwFrmFmt *pParent )
{
    SwFmtAnchor& rAnch = (SwFmtAnchor&)rSet.Get( RES_ANCHOR );

    GetIDocumentUndoRedo().StartUndo( UNDO_INSLAYFMT, NULL );

    SwFlyFrmFmt* pFmt = MakeFlySection( rAnch.GetAnchorId(), rPam.GetPoint(),
                                        &rSet, pParent );

    // If content is selected, it becomes the new frame's content.
    // Namely, it is moved into the NodeArray's appropriate section.

    if( pFmt )
    {
        do {        // middle check loop
            const SwFmtCntnt &rCntnt = pFmt->GetCntnt();
            OSL_ENSURE( rCntnt.GetCntntIdx(), "No content prepared." );
            SwNodeIndex aIndex( *(rCntnt.GetCntntIdx()), 1 );
            SwCntntNode *pNode = aIndex.GetNode().GetCntntNode();

            // Attention: Do not create an index on the stack, or we
            // cannot delete CntntNode in the end!
            SwPosition aPos( aIndex );
            aPos.nContent.Assign( pNode, 0 );

            if( pSelBoxes && !pSelBoxes->empty() )
            {
                // Table selection
                // Copy parts of a table: create a table with the same width as the
                // original one and move (copy and delete) the selected boxes.
                // The size is corrected on a percentage basis.

                SwTableNode* pTblNd = (SwTableNode*)(*pSelBoxes)[0]->
                                                GetSttNd()->FindTableNode();
                if( !pTblNd )
                    break;

                SwTable& rTbl = pTblNd->GetTable();

                // Did we select the whole table?
                if( pSelBoxes->size() == rTbl.GetTabSortBoxes().size() )
                {
                    // move the whole table
                    SwNodeRange aRg( *pTblNd, 0, *pTblNd->EndOfSectionNode(), 1 );

                    // If we move the whole table and it is located within a
                    // FlyFrame, the we create a TextNode after it.
                    // So that this FlyFrame is preserved.
                    if( aRg.aEnd.GetNode().IsEndNode() )
                        GetNodes().MakeTxtNode( aRg.aStart,
                                    (SwTxtFmtColl*)GetDfltTxtFmtColl() );

                    MoveNodeRange( aRg, aPos.nNode, DOC_MOVEDEFAULT );
                }
                else
                {
                    rTbl.MakeCopy( this, aPos, *pSelBoxes );
                    // Don't delete a part of a table with row span!!
                    // You could delete the content instead -> ToDo
                    //rTbl.DeleteSel( this, *pSelBoxes, 0, 0, sal_True, sal_True );
                }

                // If the table is within the frame, then copy without the following TextNode
                aIndex = rCntnt.GetCntntIdx()->GetNode().EndOfSectionIndex() - 1;
                OSL_ENSURE( aIndex.GetNode().GetTxtNode(),
                        "a TextNode should be here" );
                aPos.nContent.Assign( 0, 0 );       // Deregister index!
                GetNodes().Delete( aIndex, 1 );

// This is a hack: whilst FlyFrames/Headers/Footers are not undoable we delete all Undo objects
if( GetIDocumentUndoRedo().DoesUndo() )
{
    GetIDocumentUndoRedo().DelAllUndoObj();
}

            }
            else
            {
                // copy all Pams and then delete all
                SwPaM* pTmp = (SwPaM*)&rPam;
                sal_Bool bOldFlag = mbCopyIsMove;
                bool const bOldUndo = GetIDocumentUndoRedo().DoesUndo();
                bool const bOldRedlineMove(IsRedlineMove());
                mbCopyIsMove = sal_True;
                GetIDocumentUndoRedo().DoUndo(false);
                SetRedlineMove(true);
                do {
                    if( pTmp->HasMark() &&
                        *pTmp->GetPoint() != *pTmp->GetMark() )
                    {
                        CopyRange( *pTmp, aPos, false );
                    }
                    pTmp = static_cast<SwPaM*>(pTmp->GetNext());
                } while ( &rPam != pTmp );
                SetRedlineMove(bOldRedlineMove);
                mbCopyIsMove = bOldFlag;
                GetIDocumentUndoRedo().DoUndo(bOldUndo);

                pTmp = (SwPaM*)&rPam;
                do {
                    if( pTmp->HasMark() &&
                        *pTmp->GetPoint() != *pTmp->GetMark() )
                    {
                        DeleteAndJoin( *pTmp );
                    }
                    pTmp = static_cast<SwPaM*>(pTmp->GetNext());
                } while ( &rPam != pTmp );
            }
        } while( sal_False );
    }

    SetModified();

    GetIDocumentUndoRedo().EndUndo( UNDO_INSLAYFMT, NULL );

    return pFmt;
}

// Insert a DrawObject.
// The Object has to be already registered in the DrawModel.
SwDrawFrmFmt* SwDoc::Insert( const SwPaM &rRg,
                             SdrObject& rDrawObj,
                             const SfxItemSet* pFlyAttrSet,
                             SwFrmFmt* pDefFmt )
{
    SwDrawFrmFmt *pFmt = MakeDrawFrmFmt( aEmptyStr,
                                        pDefFmt ? pDefFmt : GetDfltFrmFmt() );

    const SwFmtAnchor* pAnchor = 0;
    if( pFlyAttrSet )
    {
        pFlyAttrSet->GetItemState( RES_ANCHOR, sal_False,
                                    (const SfxPoolItem**)&pAnchor );
        pFmt->SetFmtAttr( *pFlyAttrSet );
    }

    RndStdIds eAnchorId = pAnchor ? pAnchor->GetAnchorId()
                                  : pFmt->GetAnchor().GetAnchorId();

    // Didn't set the Anchor yet?
    // DrawObjecte must never end up in the Header/Footer!
    const bool bIsAtCntnt = (FLY_AT_PAGE != eAnchorId);

    const SwNodeIndex* pChkIdx = 0;
    if( !pAnchor )
    {
        pChkIdx = &rRg.GetPoint()->nNode;
    }
    else if( bIsAtCntnt )
    {
        pChkIdx = pAnchor->GetCntntAnchor()
                    ? &pAnchor->GetCntntAnchor()->nNode
                    : &rRg.GetPoint()->nNode;
    }

    // Allow drawing objects in header/footer, but control objects aren't
    // allowed in header/footer.
    if( pChkIdx &&
        ::CheckControlLayer( &rDrawObj ) &&
        IsInHeaderFooter( *pChkIdx ) )
    {
       pFmt->SetFmtAttr( SwFmtAnchor( eAnchorId = FLY_AT_PAGE ) );
    }
    else if( !pAnchor || (bIsAtCntnt && !pAnchor->GetCntntAnchor() ))
    {
        // then set it, we need this in the Undo
        SwFmtAnchor aAnch( pAnchor ? *pAnchor : pFmt->GetAnchor() );
        eAnchorId = aAnch.GetAnchorId();
        if( FLY_AT_FLY == eAnchorId )
        {
            SwPosition aPos( *rRg.GetNode()->FindFlyStartNode() );
            aAnch.SetAnchor( &aPos );
        }
        else
        {
            aAnch.SetAnchor( rRg.GetPoint() );
            if ( FLY_AT_PAGE == eAnchorId )
            {
                eAnchorId = rDrawObj.ISA( SdrUnoObj )
                                    ? FLY_AS_CHAR : FLY_AT_PARA;
                aAnch.SetType( eAnchorId );
            }
        }
        pFmt->SetFmtAttr( aAnch );
    }

    // For Draws anchored as character we set the attribute in the paragraph
    if ( FLY_AS_CHAR == eAnchorId )
    {
        xub_StrLen nStt = rRg.GetPoint()->nContent.GetIndex();
        SwFmtFlyCnt aFmt( pFmt );
        rRg.GetPoint()->nNode.GetNode().GetTxtNode()->InsertItem(
                aFmt, nStt, nStt );
    }

    SwDrawContact* pContact = new SwDrawContact( pFmt, &rDrawObj );

    // Create Frames if necessary
    if( GetCurrentViewShell() )
    {
        pFmt->MakeFrms();
        // #i42319# - follow-up of #i35635#
        // move object to visible layer
        // #i79391#
        if ( pContact->GetAnchorFrm() )
        {
            pContact->MoveObjToVisibleLayer( &rDrawObj );
        }
    }

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo( new SwUndoInsLayFmt(pFmt, 0, 0) );
    }

    SetModified();
    return pFmt;
}

/* ---------------------------------------------------------------------------
    paragraph frames - o.k. if the PaM includes the paragraph from the beginning
                       to the beginning of the next paragraph at least
    frames at character - o.k. if the PaM starts at least at the same position
                         as the frame
 ---------------------------------------------------------------------------*/
sal_Bool TstFlyRange( const SwPaM* pPam, const SwPosition* pFlyPos,
                        RndStdIds nAnchorId )
{
    sal_Bool bOk = sal_False;
    const SwPaM* pTmp = pPam;
    do {
        const sal_uInt32 nFlyIndex = pFlyPos->nNode.GetIndex();
        const SwPosition* pPaMStart = pTmp->Start();
        const SwPosition* pPaMEnd = pTmp->End();
        const sal_uInt32 nPamStartIndex = pPaMStart->nNode.GetIndex();
        const sal_uInt32 nPamEndIndex = pPaMEnd->nNode.GetIndex();
        if (FLY_AT_PARA == nAnchorId)
            bOk = (nPamStartIndex < nFlyIndex && nPamEndIndex > nFlyIndex) ||
               (((nPamStartIndex == nFlyIndex) && (pPaMStart->nContent.GetIndex() == 0)) &&
               (nPamEndIndex > nFlyIndex));
        else
        {
            xub_StrLen nFlyContentIndex = pFlyPos->nContent.GetIndex();
            xub_StrLen nPamEndContentIndex = pPaMEnd->nContent.GetIndex();
            bOk = (nPamStartIndex < nFlyIndex &&
                (( nPamEndIndex > nFlyIndex )||
                 ((nPamEndIndex == nFlyIndex) &&
                  (nPamEndContentIndex > nFlyContentIndex))) )
                ||
                       (((nPamStartIndex == nFlyIndex) &&
                      (pPaMStart->nContent.GetIndex() <= nFlyContentIndex)) &&
                     ((nPamEndIndex > nFlyIndex) ||
                     (nPamEndContentIndex > nFlyContentIndex )));
        }

    } while( !bOk && pPam != ( pTmp = (const SwPaM*)pTmp->GetNext() ));
    return bOk;
}

void SwDoc::GetAllFlyFmts( SwPosFlyFrms& rPosFlyFmts,
                           const SwPaM* pCmpRange, sal_Bool bDrawAlso,
                           sal_Bool bAsCharAlso ) const
{
    SwPosFlyFrm *pFPos = 0;
    SwFrmFmt *pFly;

    // collect all anchored somehow to paragraphs
    for( sal_uInt16 n = 0; n < GetSpzFrmFmts()->size(); ++n )
    {
        pFly = (*GetSpzFrmFmts())[ n ];
        bool bDrawFmt = bDrawAlso ? RES_DRAWFRMFMT == pFly->Which() : false;
        bool bFlyFmt = RES_FLYFRMFMT == pFly->Which();
        if( bFlyFmt || bDrawFmt )
        {
            const SwFmtAnchor& rAnchor = pFly->GetAnchor();
            SwPosition const*const pAPos = rAnchor.GetCntntAnchor();
            if (pAPos &&
                ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
                 (FLY_AT_FLY  == rAnchor.GetAnchorId()) ||
                 (FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
                 ((FLY_AS_CHAR == rAnchor.GetAnchorId()) && bAsCharAlso)))
            {
                if( pCmpRange &&
                    !TstFlyRange( pCmpRange, pAPos, rAnchor.GetAnchorId() ))
                        continue;       // not a valid FlyFrame
                pFPos = new SwPosFlyFrm( pAPos->nNode, pFly, rPosFlyFmts.size() );
                rPosFlyFmts.insert( pFPos );
            }
        }
    }

    // If we don't have a layout we can't get page anchored FlyFrames.
    // Also, page anchored FlyFrames are only returned if no range is specified.
    if( !GetCurrentViewShell() || pCmpRange )   //swmod 071108//swmod 071225
        return;

    pFPos = 0;
    SwPageFrm *pPage = (SwPageFrm*)GetCurrentLayout()->GetLower();  //swmod 080218
    while( pPage )
    {
        if( pPage->GetSortedObjs() )
        {
            SwSortedObjs &rObjs = *pPage->GetSortedObjs();
            for( sal_uInt16 i = 0; i < rObjs.Count(); ++i)
            {
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                    pFly = &(pAnchoredObj->GetFrmFmt());
                else if ( bDrawAlso )
                    pFly = &(pAnchoredObj->GetFrmFmt());
                else
                    continue;

                const SwFmtAnchor& rAnchor = pFly->GetAnchor();
                if ((FLY_AT_PARA != rAnchor.GetAnchorId()) &&
                    (FLY_AT_FLY  != rAnchor.GetAnchorId()) &&
                    (FLY_AT_CHAR != rAnchor.GetAnchorId()))
                {
                    const SwCntntFrm * pCntntFrm = pPage->FindFirstBodyCntnt();
                    if ( !pCntntFrm )
                    {
                        // Oops! An empty page.
                        // In order not to loose the whole frame (RTF) we
                        // look for the last Cntnt before the page.
                        SwPageFrm *pPrv = (SwPageFrm*)pPage->GetPrev();
                        while ( !pCntntFrm && pPrv )
                        {
                            pCntntFrm = pPrv->FindFirstBodyCntnt();
                            pPrv = (SwPageFrm*)pPrv->GetPrev();
                        }
                    }
                    if ( pCntntFrm )
                    {
                        SwNodeIndex aIdx( *pCntntFrm->GetNode() );
                        pFPos = new SwPosFlyFrm( aIdx, pFly, rPosFlyFmts.size() );
                    }
                }
                if ( pFPos )
                {
                    rPosFlyFmts.insert( pFPos );
                    pFPos = 0;
                }
            }
        }
        pPage = (SwPageFrm*)pPage->GetNext();
    }
}

/*************************************************************************
|*
|*  SwDoc::InsertLabel()
|*
|*************************************************************************/

/* #i6447# changed behaviour if lcl_CpyAttr:

   If the old item set contains the item to set (no inheritance) copy the item
   into the new set.

   If the old item set contains the item by inheritance and the new set
   contains the item, too:
   If the two items differ copy the item from the old set to the new set.

   Otherwise the new set will not be changed.
*/
static void lcl_CpyAttr( SfxItemSet &rNewSet, const SfxItemSet &rOldSet, sal_uInt16 nWhich )
{
    const SfxPoolItem *pOldItem = NULL, *pNewItem = NULL;

    rOldSet.GetItemState( nWhich, sal_False, &pOldItem);
    if (pOldItem != NULL)
        rNewSet.Put( *pOldItem );
    else
    {
        pOldItem = rOldSet.GetItem( nWhich, sal_True);
        if (pOldItem != NULL)
        {
            pNewItem = rNewSet.GetItem( nWhich, sal_True);
            if (pNewItem != NULL)
            {
                if (*pOldItem != *pNewItem)
                    rNewSet.Put( *pOldItem );
            }
            else {
                OSL_FAIL("What am I doing here?");
            }
        }
        else {
            OSL_FAIL("What am I doing here?");
        }
    }

}

static SwFlyFrmFmt *
lcl_InsertLabel(SwDoc & rDoc, SwTxtFmtColls *const pTxtFmtCollTbl,
        SwUndoInsertLabel *const pUndo,
        SwLabelType const eType, String const& rTxt, String const& rSeparator,
            const String& rNumberingSeparator,
            const sal_Bool bBefore, const sal_uInt16 nId, const sal_uLong nNdIdx,
            const String& rCharacterStyle,
            const sal_Bool bCpyBrd )
{
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

    sal_Bool bTable = sal_False;    // To save some code.

    // Get the field first, beause we retrieve the TxtColl via the field's name
    OSL_ENSURE( nId == USHRT_MAX  || nId < rDoc.GetFldTypes()->size(),
            "FldType index out of bounds." );
    SwFieldType *pType = (nId != USHRT_MAX) ? (*rDoc.GetFldTypes())[nId] : NULL;
    OSL_ENSURE(!pType || pType->Which() == RES_SETEXPFLD, "wrong Id for Label");

    SwTxtFmtColl * pColl = NULL;
    if( pType )
    {
        for( sal_uInt16 i = pTxtFmtCollTbl->size(); i; )
        {
            if( (*pTxtFmtCollTbl)[ --i ]->GetName().Equals(pType->GetName()) )
            {
                pColl = (*pTxtFmtCollTbl)[i];
                break;
            }
        }
        OSL_ENSURE( pColl, "no text collection found" );
    }

    if( !pColl )
    {
        pColl = rDoc.GetTxtCollFromPool( RES_POOLCOLL_LABEL );
    }

    SwTxtNode *pNew = NULL;
    SwFlyFrmFmt* pNewFmt = NULL;

    switch ( eType )
    {
        case LTYPE_TABLE:
            bTable = sal_True;
            // no break here
        case LTYPE_FLY:
            // At the FlySection's Beginning/End insert the corresponding Node with it's Field.
            // The Frame is created automatically.
            {
                SwStartNode *pSttNd = rDoc.GetNodes()[nNdIdx]->GetStartNode();
                OSL_ENSURE( pSttNd, "No StartNode in InsertLabel." );
                sal_uLong nNode;
                if( bBefore )
                {
                    nNode = pSttNd->GetIndex();
                    if( !bTable )
                        ++nNode;
                }
                else
                {
                    nNode = pSttNd->EndOfSectionIndex();
                    if( bTable )
                        ++nNode;
                }

                if( pUndo )
                    pUndo->SetNodePos( nNode );

                // Create Node for labeling paragraph.
                SwNodeIndex aIdx( rDoc.GetNodes(), nNode );
                pNew = rDoc.GetNodes().MakeTxtNode( aIdx, pColl );
            }
            break;

        case LTYPE_OBJECT:
            {
                // Destroy Frame,
                // insert new Frame,
                // insert the corresponding Node with Field into the new Frame,
                // insert the old Frame with the Object (Picture/OLE) paragraph-bound into the new Frame,
                // create Frames.

                // Get the FlyFrame's Format and decouple the Layout.
                SwFrmFmt *pOldFmt = rDoc.GetNodes()[nNdIdx]->GetFlyFmt();
                OSL_ENSURE( pOldFmt, "Couldn't find the Fly's Format." );
                // #i115719#
                // <title> and <description> attributes are lost when calling <DelFrms()>.
                // Thus, keep them and restore them after the calling <MakeFrms()>
                const bool bIsSwFlyFrmFmtInstance( dynamic_cast<SwFlyFrmFmt*>(pOldFmt) != 0 );
                const String sTitle( bIsSwFlyFrmFmtInstance
                                     ? static_cast<SwFlyFrmFmt*>(pOldFmt)->GetObjTitle()
                                     : String() );
                const String sDescription( bIsSwFlyFrmFmtInstance
                                           ? static_cast<SwFlyFrmFmt*>(pOldFmt)->GetObjDescription()
                                           : String() );
                pOldFmt->DelFrms();

                pNewFmt = rDoc.MakeFlyFrmFmt( rDoc.GetUniqueFrameName(),
                                rDoc.GetFrmFmtFromPool(RES_POOLFRM_FRAME) );

                /* #i6447#: Only the selected items are copied from the old
                   format. */
                SfxItemSet* pNewSet = pNewFmt->GetAttrSet().Clone( sal_True );


                // Copy only the set attributes.
                // The others should apply from the Templates.
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_PRINT );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_OPAQUE );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_PROTECT );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_SURROUND );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_VERT_ORIENT );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_HORI_ORIENT );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_LR_SPACE );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_UL_SPACE );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_BACKGROUND );
                if( bCpyBrd )
                {
                    // If there's no BoxItem at graphic, but the new Format has one, then set the
                    // default item in the new Set. Because the graphic's size has never changed!
                    const SfxPoolItem *pItem;
                    if( SFX_ITEM_SET == pOldFmt->GetAttrSet().
                            GetItemState( RES_BOX, sal_True, &pItem ))
                        pNewSet->Put( *pItem );
                    else if( SFX_ITEM_SET == pNewFmt->GetAttrSet().
                            GetItemState( RES_BOX, sal_True ))
                        pNewSet->Put( *GetDfltAttr( RES_BOX ) );

                    if( SFX_ITEM_SET == pOldFmt->GetAttrSet().
                            GetItemState( RES_SHADOW, sal_True, &pItem ))
                        pNewSet->Put( *pItem );
                    else if( SFX_ITEM_SET == pNewFmt->GetAttrSet().
                            GetItemState( RES_SHADOW, sal_True ))
                        pNewSet->Put( *GetDfltAttr( RES_SHADOW ) );
                }
                else
                {
                    // Hard-set the attributes, because they could come from the Template
                    // and then size calculations could not be correct anymore.
                    pNewSet->Put( SvxBoxItem(RES_BOX) );
                    pNewSet->Put( SvxShadowItem(RES_SHADOW) );

                }

                // Always transfer the anchor, which is a hard attribute anyways.
                pNewSet->Put( pOldFmt->GetAnchor() );

                // The new one should be changeable in it's height.
                SwFmtFrmSize aFrmSize( pOldFmt->GetFrmSize() );
                aFrmSize.SetHeightSizeType( ATT_MIN_SIZE );
                pNewSet->Put( aFrmSize );

                SwStartNode* pSttNd = rDoc.GetNodes().MakeTextSection(
                            SwNodeIndex( rDoc.GetNodes().GetEndOfAutotext() ),
                            SwFlyStartNode, pColl );
                pNewSet->Put( SwFmtCntnt( pSttNd ));

                pNewFmt->SetFmtAttr( *pNewSet );

                // InCntnts need to be treated in a special way:
                // The TxtAttribute needs to be destroyed.
                // Unfortunately, this also destroys the Format next to the Frames.
                // To avoid this, we disconnect the attribute from the Format.

                const SwFmtAnchor& rAnchor = pNewFmt->GetAnchor();
                if ( FLY_AS_CHAR == rAnchor.GetAnchorId() )
                {
                    const SwPosition *pPos = rAnchor.GetCntntAnchor();
                    SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
                    OSL_ENSURE( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
                    const xub_StrLen nIdx = pPos->nContent.GetIndex();
                    SwTxtAttr * const pHnt =
                        pTxtNode->GetTxtAttrForCharAt(nIdx, RES_TXTATR_FLYCNT);

                    OSL_ENSURE( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
                                "Missing FlyInCnt-Hint." );
                    OSL_ENSURE( pHnt && pHnt->GetFlyCnt().GetFrmFmt() == pOldFmt,
                                "Wrong TxtFlyCnt-Hint." );

                    const_cast<SwFmtFlyCnt&>(pHnt->GetFlyCnt()).SetFlyFmt(
                            pNewFmt );
                }


                // The old one should not have a flow and it should be adjusted to above and
                // middle.
                // Also, the width should be 100% and it should also adjust the hight, if changed.
                pNewSet->ClearItem();

                pNewSet->Put( SwFmtSurround( SURROUND_NONE ) );
                pNewSet->Put( SvxOpaqueItem( RES_OPAQUE, sal_True ) );
                pNewSet->Put( SwFmtVertOrient( text::VertOrientation::TOP ) );
                pNewSet->Put( SwFmtHoriOrient( text::HoriOrientation::CENTER ) );

                aFrmSize = pOldFmt->GetFrmSize();
                aFrmSize.SetWidthPercent( 100 );
                aFrmSize.SetHeightPercent( 255 );
                pNewSet->Put( aFrmSize );

                // Hard-set the attributes, because they could come from the Template
                // and then size calculations could not be correct anymore.
                if( bCpyBrd )
                {
                    pNewSet->Put( SvxBoxItem(RES_BOX) );
                    pNewSet->Put( SvxShadowItem(RES_SHADOW) );
                }
                pNewSet->Put( SvxLRSpaceItem(RES_LR_SPACE) );
                pNewSet->Put( SvxULSpaceItem(RES_UL_SPACE) );

                // The old one is paragraph-bound to the paragraph in the new one.
                SwFmtAnchor aAnch( FLY_AT_PARA );
                SwNodeIndex aAnchIdx( *pNewFmt->GetCntnt().GetCntntIdx(), 1 );
                pNew = aAnchIdx.GetNode().GetTxtNode();
                SwPosition aPos( aAnchIdx );
                aAnch.SetAnchor( &aPos );
                pNewSet->Put( aAnch );

                if( pUndo )
                    pUndo->SetFlys( *pOldFmt, *pNewSet, *pNewFmt );
                else
                    pOldFmt->SetFmtAttr( *pNewSet );

                delete pNewSet;

                // Have only the FlyFrames created.
                // We leave this to established methods (especially for InCntFlys).
                pNewFmt->MakeFrms();
                // #i115719#
                if ( bIsSwFlyFrmFmtInstance )
                {
                    static_cast<SwFlyFrmFmt*>(pOldFmt)->SetObjTitle( sTitle );
                    static_cast<SwFlyFrmFmt*>(pOldFmt)->SetObjDescription( sDescription );
                }
            }
            break;

        default:
            OSL_ENSURE(false, "unknown LabelType?");
    }
    OSL_ENSURE( pNew, "No Label inserted" );
    if( pNew )
    {
        // #i61007# order of captions
        sal_Bool bOrderNumberingFirst = SW_MOD()->GetModuleConfig()->IsCaptionOrderNumberingFirst();
        // Work up string
        String aTxt;
        if( bOrderNumberingFirst )
        {
            aTxt = rNumberingSeparator;
        }
        if( pType)
        {
            aTxt += pType->GetName();
            if( !bOrderNumberingFirst )
                aTxt += ' ';
        }
        xub_StrLen nIdx = aTxt.Len();
        if( rTxt.Len() > 0 )
        {
            aTxt += rSeparator;
        }
        xub_StrLen nSepIdx = aTxt.Len();
        aTxt += rTxt;

        // Insert string
        SwIndex aIdx( pNew, 0 );
        pNew->InsertText( aTxt, aIdx );

        // Insert field
        if(pType)
        {
            SwSetExpField aFld( (SwSetExpFieldType*)pType, aEmptyStr, SVX_NUM_ARABIC);
            if( bOrderNumberingFirst )
                nIdx = 0;
            SwFmtFld aFmt( aFld );
            pNew->InsertItem( aFmt, nIdx, nIdx );
            if(rCharacterStyle.Len())
            {
                SwCharFmt* pCharFmt = rDoc.FindCharFmtByName(rCharacterStyle);
                if( !pCharFmt )
                {
                    const sal_uInt16 nMyId = SwStyleNameMapper::GetPoolIdFromUIName(rCharacterStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
                    pCharFmt = rDoc.GetCharFmtFromPool( nMyId );
                }
                if (pCharFmt)
                {
                    SwFmtCharFmt aCharFmt( pCharFmt );
                    pNew->InsertItem( aCharFmt, 0,
                        nSepIdx + 1, nsSetAttrMode::SETATTR_DONTEXPAND );
                }
            }
        }

        if ( bTable )
        {
            if ( bBefore )
            {
                if ( !pNew->GetSwAttrSet().GetKeep().GetValue()  )
                    pNew->SetAttr( SvxFmtKeepItem( sal_True, RES_KEEP ) );
            }
            else
            {
                SwTableNode *const pNd =
                    rDoc.GetNodes()[nNdIdx]->GetStartNode()->GetTableNode();
                SwTable &rTbl = pNd->GetTable();
                if ( !rTbl.GetFrmFmt()->GetKeep().GetValue() )
                    rTbl.GetFrmFmt()->SetFmtAttr( SvxFmtKeepItem( sal_True, RES_KEEP ) );
                if ( pUndo )
                    pUndo->SetUndoKeep();
            }
        }
        rDoc.SetModified();
    }

    return pNewFmt;
}

SwFlyFrmFmt *
SwDoc::InsertLabel(
        SwLabelType const eType, String const& rTxt, String const& rSeparator,
        String const& rNumberingSeparator,
        sal_Bool const bBefore, sal_uInt16 const nId, sal_uLong const nNdIdx,
        String const& rCharacterStyle,
        sal_Bool const bCpyBrd )
{
    SwUndoInsertLabel * pUndo(0);
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoInsertLabel(
                        eType, rTxt, rSeparator, rNumberingSeparator,
                        bBefore, nId, rCharacterStyle, bCpyBrd );
    }

    SwFlyFrmFmt *const pNewFmt = lcl_InsertLabel(*this, pTxtFmtCollTbl, pUndo,
            eType, rTxt, rSeparator, rNumberingSeparator, bBefore,
            nId, nNdIdx, rCharacterStyle, bCpyBrd);

    if (pUndo)
    {
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }
    else
    {
        GetIDocumentUndoRedo().DelAllUndoObj();
    }

    return pNewFmt;
}


/*************************************************************************
|*
|*  SwDoc::InsertDrawLabel()
|*
|*************************************************************************/
static SwFlyFrmFmt *
lcl_InsertDrawLabel( SwDoc & rDoc, SwTxtFmtColls *const pTxtFmtCollTbl,
        SwUndoInsertLabel *const pUndo, SwDrawFrmFmt *const pOldFmt,
        String const& rTxt,
                                     const String& rSeparator,
                                     const String& rNumberSeparator,
                                     const sal_uInt16 nId,
                                     const String& rCharacterStyle,
                                     SdrObject& rSdrObj )
{
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
    ::sw::DrawUndoGuard const drawUndoGuard(rDoc.GetIDocumentUndoRedo());

    // Because we get by the TxtColl's name, we need to create the field first.
    OSL_ENSURE( nId == USHRT_MAX  || nId < rDoc.GetFldTypes()->size(),
            "FldType index out of bounds" );
    SwFieldType *pType = nId != USHRT_MAX ? (*rDoc.GetFldTypes())[nId] : 0;
    OSL_ENSURE( !pType || pType->Which() == RES_SETEXPFLD, "Wrong label id" );

    SwTxtFmtColl *pColl = NULL;
    if( pType )
    {
        for( sal_uInt16 i = pTxtFmtCollTbl->size(); i; )
        {
            if( (*pTxtFmtCollTbl)[ --i ]->GetName().Equals(pType->GetName()) )
            {
                pColl = (*pTxtFmtCollTbl)[i];
                break;
            }
        }
        OSL_ENSURE( pColl, "no text collection found" );
    }

    if( !pColl )
    {
        pColl = rDoc.GetTxtCollFromPool( RES_POOLCOLL_LABEL );
    }

    SwTxtNode* pNew = NULL;
    SwFlyFrmFmt* pNewFmt = NULL;

    // Destroy Frame,
    // insert new Frame,
    // insert the corresponding Node with Field into the new Frame,
    // insert the old Frame with the Object (Picture/OLE) paragraph-bound into the new Frame,
    // create Frames.

    // Keep layer ID of drawing object before removing
    // its frames.
    // Note: The layer ID is passed to the undo and have to be the correct value.
    //       Removing the frames of the drawing object changes its layer.
    const SdrLayerID nLayerId = rSdrObj.GetLayer();

    pOldFmt->DelFrms();

    // InCntnts need to be treated in a special way:
    // The TxtAttribute needs to be destroyed.
    // Unfortunately, this also destroys the Format next to the Frames.
    // To avoid this, we disconnect the attribute from the Format.
    SfxItemSet* pNewSet = pOldFmt->GetAttrSet().Clone( sal_False );

    // Protect the Frame's size and position
    if ( rSdrObj.IsMoveProtect() || rSdrObj.IsResizeProtect() )
    {
        SvxProtectItem aProtect(RES_PROTECT);
        aProtect.SetCntntProtect( sal_False );
        aProtect.SetPosProtect( rSdrObj.IsMoveProtect() );
        aProtect.SetSizeProtect( rSdrObj.IsResizeProtect() );
        pNewSet->Put( aProtect );
    }

    // Take over the text wrap
    lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_SURROUND );

    // Send the frame to the back, if needed.
    // Consider the 'invisible' hell layer.
    if ( rDoc.GetHellId() != nLayerId &&
         rDoc.GetInvisibleHellId() != nLayerId )
    {
        SvxOpaqueItem aOpaque( RES_OPAQUE );
        aOpaque.SetValue( sal_True );
        pNewSet->Put( aOpaque );
    }

    // Take over position
    // #i26791# - use directly drawing object's positioning attributes
    pNewSet->Put( pOldFmt->GetHoriOrient() );
    pNewSet->Put( pOldFmt->GetVertOrient() );

    pNewSet->Put( pOldFmt->GetAnchor() );

    // The new one should be variable in it's height!
     Size aSz( rSdrObj.GetCurrentBoundRect().GetSize() );
    SwFmtFrmSize aFrmSize( ATT_MIN_SIZE, aSz.Width(), aSz.Height() );
    pNewSet->Put( aFrmSize );

    // Apply the margin to the new Frame.
    // Don't set a border, use the one from the Template.
    pNewSet->Put( pOldFmt->GetLRSpace() );
    pNewSet->Put( pOldFmt->GetULSpace() );

    SwStartNode* pSttNd =
        rDoc.GetNodes().MakeTextSection(
            SwNodeIndex( rDoc.GetNodes().GetEndOfAutotext() ),
                                    SwFlyStartNode, pColl );

    pNewFmt = rDoc.MakeFlyFrmFmt( rDoc.GetUniqueFrameName(),
                 rDoc.GetFrmFmtFromPool( RES_POOLFRM_FRAME ) );

    // Set border and shadow to default if the template contains any.
    if( SFX_ITEM_SET == pNewFmt->GetAttrSet().GetItemState( RES_BOX, sal_True ))
        pNewSet->Put( *GetDfltAttr( RES_BOX ) );

    if( SFX_ITEM_SET == pNewFmt->GetAttrSet().GetItemState(RES_SHADOW,sal_True))
        pNewSet->Put( *GetDfltAttr( RES_SHADOW ) );

    pNewFmt->SetFmtAttr( SwFmtCntnt( pSttNd ));
    pNewFmt->SetFmtAttr( *pNewSet );

    const SwFmtAnchor& rAnchor = pNewFmt->GetAnchor();
    if ( FLY_AS_CHAR == rAnchor.GetAnchorId() )
    {
        const SwPosition *pPos = rAnchor.GetCntntAnchor();
        SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
        OSL_ENSURE( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
        const xub_StrLen nIdx = pPos->nContent.GetIndex();
        SwTxtAttr * const pHnt =
            pTxtNode->GetTxtAttrForCharAt( nIdx, RES_TXTATR_FLYCNT );

#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
                    "Missing FlyInCnt-Hint." );
        OSL_ENSURE( pHnt && ((SwFmtFlyCnt&)pHnt->GetFlyCnt()).
                    GetFrmFmt() == (SwFrmFmt*)pOldFmt,
                    "Wrong TxtFlyCnt-Hint." );
#endif
        const_cast<SwFmtFlyCnt&>(pHnt->GetFlyCnt()).SetFlyFmt( pNewFmt );
    }


    // The old one should not have a flow
    // and it should be adjusted to above and middle.
    pNewSet->ClearItem();

    pNewSet->Put( SwFmtSurround( SURROUND_NONE ) );
    if (nLayerId == rDoc.GetHellId())
    {
    // Consider drawing objects in the 'invisible' hell layer
        rSdrObj.SetLayer( rDoc.GetHeavenId() );
    }
    else if (nLayerId == rDoc.GetInvisibleHellId())
    {
        rSdrObj.SetLayer( rDoc.GetInvisibleHeavenId() );
    }
    pNewSet->Put( SvxLRSpaceItem( RES_LR_SPACE ) );
    pNewSet->Put( SvxULSpaceItem( RES_UL_SPACE ) );

    // #i26791# - set position of the drawing object, which is labeled.
    pNewSet->Put( SwFmtVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::FRAME ) );
    pNewSet->Put( SwFmtHoriOrient( 0, text::HoriOrientation::CENTER, text::RelOrientation::FRAME ) );

    // The old one is paragraph-bound to the new one's paragraph.
    SwFmtAnchor aAnch( FLY_AT_PARA );
    SwNodeIndex aAnchIdx( *pNewFmt->GetCntnt().GetCntntIdx(), 1 );
    pNew = aAnchIdx.GetNode().GetTxtNode();
    SwPosition aPos( aAnchIdx );
    aAnch.SetAnchor( &aPos );
    pNewSet->Put( aAnch );

    if( pUndo )
    {
        pUndo->SetFlys( *pOldFmt, *pNewSet, *pNewFmt );
        // #i26791# - position no longer needed
        pUndo->SetDrawObj( nLayerId );
    }
    else
        pOldFmt->SetFmtAttr( *pNewSet );

    delete pNewSet;

    // Have only the FlyFrames created.
    // We leave this to established methods (especially for InCntFlys).
    pNewFmt->MakeFrms();

    OSL_ENSURE( pNew, "No Label inserted" );

    if( pNew )
    {
        //#i61007# order of captions
        sal_Bool bOrderNumberingFirst = SW_MOD()->GetModuleConfig()->IsCaptionOrderNumberingFirst();

        // prepare string
        String aTxt;
        if( bOrderNumberingFirst )
        {
            aTxt = rNumberSeparator;
        }
        if ( pType )
        {
            aTxt += pType->GetName();
            if( !bOrderNumberingFirst )
                aTxt += ' ';
        }
        xub_StrLen nIdx = aTxt.Len();
        aTxt += rSeparator;
        xub_StrLen nSepIdx = aTxt.Len();
        aTxt += rTxt;

        // insert text
        SwIndex aIdx( pNew, 0 );
        pNew->InsertText( aTxt, aIdx );

        // insert field
        if ( pType )
        {
            SwSetExpField aFld( (SwSetExpFieldType*)pType, aEmptyStr, SVX_NUM_ARABIC );
            if( bOrderNumberingFirst )
                nIdx = 0;
            SwFmtFld aFmt( aFld );
            pNew->InsertItem( aFmt, nIdx, nIdx );
            if ( rCharacterStyle.Len() )
            {
                SwCharFmt * pCharFmt = rDoc.FindCharFmtByName(rCharacterStyle);
                if ( !pCharFmt )
                {
                    const sal_uInt16 nMyId = SwStyleNameMapper::GetPoolIdFromUIName( rCharacterStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
                    pCharFmt = rDoc.GetCharFmtFromPool( nMyId );
                }
                if ( pCharFmt )
                {
                    SwFmtCharFmt aCharFmt( pCharFmt );
                    pNew->InsertItem( aCharFmt, 0, nSepIdx + 1,
                            nsSetAttrMode::SETATTR_DONTEXPAND );
                }
            }
        }
    }

    return pNewFmt;
}

SwFlyFrmFmt* SwDoc::InsertDrawLabel(
        String const& rTxt,
        String const& rSeparator,
        String const& rNumberSeparator,
        sal_uInt16 const nId,
        String const& rCharacterStyle,
        SdrObject& rSdrObj )
{
    SwDrawContact *const pContact =
        static_cast<SwDrawContact*>(GetUserCall( &rSdrObj ));
    OSL_ENSURE( RES_DRAWFRMFMT == pContact->GetFmt()->Which(),
            "InsertDrawLabel(): not a DrawFrmFmt" );
    if (!pContact)
        return 0;

    SwDrawFrmFmt* pOldFmt = (SwDrawFrmFmt *)pContact->GetFmt();
    if (!pOldFmt)
        return 0;

    SwUndoInsertLabel * pUndo = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo();
        pUndo = new SwUndoInsertLabel(
            LTYPE_DRAW, rTxt, rSeparator, rNumberSeparator, sal_False,
            nId, rCharacterStyle, sal_False );
    }

    SwFlyFrmFmt *const pNewFmt = lcl_InsertDrawLabel(
        *this, pTxtFmtCollTbl, pUndo, pOldFmt,
        rTxt, rSeparator, rNumberSeparator, nId, rCharacterStyle, rSdrObj);

    if (pUndo)
    {
        GetIDocumentUndoRedo().AppendUndo( pUndo );
    }
    else
    {
        GetIDocumentUndoRedo().DelAllUndoObj();
    }

    return pNewFmt;
}


/*************************************************************************
|*
|*  IDocumentTimerAccess methods
|*
|*************************************************************************/
void SwDoc::StartIdling()
{
    mbStartIdleTimer = sal_True;
    if( !mIdleBlockCount )
        aIdleTimer.Start();
}

void SwDoc::StopIdling()
{
    mbStartIdleTimer = sal_False;
    aIdleTimer.Stop();
}

void SwDoc::BlockIdling()
{
    aIdleTimer.Stop();
    ++mIdleBlockCount;
}

void SwDoc::UnblockIdling()
{
    --mIdleBlockCount;
    if( !mIdleBlockCount && mbStartIdleTimer && !aIdleTimer.IsActive() )
        aIdleTimer.Start();
}

/*************************************************************************
|*
|*  SwDoc::DoIdleJobs()
|*
|*************************************************************************/
IMPL_LINK( SwDoc, DoIdleJobs, Timer *, pTimer )
{
#ifdef TIMELOG
    static ::rtl::Logfile* pModLogFile = 0;
    if( !pModLogFile )
        pModLogFile = new ::rtl::Logfile( "First DoIdleJobs" );
#endif

    SwRootFrm* pTmpRoot = GetCurrentLayout();//swmod 080219
    if( pTmpRoot &&
        !SfxProgress::GetActiveProgress( pDocShell ) )
    {
        ViewShell *pSh, *pStartSh;
        pSh = pStartSh = GetCurrentViewShell();
        do {
            if( pSh->ActionPend() )
            {
                if( pTimer )
                    pTimer->Start();
                return 0;
            }
            pSh = (ViewShell*)pSh->GetNext();
        } while( pSh != pStartSh );

        if( pTmpRoot->IsNeedGrammarCheck() )
        {
            sal_Bool bIsOnlineSpell = pSh->GetViewOptions()->IsOnlineSpell();
            sal_Bool bIsAutoGrammar = sal_False;
            SvtLinguConfig().GetProperty( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        UPN_IS_GRAMMAR_AUTO )) ) >>= bIsAutoGrammar;

            if (bIsOnlineSpell && bIsAutoGrammar)
                StartGrammarChecking( *this );
        }
        SwFldUpdateFlags nFldUpdFlag;
        std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();//swmod 080320
        std::set<SwRootFrm*>::iterator pLayIter = aAllLayouts.begin();
        for ( ;pLayIter != aAllLayouts.end();++pLayIter )
        {
            if ((*pLayIter)->IsIdleFormat())
            {
                (*pLayIter)->GetCurrShell()->LayoutIdle();
                break;
            }
        }
        bool bAllValid = pLayIter == aAllLayouts.end() ? 1 : 0;
        if( bAllValid && ( AUTOUPD_FIELD_ONLY ==
                 ( nFldUpdFlag = getFieldUpdateFlags(true) )
                    || AUTOUPD_FIELD_AND_CHARTS == nFldUpdFlag ) &&
                GetUpdtFlds().IsFieldsDirty() &&
                !GetUpdtFlds().IsInUpdateFlds() &&
                !IsExpFldsLocked()
                // If we switch the field name the Fields are not updated.
                // So the "backgorund update" should always be carried out
                /* && !pStartSh->GetViewOptions()->IsFldName()*/ )
        {
            //  Action brackets!
            GetUpdtFlds().SetInUpdateFlds( sal_True );

            pTmpRoot->StartAllAction();

            // no jump on update of fields #i85168#
            const sal_Bool bOldLockView = pStartSh->IsViewLocked();
            pStartSh->LockView( sal_True );

            GetSysFldType( RES_CHAPTERFLD )->ModifyNotification( 0, 0 );    // ChapterField
            UpdateExpFlds( 0, sal_False );      // Updates ExpressionFields
            UpdateTblFlds(NULL);                // Tables
            UpdateRefFlds(NULL);                // References

            pTmpRoot->EndAllAction();

            pStartSh->LockView( bOldLockView );

            GetUpdtFlds().SetInUpdateFlds( sal_False );
            GetUpdtFlds().SetFieldsDirty( sal_False );
        }
    }   //swmod 080219
#ifdef TIMELOG
    if( pModLogFile && 1 != (long)pModLogFile )
        delete pModLogFile, ((long&)pModLogFile) = 1;
#endif
    if( pTimer )
        pTimer->Start();
    return 0;
}

IMPL_STATIC_LINK( SwDoc, BackgroundDone, SvxBrushItem*, EMPTYARG )
{
    ViewShell *pSh, *pStartSh;
    pSh = pStartSh = pThis->GetCurrentViewShell();  //swmod 071108//swmod 071225
    if( pStartSh )
        do {
            if( pSh->GetWin() )
            {
                // Make sure to repaint with virtual device
                pSh->LockPaint();
                pSh->UnlockPaint( sal_True );
            }
            pSh = (ViewShell*)pSh->GetNext();
        } while( pSh != pStartSh );
    return 0;
}

static String lcl_GetUniqueFlyName( const SwDoc* pDoc, sal_uInt16 nDefStrId )
{
    ResId aId( nDefStrId, *pSwResMgr );
    String aName( aId );
    xub_StrLen nNmLen = aName.Len();

    const SwFrmFmts& rFmts = *pDoc->GetSpzFrmFmts();

    sal_uInt16 nNum, nTmp, nFlagSize = ( rFmts.size() / 8 ) +2;
    sal_uInt8* pSetFlags = new sal_uInt8[ nFlagSize ];
    sal_uInt16 n;

    memset( pSetFlags, 0, nFlagSize );

    for( n = 0; n < rFmts.size(); ++n )
    {
        const SwFrmFmt* pFlyFmt = rFmts[ n ];
        if( RES_FLYFRMFMT == pFlyFmt->Which() &&
            pFlyFmt->GetName().Match( aName ) == nNmLen )
        {
            // Only get and set the Flag
            nNum = static_cast< sal_uInt16 >( pFlyFmt->GetName().Copy( nNmLen ).ToInt32() );
            if( nNum-- && nNum < rFmts.size() )
                pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
        }
    }

    // All numbers are flagged accordingly, so determine the right one
    nNum = rFmts.size();
    for( n = 0; n < nFlagSize; ++n )
        if( 0xff != ( nTmp = pSetFlags[ n ] ))
        {
            // so determine the number
            nNum = n * 8;
            while( nTmp & 1 )
                ++nNum, nTmp >>= 1;
            break;
        }

    delete [] pSetFlags;
    return aName += String::CreateFromInt32( ++nNum );
}

String SwDoc::GetUniqueGrfName() const
{
    return lcl_GetUniqueFlyName( this, STR_GRAPHIC_DEFNAME );
}

String SwDoc::GetUniqueOLEName() const
{
    return lcl_GetUniqueFlyName( this, STR_OBJECT_DEFNAME );
}

String SwDoc::GetUniqueFrameName() const
{
    return lcl_GetUniqueFlyName( this, STR_FRAME_DEFNAME );
}

const SwFlyFrmFmt* SwDoc::FindFlyByName( const String& rName, sal_Int8 nNdTyp ) const
{
    const SwFrmFmts& rFmts = *GetSpzFrmFmts();
    for( sal_uInt16 n = rFmts.size(); n; )
    {
        const SwFrmFmt* pFlyFmt = rFmts[ --n ];
        const SwNodeIndex* pIdx;
        if( RES_FLYFRMFMT == pFlyFmt->Which() && pFlyFmt->GetName() == rName &&
            0 != ( pIdx = pFlyFmt->GetCntnt().GetCntntIdx() ) &&
            pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            if( nNdTyp )
            {
                // query for the right NodeType
                const SwNode* pNd = GetNodes()[ pIdx->GetIndex()+1 ];
                if( nNdTyp == ND_TEXTNODE
                        ? !pNd->IsNoTxtNode()
                        : nNdTyp == pNd->GetNodeType() )
                    return (SwFlyFrmFmt*)pFlyFmt;
            }
            else
                return (SwFlyFrmFmt*)pFlyFmt;
        }
    }
    return 0;
}

void SwDoc::SetFlyName( SwFlyFrmFmt& rFmt, const String& rName )
{
    String sName( rName );
    if( !rName.Len() || FindFlyByName( rName ) )
    {
        sal_uInt16 nTyp = STR_FRAME_DEFNAME;
        const SwNodeIndex* pIdx = rFmt.GetCntnt().GetCntntIdx();
        if( pIdx && pIdx->GetNode().GetNodes().IsDocNodes() )
            switch( GetNodes()[ pIdx->GetIndex() + 1 ]->GetNodeType() )
            {
            case ND_GRFNODE:    nTyp = STR_GRAPHIC_DEFNAME; break;
            case ND_OLENODE:    nTyp = STR_OBJECT_DEFNAME;  break;
            }
        sName = lcl_GetUniqueFlyName( this, nTyp );
    }
    rFmt.SetName( sName, sal_True );
    SetModified();
}

void SwDoc::SetAllUniqueFlyNames()
{
    sal_uInt16 n, nFlyNum = 0, nGrfNum = 0, nOLENum = 0;

    ResId nFrmId( STR_FRAME_DEFNAME, *pSwResMgr ),
          nGrfId( STR_GRAPHIC_DEFNAME, *pSwResMgr ),
          nOLEId( STR_OBJECT_DEFNAME, *pSwResMgr );
    String sFlyNm( nFrmId );
    String sGrfNm( nGrfId );
    String sOLENm( nOLEId );

    if( 255 < ( n = GetSpzFrmFmts()->size() ))
        n = 255;
    SwFrmFmts aArr;
    aArr.reserve( n );
    SwFrmFmt* pFlyFmt;
    sal_Bool bLoadedFlag = sal_True;            // something for the Layout

    for( n = GetSpzFrmFmts()->size(); n; )
    {
        if( RES_FLYFRMFMT == (pFlyFmt = (*GetSpzFrmFmts())[ --n ])->Which() )
        {
            sal_uInt16 *pNum = 0;
            xub_StrLen nLen;
            const String& rNm = pFlyFmt->GetName();
            if( rNm.Len() )
            {
                if( rNm.Match( sGrfNm ) == ( nLen = sGrfNm.Len() ))
                    pNum = &nGrfNum;
                else if( rNm.Match( sFlyNm ) == ( nLen = sFlyNm.Len() ))
                    pNum = &nFlyNum;
                else if( rNm.Match( sOLENm ) == ( nLen = sOLENm.Len() ))
                    pNum = &nOLENum;

                if ( pNum && *pNum < ( nLen = static_cast< xub_StrLen >( rNm.Copy( nLen ).ToInt32() ) ) )
                    *pNum = nLen;
            }
            else
                // we want to set that afterwards
                aArr.push_back( pFlyFmt );

        }
        if( bLoadedFlag )
        {
            const SwFmtAnchor& rAnchor = pFlyFmt->GetAnchor();
            if (((FLY_AT_PAGE == rAnchor.GetAnchorId()) &&
                 rAnchor.GetCntntAnchor()) ||
                // Or are DrawObjects adjusted relatively to something?
                ( RES_DRAWFRMFMT == pFlyFmt->Which() && (
                    SFX_ITEM_SET == pFlyFmt->GetItemState(
                                        RES_VERT_ORIENT )||
                    SFX_ITEM_SET == pFlyFmt->GetItemState(
                                        RES_HORI_ORIENT ))) )
            {
                bLoadedFlag = sal_False;
            }
        }
    }

    const SwNodeIndex* pIdx;

    for( n = aArr.size(); n; )
        if( 0 != ( pIdx = ( pFlyFmt = aArr[ --n ])->GetCntnt().GetCntntIdx() )
            && pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            sal_uInt16 nNum;
            String sNm;
            switch( GetNodes()[ pIdx->GetIndex() + 1 ]->GetNodeType() )
            {
            case ND_GRFNODE:
                sNm = sGrfNm;
                nNum = ++nGrfNum;
                break;
            case ND_OLENODE:
                sNm = sOLENm;
                nNum = ++nOLENum;
                break;
            default:
                sNm = sFlyNm;
                nNum = ++nFlyNum;
                break;
            }
            pFlyFmt->SetName( sNm += String::CreateFromInt32( nNum ));
        }
    aArr.clear();

    if( !GetFtnIdxs().empty() )
    {
        SwTxtFtn::SetUniqueSeqRefNo( *this );
        // #i52775# Chapter footnotes did not get updated correctly.
        // Calling UpdateAllFtn() instead of UpdateFtn() solves this problem,
        // but I do not dare to call UpdateAllFtn() in all cases: Safety first.
        if ( FTNNUM_CHAPTER == GetFtnInfo().eNum )
        {
            GetFtnIdxs().UpdateAllFtn();
        }
        else
        {
            SwNodeIndex aTmp( GetNodes() );
            GetFtnIdxs().UpdateFtn( aTmp );
        }
    }

    // Found a new document, but not a page anchored Frame/DrawObjects
    // that are anchored to another Node.
    if( bLoadedFlag )
        SetLoaded( sal_True );
}

sal_Bool SwDoc::IsInHeaderFooter( const SwNodeIndex& rIdx ) const
{
    // If there's a Layout, use it!
    // That can also be a Fly in a Fly in the Header.
    // Is also used by sw3io, to determine if a Redline object is
    // in the Header or Footer.
    // Because Redlines are also attached to Start and EndNoden,
    // the Index must not necessarily be from a ContentNode.
    SwNode* pNd = &rIdx.GetNode();
    if( pNd->IsCntntNode() && pCurrentView )//swmod 071029//swmod 071225
    {
        const SwFrm *pFrm = pNd->GetCntntNode()->getLayoutFrm( GetCurrentLayout() );
        if( pFrm )
        {
            const SwFrm *pUp = pFrm->GetUpper();
            while ( pUp && !pUp->IsHeaderFrm() && !pUp->IsFooterFrm() )
            {
                if ( pUp->IsFlyFrm() )
                    pUp = ((SwFlyFrm*)pUp)->GetAnchorFrm();
                pUp = pUp->GetUpper();
            }
            if ( pUp )
                return sal_True;

            return sal_False;
        }
    }


    const SwNode* pFlyNd = pNd->FindFlyStartNode();
    while( pFlyNd )
    {
        // get up by using the Anchor
        sal_uInt16 n;
        for( n = 0; n < GetSpzFrmFmts()->size(); ++n )
        {
            const SwFrmFmt* pFmt = (*GetSpzFrmFmts())[ n ];
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if( pIdx && pFlyNd == &pIdx->GetNode() )
            {
                const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
                if ((FLY_AT_PAGE == rAnchor.GetAnchorId()) ||
                    !rAnchor.GetCntntAnchor() )
                {
                    return sal_False;
                }

                pNd = &rAnchor.GetCntntAnchor()->nNode.GetNode();
                pFlyNd = pNd->FindFlyStartNode();
                break;
            }
        }
        if( n >= GetSpzFrmFmts()->size() )
        {
            OSL_ENSURE( mbInReading, "Found a FlySection but not a Format!" );
            return sal_False;
        }
    }

    return 0 != pNd->FindHeaderStartNode() ||
            0 != pNd->FindFooterStartNode();
}

short SwDoc::GetTextDirection( const SwPosition& rPos,
                               const Point* pPt ) const
{
    short nRet = -1;

    SwCntntNode *pNd = rPos.nNode.GetNode().GetCntntNode();

    // #i42921# - use new method <SwCntntNode::GetTextDirection(..)>
    if ( pNd )
    {
        nRet = pNd->GetTextDirection( rPos, pPt );
    }
    if ( nRet == -1 )
    {
        const SvxFrameDirectionItem* pItem = 0;
        if( pNd )
        {
            // Are we in a FlyFrame? Then look at that for the correct attribute
            const SwFrmFmt* pFlyFmt = pNd->GetFlyFmt();
            while( pFlyFmt )
            {
                pItem = &pFlyFmt->GetFrmDir();
                if( FRMDIR_ENVIRONMENT == pItem->GetValue() )
                {
                    pItem = 0;
                    const SwFmtAnchor* pAnchor = &pFlyFmt->GetAnchor();
                    if ((FLY_AT_PAGE != pAnchor->GetAnchorId()) &&
                        pAnchor->GetCntntAnchor())
                    {
                        pFlyFmt = pAnchor->GetCntntAnchor()->nNode.
                                            GetNode().GetFlyFmt();
                    }
                    else
                        pFlyFmt = 0;
                }
                else
                    pFlyFmt = 0;
            }

            if( !pItem )
            {
                const SwPageDesc* pPgDsc = pNd->FindPageDesc( sal_False );
                if( pPgDsc )
                    pItem = &pPgDsc->GetMaster().GetFrmDir();
            }
        }
        if( !pItem )
            pItem = (SvxFrameDirectionItem*)&GetAttrPool().GetDefaultItem(
                                                            RES_FRAMEDIR );
        nRet = pItem->GetValue();
    }
    return nRet;
}

sal_Bool SwDoc::IsInVerticalText( const SwPosition& rPos, const Point* pPt ) const
{
    const short nDir = GetTextDirection( rPos, pPt );
    return FRMDIR_VERT_TOP_RIGHT == nDir || FRMDIR_VERT_TOP_LEFT == nDir;
}

void SwDoc::SetCurrentViewShell( ViewShell* pNew )
{
    pCurrentView = pNew;
}

SwLayouter* SwDoc::GetLayouter()
{
    return pLayouter;
}

const SwLayouter* SwDoc::GetLayouter() const
{
    return pLayouter;
}

void SwDoc::SetLayouter( SwLayouter* pNew )
{
    pLayouter = pNew;
}

const ViewShell *SwDoc::GetCurrentViewShell() const
{
    return pCurrentView;
}

ViewShell *SwDoc::GetCurrentViewShell()
{
    return pCurrentView;
}

//swmod 080219
// It must be able to communicate to a ViewShell. This is going to be removed later.
const SwRootFrm *SwDoc::GetCurrentLayout() const
{
    if(GetCurrentViewShell())
        return GetCurrentViewShell()->GetLayout();
    return 0;
}

SwRootFrm *SwDoc::GetCurrentLayout()
{
    if(GetCurrentViewShell())
        return GetCurrentViewShell()->GetLayout();
    return 0;
}

bool SwDoc::HasLayout() const
{
    // if there is a view, there is always a layout
    return (pCurrentView != 0);
}

std::set<SwRootFrm*> SwDoc::GetAllLayouts()
{
    std::set<SwRootFrm*> aAllLayouts;
    ViewShell *pStart = GetCurrentViewShell();
    ViewShell *pTemp = pStart;
    if ( pTemp )
    {
        do
        {
            if (pTemp->GetLayout())
            {
                aAllLayouts.insert(pTemp->GetLayout());
                pTemp = (ViewShell*)pTemp->GetNext();
            }
        } while(pTemp!=pStart);
    }

    return aAllLayouts;
}//swmod 070825

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
