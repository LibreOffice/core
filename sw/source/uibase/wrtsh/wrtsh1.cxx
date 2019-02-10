/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/util/XModifiable.hpp>

#include <svx/dialogs.hrc>

#include <math.h>
#include <hintids.hxx>
#include <sot/exchange.hxx>
#include <svx/hdft.hxx>
#include <svx/svdview.hxx>
#include <svl/itemiter.hxx>
#include <tools/bigint.hxx>
#include <svtools/insdlg.hxx>
#include <sfx2/frmdescr.hxx>
#include <sfx2/ipclient.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/soerr.hxx>
#include <unotools/moduleoptions.hxx>
#include <editeng/rsiditem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/ulspitem.hxx>
#include <vcl/graph.hxx>
#include <sfx2/printer.hxx>
#include <unotools/charclass.hxx>
#include <comphelper/storagehelper.hxx>
#include <svx/svxdlg.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <frmfmt.hxx>
#include <fmtftn.hxx>
#include <fmthdft.hxx>
#include <fmtpdsc.hxx>
#include <txtfrm.hxx>
#include <wdocsh.hxx>
#include <basesh.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <uitool.hxx>
#include <cmdid.h>
#include <cfgitems.hxx>
#include <pagedesc.hxx>
#include <frmmgr.hxx>
#include <shellio.hxx>
#include <uinums.hxx>
#include <swundo.hxx>
#include <swcli.hxx>
#include <poolfmt.hxx>
#include <wview.hxx>
#include <edtwin.hxx>
#include <fmtcol.hxx>
#include <swtable.hxx>
#include <caption.hxx>
#include <viscrs.hxx>
#include <swdtflvr.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <SwStyleNameMapper.hxx>
#include <sfx2/request.hxx>
#include <paratr.hxx>
#include <ndtxt.hxx>
#include <editeng/acorrcfg.hxx>
#include <IMark.hxx>
#include <sfx2/bindings.hxx>

// -> #111827#
#include <SwRewriter.hxx>
#include <strings.hrc>
// <- #111827#

#include <toolkit/helper/vclunohelper.hxx>
#include <sfx2/viewfrm.hxx>

#include <PostItMgr.hxx>
#include <FrameControlsManager.hxx>

#include <sfx2/msgpool.hxx>
#include <svtools/embedhlp.hxx>
#include <memory>

using namespace sw::mark;
using namespace com::sun::star;

#define BITFLD_INI_LIST \
        m_bClearMark = \
        m_bIns = true;\
        m_bAddMode = \
        m_bBlockMode = \
        m_bExtMode = \
        m_bInSelect = \
        m_bLayoutMode = \
        m_bSelWrd = \
        m_bSelLn = \
        m_bRetainSelection = false; \
        m_bIsInClickToEdit = false;

static SvxAutoCorrect* lcl_IsAutoCorr()
{
    SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get().GetAutoCorrect();
    if( pACorr && !pACorr->IsAutoCorrFlag( ACFlags::CapitalStartSentence | ACFlags::CapitalStartWord |
                            ACFlags::AddNonBrkSpace | ACFlags::ChgOrdinalNumber |
                            ACFlags::ChgToEnEmDash | ACFlags::SetINetAttr | ACFlags::Autocorrect ))
        pACorr = nullptr;
    return pACorr;
}

void SwWrtShell::NoEdit(bool bHideCursor)
{
    if(bHideCursor)
        HideCursor();
}

void SwWrtShell::Edit()
{
    if (CanInsert())
    {
        ShowCursor();
    }
}

bool SwWrtShell::IsEndWrd()
{
    SwMvContext aMvContext(this);
    if(IsEndPara() && !IsSttPara())
        return true;

    return IsEndWord();
}

// Insert string
void SwWrtShell::InsertByWord( const OUString & rStr)
{
    if( !rStr.isEmpty() )
    {
        bool bDelim = GetAppCharClass().isLetterNumeric( rStr, 0 );
        sal_Int32 nPos = 0, nStt = 0;
        for( ; nPos < rStr.getLength(); nPos++ )
           {
            bool bTmpDelim = GetAppCharClass().isLetterNumeric( rStr, nPos );
            if( bTmpDelim != bDelim )
            {
                Insert( rStr.copy( nStt, nPos - nStt ));
                nStt = nPos;
            }
        }
        if( nStt != nPos )
            Insert( rStr.copy( nStt, nPos - nStt ));
    }
}

void SwWrtShell::Insert( const OUString &rStr )
{
    ResetCursorStack();
    if( !CanInsert() )
        return;

    bool bStarted = false;
    bool bHasSel = HasSelection(),
         bCallIns = m_bIns /*|| bHasSel*/;
    bool bDeleted = false;

    SfxItemSet aCharAttrSet(
        GetAttrPool(),
        svl::Items<RES_CHRATR_BEGIN, RES_CHRATR_END - 1>{});

    if( bHasSel || ( !m_bIns && SelectHiddenRange() ) )
    {
            // Only here parenthesizing, because the normal
            // insert is already in parentheses at Editshell.
        StartAllAction();

        SwRewriter aRewriter;

        aRewriter.AddRule(UndoArg1, GetCursorDescr());
        aRewriter.AddRule(UndoArg2, SwResId(STR_YIELDS));
        {
            OUString aTmpStr;
            aTmpStr += SwResId(STR_START_QUOTE);
            aTmpStr += rStr;
            aTmpStr += SwResId(STR_END_QUOTE);

            aRewriter.AddRule(UndoArg3, aTmpStr);
        }

        // tdf#79717 Save character formatting of the start of the selection
        const SwPosition *pStart = GetCursor()->Start();
        SwPaM aPaM(pStart->nNode.GetNode(), pStart->nContent.GetIndex(),
                   pStart->nNode.GetNode(), pStart->nContent.GetIndex() + 1);
        aCharAttrSet.ClearItem(RES_CHRATR_RSID);
        GetPaMAttr(&aPaM, aCharAttrSet);

        StartUndo(SwUndoId::REPLACE, &aRewriter);
        bStarted = true;
        bDeleted = DelRight();
    }

    bCallIns ?
        SwEditShell::Insert2( rStr, bDeleted ) : SwEditShell::Overwrite( rStr );

    if( bDeleted )
    {
        // tdf#79717 Restore formatting of the deleted selection
        SwPosition* pEnd = GetCursor()->Start();
        SwPaM aPaM(pEnd->nNode.GetNode(), pEnd->nContent.GetIndex() - rStr.getLength(),
                   pEnd->nNode.GetNode(), pEnd->nContent.GetIndex());

        std::set<sal_uInt16> aAttribs;
        for (sal_uInt16 i = RES_CHRATR_BEGIN; i < RES_CHRATR_END; ++i)
            if (i != sal_uInt16(RES_CHRATR_RSID))
                aAttribs.insert(aAttribs.end(), i);
        ResetAttr(aAttribs, &aPaM);

        SetAttrSet(aCharAttrSet, SetAttrMode::DEFAULT, &aPaM);
    }

    if( bStarted )
    {
        EndAllAction();
        EndUndo();
    }
}

// Maximum height limit not possible, because the maximum height
// of the current frame can not be obtained.

