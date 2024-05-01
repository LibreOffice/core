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
#include <com/sun/star/lang/XInitialization.hpp>

#include <hintids.hxx>
#include <sot/exchange.hxx>
#include <svx/xfillit0.hxx>
#include <svx/hdft.hxx>
#include <svx/svdview.hxx>
#include <svl/itemiter.hxx>
#include <tools/bigint.hxx>
#include <svtools/insdlg.hxx>
#include <sfx2/ipclient.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/ulspitem.hxx>
#include <vcl/graph.hxx>
#include <unotools/charclass.hxx>
#include <comphelper/storagehelper.hxx>
#include <svx/svxdlg.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <dialoghelp.hxx>
#include <frmfmt.hxx>
#include <fmtftn.hxx>
#include <fmthdft.hxx>
#include <fmtpdsc.hxx>
#include <txtfrm.hxx>
#include <wdocsh.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <cmdid.h>
#include <pagedesc.hxx>
#include <frmmgr.hxx>
#include <swundo.hxx>
#include <swcli.hxx>
#include <poolfmt.hxx>
#include <edtwin.hxx>
#include <fmtcol.hxx>
#include <swtable.hxx>
#include <viscrs.hxx>
#include <swdtflvr.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <SwCapObjType.hxx>
#include <SwStyleNameMapper.hxx>
#include <sfx2/request.hxx>
#include <paratr.hxx>
#include <ndtxt.hxx>
#include <editeng/acorrcfg.hxx>
#include <IMark.hxx>
#include <sfx2/bindings.hxx>
#include <flyfrm.hxx>

// -> #111827#
#include <SwRewriter.hxx>
#include <strings.hrc>
// <- #111827#

#include <toolkit/helper/vclunohelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>
#include <osl/diagnose.h>
#include <o3tl/unit_conversion.hxx>
#include <officecfg/Office/Common.hxx>

#include <PostItMgr.hxx>
#include <FrameControlsManager.hxx>
#include <fldmgr.hxx>
#include <docufld.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <fmtfld.hxx>

#include <sfx2/msgpool.hxx>
#include <sfx2/msg.hxx>
#include <svtools/embedhlp.hxx>
#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>
#include <svx/postattr.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/propertyvalue.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>
#include <memory>

#include "../../core/crsr/callnk.hxx"
#include <frmtool.hxx>
#include <viewopt.hxx>

#include <IDocumentRedlineAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <UndoInsert.hxx>
#include <UndoCore.hxx>
#include <formatlinebreak.hxx>
#include <formatcontentcontrol.hxx>
#include <textcontentcontrol.hxx>

using namespace sw::mark;
using namespace com::sun::star;
namespace {

void collectUIInformation(const OUString& rAction, const OUString& aParameters)
{
    EventDescription aDescription;
    aDescription.aAction = rAction;
    aDescription.aParameters = {{"parameters", aParameters}};
    aDescription.aID = "writer_edit";
    aDescription.aKeyWord = "SwEditWinUIObject";
    aDescription.aParent = "MainWindow";
    UITestLogger::getInstance().logEvent(aDescription);
}

}

sal_uInt32 MakeAllOutlineContentTemporarilyVisible::nLock = 0;

static bool lcl_IsAllowed(const SwWrtShell* rSh)
{
    if (rSh->GetViewOptions()->IsShowOutlineContentVisibilityButton() && rSh->IsEndPara())
    {
        SwTextNode* pTextNode = rSh->GetCursor()->GetPointNode().GetTextNode();
        if (pTextNode && pTextNode->IsOutline())
        {
            // disallow if this is an outline node having folded content
            bool bVisible = true;
            pTextNode->GetAttrOutlineContentVisible(bVisible);
            if (!bVisible)
                return false;
        }
    }
    return true;
}

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
                            ACFlags::AddNonBrkSpace | ACFlags::ChgOrdinalNumber | ACFlags::TransliterateRTL |
                            ACFlags::ChgToEnEmDash | ACFlags::SetINetAttr | ACFlags::Autocorrect |
                            ACFlags::SetDOIAttr ))
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
    if( rStr.isEmpty() )
        return;

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

void SwWrtShell::Insert( const OUString &rStr )
{
    ResetCursorStack();
    if( !CanInsert() )
        return;

    bool bStarted = false;
    bool bHasSel = HasSelection(),
         bCallIns = m_bIns /*|| bHasSel*/;
    bool bDeleted = false;

    if( bHasSel || ( !m_bIns && IsInHiddenRange(/*bSelect=*/true) ) )
    {
            // Only here parenthesizing, because the normal
            // insert is already in parentheses at Editshell.
        StartAllAction();

        SwRewriter aRewriter;

        aRewriter.AddRule(UndoArg1, GetCursorDescr());
        aRewriter.AddRule(UndoArg2, SwResId(STR_YIELDS));
        {
            OUString aTmpStr = SwResId(STR_START_QUOTE) +
                rStr + SwResId(STR_END_QUOTE);

            aRewriter.AddRule(UndoArg3, aTmpStr);
        }

        StartUndo(SwUndoId::REPLACE, &aRewriter);
        bStarted = true;
        Push();
        // let's interpret a selection within the same node as "replace"
        bDeleted = DelRight(GetCursor()->GetPoint()->GetNode() == GetCursor()->GetMark()->GetNode());
        Pop(SwCursorShell::PopMode::DeleteCurrent); // Restore selection (if tracking changes)
        NormalizePam(false); // tdf#127635 put point at the end of deletion
        ClearMark();
    }

    bCallIns ?
        SwEditShell::Insert2( rStr, bDeleted ) : SwEditShell::Overwrite( rStr );

    // Check whether node is content control
    SwTextContentControl* pTextContentControl = CursorInsideContentControl();
    if (pTextContentControl)
    {
        std::shared_ptr<SwContentControl> pContentControl =
            pTextContentControl->GetContentControl().GetContentControl();
        if (pContentControl)
        {
            // Set showingPlcHdr to false as node has been edited
            pContentControl->SetShowingPlaceHolder(false);
        }
    }

    if( bStarted )
    {
        EndUndo();
        EndAllAction();
    }
}

// Maximum height limit not possible, because the maximum height
// of the current frame can not be obtained.