void SwWrtShell::Insert( const OUString &rPath, const OUString &rFilter,
                         const Graphic &rGrf, SwFlyFrameAttrMgr *pFrameMgr,
                         RndStdIds nAnchorType )
{
    ResetCursorStack();
    if ( !CanInsert() )
        return;

    StartAllAction();

    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, SwResId(STR_GRAPHIC));

    StartUndo(SwUndoId::INSERT, &aRewriter);

    if ( HasSelection() )
        DelRight();
        // Inserted graphics in its own paragraph,
        // if at the end of a non-empty paragraph.
    //For i120928,avoid to split node

    EnterSelFrameMode();

    bool bSetGrfSize = true;
    bool bOwnMgr     = false;

    if ( !pFrameMgr )
    {
        bOwnMgr = true;
        pFrameMgr = new SwFlyFrameAttrMgr( true, this, Frmmgr_Type::GRF );

        // CAUTION
        // GetAttrSet makes an adjustment
        // While pasting is a SwFrameSize present
        // because of the DEF-Framesize
        // These must be removed explicitly for the optimal size.
        pFrameMgr->DelAttr(RES_FRM_SIZE);

        if (nAnchorType != RndStdIds::FLY_AT_PARA)
            // Something other than at-para was requested.
            pFrameMgr->SetAnchor(nAnchorType);
    }
    else
    {
        Size aSz( pFrameMgr->GetSize() );
        if ( !aSz.Width() || !aSz.Height() )
        {
            aSz.setWidth(567);
            aSz.setHeight( 567);
            pFrameMgr->SetSize( aSz );
        }
        else if ( aSz.Width() != DFLT_WIDTH && aSz.Height() != DFLT_HEIGHT )
            bSetGrfSize = false;

        pFrameMgr->SetHeightSizeType(ATT_FIX_SIZE);
    }

    // Insert the graphic
    SwFEShell::Insert(rPath, rFilter, &rGrf, &pFrameMgr->GetAttrSet());
    if ( bOwnMgr )
        pFrameMgr->UpdateAttrMgr();

    if( bSetGrfSize )
    {
        Size aGrfSize, aBound = GetGraphicDefaultSize();
        GetGrfSize( aGrfSize );

        // Add the margin attributes to GrfSize,
        // because these counts at the margin additionally
        aGrfSize.AdjustWidth(pFrameMgr->CalcWidthBorder() );
        aGrfSize.AdjustHeight(pFrameMgr->CalcHeightBorder() );

        const BigInt aTempWidth( aGrfSize.Width() );
        const BigInt aTempHeight( aGrfSize.Height());

        // Fit width if necessary, scale down the height proportional thereafter.
        if( aGrfSize.Width() > aBound.Width() )
        {
            aGrfSize.setWidth( aBound.Width() );
            aGrfSize.setHeight( BigInt(aBound.Width()) * aTempHeight / aTempWidth );
        }
        // Fit height if necessary, scale down the width proportional thereafter.
        if( aGrfSize.Height() > aBound.Height() )
        {
            aGrfSize.setHeight( aBound.Height() );
            aGrfSize.setWidth(  BigInt(aBound.Height()) * aTempWidth / aTempHeight );
        }
        pFrameMgr->SetSize( aGrfSize );
        pFrameMgr->UpdateFlyFrame();
    }
    if ( bOwnMgr )
        delete pFrameMgr;

    EndUndo();
    EndAllAction();
}

// Insert an OLE-Object into the CORE.
// if no object is transferred, then one will be created.

void SwWrtShell::InsertObject( const svt::EmbeddedObjectRef& xRef, SvGlobalName const *pName,
                               sal_uInt16 nSlotId )
{
    ResetCursorStack();
    if( !CanInsert() )
        return;

    if( !xRef.is() )
    {
        // temporary storage
        svt::EmbeddedObjectRef xObj;
        uno::Reference < embed::XStorage > xStor = comphelper::OStorageHelper::GetTemporaryStorage();
        bool bDoVerb = true;
        if ( pName )
        {
            comphelper::EmbeddedObjectContainer aCnt( xStor );
            OUString aName;
            // TODO/LATER: get aspect?
            xObj.Assign( aCnt.CreateEmbeddedObject( pName->GetByteSequence(), aName ), embed::Aspects::MSOLE_CONTENT );
        }
        else
        {
            SvObjectServerList aServerList;
            switch (nSlotId)
            {
                case SID_INSERT_OBJECT:
                {
                    aServerList.FillInsertObjects();
                    aServerList.Remove( SwDocShell::Factory().GetClassId() );
                    [[fallthrough]];
                }

                // TODO/LATER: recording! Convert properties to items
                case SID_INSERT_FLOATINGFRAME:
                {
                    SfxSlotPool* pSlotPool = SW_MOD()->GetSlotPool();
                    const SfxSlot* pSlot = pSlotPool->GetSlot(nSlotId);
                    OString aCmd(".uno:");
                    aCmd += pSlot->GetUnoName();
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    vcl::Window* pWin = GetWin();
                    ScopedVclPtr<SfxAbstractInsertObjectDialog> pDlg(pFact->CreateInsertObjectDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                OUString::fromUtf8( aCmd ), xStor, &aServerList));
                    if (pDlg)
                    {
                        pDlg->Execute();
                        bDoVerb = pDlg->IsCreateNew();
                        OUString aIconMediaType;
                        uno::Reference< io::XInputStream > xIconMetaFile = pDlg->GetIconIfIconified( &aIconMediaType );
                        xObj.Assign( pDlg->GetObject(),
                                     xIconMetaFile.is() ? embed::Aspects::MSOLE_ICON : embed::Aspects::MSOLE_CONTENT );
                        if ( xIconMetaFile.is() )
                            xObj.SetGraphicStream( xIconMetaFile, aIconMediaType );
                    }

                    break;
                }

                default:
                    break;
            }
        }

        if ( xObj.is() )
        {
            if( InsertOleObject( xObj ) && bDoVerb )
            {
                SfxInPlaceClient* pClient = GetView().FindIPClient( xObj.GetObject(), &GetView().GetEditWin() );
                if ( !pClient )
                {
                    pClient = new SwOleClient( &GetView(), &GetView().GetEditWin(), xObj );
                    SetCheckForOLEInCaption( true );
                }

                if ( xObj.GetViewAspect() == embed::Aspects::MSOLE_ICON )
                {
                    SwRect aArea = GetAnyCurRect( CurRectType::FlyEmbeddedPrt, nullptr, xObj.GetObject() );
                    aArea.Pos() += GetAnyCurRect( CurRectType::FlyEmbedded, nullptr, xObj.GetObject() ).Pos();
                    MapMode aMapMode( MapUnit::MapTwip );
                    Size aSize = xObj.GetSize( &aMapMode );
                    aArea.Width( aSize.Width() );
                    aArea.Height( aSize.Height() );
                    RequestObjectResize( aArea, xObj.GetObject() );
                }
                else
                    CalcAndSetScale( xObj );

                //#50270# We don't need to handle error, this is handled by the
                //DoVerb in the SfxViewShell
                pClient->DoVerb(embed::EmbedVerbs::MS_OLEVERB_SHOW);

                // TODO/LATER: set document name - should be done in Client
            }
        }
    }
    else
    {
        if( HasSelection() )
            DelRight();
        InsertOleObject( xRef );
    }
}

// Insert object into the Core.
// From ClipBoard or Insert

bool SwWrtShell::InsertOleObject( const svt::EmbeddedObjectRef& xRef, SwFlyFrameFormat **pFlyFrameFormat )
{
    ResetCursorStack();
    StartAllAction();

    StartUndo(SwUndoId::INSERT);

    //Some differences between StarMath and any other objects:
    //1. Selections should be deleted. For StarMath the Text should be
    //   passed to the Object
    //2. If the cursor is at the end of an non empty paragraph a paragraph
    //   break should be inserted. StarMath objects are character bound and
    //   no break should be inserted.
    //3. If an selection is passed to a StarMath object, this object should
    //   not be activated. false should be returned then.
    bool bStarMath = true;
    bool bActivate = true;

    // set parent to get correct VisArea(in case of object needing parent printer)
    uno::Reference < container::XChild > xChild( xRef.GetObject(), uno::UNO_QUERY );
    if ( xChild.is() )
        xChild->setParent( mxDoc->GetDocShell()->GetModel() );

    SvGlobalName aCLSID( xRef->getClassID() );
    bStarMath = ( SotExchange::IsMath( aCLSID ) != 0 );
    if( IsSelection() )
    {
        if( bStarMath )
        {
            OUString aMathData;
            GetSelectedText( aMathData, ParaBreakType::ToOnlyCR );

            if( !aMathData.isEmpty() && svt::EmbeddedObjectRef::TryRunningState( xRef.GetObject() ) )
            {
                uno::Reference < beans::XPropertySet > xSet( xRef->getComponent(), uno::UNO_QUERY );
                if ( xSet.is() )
                {
                    try
                    {
                        xSet->setPropertyValue("Formula", uno::makeAny( aMathData ) );
                        bActivate = false;
                    }
                    catch (const uno::Exception&)
                    {
                    }
                }
            }
        }
        DelRight();
    }

    if ( !bStarMath )
        SwFEShell::SplitNode( false, false );

    EnterSelFrameMode();

    SwFlyFrameAttrMgr aFrameMgr( true, this, Frmmgr_Type::OLE );
    aFrameMgr.SetHeightSizeType(ATT_FIX_SIZE);

    SwRect aBound;
    CalcBoundRect( aBound, aFrameMgr.GetAnchor() );

    //The Size should be suggested by the OLE server
    MapMode aMapMode( MapUnit::MapTwip );
    Size aSz = xRef.GetSize( &aMapMode );

    //Object size can be limited
    if ( aSz.Width() > aBound.Width() )
    {
        //Always limit proportional.
        aSz.setHeight( aSz.Height() * aBound.Width() / aSz.Width() );
        aSz.setWidth( aBound.Width() );
    }
    aFrameMgr.SetSize( aSz );
    SwFlyFrameFormat *pFormat = SwFEShell::InsertObject( xRef, &aFrameMgr.GetAttrSet() );

    // --> #i972#
    if ( bStarMath && mxDoc->getIDocumentSettingAccess().get( DocumentSettingId::MATH_BASELINE_ALIGNMENT ) )
        AlignFormulaToBaseline( xRef.GetObject() );

    if (pFlyFrameFormat)
        *pFlyFrameFormat = pFormat;

    if ( SotExchange::IsChart( aCLSID ) )
    {
        uno::Reference< embed::XEmbeddedObject > xEmbeddedObj( xRef.GetObject(), uno::UNO_QUERY );
        if ( xEmbeddedObj.is() )
        {
            bool bDisableDataTableDialog = false;
            svt::EmbeddedObjectRef::TryRunningState( xEmbeddedObj );
            uno::Reference< beans::XPropertySet > xProps( xEmbeddedObj->getComponent(), uno::UNO_QUERY );
            if ( xProps.is() &&
                 ( xProps->getPropertyValue("DisableDataTableDialog") >>= bDisableDataTableDialog ) &&
                 bDisableDataTableDialog )
            {
                xProps->setPropertyValue("DisableDataTableDialog",
                    uno::makeAny( false ) );
                xProps->setPropertyValue("DisableComplexChartTypes",
                    uno::makeAny( false ) );
                uno::Reference< util::XModifiable > xModifiable( xProps, uno::UNO_QUERY );
                if ( xModifiable.is() )
                {
                    xModifiable->setModified( true );
                }
            }
        }
    }

    EndAllAction();
    GetView().AutoCaption(OLE_CAP, &aCLSID);

    SwRewriter aRewriter;

    if ( bStarMath )
        aRewriter.AddRule(UndoArg1, SwResId(STR_MATH_FORMULA));
    else if ( SotExchange::IsChart( aCLSID ) )
        aRewriter.AddRule(UndoArg1, SwResId(STR_CHART));
    else
        aRewriter.AddRule(UndoArg1, SwResId(STR_OLE));

    EndUndo(SwUndoId::INSERT, &aRewriter);

    return bActivate;
}

// The current selected OLE object will be loaded with the
// verb into the server.

void SwWrtShell::LaunchOLEObj( long nVerb )
{
    if ( GetCntType() == CNT_OLE &&
         !GetView().GetViewFrame()->GetFrame().IsInPlace() )
    {
        svt::EmbeddedObjectRef& xRef = GetOLEObject();
        OSL_ENSURE( xRef.is(), "OLE not found" );
        SfxInPlaceClient* pCli=nullptr;

        pCli = GetView().FindIPClient( xRef.GetObject(), &GetView().GetEditWin() );
        if ( !pCli )
            pCli = new SwOleClient( &GetView(), &GetView().GetEditWin(), xRef );

        static_cast<SwOleClient*>(pCli)->SetInDoVerb( true );

        CalcAndSetScale( xRef );
        pCli->DoVerb( nVerb );

        static_cast<SwOleClient*>(pCli)->SetInDoVerb( false );
        CalcAndSetScale( xRef );
    }
}

void SwWrtShell::MoveObjectIfActive( svt::EmbeddedObjectRef& xObj, const Point& rOffset )
{
    try
    {
        sal_Int32 nState = xObj->getCurrentState();
        if ( nState == css::embed::EmbedStates::INPLACE_ACTIVE
          || nState == css::embed::EmbedStates::UI_ACTIVE )
        {
            SfxInPlaceClient* pCli =
                GetView().FindIPClient( xObj.GetObject(), &(GetView().GetEditWin()) );
            if ( pCli )
            {
                tools::Rectangle aArea = pCli->GetObjArea();
                aArea += rOffset;
                pCli->SetObjArea( aArea );
            }
        }
    }
    catch (const uno::Exception&)
    {
    }
}