void SwWrtShell::InsertGraphic( const OUString &rPath, const OUString &rFilter,
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
        pFrameMgr = new SwFlyFrameAttrMgr( true, this, Frmmgr_Type::GRF, nullptr );

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
            aSz.setWidth(o3tl::toTwips(1, o3tl::Length::cm));
            aSz.setHeight(o3tl::toTwips(1, o3tl::Length::cm));
            pFrameMgr->SetSize( aSz );
        }
        else if ( aSz.Width() != DFLT_WIDTH && aSz.Height() != DFLT_HEIGHT )
            bSetGrfSize = false;

        pFrameMgr->SetHeightSizeType(SwFrameSize::Fixed);
    }

    // during change tracking, insert the image anchored as character
    // (to create an SwRangeRedline on its anchor point)
    if ( IsRedlineOn() && nAnchorType != RndStdIds::FLY_AS_CHAR )
        pFrameMgr->SetAnchor( RndStdIds::FLY_AS_CHAR );

    // Insert the graphic
    SwFEShell::Insert(rPath, rFilter, &rGrf, &pFrameMgr->GetAttrSet());
    if ( bOwnMgr )
        pFrameMgr->UpdateAttrMgr();

    if( bSetGrfSize )
    {
        Size aSizePixel = rGrf.GetSizePixel();
        Size aBound = GetGraphicDefaultSize();

        sal_Int32 nPreferredDPI = mxDoc->getIDocumentSettingAccess().getImagePreferredDPI();
        Size aGrfSize;

        if (nPreferredDPI > 0)
        {
            auto nWidth = o3tl::toTwips(aSizePixel.Width() / double(nPreferredDPI), o3tl::Length::in);
            auto nHeight = o3tl::toTwips(aSizePixel.Height() / double(nPreferredDPI), o3tl::Length::in);
            aGrfSize = Size(nWidth, nHeight);
        }
        else
        {
            GetGrfSize(aGrfSize);
        }

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
                    if (officecfg::Office::Common::Security::Scripting::DisableActiveContent::get())
                    {
                        std::unique_ptr<weld::MessageDialog> xError(
                            Application::CreateMessageDialog(
                                nullptr, VclMessageType::Warning, VclButtonsType::Ok,
                                SvtResId(STR_WARNING_ACTIVE_CONTENT_DISABLED)));
                        xError->run();
                        break;
                    }
                    aServerList.FillInsertObjects();
                    aServerList.Remove( SwDocShell::Factory().GetClassId() );
                    [[fallthrough]];
                }

                // TODO/LATER: recording! Convert properties to items
                case SID_INSERT_FLOATINGFRAME:
                {
                    SfxSlotPool* pSlotPool = SW_MOD()->GetSlotPool();
                    const SfxSlot* pSlot = pSlotPool->GetSlot(nSlotId);
                    OUString aCmd = pSlot->GetCommand();
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    ScopedVclPtr<SfxAbstractInsertObjectDialog> pDlg(pFact->CreateInsertObjectDialog(GetFrameWeld(mxDoc->GetDocShell()),
                                aCmd, xStor, &aServerList));
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
    //tdf#125100 Ensure that ole object is initially shown as pictogram
    comphelper::EmbeddedObjectContainer& rEmbeddedObjectContainer = mxDoc->GetDocShell()->getEmbeddedObjectContainer();
    bool bSaveUserAllowsLinkUpdate = rEmbeddedObjectContainer.getUserAllowsLinkUpdate();
    rEmbeddedObjectContainer.setUserAllowsLinkUpdate(true);

    ResetCursorStack();
    StartAllAction();

    StartUndo(SwUndoId::INSERT);

    //Some differences between StarMath and any other objects:
    //1. Selections should be deleted. For StarMath the Text should be
    //   passed to the Object
    //2. If the cursor is at the end of a non empty paragraph a paragraph
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
                        xSet->setPropertyValue(u"Formula"_ustr, uno::Any( aMathData ) );
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

    const SvGlobalName* pName = nullptr;
    SvGlobalName aObjClsId;
    if (xRef.is())
    {
        aObjClsId = SvGlobalName(xRef.GetObject()->getClassID());
        pName = &aObjClsId;
    }
    SwFlyFrameAttrMgr aFrameMgr( true, this, Frmmgr_Type::OLE, pName );
    aFrameMgr.SetHeightSizeType(SwFrameSize::Fixed);

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
        uno::Reference< embed::XEmbeddedObject > xEmbeddedObj = xRef.GetObject();
        if ( xEmbeddedObj.is() )
        {
            bool bDisableDataTableDialog = false;
            svt::EmbeddedObjectRef::TryRunningState( xEmbeddedObj );
            uno::Reference< beans::XPropertySet > xProps( xEmbeddedObj->getComponent(), uno::UNO_QUERY );
            if ( xProps.is() &&
                 ( xProps->getPropertyValue(u"DisableDataTableDialog"_ustr) >>= bDisableDataTableDialog ) &&
                 bDisableDataTableDialog )
            {
                xProps->setPropertyValue(u"DisableDataTableDialog"_ustr,
                    uno::Any( false ) );
                xProps->setPropertyValue(u"DisableComplexChartTypes"_ustr,
                    uno::Any( false ) );
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

    rEmbeddedObjectContainer.setUserAllowsLinkUpdate(bSaveUserAllowsLinkUpdate);

    return bActivate;
}

// The current selected OLE object will be loaded with the
// verb into the server.
void SwWrtShell::LaunchOLEObj(sal_Int32 nVerb)
{
    if ( GetCntType() != CNT_OLE ||
         GetView().GetViewFrame().GetFrame().IsInPlace() )
        return;

    svt::EmbeddedObjectRef& xRef = GetOLEObject();
    OSL_ENSURE( xRef.is(), "OLE not found" );

    // LOK: we don't want to handle any other embedded objects than
    // charts, there are too many problems with eg. embedded spreadsheets
    // (like it creates a separate view for the calc sheet)
    if (comphelper::LibreOfficeKit::isActive())
    {
        const auto classId = xRef->getClassID();
        if (!SotExchange::IsChart(classId) && !SotExchange::IsMath(classId))
            return;
    }

    SfxInPlaceClient* pCli = GetView().FindIPClient( xRef.GetObject(), &GetView().GetEditWin() );
    if ( !pCli )
        pCli = new SwOleClient( &GetView(), &GetView().GetEditWin(), xRef );

    uno::Reference<lang::XInitialization> xOLEInit(xRef.GetObject(), uno::UNO_QUERY);
    if (xOLEInit.is())
    {
        uno::Sequence<beans::PropertyValue> aArguments
            = { comphelper::makePropertyValue(u"ReadOnly"_ustr, pCli->IsProtected()) };
        xOLEInit->initialize({ uno::Any(aArguments) });
    }

    static_cast<SwOleClient*>(pCli)->SetInDoVerb( true );

    CalcAndSetScale( xRef );
    pCli->DoVerb( nVerb );

    static_cast<SwOleClient*>(pCli)->SetInDoVerb( false );
    CalcAndSetScale( xRef );
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
        tools::Long nX, nY;
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
        double nWidth(pCli->GetScaleWidth());
        double nHeight(pCli->GetScaleHeight());
        if (nWidth && nHeight)
        {
            aArea.Width ( aArea.Width()  / nWidth );
            aArea.Height( aArea.Height() / nHeight );
        }
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
void SwWrtShell::InsertPageBreak(const OUString *pPageDesc, const ::std::optional<sal_uInt16>& oPgNum )
{
    if (!lcl_IsAllowed(this))
        return;

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
    collectUIInformation(u"BREAK_PAGE"_ustr, u"parameter"_ustr);
}

// Insert enclosing characters
// Selections will be overwritten
void SwWrtShell::InsertEnclosingChars(std::u16string_view sStartStr, std::u16string_view sEndStr)
{
    for (SwPaM& rPaM : SwWrtShell::GetCursor()->GetRingContainer())
    {
        const OUString aStr = sStartStr + rPaM.GetText() + sEndStr;
        SwViewShell::getIDocumentContentOperations().ReplaceRange(rPaM, aStr, false);
    }
}

// Insert hard page break;
// Selections will be overwritten

void SwWrtShell::InsertLineBreak(std::optional<SwLineBreakClear> oClear)
{
    if (!lcl_IsAllowed(this))
        return;

    ResetCursorStack();
    if( CanInsert() )
    {
        if(HasSelection())
            DelRight();

        const sal_Unicode cIns = 0x0A;
        SwLineBreakClear eClear = SwLineBreakClear::NONE;
        if (oClear.has_value())
        {
            eClear = *oClear;
        }
        SvxAutoCorrect* pACorr = lcl_IsAutoCorr();
        if (pACorr && eClear == SwLineBreakClear::NONE)
            AutoCorrect( *pACorr, cIns );
        else
        {
            if (eClear == SwLineBreakClear::NONE)
            {
                SwWrtShell::Insert(OUString(cIns));
            }
            else
            {
                SwFormatLineBreak aLineBreak(eClear);
                SetAttrItem(aLineBreak);
            }
        }
    }
}

// Insert hard column break;
// Selections will be overwritten

void SwWrtShell::InsertColumnBreak()
{
    if (!lcl_IsAllowed(this))
        return;

    SwActContext aActContext(this);
    ResetCursorStack();
    if( !CanInsert() )
        return;

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

void SwWrtShell::InsertContentControl(SwContentControlType eType)
{
    if (!lcl_IsAllowed(this))
    {
        return;
    }

    ResetCursorStack();
    if (!CanInsert())
    {
        return;
    }

    auto pContentControl = std::make_shared<SwContentControl>(nullptr);
    OUString aPlaceholder;
    switch (eType)
    {
        case SwContentControlType::RICH_TEXT:
        case SwContentControlType::PLAIN_TEXT:
        {
            pContentControl->SetShowingPlaceHolder(true);
            if (eType == SwContentControlType::PLAIN_TEXT)
            {
                pContentControl->SetPlainText(true);
            }
            if (!HasSelection())
            {
                aPlaceholder = SwResId(STR_CONTENT_CONTROL_PLACEHOLDER);
            }
            break;
        }
        case SwContentControlType::CHECKBOX:
        {
            pContentControl->SetCheckbox(true);
            // Ballot Box with X
            pContentControl->SetCheckedState(u"\u2612"_ustr);
            // Ballot Box
            pContentControl->SetUncheckedState(u"\u2610"_ustr);
            aPlaceholder = u"\u2610"_ustr;
            break;
        }
        case SwContentControlType::COMBO_BOX:
        case SwContentControlType::DROP_DOWN_LIST:
        {
            if (eType == SwContentControlType::COMBO_BOX)
            {
                pContentControl->SetComboBox(true);
            }
            else if (eType == SwContentControlType::DROP_DOWN_LIST)
            {
                pContentControl->SetDropDown(true);
            }

            pContentControl->SetShowingPlaceHolder(true);
            if (!HasSelection())
            {
                aPlaceholder = SwResId(STR_DROPDOWN_CONTENT_CONTROL_PLACEHOLDER);
            }
            SwContentControlListItem aListItem;
            aListItem.m_aValue = aPlaceholder;
            pContentControl->SetListItems({ aListItem });
            break;
        }
        case SwContentControlType::PICTURE:
        {
            // Set up the picture content control.
            pContentControl->SetShowingPlaceHolder(true);
            pContentControl->SetPicture(true);

            // Create the placeholder bitmap.
            BitmapEx aBitmap(Size(1, 1), vcl::PixelFormat::N24_BPP);
            Color aColor = SvtOptionsDrawinglayer::getHilightColor();
            aColor.IncreaseLuminance(255 * 0.75);
            aBitmap.Erase(aColor);
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, SwResId(STR_GRAPHIC_DEFNAME));
            StartUndo(SwUndoId::INSERT, &aRewriter);
            LockPaint(LockPaintReason::InsertGraphic);
            StartAction();
            InsertGraphic(OUString(), OUString(), aBitmap, nullptr, RndStdIds::FLY_AS_CHAR);

            // Set properties on the bitmap.
            SfxItemSetFixed<RES_FRM_SIZE, RES_FRM_SIZE> aSet(GetDoc()->GetAttrPool());
            GetFlyFrameAttr(aSet);
            SwFormatFrameSize aSize(SwFrameSize::Fixed, 3000, 3000);
            aSet.Put(aSize);
            SetFlyFrameAttr(aSet);
            SwFrameFormat* pFrameFormat = GetFlyFrameFormat();
            EndAction();
            UnlockPaint();
            EndUndo();

            // Go after the anchor position.
            UnSelectFrame();
            LeaveSelFrameMode();
            {
                SwCursor* pCursor = getShellCursor(true);
                pCursor->DeleteMark();
                const SwFormatAnchor& rFormatAnchor = pFrameFormat->GetAnchor();
                pCursor->GetPoint()->Assign( *rFormatAnchor.GetAnchorContentNode(), rFormatAnchor.GetAnchorContentOffset() + 1);
            }

            // Select before the anchor position.
            Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
            break;
        }
        case SwContentControlType::DATE:
        {
            pContentControl->SetShowingPlaceHolder(true);
            pContentControl->SetDate(true);
            SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();
            sal_uInt32 nStandardFormat = pFormatter->GetStandardFormat(SvNumFormatType::DATE);
            const SvNumberformat* pFormat = pFormatter->GetEntry(nStandardFormat);
            pContentControl->SetDateFormat(pFormat->GetFormatstring());
            pContentControl->SetDateLanguage(LanguageTag(pFormat->GetLanguage()).getBcp47());
            if (!HasSelection())
            {
                aPlaceholder = SwResId(STR_DATE_CONTENT_CONTROL_PLACEHOLDER);
            }
            break;
        }
    }
    if (aPlaceholder.getLength())
    {
        Insert(aPlaceholder);
        Left(SwCursorSkipMode::Chars, /*bSelect=*/true, aPlaceholder.getLength(),
                /*bBasicCall=*/false);
    }

    const RedlineFlags oldRedlineFlags = getIDocumentRedlineAccess().GetRedlineFlags();
    getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::Ignore);
    SwFormatContentControl aContentControl(pContentControl, RES_TXTATR_CONTENTCONTROL);
    SetAttrItem(aContentControl);
    getIDocumentRedlineAccess().SetRedlineFlags(oldRedlineFlags);
}

// Insert footnote
// rStr - optional footnote mark

void SwWrtShell::InsertFootnote(const OUString &rStr, bool bEndNote, bool bEdit )
{
    ResetCursorStack();
    if( !CanInsert() )
        return;

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
        Left(SwCursorSkipMode::Chars, false, 1, false );
        GotoFootnoteText();
    }
    m_aNavigationMgr.addEntry(aPos);
}

// tdf#141634
static bool lcl_FoldedOutlineNodeEndOfParaSplit(SwWrtShell *pThis)
{
    SwTextNode* pTextNode = pThis->GetCursor()->GetPointNode().GetTextNode();
    if (pTextNode && pTextNode->IsOutline())
    {
        bool bVisible = true;
        pTextNode->GetAttrOutlineContentVisible(bVisible);
        if (!bVisible)
        {
            const SwNodes& rNodes = pThis->GetNodes();
            const SwOutlineNodes& rOutlineNodes = rNodes.GetOutLineNds();
            SwOutlineNodes::size_type nPos;
            (void) rOutlineNodes.Seek_Entry(pTextNode, &nPos);

            SwNode* pSttNd = rOutlineNodes[nPos];

            // determine end node of folded outline content
            SwNode* pEndNd = &rNodes.GetEndOfContent();
            if (rOutlineNodes.size() > nPos + 1)
                pEndNd = rOutlineNodes[nPos + 1];

            if (pThis->GetViewOptions()->IsTreatSubOutlineLevelsAsContent())
            {
                // get the next outline node after the folded outline content (iPos)
                // it is the next outline node with the same level or less
                int nLevel = pSttNd->GetTextNode()->GetAttrOutlineLevel();
                SwOutlineNodes::size_type iPos = nPos;
                while (++iPos < rOutlineNodes.size() &&
                       rOutlineNodes[iPos]->GetTextNode()->GetAttrOutlineLevel() > nLevel);

                // get the correct end node
                // the outline node may be in frames, headers, footers special section of doc model
                SwNode* pStartOfSectionNodeSttNd = pSttNd->StartOfSectionNode();
                while (pStartOfSectionNodeSttNd->StartOfSectionNode()
                       != pStartOfSectionNodeSttNd->StartOfSectionNode()->StartOfSectionNode())
                {
                    pStartOfSectionNodeSttNd = pStartOfSectionNodeSttNd->StartOfSectionNode();
                }
                pEndNd = pStartOfSectionNodeSttNd->EndOfSectionNode();

                if (iPos < rOutlineNodes.size())
                {
                    SwNode* pStartOfSectionNode = rOutlineNodes[iPos]->StartOfSectionNode();
                    while (pStartOfSectionNode->StartOfSectionNode()
                           != pStartOfSectionNode->StartOfSectionNode()->StartOfSectionNode())
                    {
                        pStartOfSectionNode = pStartOfSectionNode->StartOfSectionNode();
                    }
                    if (pStartOfSectionNodeSttNd == pStartOfSectionNode)
                        pEndNd = rOutlineNodes[iPos];
                }
            }

            // table, text box, header, footer
            if (pSttNd->GetTableBox() || pSttNd->GetIndex() < rNodes.GetEndOfExtras().GetIndex())
            {
                // insert before section end node
                if (pSttNd->EndOfSectionIndex() < pEndNd->GetIndex())
                {
                    SwNodeIndex aIdx(*pSttNd->EndOfSectionNode());
                    while (aIdx.GetNode().IsEndNode())
                        --aIdx;
                    ++aIdx;
                    pEndNd = &aIdx.GetNode();
                }
            }
            // if pSttNd isn't in table but pEndNd is then insert after table
            else if (pEndNd->GetTableBox())
            {
                pEndNd = pEndNd->FindTableNode();
                SwNodeIndex aIdx(*pEndNd, -1);
                // account for nested tables
                while (aIdx.GetNode().GetTableBox())
                {
                    pEndNd = aIdx.GetNode().FindTableNode();
                    aIdx.Assign(*pEndNd, -1);
                }
                aIdx.Assign(*pEndNd->EndOfSectionNode(), +1);
                pEndNd = &aIdx.GetNode();
            }
            // end node determined

            // now insert the new outline node
            SwDoc* pDoc = pThis->GetDoc();

            // insert at end of tablebox doesn't work correct without
            MakeAllOutlineContentTemporarilyVisible a(pDoc);

            SwTextNode* pNd = pDoc->GetNodes().MakeTextNode(*pEndNd, pTextNode->GetTextColl(), true);

            (void) rOutlineNodes.Seek_Entry(pNd, &nPos);
            pThis->GotoOutline(nPos);

            if (pDoc->GetIDocumentUndoRedo().DoesUndo())
            {
                pDoc->GetIDocumentUndoRedo().ClearRedo();
                pDoc->GetIDocumentUndoRedo().AppendUndo(std::make_unique<SwUndoInsert>(*pNd));
                pDoc->GetIDocumentUndoRedo().AppendUndo(std::make_unique<SwUndoFormatColl>
                                                        (SwPaM(*pNd), pNd->GetTextColl(), true, true));
            }

            pThis->SetModified();
            return true;
        }
    }
    return false;
}

// SplitNode; also, because
//                  - of deleting selected content;
//                  - of reset of the Cursorstack if necessary.

void SwWrtShell::SplitNode( bool bAutoFormat )
{
    ResetCursorStack();
    if( !CanInsert() )
        return;

    SwActContext aActContext(this);

    m_rView.GetEditWin().FlushInBuffer();
    StartUndo(SwUndoId::SPLITNODE);

    bool bHasSel = HasSelection();
    if (bHasSel)
        DelRight();

    bool bHandled = false;
    if (GetViewOptions()->IsShowOutlineContentVisibilityButton() && IsEndPara())
        bHandled = lcl_FoldedOutlineNodeEndOfParaSplit(this);

    if (!bHandled)
        SwFEShell::SplitNode( bAutoFormat );

    EndUndo(SwUndoId::SPLITNODE);
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
    StartUndo(SwUndoId::NUMORNONUM);

    const SwNumRule* pNumRule = GetNumRuleAtCurrCursorPos();

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
                        *GetLayout(), GetCursor()->GetPoint()->GetNode());
                if ( pTextNode && !pTextNode->IsCountedInList() )
                {
                    // check, if numbering of the outline level of the paragraph
                    // style is active. If not, activate this outline level.
                    nActivateOutlineLvl = pColl->GetAssignedOutlineStyleLevel();
                    OSL_ENSURE( pColl->IsAssignedToListLevelOfOutlineStyle(),
                            "<SwWrtShell::NumOrBulletOn(..)> - paragraph style with outline rule, but no outline level" );
                    if ( pColl->IsAssignedToListLevelOfOutlineStyle() &&
                         pCollRule->Get( o3tl::narrowing<sal_uInt16>(nActivateOutlineLvl) ).GetNumberingType()
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
                if ( pCollRule->Get( o3tl::narrowing<sal_uInt16>(nActivateOutlineLvl) ).GetNumberingType()
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
                     pCollRule->Get( o3tl::narrowing<sal_uInt16>(nActivateOutlineLvl) ).GetNumberingType()
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
                    *GetLayout(), GetCursor()->GetPoint()->GetNode());

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

                SwNumFormat aFormat(aNumRule.Get(o3tl::narrowing<sal_uInt16>(nLevel)));

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
                    aFormat.SetListFormat(u""_ustr, u""_ustr, nLevel);
                }
                aNumRule.Set(o3tl::narrowing<sal_uInt16>(nLevel), aFormat);
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
            pChrFormat = GetCharFormatFromPool( RES_POOLCHR_BULLET_LEVEL );
        }

        const SwTextNode *const pTextNode = sw::GetParaPropsNode(*GetLayout(),
                GetCursor()->GetPoint()->GetNode());
        const SwTwips nWidthOfTabs = pTextNode
                                     ? pTextNode->GetWidthOfLeadingTabs()
                                     : 0;
        GetDoc()->getIDocumentContentOperations().RemoveLeadingWhiteSpace(*GetCursor());

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
                aFormat.SetListFormat(u""_ustr, u""_ustr, nLvl);
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

    if (!pCurNumRule)
        return;

    DelNumRules();

    // #126346# - Cursor can not be anymore in front of
    // a label, because numbering/bullet is switched off.
    SetInFrontOfLabel( false );
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

            if (svx::checkForSelectedFontWork( GetDrawView() ))
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
    {
        nCnt |= SelectionType::Table | SelectionType::TableCell;
        SwTable::SearchType eTableSel = GetEnhancedTableSelection();
        if ( eTableSel == SwTable::SEARCH_ROW )
            nCnt |= SelectionType::TableRow;
        else if ( eTableSel == SwTable::SEARCH_COL )
            nCnt |= SelectionType::TableCol;
    }

    // Do not pop up numbering toolbar, if the text node has a numbering of type SVX_NUM_NUMBER_NONE.
    const SwNumRule* pNumRule = GetNumRuleAtCurrCursorPos();
    if ( pNumRule )
    {
        const SwTextNode* pTextNd =
            sw::GetParaPropsNode(*GetLayout(), GetCursor()->GetPoint()->GetNode());

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

SwFrameFormat *SwWrtShell::GetTableStyle(std::u16string_view rFormatName)
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
    SfxItemSetFixed<
            RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
            RES_PARATR_BEGIN, RES_PARATR_END - 1,
            RES_FRMATR_BEGIN, RES_FRMATR_END - 1,
            SID_ATTR_TABSTOP_DEFAULTS,SID_ATTR_TABSTOP_OFFSET,
            SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
            SID_ATTR_PARA_MODEL, SID_ATTR_PARA_KEEP,
            SID_ATTR_PARA_PAGENUM, SID_ATTR_PARA_PAGENUM>  aCoreSet( GetAttrPool() );
    GetPaMAttr( pCursor, aCoreSet );
    bool bReset = false;

    // ITEM: SfxItemIter and removing SfxPoolItems:
    std::vector<sal_uInt16> aDeleteWhichIDs;

    for (SfxItemIter aIter(aCoreSet); !aIter.IsAtEnd(); aIter.NextItem())
    {
        if(!IsInvalidItem(aIter.GetCurItem()))
        {
            if (SfxItemState::SET == aIter.GetItemState() &&
                SfxItemState::SET == rStyleSet.GetItemState(aIter.GetCurWhich()))
            {
                aDeleteWhichIDs.push_back(aIter.GetCurWhich());
                bReset = true;
            }
        }
    }

    for (auto nDelWhich : aDeleteWhichIDs)
        aCoreSet.ClearItem(nDelWhich);

    StartAction();
    if(bReset)
    {
        ResetAttr({}, pCursor);
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
    if(!CanInsert())
        return;

    bool bStarted = false;
    SwRewriter aRewriter;

    if(HasSelection())
    {
            // Only parentheses here, because the regular insert
            // is already clipped to the editshell
        StartAllAction();

        OUString aTmpStr1 = SwResId(STR_START_QUOTE) +
            GetSelText() +
            SwResId(STR_END_QUOTE);
        OUString aTmpStr3 = SwResId(STR_START_QUOTE) +
            OUStringChar(cChar) +
            SwResId(STR_END_QUOTE);
        aRewriter.AddRule( UndoArg1, aTmpStr1 );
        aRewriter.AddRule( UndoArg2, SwResId(STR_YIELDS) );
        aRewriter.AddRule( UndoArg3, aTmpStr3 );

        StartUndo( SwUndoId::REPLACE, &aRewriter );
        bStarted = true;
        DelRight(true);
    }
    SwEditShell::AutoCorrect( rACorr, IsInsMode(), cChar );

    if(bStarted)
    {
        EndAllAction();
        EndUndo( SwUndoId::REPLACE, &aRewriter );
    }
}

// Some kind of controlled copy ctor

SwWrtShell::SwWrtShell( SwWrtShell& rSh, vcl::Window *_pWin, SwView &rShell )
    : SwFEShell(rSh, _pWin)
    , m_rView(rShell)
    , m_aNavigationMgr(*this)
{
    BITFLD_INI_LIST
    CurrShell aCurr( this );

    SetSfxViewShell( static_cast<SfxViewShell *>(&rShell) );
    SetFlyMacroLnk( LINK(this, SwWrtShell, ExecFlyMac) );

    // place the cursor on the first field...
    IFieldmark *pBM = nullptr;
    if (IsFormProtected() && (pBM = GetFieldmarkAfter()) !=nullptr) {
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
    CurrShell aCurr( this );
    SetSfxViewShell( static_cast<SfxViewShell *>(&rShell) );
    SetFlyMacroLnk( LINK(this, SwWrtShell, ExecFlyMac) );

    // place the cursor on the first field...
    IFieldmark *pBM = nullptr;
    if (IsFormProtected() && (pBM = GetFieldmarkAfter()) !=nullptr) {
        GotoFieldmark(pBM);
    }
}

SwWrtShell::~SwWrtShell()
{
    CurrShell aCurr( this );
    while(IsModePushed())
        PopMode();
    while(PopCursor(false))
        ;
    SwTransferable::ClearSelection( *this );
}

bool SwWrtShell::Pop(SwCursorShell::PopMode const eDelete)
{
    ::std::optional<SwCallLink> aLink(std::in_place, *this);
    return Pop(eDelete, aLink);
}

bool SwWrtShell::Pop(SwCursorShell::PopMode const eDelete, ::std::optional<SwCallLink>& roLink)
{
    bool bRet = SwCursorShell::Pop(eDelete, roLink);
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
    GetView().GetViewFrame().GetBindings().Invalidate( FN_VIEW_META_CHARS );
}

void SwWrtShell::SetReadonlyOption(bool bSet)
{
    GetView().GetEditWin().GetFrameControlsManager().SetReadonlyControls( bSet );
    SwViewShell::SetReadonlyOption( bSet );
}

// Switch on/off header or footer of a page style - if an empty name is
// given all styles are changed

void SwWrtShell::ChangeHeaderOrFooter(
    std::u16string_view rStyleName, bool bHeader, bool bOn, bool bShowWarning)
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
        if( rStyleName.empty() || rStyleName == sTmp )
        {
            bool bChgd = false;

            if( bShowWarning && !bOn && GetActiveView() && GetActiveView() == &GetView() &&
                ( (bHeader && aDesc.GetMaster().GetHeader().IsActive()) ||
                  (!bHeader && aDesc.GetMaster().GetFooter().IsActive()) ) )
            {
                bShowWarning = false;
                //Actions have to be closed while the dialog is showing
                EndAllAction();

                weld::Window* pParent = GetView().GetFrameWeld();
                short nResult;
                if (bHeader) {
                    nResult = DeleteHeaderDialog(pParent).run();
                } else {
                    nResult = DeleteFooterDialog(pParent).run();
                }

                bExecute = nResult == RET_YES;
                StartAllAction();
                if (nResult == RET_YES)
                    ToggleHeaderFooterEdit();
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
                    // keep in sync with FN_PGNUMBER_WIZARD
                    constexpr tools::Long constTwips_5mm = o3tl::toTwips(5, o3tl::Length::mm);
                    SvxULSpaceItem aUL(bHeader ? 0 : constTwips_5mm, bHeader ? constTwips_5mm : 0, RES_UL_SPACE );
                    SwFrameFormat* pFormat = bHeader ?
                        const_cast<SwFrameFormat*>(rMaster.GetHeader().GetHeaderFormat()) :
                        const_cast<SwFrameFormat*>(rMaster.GetFooter().GetFooterFormat());
                    pFormat->SetFormatAttr( aUL );
                    XFillStyleItem aFill(drawing::FillStyle_NONE);
                    pFormat->SetFormatAttr(aFill);
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
                            rStyleName.empty() ? SIZE_MAX : nFrom,
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

void SwWrtShell::InsertPostIt(SwFieldMgr& rFieldMgr, const SfxRequest& rReq)
{
    SwPostItField* pPostIt = dynamic_cast<SwPostItField*>(rFieldMgr.GetCurField());
    bool bNew = !(pPostIt && pPostIt->GetTyp()->Which() == SwFieldIds::Postit);
    if (bNew || GetView().GetPostItMgr()->IsAnswer())
    {
        const SvxPostItAuthorItem* pAuthorItem = rReq.GetArg<SvxPostItAuthorItem>(SID_ATTR_POSTIT_AUTHOR);
        OUString sAuthor;
        if ( pAuthorItem )
            sAuthor = pAuthorItem->GetValue();
        else
        {
            std::size_t nAuthor = SW_MOD()->GetRedlineAuthor();
            sAuthor = SW_MOD()->GetRedlineAuthor(nAuthor);
        }

        const SvxPostItTextItem* pTextItem = rReq.GetArg<SvxPostItTextItem>(SID_ATTR_POSTIT_TEXT);
        OUString sText;
        if ( pTextItem )
            sText = pTextItem->GetValue();

        // If we have a text already registered for answer, use that
        if (GetView().GetPostItMgr()->IsAnswer() && !GetView().GetPostItMgr()->GetAnswerText().isEmpty())
        {
            sText = GetView().GetPostItMgr()->GetAnswerText();
            GetView().GetPostItMgr()->RegisterAnswerText(OUString());
        }

        if ( HasSelection() && !IsTableMode() )
        {
            KillPams();
        }

        // #i120513# Inserting a comment into an autocompletion crashes
        // --> suggestion has to be removed before
        GetView().GetEditWin().StopQuickHelp();

        SwInsertField_Data aData(SwFieldTypesEnum::Postit, 0, sAuthor, sText, 0);

        if (IsSelFrameMode())
        {
            SwFlyFrame* pFly = GetSelectedFlyFrame();

            // Remember the anchor of the selected object before deletion.
            std::optional<SwPosition> oAnchor;
            if (pFly)
            {
                SwFrameFormat* pFormat = pFly->GetFormat();
                if (pFormat)
                {
                    RndStdIds eAnchorId = pFormat->GetAnchor().GetAnchorId();
                    if ((eAnchorId == RndStdIds::FLY_AS_CHAR || eAnchorId == RndStdIds::FLY_AT_CHAR) && pFormat->GetAnchor().GetAnchorNode())
                    {
                        oAnchor.emplace(*pFormat->GetAnchor().GetContentAnchor());
                    }
                }
            }

            // A frame is selected, end frame selection.
            EnterStdMode();
            GetView().AttrChangedNotify(nullptr);

            // Set up text selection, so the anchor of the frame will be the anchor of the
            // comment.
            if (pFly)
            {
                if (oAnchor)
                    *GetCurrentShellCursor().GetPoint() = *oAnchor;
                SwFrameFormat* pFormat = pFly->GetFormat();
                if (pFormat && pFormat->GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR)
                {
                    Right(SwCursorSkipMode::Cells, /*bSelect=*/true, 1, /*bBasicCall=*/false, /*bVisual=*/true);
                }
                else if (pFormat && pFormat->GetAnchor().GetAnchorId() == RndStdIds::FLY_AT_CHAR)
                {
                    aData.m_oAnnotationRange.emplace(*GetCurrentShellCursor().Start(),
                                                     *GetCurrentShellCursor().End());
                }
            }
        }

        rFieldMgr.InsertField( aData );

        Push();
        SwCursorShell::Left(1, SwCursorSkipMode::Chars);
        pPostIt = static_cast<SwPostItField*>(rFieldMgr.GetCurField());
        Pop(SwCursorShell::PopMode::DeleteCurrent); // Restore cursor position
    }

    // Client has disabled annotations rendering, no need to
    // focus the postit field
    if (comphelper::LibreOfficeKit::isActive() && !comphelper::LibreOfficeKit::isTiledAnnotations())
        return;

    if (pPostIt)
    {
        SwFieldType* pType = GetDoc()->getIDocumentFieldsAccess().GetFieldType(SwFieldIds::Postit, OUString(), false);
        if(auto pFormat = pType->FindFormatForField(pPostIt))
            pFormat->Broadcast( SwFormatFieldHint( nullptr, SwFormatFieldHintWhich::FOCUS, &GetView() ) );
    }
}

bool SwWrtShell::IsOutlineContentVisible(const size_t nPos)
{
    const SwOutlineNodes& rOutlineNodes = GetDoc()->GetNodes().GetOutLineNds();
    const SwNode* pOutlineNode = rOutlineNodes[nPos];

    // no layout frame means outline folding is set to include sub levels and the outline node has
    // a parent outline node with outline content visible attribute false (folded outline content)
    if (!pOutlineNode->GetTextNode()->getLayoutFrame(GetLayout()))
        return false;

    // try the next node to determine if this outline node has visible content
    SwNodeIndex aIdx(*pOutlineNode, +1);
    if (aIdx.GetNode() == aIdx.GetNodes().GetEndOfContent()) // end of regular content
        return false;

    if (aIdx.GetNode().IsTextNode() || aIdx.GetNode().IsTableNode() ||
            aIdx.GetNode().IsSectionNode())
    {
        // * sublevels treated as outline content
        //     If next node (aIdx) doesn't have a layout frame
        //     then this outline node does not have visible outline content.
        // * sublevels NOT treated as outline content
        //     If the next node (aIdx) is the next outline node
        //     then return the outline content visible attribute value.
        if (!GetViewOptions()->IsTreatSubOutlineLevelsAsContent() &&
                nPos + 1 < rOutlineNodes.size() &&
                rOutlineNodes[nPos + 1] == &aIdx.GetNode())
            return GetAttrOutlineContentVisible(nPos);

        if (aIdx.GetNode().IsTextNode())
            return aIdx.GetNode().GetTextNode()->getLayoutFrame(GetLayout());
        if (aIdx.GetNode().IsTableNode())
        {
            SwTable& rTable = aIdx.GetNode().GetTableNode()->GetTable();
            return rTable.HasLayout();
        }
        if (aIdx.GetNode().IsSectionNode())
        {
            const auto pFormat = aIdx.GetNode().GetSectionNode()->GetSection().GetFormat();
            return pFormat && pFormat->IsVisible();
        }
    }

    return true;
}

void SwWrtShell::MakeOutlineLevelsVisible(const int nLevel)
{
    MakeAllOutlineContentTemporarilyVisible a(GetDoc());

    m_rView.SetMaxOutlineLevelShown(nLevel);

    bool bDocChanged = false;

    const SwOutlineNodes& rOutlineNodes = GetNodes().GetOutLineNds();

    // Make all missing frames.
    for (SwOutlineNodes::size_type nPos = 0; nPos < rOutlineNodes.size(); ++nPos)
    {
        SwNode* pNode = rOutlineNodes[nPos];
        if (!pNode->GetTextNode()->getLayoutFrame(GetLayout()))
        {
            SwNodeIndex aIdx(*pNode, +1);
            // Make the outline paragraph frame
            MakeFrames(GetDoc(), *pNode, aIdx.GetNode());
            // Make the outline content visible but don't set the outline visible attribute and
            // don't make outline content made visible not visible that have outline visible
            // attribute false. Visibility will be taken care of when
            // MakeAllOutlineContentTemporarilyVisible goes out of scope.
            MakeOutlineContentVisible(nPos, true, false);
            bDocChanged = true;
        }
    }
    // Remove outline paragraph frame and outline content frames above given level.
    for (SwOutlineNodes::size_type nPos = 0; nPos < rOutlineNodes.size(); ++nPos)
    {
        SwNode* pNode = rOutlineNodes[nPos];
        auto nOutlineLevel = pNode->GetTextNode()->GetAttrOutlineLevel();
        if (nOutlineLevel > nLevel)
        {
            // Remove the outline content but don't set the outline visible attribute. Visibility
            // will be taken care of when MakeAllOutlineContentTemporarilyVisible goes out of scope.
            MakeOutlineContentVisible(nPos, false, false);
            // Remove the outline paragraph frame.
            pNode->GetTextNode()->DelFrames(GetLayout());
            bDocChanged = true;
        }
    }

    // Broadcast DocChanged if document layout has changed so the Navigator will be updated.
    if (bDocChanged)
        GetDoc()->GetDocShell()->Broadcast(SfxHint(SfxHintId::DocChanged));
}

void SwWrtShell::MakeOutlineContentVisible(const size_t nPos, bool bMakeVisible, bool bSetAttrOutlineVisibility)
{
    const SwNodes& rNodes = GetNodes();
    const SwOutlineNodes& rOutlineNodes = rNodes.GetOutLineNds();

    SwNode* pSttNd = rOutlineNodes[nPos];

    // determine end node
    SwNode* pEndNd = &rNodes.GetEndOfContent();
    if (rOutlineNodes.size() > nPos + 1)
        pEndNd = rOutlineNodes[nPos + 1];

    if (GetViewOptions()->IsTreatSubOutlineLevelsAsContent())
    {
        // get the last outline node to include (iPos)
        int nLevel = pSttNd->GetTextNode()->GetAttrOutlineLevel();
        int nMaxOutlineLevelShown = m_rView.GetMaxOutlineLevelShown();
        SwOutlineNodes::size_type iPos = nPos;
        while (++iPos < rOutlineNodes.size() &&
               rOutlineNodes[iPos]->GetTextNode()->GetAttrOutlineLevel() > nLevel &&
               rOutlineNodes[iPos]->GetTextNode()->GetAttrOutlineLevel() <= nMaxOutlineLevelShown);

        // get the correct end node
        // the outline node may be in frames, headers, footers special section of doc model
        SwNode* pStartOfSectionNodeSttNd = pSttNd->StartOfSectionNode();
        while (pStartOfSectionNodeSttNd->StartOfSectionNode()
               != pStartOfSectionNodeSttNd->StartOfSectionNode()->StartOfSectionNode())
        {
            pStartOfSectionNodeSttNd = pStartOfSectionNodeSttNd->StartOfSectionNode();
        }
        pEndNd = pStartOfSectionNodeSttNd->EndOfSectionNode();

        if (iPos < rOutlineNodes.size())
        {
            SwNode* pStartOfSectionNode = rOutlineNodes[iPos]->StartOfSectionNode();
            while (pStartOfSectionNode->StartOfSectionNode()
                   != pStartOfSectionNode->StartOfSectionNode()->StartOfSectionNode())
            {
                pStartOfSectionNode = pStartOfSectionNode->StartOfSectionNode();
            }
            if (pStartOfSectionNodeSttNd == pStartOfSectionNode)
                pEndNd = rOutlineNodes[iPos];
        }
    }

    // table, text box, header, footer
    if (pSttNd->GetTableBox() || pSttNd->GetIndex() < rNodes.GetEndOfExtras().GetIndex())
    {
        // limit to within section
        if (pSttNd->EndOfSectionIndex() < pEndNd->GetIndex())
            pEndNd = pSttNd->EndOfSectionNode();
    }
    // if pSttNd isn't in table but pEndNd is, skip over all outline nodes in table
    else if (pEndNd->GetTableBox())
    {
        pEndNd = &rNodes.GetEndOfContent();
        for (size_t nOutlinePos = nPos + 2; nOutlinePos < rOutlineNodes.size(); nOutlinePos++)
        {
            if (!(rOutlineNodes[nOutlinePos]->GetTableBox()))
            {
                pEndNd = rOutlineNodes[nOutlinePos];
                break;
            }
        }
    }
    // end node determined

    // Remove content frames from the next node after the starting outline node to
    // the determined ending node. Always do this to prevent the chance of duplicate
    // frames being made. They will be remade below if needed.
    SwNodeIndex aIdx(*pSttNd, +1);
    while (aIdx != *pEndNd)
    {
        SwNode* pNd = &aIdx.GetNode();
        if (pNd->IsContentNode())
            pNd->GetContentNode()->DelFrames(nullptr);
        else if (pNd->IsTableNode())
            pNd->GetTableNode()->DelFrames(nullptr);
        ++aIdx;
    }

    if (bMakeVisible) // make outline nodes outline content visible
    {
        // reset the index marker and make frames
        aIdx.Assign(*pSttNd, +1);
        MakeFrames(GetDoc(), aIdx.GetNode(), *pEndNd);

        if (bSetAttrOutlineVisibility)
        {
            pSttNd->GetTextNode()->SetAttrOutlineContentVisible(true);

            // make outline content made visible that have outline visible attribute false not visible
            while (aIdx != *pEndNd)
            {
                SwNode* pNd = &aIdx.GetNode();
                if (pNd->IsTextNode() && pNd->GetTextNode()->IsOutline())
                {
                    SwTextNode* pTextNd = pNd->GetTextNode();
                    bool bOutlineContentVisibleAttr = true;
                    pTextNd->GetAttrOutlineContentVisible(bOutlineContentVisibleAttr);
                    if (!bOutlineContentVisibleAttr)
                    {
                        SwOutlineNodes::size_type iPos;
                        if (rOutlineNodes.Seek_Entry(pTextNd, &iPos))
                        {
                            if (pTextNd->getLayoutFrame(nullptr))
                                MakeOutlineContentVisible(iPos, false);
                        }
                    }
                }
                ++aIdx;
            }
        }
    }
    else if (bSetAttrOutlineVisibility)
        pSttNd->GetTextNode()->SetAttrOutlineContentVisible(false);
}

// make content visible or not visible only if needed
void SwWrtShell::InvalidateOutlineContentVisibility()
{
    GetView().GetEditWin().GetFrameControlsManager().HideControls(FrameControlType::Outline);

    const SwOutlineNodes& rOutlineNds = GetNodes().GetOutLineNds();
    for (SwOutlineNodes::size_type nPos = 0; nPos < rOutlineNds.size(); ++nPos)
    {
        bool bIsOutlineContentVisible = IsOutlineContentVisible(nPos);
        bool bOutlineContentVisibleAttr = true;
        rOutlineNds[nPos]->GetTextNode()->GetAttrOutlineContentVisible(bOutlineContentVisibleAttr);
        if (!bIsOutlineContentVisible && bOutlineContentVisibleAttr)
            MakeOutlineContentVisible(nPos);
        else if (bIsOutlineContentVisible && !bOutlineContentVisibleAttr)
            MakeOutlineContentVisible(nPos, false);
    }
}

void SwWrtShell::MakeAllFoldedOutlineContentVisible(bool bMakeVisible)
{
    if (bMakeVisible)
    {
        // make all content visible

        // When shortcut is assigned to the show outline content visibility button and used to
        // toggle the feature and the mouse pointer is on an outline frame the button will not
        // be removed. An easy way to make sure the button does not remain shown is to use the
        // HideControls function.
        GetView().GetEditWin().GetFrameControlsManager().HideControls(FrameControlType::Outline);

        // temporarily set outline content visible attribute true for folded outline nodes
        std::vector<SwNode*> aFoldedOutlineNodeArray;
        for (SwNode* pNd: GetNodes().GetOutLineNds())
        {
            bool bOutlineContentVisibleAttr = true;
            pNd->GetTextNode()->GetAttrOutlineContentVisible(bOutlineContentVisibleAttr);
            if (!bOutlineContentVisibleAttr)
            {
                aFoldedOutlineNodeArray.push_back(pNd);
                pNd->GetTextNode()->SetAttrOutlineContentVisible(true);
            }
        }

        StartAction();
        InvalidateOutlineContentVisibility();
        EndAction();

        // restore outline content visible attribute for folded outline nodes
        for (SwNode* pNd: aFoldedOutlineNodeArray)
            pNd->GetTextNode()->SetAttrOutlineContentVisible(false);
    }
    else
    {
        AssureStdMode();

        // Get the outline position of the cursor so the cursor can be place at a visible outline
        // node if it is not visible after InvalidateOutlineContentVisiblity below.
        SwOutlineNodes::size_type nPos = GetOutlinePos();

        StartAction();
        InvalidateOutlineContentVisibility();
        EndAction();

        // If needed, find a visible outline node to place the cursor.
        if (nPos != SwOutlineNodes::npos && !IsOutlineContentVisible(nPos))
        {
            while (nPos != SwOutlineNodes::npos &&
                   !GetNodes().GetOutLineNds()[nPos]->GetTextNode()->getLayoutFrame(GetLayout()))
                --nPos;
            if (nPos != SwOutlineNodes::npos)
                GotoOutline(nPos);
        }
    }
    GetView().GetDocShell()->Broadcast(SfxHint(SfxHintId::DocChanged));
}

bool SwWrtShell::GetAttrOutlineContentVisible(const size_t nPos) const
{
    bool bVisibleAttr = true;
    GetNodes().GetOutLineNds()[nPos]->GetTextNode()->GetAttrOutlineContentVisible(bVisibleAttr);
    return bVisibleAttr;
}

bool SwWrtShell::HasFoldedOutlineContentSelected() const
{
    // No need to check for selection over folded outline content when there are no outline nodes.
    if (GetDoc()->GetNodes().GetOutLineNds().empty())
        return false;
    for(const SwPaM& rPaM : GetCursor()->GetRingContainer())
    {
        SwPaM aPaM(*rPaM.GetMark(), *rPaM.GetPoint());
        aPaM.Normalize();
        SwNodeIndex aPointIdx(aPaM.GetPoint()->GetNode());
        SwNodeIndex aMarkIdx(aPaM.GetMark()->GetNode());
        // Prevent crash in the for loop below by adjusting the mark if it is set to the end of
        // content node.
        if (aMarkIdx.GetNode() == GetDoc()->GetNodes().GetEndOfContent())
            --aMarkIdx;
        if (aPointIdx == aMarkIdx)
            continue;
        // Return true if any nodes in PaM are folded outline content nodes.
        SwOutlineNodes::size_type nPos;
        for (SwNodeIndex aIdx = aPointIdx; aIdx <= aMarkIdx; ++aIdx)
        {
            // To allow delete when the start of the selection is at the start of a
            // paragraph and the end of the selection is at the start of a paragraph and there
            // are no folded outline content nodes in between.
            if (aIdx == aMarkIdx && aPaM.GetPoint()->GetContentIndex() == 0 &&
                    aPaM.GetMark()->GetContentIndex() == 0)
                return false;

            if (GetDoc()->GetNodes().GetOutLineNds().Seek_Entry(&(aIdx.GetNode()), &nPos) &&
                    !GetAttrOutlineContentVisible(nPos))
                return true;
        }
    }
    return false;
}

void SwWrtShell::InfoReadOnlyDialog(bool bAsync) const
{
    if (bAsync)
    {
        auto xInfo = std::make_shared<weld::MessageDialogController>(
                    GetView().GetFrameWeld(), "modules/swriter/ui/inforeadonlydialog.ui", "InfoReadonlyDialog");
        if (GetViewOptions()->IsShowOutlineContentVisibilityButton() &&
                HasFoldedOutlineContentSelected())
        {
            xInfo->set_primary_text(SwResId(STR_INFORODLG_FOLDED_PRIMARY));
            xInfo->set_secondary_text(SwResId(STR_INFORODLG_FOLDED_SECONDARY));
        }
        weld::DialogController::runAsync(xInfo, [](int) {});
    }
    else
    {
        std::unique_ptr<weld::Builder>
                xBuilder(Application::CreateBuilder(GetView().GetFrameWeld(),
                                                    u"modules/swriter/ui/inforeadonlydialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog>
                xInfo(xBuilder->weld_message_dialog(u"InfoReadonlyDialog"_ustr));
        if (GetViewOptions()->IsShowOutlineContentVisibilityButton() &&
                HasFoldedOutlineContentSelected())
        {
            xInfo->set_primary_text(SwResId(STR_INFORODLG_FOLDED_PRIMARY));
            xInfo->set_secondary_text(SwResId(STR_INFORODLG_FOLDED_SECONDARY));
        }
        xInfo->run();
    }
}

bool SwWrtShell::WarnHiddenSectionDialog() const
{
    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(
        GetView().GetFrameWeld(), u"modules/swriter/ui/warnhiddensectiondialog.ui"_ustr));
    std::unique_ptr<weld::MessageDialog> xQuery(
        xBuilder->weld_message_dialog(u"WarnHiddenSectionDialog"_ustr));
    if (GetViewOptions()->IsShowOutlineContentVisibilityButton()
        && HasFoldedOutlineContentSelected())
    {
        xQuery->set_primary_text(SwResId(STR_INFORODLG_FOLDED_PRIMARY));
        xQuery->set_secondary_text(SwResId(STR_INFORODLG_FOLDED_SECONDARY));
    }

    return (RET_YES == xQuery->run());
}

bool SwWrtShell::WarnSwitchToDesignModeDialog() const
{
    std::unique_ptr<weld::MessageDialog> xQuery(Application::CreateMessageDialog(nullptr,
        VclMessageType::Question, VclButtonsType::YesNo, SwResId(STR_A11Y_DESIGN_MODE_PRIMARY)));
    xQuery->set_default_response(RET_YES);
    xQuery->set_title(SwResId(STR_A11Y_DESIGN_MODE_TITLE));
    xQuery->set_secondary_text(SwResId(STR_A11Y_DESIGN_MODE_SECONDARY));

    return (RET_YES == xQuery->run());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