void SwWrtShell::CalcAndSetScale( svt::EmbeddedObjectRef& xObj,
                                  const SwRect *pFlyPrtRect,
                                  const SwRect *pFlyFrameRect,
                                  const bool bNoTextFramePrtAreaChanged )
{
    // Setting the scale of the client. This arises from the difference
    // between the VisArea of the object and the ObjArea.
    OSL_ENSURE( xObj.is(), "ObjectRef not  valid" );

    sal_Int64 nAspect = xObj.GetViewAspect();
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        return; // the replacement image is completely controlled by container in this case

    sal_Int64 nMisc = 0;
    bool bLinkingChart = false;

    try
    {
        nMisc = xObj->getStatus( nAspect );

        // This can surely only be a non-active object, if desired they
        // get the new size set as VisArea (StarChart).
        if( embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE & nMisc )
        {
            // TODO/MBA: testing
            SwRect aRect( pFlyPrtRect ? *pFlyPrtRect
                        : GetAnyCurRect( CurRectType::FlyEmbeddedPrt, nullptr, xObj.GetObject() ));
            if( !aRect.IsEmpty() )
            {
                // TODO/LEAN: getMapUnit can switch object to running state
                // xObj.TryRunningState();

                MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );

                // TODO/LATER: needs complete VisArea?!
                Size aSize( OutputDevice::LogicToLogic(aRect.SVRect(), MapMode(MapUnit::MapTwip), MapMode(aUnit)).GetSize() );
                awt::Size aSz;
                aSz.Width = aSize.Width();
                aSz.Height = aSize.Height();

                // Action 'setVisualAreaSize' doesn't have to turn on the
                // modified state of the document, either.
                bool bModified = false;
                uno::Reference<util::XModifiable> xModifiable(xObj->getComponent(), uno::UNO_QUERY);
                if (xModifiable.is())
                    bModified = xModifiable->isModified();
                xObj->setVisualAreaSize( nAspect, aSz );
                xModifiable.set(xObj->getComponent(), uno::UNO_QUERY);
                if (xModifiable.is() && xModifiable->isModified() && !bModified)
                    xModifiable->setModified(bModified);

                // #i48419# - action 'UpdateReplacement' doesn't
                // have to change the modified state of the document.
                // This is only a workaround for the defect, that this action
                // modifies a document after load, because unnecessarily the
                // replacement graphic is updated, in spite of the fact that
                // nothing has been changed.
                // If the replacement graphic changes by this action, the document
                // will be already modified via other mechanisms.
                {
                    bool bResetEnableSetModified(false);
                    if ( GetDoc()->GetDocShell()->IsEnableSetModified() )
                    {
                        GetDoc()->GetDocShell()->EnableSetModified( false );
                        bResetEnableSetModified = true;
                    }

                    //#i79576# don't destroy chart replacement images on load
                    //#i79578# don't request a new replacement image for charts to often
                    //a chart sends a modified call to the framework if it was changed
                    //thus the replacement update is already handled elsewhere
                    if ( !SotExchange::IsChart( xObj->getClassID() ) )
                        xObj.UpdateReplacement();

                    if ( bResetEnableSetModified )
                    {
                        GetDoc()->GetDocShell()->EnableSetModified();
                    }
                }
            }

            // TODO/LATER: this is only a workaround,
            uno::Reference< chart2::XChartDocument > xChartDocument( xObj->getComponent(), uno::UNO_QUERY );
            bLinkingChart = ( xChartDocument.is() && !xChartDocument->hasInternalDataProvider() );
        }
    }
    catch (const uno::Exception&)
    {
        // TODO/LATER: handle the error
        return;
    }

    SfxInPlaceClient* pCli = GetView().FindIPClient( xObj.GetObject(), &GetView().GetEditWin() );
    if ( !pCli )
    {
        if ( (embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY & nMisc)
             || bLinkingChart
             // --> OD #i117189# - refine condition for non-resizable objects
             // non-resizable objects need to be set the size back by this method
             || ( bNoTextFramePrtAreaChanged && nMisc & embed::EmbedMisc::EMBED_NEVERRESIZE ) )
        {
            pCli = new SwOleClient( &GetView(), &GetView().GetEditWin(), xObj );
        }
        else
            return;
    }

    // TODO/LEAN: getMapUnit can switch object to running state
    // xObj.TryRunningState();

    awt::Size aSize;
    try
    {
        aSize = xObj->getVisualAreaSize( nAspect );
    }
    catch (const embed::NoVisualAreaSizeException&)
    {
        OSL_FAIL("Can't get visual area size!" );
        // the scaling will not be done
    }
    catch (const uno::Exception&)
    {
        // TODO/LATER: handle the error
        OSL_FAIL("Can't get visual area size!" );
        return;
    }

    Size _aVisArea( aSize.Width, aSize.Height );

    Fraction aScaleWidth( 1, 1 );
    Fraction aScaleHeight( 1, 1 );

    bool bUseObjectSize = false;

    // As long as there comes no reasonable size from the object,
    // nothing can be scaled.
    if( _aVisArea.Width() && _aVisArea.Height() )
    {
        const MapMode aTmp( MapUnit::MapTwip );
        MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
        _aVisArea = OutputDevice::LogicToLogic(_aVisArea, MapMode(aUnit), aTmp);
        Size aObjArea;
        if ( pFlyPrtRect )
            aObjArea = pFlyPrtRect->SSize();
        else
            aObjArea = GetAnyCurRect( CurRectType::FlyEmbeddedPrt, nullptr, xObj.GetObject() ).SSize();

        // differ the aObjArea and _aVisArea by 1 Pixel then set new VisArea
        long nX, nY;
        SwSelPaintRects::Get1PixelInLogic( *this, &nX, &nY );
        if( !( _aVisArea.Width() - nX <= aObjArea.Width() &&
               _aVisArea.Width() + nX >= aObjArea.Width() &&
               _aVisArea.Height()- nY <= aObjArea.Height()&&
               _aVisArea.Height()+ nY >= aObjArea.Height() ))
        {
            if ( nMisc & embed::EmbedMisc::EMBED_NEVERRESIZE )
            {
                // the object must not be scaled,
                // the size stored in object must be used for restoring
                bUseObjectSize = true;
            }
            else
            {
                aScaleWidth = Fraction( aObjArea.Width(),   _aVisArea.Width() );
                aScaleHeight = Fraction( aObjArea.Height(), _aVisArea.Height());
            }
        }
    }

    // Now is the favorable time to set the ObjArea.
    // The Scaling must be considered.
    SwRect aArea;
    if ( pFlyPrtRect )
    {
        aArea = *pFlyPrtRect;
        aArea += pFlyFrameRect->Pos();
    }
    else
    {
        aArea = GetAnyCurRect( CurRectType::FlyEmbeddedPrt, nullptr, xObj.GetObject() );
        aArea.Pos() += GetAnyCurRect( CurRectType::FlyEmbedded, nullptr, xObj.GetObject() ).Pos();
    }

    if ( bUseObjectSize )
    {
        // --> this moves non-resizable object so that when adding borders the baseline remains the same
        const SwFlyFrameFormat *pFlyFrameFormat = dynamic_cast< const SwFlyFrameFormat * >( GetFlyFrameFormat() );
        OSL_ENSURE( pFlyFrameFormat, "Could not find fly frame." );
        if ( pFlyFrameFormat )
        {
            const Point &rPoint = pFlyFrameFormat->GetLastFlyFramePrtRectPos();
            SwRect aRect( pFlyPrtRect ? *pFlyPrtRect
                        : GetAnyCurRect( CurRectType::FlyEmbeddedPrt, nullptr, xObj.GetObject() ));
            aArea += rPoint - aRect.Pos(); // adjust area by diff of printing area position in order to keep baseline alignment correct.
        }
        aArea.Width ( _aVisArea.Width() );
        aArea.Height( _aVisArea.Height() );
        RequestObjectResize( aArea, xObj.GetObject() );
    }
    else
    {
        aArea.Width ( long( aArea.Width()  / pCli->GetScaleWidth() ) );
        aArea.Height( long( aArea.Height() / pCli->GetScaleHeight() ) );
    }

    pCli->SetObjAreaAndScale( aArea.SVRect(), aScaleWidth, aScaleHeight );
}

void SwWrtShell::ConnectObj( svt::EmbeddedObjectRef& xObj, const SwRect &rPrt,
                            const SwRect &rFrame )
{
    SfxInPlaceClient* pCli = GetView().FindIPClient( xObj.GetObject(), &GetView().GetEditWin());
    if ( !pCli )
        new SwOleClient( &GetView(), &GetView().GetEditWin(), xObj );
    CalcAndSetScale( xObj, &rPrt, &rFrame );
}

// Insert hard page break;
// Selections will be overwritten
void SwWrtShell::InsertPageBreak(const OUString *pPageDesc, const ::boost::optional<sal_uInt16>& oPgNum )
{
    ResetCursorStack();
    if( CanInsert() )
    {
        SwActContext aActContext(this);
        StartUndo(SwUndoId::UI_INSERT_PAGE_BREAK);

        if ( !IsCursorInTable() )
        {
            if(HasSelection())
                DelRight();
            SwFEShell::SplitNode();
            // delete the numbered attribute of the last line if the last line is empty
            GetDoc()->ClearLineNumAttrs( *GetCursor()->GetPoint() );
        }

        const SwPageDesc *pDesc = pPageDesc
                                ? FindPageDescByName( *pPageDesc, true ) : nullptr;
        if( pDesc )
        {
            SwFormatPageDesc aDesc( pDesc );
            aDesc.SetNumOffset( oPgNum );
            SetAttrItem( aDesc );
        }
        else
            SetAttrItem( SvxFormatBreakItem(SvxBreak::PageBefore, RES_BREAK) );
        EndUndo(SwUndoId::UI_INSERT_PAGE_BREAK);
    }
}

// Insert hard page break;
// Selections will be overwritten

void SwWrtShell::InsertLineBreak()
{
    ResetCursorStack();
    if( CanInsert() )
    {
        if(HasSelection())
            DelRight();

        const sal_Unicode cIns = 0x0A;
        SvxAutoCorrect* pACorr = lcl_IsAutoCorr();
        if( pACorr )
            AutoCorrect( *pACorr, cIns );
        else
            SwWrtShell::Insert( OUString( cIns ) );
    }
}

// Insert hard column break;
// Selections will be overwritten

void SwWrtShell::InsertColumnBreak()
{
    SwActContext aActContext(this);
    ResetCursorStack();
    if( CanInsert() )
    {
        StartUndo(SwUndoId::UI_INSERT_COLUMN_BREAK);

        if ( !IsCursorInTable() )
        {
            if(HasSelection())
                DelRight();
            SwFEShell::SplitNode( false, false );
        }
        SetAttrItem(SvxFormatBreakItem(SvxBreak::ColumnBefore, RES_BREAK));

        EndUndo(SwUndoId::UI_INSERT_COLUMN_BREAK);
    }
}

// Insert footnote
// rStr - optional footnote mark

void SwWrtShell::InsertFootnote(const OUString &rStr, bool bEndNote, bool bEdit )
{
    ResetCursorStack();
    if( CanInsert() )
    {
        if(HasSelection())
        {
            //collapse cursor to the end
            if(!IsCursorPtAtEnd())
                SwapPam();
            ClearMark();
        }
        SwPosition aPos = *GetCursor()->GetPoint();
        SwFormatFootnote aFootNote( bEndNote );
        if(!rStr.isEmpty())
            aFootNote.SetNumStr( rStr );

        SetAttrItem(aFootNote);

        if( bEdit )
        {
            // For editing the footnote text.
            Left(CRSR_SKIP_CHARS, false, 1, false );
            GotoFootnoteText();
        }
        m_aNavigationMgr.addEntry(aPos);
    }
}

// SplitNode; also, because
//                  - of deleting selected content;
//                  - of reset of the Cursorstack if necessary.

void SwWrtShell::SplitNode( bool bAutoFormat )
{
    ResetCursorStack();
    if( CanInsert() )
    {
        SwActContext aActContext(this);

        m_rView.GetEditWin().FlushInBuffer();
        bool bHasSel = HasSelection();
        if( bHasSel )
        {
            StartUndo( SwUndoId::INSERT );
            DelRight();
        }

        SwFEShell::SplitNode( bAutoFormat );
        if( bHasSel )
            EndUndo( SwUndoId::INSERT );
    }
}

// Turn on numbering
// Parameter:   Optional specification of a name for the named list;
//              this indicates a position if it is possible to convert them
//              into a number and less than nMaxRules.

// To test the CharFormats at the numbering
// external void SetNumChrFormat( SwWrtShell*, SwNumRules& );

// -> #i40041#
// Preconditions (as far as OD has figured out):
// - <SwEditShell::HasNumber()> is false, if <bNum> is true
// - <SwEditShell::HasBullet()> is false, if <bNum> is false
// Behavior of method is determined by the current situation at the current
// cursor position in the document.
void SwWrtShell::NumOrBulletOn(bool bNum)
{
    // determine numbering rule found at current cursor position in the document.
    const SwNumRule* pCurRule = GetNumRuleAtCurrCursorPos();

    StartUndo(SwUndoId::NUMORNONUM);

    const SwNumRule * pNumRule = pCurRule;

    // - activate outline rule respectively turning on outline rule for
    //   current text node. But, only for turning on a numbering (<bNum> == true).
    // - overwrite found numbering rule at current cursor position, if
    //   no numbering rule can be retrieved from the paragraph style.
    bool bContinueFoundNumRule( false );
    bool bActivateOutlineRule( false );
    int nActivateOutlineLvl( MAXLEVEL );    // only relevant, if <bActivateOutlineRule> == true
    SwTextFormatColl * pColl = GetCurTextFormatColl();
    if ( pColl )
    {
        // retrieve numbering rule at paragraph
        // style, which is found at current cursor position in the document.
        SwNumRule* pCollRule = mxDoc->FindNumRulePtr(pColl->GetNumRule().GetValue());
        // #125993# - The outline numbering rule isn't allowed
        // to be derived from a parent paragraph style to a derived one.
        // Thus check, if the found outline numbering rule is directly
        // set at the paragraph style <pColl>. If not, set <pCollRule> to NULL
        if ( pCollRule && pCollRule == GetDoc()->GetOutlineNumRule() )
        {
            const SwNumRule* pDirectCollRule =
                    mxDoc->FindNumRulePtr(pColl->GetNumRule( false ).GetValue());
            if ( !pDirectCollRule )
            {
                pCollRule = nullptr;
            }
        }

        if ( !pCollRule )
        {
            pNumRule = pCollRule;
        }
        // no activation or continuation of outline numbering in Writer/Web document
        else if ( bNum &&
                  !dynamic_cast<SwWebDocShell*>(GetDoc()->GetDocShell()) &&
                  pCollRule == GetDoc()->GetOutlineNumRule() )
        {
            if ( pNumRule == pCollRule )
            {
                // check, if text node at current cursor positioned is counted.
                // If not, let it been counted. Then it has to be checked,
                // of the outline numbering has to be activated or continued.
                SwTextNode const*const pTextNode = sw::GetParaPropsNode(
                        *GetLayout(), GetCursor()->GetPoint()->nNode);
                if ( pTextNode && !pTextNode->IsCountedInList() )
                {
                    // check, if numbering of the outline level of the paragraph
                    // style is active. If not, activate this outline level.
                    nActivateOutlineLvl = pColl->GetAssignedOutlineStyleLevel();
                    OSL_ENSURE( pColl->IsAssignedToListLevelOfOutlineStyle(),
                            "<SwWrtShell::NumOrBulletOn(..)> - paragraph style with outline rule, but no outline level" );
                    if ( pColl->IsAssignedToListLevelOfOutlineStyle() &&
                         pCollRule->Get( static_cast<sal_uInt16>(nActivateOutlineLvl) ).GetNumberingType()
                            == SVX_NUM_NUMBER_NONE )
                    {
                        // activate outline numbering
                        bActivateOutlineRule = true;
                    }
                    else
                    {
                        // turning on outline numbering at current cursor position
                        bContinueFoundNumRule = true;
                    }
                }
                else
                {
                    // #i101234#
                    // activate outline numbering, because from the precondition
                    // it's known, that <SwEdit::HasNumber()> == false
                    bActivateOutlineRule = true;
                    nActivateOutlineLvl = pColl->GetAssignedOutlineStyleLevel();
                }
            }
            else if ( !pNumRule )
            {
                // #i101234#
                // Check, if corresponding list level of the outline numbering
                // has already a numbering format set.
                nActivateOutlineLvl = pColl->GetAssignedOutlineStyleLevel();
                if ( pCollRule->Get( static_cast<sal_uInt16>(nActivateOutlineLvl) ).GetNumberingType()
                                == SVX_NUM_NUMBER_NONE )
                {
                    // activate outline numbering, because from the precondition
                    // it's known, that <SwEdit::HasNumber()> == false
                    bActivateOutlineRule = true;
                }
                else
                {
                    // turning on outline numbering at current cursor position
                    bContinueFoundNumRule = true;
                }
            }
            else
            {
                // check, if numbering of the outline level of the paragraph
                // style is active. If not, activate this outline level.
                nActivateOutlineLvl = pColl->GetAssignedOutlineStyleLevel();
                OSL_ENSURE( pColl->IsAssignedToListLevelOfOutlineStyle(),
                        "<SwWrtShell::NumOrBulletOn(..)> - paragraph style with outline rule, but no outline level" );
                if ( pColl->IsAssignedToListLevelOfOutlineStyle() &&
                     pCollRule->Get( static_cast<sal_uInt16>(nActivateOutlineLvl) ).GetNumberingType()
                        == SVX_NUM_NUMBER_NONE )
                {
                    // activate outline numbering
                    bActivateOutlineRule = true;
                }
                else
                {
                    // turning on outline numbering at current cursor position
                    bContinueFoundNumRule = true;
                }
            }
            pNumRule = pCollRule;
        }
    }

    // Only automatic numbering/bullet rules should be changed.
    // Note: The outline numbering rule is also an automatic one. It's only
    //       changed, if it has to be activated.
    if ( pNumRule )
    {
        if ( !pNumRule->IsAutoRule() )
        {
            pNumRule = nullptr;
        }
        else if ( pNumRule == GetDoc()->GetOutlineNumRule() &&
                  !bActivateOutlineRule && !bContinueFoundNumRule )
        {
            pNumRule = nullptr;
        }
    }

    // Search for a previous numbering/bullet rule to continue it.
    OUString sContinuedListId;
    if ( !pNumRule )
    {
        pNumRule = GetDoc()->SearchNumRule( *GetCursor()->GetPoint(),
                                            false, bNum, false, 0,
                                            sContinuedListId, GetLayout() );
        bContinueFoundNumRule = pNumRule != nullptr;
    }

    if (pNumRule)
    {
        SwNumRule aNumRule(*pNumRule);

        // do not change found numbering/bullet rule, if it should only be continued.
        if ( !bContinueFoundNumRule )
        {
            SwTextNode const*const pTextNode = sw::GetParaPropsNode(
                    *GetLayout(), GetCursor()->GetPoint()->nNode);

            if (pTextNode)
            {
                // use above retrieve outline level, if outline numbering has to be activated.
                int nLevel = bActivateOutlineRule
                              ? nActivateOutlineLvl
                              : pTextNode->GetActualListLevel();

                if (nLevel < 0)
                    nLevel = 0;

                if (nLevel >= MAXLEVEL)
                    nLevel = MAXLEVEL - 1;

                SwNumFormat aFormat(aNumRule.Get(static_cast<sal_uInt16>(nLevel)));

                if (bNum)
                    aFormat.SetNumberingType(SVX_NUM_ARABIC);
                else
                {
                    // #i63395# Only apply user defined default bullet font
                    if ( numfunc::IsDefBulletFontUserDefined() )
                    {
                        const vcl::Font* pFnt = &numfunc::GetDefBulletFont();
                        aFormat.SetBulletFont( pFnt );
                    }
                    aFormat.SetBulletChar( numfunc::GetBulletChar(static_cast<sal_uInt8>(nLevel)));
                    aFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                    // #i93908# clear suffix for bullet lists
                    aFormat.SetPrefix(OUString());
                    aFormat.SetSuffix(OUString());
                }
                aNumRule.Set(static_cast<sal_uInt16>(nLevel), aFormat);
            }
        }

        // reset indent attribute on applying list style
        SetCurNumRule( aNumRule, false, sContinuedListId, true );
    }
    else
    {
        // #i95907#
        const SvxNumberFormat::SvxNumPositionAndSpaceMode ePosAndSpaceMode(
                                    numfunc::GetDefaultPositionAndSpaceMode() );
        SwNumRule aNumRule( GetUniqueNumRuleName(), ePosAndSpaceMode );
        // Append the character template at the numbering.
        SwCharFormat* pChrFormat;
        SwDocShell* pDocSh = GetView().GetDocShell();
        // #i63395#
        // Only apply user defined default bullet font
        const vcl::Font* pFnt = numfunc::IsDefBulletFontUserDefined()
                           ? &numfunc::GetDefBulletFont()
                           : nullptr;

        if (bNum)
        {
            pChrFormat = GetCharFormatFromPool( RES_POOLCHR_NUM_LEVEL );
        }
        else
        {
            pChrFormat = GetCharFormatFromPool( RES_POOLCHR_BUL_LEVEL );
        }

        const SwTextNode *const pTextNode = sw::GetParaPropsNode(*GetLayout(),
                GetCursor()->GetPoint()->nNode);
        const SwTwips nWidthOfTabs = pTextNode
                                     ? pTextNode->GetWidthOfLeadingTabs()
                                     : 0;
        GetDoc()->getIDocumentContentOperations().RemoveLeadingWhiteSpace( *GetCursor()->GetPoint() );

        const bool bHtml = dynamic_cast<SwWebDocShell*>( pDocSh ) !=  nullptr;
        const bool bRightToLeft = IsInRightToLeftText();
        for( sal_uInt8 nLvl = 0; nLvl < MAXLEVEL; ++nLvl )
        {
            SwNumFormat aFormat( aNumRule.Get( nLvl ) );
            aFormat.SetCharFormat( pChrFormat );

            if (! bNum)
            {
                // #i63395#
                // Only apply user defined default bullet font
                if ( pFnt )
                {
                    aFormat.SetBulletFont( pFnt );
                }
                aFormat.SetBulletChar( numfunc::GetBulletChar(nLvl) );
                aFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                // #i93908# clear suffix for bullet lists
                aFormat.SetPrefix(OUString());
                aFormat.SetSuffix(OUString());
            }

            // #i95907#
            if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                if(bHtml && nLvl)
                {
                    // 1/2" for HTML
                    aFormat.SetAbsLSpace(nLvl * 720);
                }
                else if ( nWidthOfTabs > 0 )
                {
                    aFormat.SetAbsLSpace(nWidthOfTabs + nLvl * 720);
                }
            }

            // #i38904#  Default alignment for
            // numbering/bullet should be rtl in rtl paragraph:
            if ( bRightToLeft )
            {
                aFormat.SetNumAdjust( SvxAdjust::Right );
            }

            aNumRule.Set( nLvl, aFormat );
        }

        // #i95907#
        if ( pTextNode &&
             ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
        {

            const SwTwips nTextNodeIndent = pTextNode->GetAdditionalIndentForStartingNewList();
            if ( ( nTextNodeIndent + nWidthOfTabs ) != 0 )
            {
                // #i111172#/fdo#85666
                // If text node is already inside a list, assure that the indents
                // are the same. Thus, adjust the indent change value by subtracting
                // indents of to be applied list style.
                SwTwips nIndentChange = nTextNodeIndent + nWidthOfTabs;
                if ( pTextNode->GetNumRule() )
                {
                    int nLevel = pTextNode->GetActualListLevel();

                    if (nLevel < 0)
                        nLevel = 0;

                    if (nLevel >= MAXLEVEL)
                        nLevel = MAXLEVEL - 1;

                    const SwNumFormat& aFormat( aNumRule.Get( nLevel ) );
                    if ( aFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
                    {
                        nIndentChange -= aFormat.GetIndentAt() + aFormat.GetFirstLineIndent();
                    }
                }
                aNumRule.ChangeIndent( nIndentChange );
            }
        }
        // reset indent attribute on applying list style
        // start new list
        SetCurNumRule( aNumRule, true, OUString(), true );
    }

    EndUndo(SwUndoId::NUMORNONUM);
}
// <- #i40041#

void SwWrtShell::NumOn()
{
    NumOrBulletOn(true);
}

void SwWrtShell::NumOrBulletOff()
{
    const SwNumRule * pCurNumRule = GetNumRuleAtCurrCursorPos();

    if (pCurNumRule)
    {
        if (pCurNumRule->IsOutlineRule())
        {
            SwNumRule aNumRule(*pCurNumRule);

            SwTextNode * pTextNode =
                sw::GetParaPropsNode(*GetLayout(), GetCursor()->GetPoint()->nNode);

            if (pTextNode)
            {
                int nLevel = pTextNode->GetActualListLevel();

                if (nLevel < 0)
                    nLevel = 0;

                if (nLevel >= MAXLEVEL)
                    nLevel = MAXLEVEL - 1;

                SwNumFormat aFormat(aNumRule.Get(static_cast<sal_uInt16>(nLevel)));

                aFormat.SetNumberingType(SVX_NUM_NUMBER_NONE);
                aNumRule.Set(nLevel, aFormat);

                // no start or continuation of a list - the outline style is only changed.
                SetCurNumRule( aNumRule, false );
            }
        }
        else
        {
            DelNumRules();
        }

        // #126346# - Cursor can not be anymore in front of
        // a label, because numbering/bullet is switched off.
        SetInFrontOfLabel( false );
    }
}
// <- #i29560#

// Request Default-Bulletlist

void SwWrtShell::BulletOn()
{
    NumOrBulletOn(false);
}

SelectionType SwWrtShell::GetSelectionType() const
{
    // ContentType cannot be determined within a Start-/EndAction.
    // Because there is no invalid value TEXT will be returned.
    // The value does not matter, it may be updated in EndAction anyway.

    if (ActionPend())
        return IsSelFrameMode() ? SelectionType::Frame : SelectionType::Text;

    SwView &_rView = const_cast<SwView&>(GetView());
    if (_rView.GetPostItMgr() && _rView.GetPostItMgr()->HasActiveSidebarWin() )
        return SelectionType::PostIt;

    // Inserting a frame is not a DrawMode
    SelectionType nCnt;
    if ( !_rView.GetEditWin().IsFrameAction() &&
        (IsObjSelected() || (_rView.IsDrawMode() && !IsFrameSelected()) ))
    {
        if (GetDrawView()->IsTextEdit())
            nCnt = SelectionType::DrawObjectEditMode;
        else
        {
            if (GetView().IsFormMode()) // Only Form selected
                nCnt = SelectionType::DbForm;
            else
                nCnt = SelectionType::DrawObject;            // Any draw object

            if (_rView.IsBezierEditMode())
                nCnt |= SelectionType::Ornament;
            else if( GetDrawView()->GetContext() == SdrViewContext::Media )
                nCnt |= SelectionType::Media;

            if (svx::checkForSelectedCustomShapes( GetDrawView(), true /* bOnlyExtruded */ ))
            {
                nCnt |= SelectionType::ExtrudedCustomShape;
            }
            sal_uInt32 nCheckStatus = 0;
            if (svx::checkForSelectedFontWork( GetDrawView(), nCheckStatus ))
            {
                nCnt |= SelectionType::FontWork;
            }
        }

        return nCnt;
    }

    nCnt = static_cast<SelectionType>(GetCntType());

    if ( IsFrameSelected() )
    {
        if (_rView.IsDrawMode())
            _rView.LeaveDrawCreate();   // clean up (Bug #45639)
        if ( !(nCnt & (SelectionType::Graphic | SelectionType::Ole)) )
            return SelectionType::Frame;
    }

    if ( IsCursorInTable() )
        nCnt |= SelectionType::Table;

    if ( IsTableMode() )
        nCnt |= SelectionType::Table | SelectionType::TableCell;

    // Do not pop up numbering toolbar, if the text node has a numbering of type SVX_NUM_NUMBER_NONE.
    const SwNumRule* pNumRule = GetNumRuleAtCurrCursorPos();
    if ( pNumRule )
    {
        const SwTextNode* pTextNd =
            sw::GetParaPropsNode(*GetLayout(), GetCursor()->GetPoint()->nNode);

        if ( pTextNd && pTextNd->IsInList() )
        {
            int nLevel = pTextNd->GetActualListLevel();

            if (nLevel < 0)
                nLevel = 0;

            if (nLevel >= MAXLEVEL)
                nLevel = MAXLEVEL - 1;

            const SwNumFormat& rFormat = pNumRule->Get(nLevel);
            if ( SVX_NUM_NUMBER_NONE != rFormat.GetNumberingType() )
                nCnt |= SelectionType::NumberList;
        }
    }

    return nCnt;
}

// Find the text collection with the name rCollname
// Returns:   Pointer at the collection or 0, if no
//            text collection with this name exists, or
//            this is a default template.

SwTextFormatColl *SwWrtShell::GetParaStyle(const OUString &rCollName, GetStyle eCreate )
{
    SwTextFormatColl* pColl = FindTextFormatCollByName( rCollName );
    if( !pColl && GETSTYLE_NOCREATE != eCreate )
    {
        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( rCollName, SwGetPoolIdFromName::TxtColl );
        if( USHRT_MAX != nId || GETSTYLE_CREATEANY == eCreate )
            pColl = GetTextCollFromPool( nId );
    }
    return pColl;
}

// Find the text collection with the name rCollname
// Returns:   Pointer at the collection or 0, if no
//            character template with this name exists, or
//            this is a default template or template is automatic.

SwCharFormat *SwWrtShell::GetCharStyle(const OUString &rFormatName, GetStyle eCreate )
{
    SwCharFormat* pFormat = FindCharFormatByName( rFormatName );
    if( !pFormat && GETSTYLE_NOCREATE != eCreate )
    {
        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( rFormatName, SwGetPoolIdFromName::ChrFmt );
        if( USHRT_MAX != nId || GETSTYLE_CREATEANY == eCreate )
            pFormat = static_cast<SwCharFormat*>(GetFormatFromPool( nId ));
    }
    return pFormat;
}

// Find the table format with the name rFormatname
// Returns:   Pointer at the collection or 0, if no
//            frame format with this name exists or
//            this is a default format or the format is automatic.

SwFrameFormat *SwWrtShell::GetTableStyle(const OUString &rFormatName)
{
    for( size_t i = GetTableFrameFormatCount(); i; )
    {
        SwFrameFormat *pFormat = &GetTableFrameFormat( --i );
        if( !pFormat->IsDefault() &&
            pFormat->GetName() == rFormatName && IsUsed( *pFormat ) )
            return pFormat;
    }
    return nullptr;
}

void SwWrtShell::addCurrentPosition() {
    SwPaM* pPaM = GetCursor();
    m_aNavigationMgr.addEntry(*pPaM->GetPoint());
}

// Applying templates

void SwWrtShell::SetPageStyle(const OUString &rCollName)
{
    if( !SwCursorShell::HasSelection() && !IsSelFrameMode() && !IsObjSelected() )
    {
        SwPageDesc* pDesc = FindPageDescByName( rCollName, true );
        if( pDesc )
            ChgCurPageDesc( *pDesc );
    }
}

// Access templates

OUString const & SwWrtShell::GetCurPageStyle() const
{
    return GetPageDesc(GetCurPageDesc( false/*bCalcFrame*/ )).GetName();
}

// Change the current template referring to the existing change.

void SwWrtShell::QuickUpdateStyle()
{
    SwTextFormatColl *pColl = GetCurTextFormatColl();

    // Default cannot be changed
    if(pColl && !pColl->IsDefault())
    {
        FillByEx(pColl);
            // Also apply the template to remove hard attribute assignment.
        SetTextFormatColl(pColl);
    }
}

void SwWrtShell::AutoUpdatePara(SwTextFormatColl* pColl, const SfxItemSet& rStyleSet, SwPaM* pPaM )
{
    SwPaM* pCursor = pPaM ? pPaM : GetCursor( );
    SfxItemSet aCoreSet(
        GetAttrPool(),
        svl::Items<
            RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
            RES_PARATR_BEGIN, RES_PARATR_END - 1,
            RES_FRMATR_BEGIN, RES_FRMATR_END - 1,
            SID_ATTR_TABSTOP_DEFAULTS,SID_ATTR_TABSTOP_OFFSET,
            SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
            SID_ATTR_PARA_MODEL, SID_ATTR_PARA_KEEP,
            SID_ATTR_PARA_PAGENUM, SID_ATTR_PARA_PAGENUM>{});
    GetPaMAttr( pCursor, aCoreSet );
    bool bReset = false;
    SfxItemIter aParaIter( aCoreSet );
    const SfxPoolItem* pParaItem = aParaIter.FirstItem();
    while( pParaItem )
    {
        if(!IsInvalidItem(pParaItem))
        {
            sal_uInt16 nWhich = pParaItem->Which();
            if(SfxItemState::SET == aCoreSet.GetItemState(nWhich) &&
               SfxItemState::SET == rStyleSet.GetItemState(nWhich))
            {
                aCoreSet.ClearItem(nWhich);
                bReset = true;
            }
        }
        pParaItem = aParaIter.NextItem();
    }
    StartAction();
    if(bReset)
    {
        ResetAttr( std::set<sal_uInt16>(), pCursor );
        SetAttrSet(aCoreSet, SetAttrMode::DEFAULT, pCursor);
    }
    mxDoc->ChgFormat(*pColl, rStyleSet );
    EndAction();
}

void SwWrtShell::AutoUpdateFrame( SwFrameFormat* pFormat, const SfxItemSet& rStyleSet )
{
    StartAction();

    ResetFlyFrameAttr( &rStyleSet );
    pFormat->SetFormatAttr( rStyleSet );

    EndAction();
}

void SwWrtShell::AutoCorrect( SvxAutoCorrect& rACorr, sal_Unicode cChar )
{
    ResetCursorStack();
    if(CanInsert())
    {
        bool bStarted = false;
        SwRewriter aRewriter;

        if(HasSelection())
        {
                // Only parentheses here, because the regular insert
                // is already clipped to the editshell
            StartAllAction();

            OUString aTmpStr1;
            aTmpStr1 += SwResId(STR_START_QUOTE);
            aTmpStr1 += GetSelText();
            aTmpStr1 += SwResId(STR_END_QUOTE);
            OUString aTmpStr3;
            aTmpStr3 += SwResId(STR_START_QUOTE);
            aTmpStr3 += OUStringLiteral1(cChar);
            aTmpStr3 += SwResId(STR_END_QUOTE);
            aRewriter.AddRule( UndoArg1, aTmpStr1 );
            aRewriter.AddRule( UndoArg2, SwResId(STR_YIELDS) );
            aRewriter.AddRule( UndoArg3, aTmpStr3 );

            StartUndo( SwUndoId::REPLACE, &aRewriter );
            bStarted = true;
            DelRight();
        }
        SwEditShell::AutoCorrect( rACorr, IsInsMode(), cChar );

        if(bStarted)
        {
            EndAllAction();
            EndUndo( SwUndoId::REPLACE, &aRewriter );
        }
    }
}

// Some kind of controlled copy ctor

SwWrtShell::SwWrtShell( SwWrtShell& rSh, vcl::Window *_pWin, SwView &rShell )
    : SwFEShell(rSh, _pWin)
    , m_rView(rShell)
    , m_aNavigationMgr(*this)
{
    BITFLD_INI_LIST
    SET_CURR_SHELL( this );

    SetSfxViewShell( static_cast<SfxViewShell *>(&rShell) );
    SetFlyMacroLnk( LINK(this, SwWrtShell, ExecFlyMac) );

    // place the cursor on the first field...
    IFieldmark *pBM = nullptr;
    if ( IsFormProtected() && ( pBM = GetFieldmarkAfter( ) ) !=nullptr ) {
        GotoFieldmark(pBM);
    }
}

SwWrtShell::SwWrtShell( SwDoc& rDoc, vcl::Window *_pWin, SwView &rShell,
                        const SwViewOption *pViewOpt )
    : SwFEShell(rDoc, _pWin, pViewOpt)
    , m_rView(rShell)
    , m_aNavigationMgr(*this)
{
    BITFLD_INI_LIST
    SET_CURR_SHELL( this );
    SetSfxViewShell( static_cast<SfxViewShell *>(&rShell) );
    SetFlyMacroLnk( LINK(this, SwWrtShell, ExecFlyMac) );

    // place the cursor on the first field...
    IFieldmark *pBM = nullptr;
    if ( IsFormProtected() && ( pBM = GetFieldmarkAfter( ) ) !=nullptr ) {
        GotoFieldmark(pBM);
    }
}

SwWrtShell::~SwWrtShell()
{
    SET_CURR_SHELL( this );
    while(IsModePushed())
        PopMode();
    while(PopCursor(false))
        ;
    SwTransferable::ClearSelection( *this );
}

bool SwWrtShell::Pop(SwCursorShell::PopMode const eDelete)
{
    bool bRet = SwCursorShell::Pop(eDelete);
    if( bRet && IsSelection() )
    {
        m_fnSetCursor = &SwWrtShell::SetCursorKillSel;
        m_fnKillSel = &SwWrtShell::ResetSelect;
    }
    return bRet;
}

bool SwWrtShell::CanInsert()
{
    if(IsSelFrameMode())
    {
        return false;
    }

    if(IsObjSelected())
    {
        return false;
    }

    if(GetView().GetDrawFuncPtr())
    {
        return false;
    }

    if(GetView().GetPostItMgr()->GetActiveSidebarWin())
    {
        return false;
    }

    return true;
}

void SwWrtShell::ChgDBData(const SwDBData& aDBData)
{
    SwEditShell::ChgDBData(aDBData);
    //notify the db-beamer if available
    GetView().NotifyDBChanged();
}

OUString SwWrtShell::GetSelDescr() const
{
    OUString aResult;

    SelectionType nSelType = GetSelectionType();
    switch (nSelType)
    {
    case SelectionType::Graphic:
        aResult = SwResId(STR_GRAPHIC);

        break;
    case SelectionType::Frame:
        {
            const SwFrameFormat * pFrameFormat = GetSelectedFrameFormat();

            if (pFrameFormat)
                aResult = pFrameFormat->GetDescription();
        }
        break;
    case SelectionType::DrawObject:
        {
            aResult = SwResId(STR_DRAWING_OBJECTS);
        }
        break;
    default:
        if (mxDoc)
            aResult = GetCursorDescr();
    }

    return aResult;
}

void SwWrtShell::ApplyViewOptions( const SwViewOption &rOpt )
{
    SwFEShell::ApplyViewOptions( rOpt );
    //#i115062# invalidate meta character slot
    GetView().GetViewFrame()->GetBindings().Invalidate( FN_VIEW_META_CHARS );
}

void SwWrtShell::SetReadonlyOption(bool bSet)
{
    GetView().GetEditWin().GetFrameControlsManager().SetReadonlyControls( bSet );
    SwViewShell::SetReadonlyOption( bSet );
}

// Switch on/off header or footer of a page style - if an empty name is
// given all styles are changed

void SwWrtShell::ChangeHeaderOrFooter(
    const OUString& rStyleName, bool bHeader, bool bOn, bool bShowWarning)
{
    SdrView *const pSdrView = GetDrawView();
    if (pSdrView && pSdrView->IsTextEdit())
    {   // tdf#107474 deleting header may delete active drawing object
        pSdrView->SdrEndTextEdit(true);
    }
    addCurrentPosition();
    StartAllAction();
    StartUndo( SwUndoId::HEADER_FOOTER ); // #i7983#
    bool bExecute = true;
    bool bCursorSet = false;
    for( size_t nFrom = 0, nTo = GetPageDescCnt();
            nFrom < nTo; ++nFrom )
    {
        SwPageDesc aDesc( GetPageDesc( nFrom ));
        OUString sTmp(aDesc.GetName());
        if( rStyleName.isEmpty() || rStyleName == sTmp )
        {
            bool bChgd = false;

            if( bShowWarning && !bOn && GetActiveView() && GetActiveView() == &GetView() &&
                ( (bHeader && aDesc.GetMaster().GetHeader().IsActive()) ||
                  (!bHeader && aDesc.GetMaster().GetFooter().IsActive()) ) )
            {
                bShowWarning = false;
                //Actions have to be closed while the dialog is showing
                EndAllAction();

                weld::Window* pParent = GetView().GetViewFrame()->GetWindow().GetFrameWeld();
                short nResult;
                if (bHeader) {
                    nResult = DeleteHeaderDialog(pParent).run();
                } else {
                    nResult = DeleteFooterDialog(pParent).run();
                }

                bExecute = nResult == RET_YES;
                StartAllAction();
            }
            if( bExecute )
            {
                bChgd = true;
                SwFrameFormat &rMaster = aDesc.GetMaster();
                if(bHeader)
                    rMaster.SetFormatAttr( SwFormatHeader( bOn ));
                else
                    rMaster.SetFormatAttr( SwFormatFooter( bOn ));
                if( bOn )
                {
                    SvxULSpaceItem aUL(bHeader ? 0 : MM50, bHeader ? MM50 : 0, RES_UL_SPACE );
                    SwFrameFormat* pFormat = bHeader ?
                        const_cast<SwFrameFormat*>(rMaster.GetHeader().GetHeaderFormat()) :
                        const_cast<SwFrameFormat*>(rMaster.GetFooter().GetFooterFormat());
                    pFormat->SetFormatAttr( aUL );
                }
            }
            if( bChgd )
            {
                ChgPageDesc( nFrom, aDesc );

                if( !bCursorSet && bOn )
                {
                    if ( !IsHeaderFooterEdit() )
                        ToggleHeaderFooterEdit();
                    bCursorSet = SetCursorInHdFt(
                            rStyleName.isEmpty() ? SIZE_MAX : nFrom,
                            bHeader );
                }
            }
        }
    }
    EndUndo( SwUndoId::HEADER_FOOTER ); // #i7983#
    EndAllAction();
}

void SwWrtShell::SetShowHeaderFooterSeparator( FrameControlType eControl, bool bShow )
{
    SwViewShell::SetShowHeaderFooterSeparator( eControl, bShow );
    if ( !bShow )
        GetView().GetEditWin().GetFrameControlsManager().HideControls( eControl );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
