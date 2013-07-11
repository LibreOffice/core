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


#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/util/XModifiable.hpp>

#include <svx/dialogs.hrc>

#include <math.h>
#include <hintids.hxx>
#include <svx/hdft.hxx>
#include <svx/svdview.hxx>
#include <sot/factory.hxx>
#include <svl/itemiter.hxx>
#include <tools/bigint.hxx>
#include <sot/storage.hxx>
#include <svtools/insdlg.hxx>
#include <sfx2/frmdescr.hxx>
#include <sfx2/ipclient.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/soerr.hxx>
#include <unotools/moduleoptions.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/ulspitem.hxx>
#include <vcl/graph.hxx>
#include <vcl/msgbox.hxx>
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
#include <swundo.hxx>  // for Undo-Ids
#include <swcli.hxx>
#include <poolfmt.hxx>
#include <wview.hxx>
#include <edtwin.hxx>
#include <fmtcol.hxx>
#include <swtable.hxx>
#include <caption.hxx>
#include <viscrs.hxx>
#include <swdtflvr.hxx>
#include <crsskip.hxx>
#include <doc.hxx>
#include <wrtsh.hrc>
#include <SwStyleNameMapper.hxx>
#include <sfx2/request.hxx>
#include <paratr.hxx>
#include <ndtxt.hxx>
#include <editeng/acorrcfg.hxx>
#include <IMark.hxx>
#include <sfx2/bindings.hxx>
#include <svx/dialmgr.hxx>

// -> #111827#
#include <SwRewriter.hxx>
#include <comcore.hrc>
// <- #111827#

#include <toolkit/helper/vclunohelper.hxx>
#include <sfx2/viewfrm.hxx>

#include "PostItMgr.hxx"
#include <sfx2/msgpool.hxx>

using namespace sw::mark;
using namespace com::sun::star;

#define COMMON_INI_LIST \
        fnDrag(&SwWrtShell::BeginDrag),\
        fnSetCrsr(&SwWrtShell::SetCrsr),\
        fnEndDrag(&SwWrtShell::EndDrag),\
        fnKillSel(&SwWrtShell::Ignore),\
        pModeStack(0), \
        ePageMove(MV_NO),\
        pCrsrStack(0),  \
        rView(rShell),\
        aNavigationMgr(*this), \
        bDestOnStack(false)

#define BITFLD_INI_LIST \
        bClearMark = \
        bIns = sal_True;\
        bAddMode = \
        bBlockMode = \
        bExtMode = \
        bInSelect = \
        bCopy = \
        bLayoutMode = \
        bSelWrd = \
        bSelLn = \
        mbRetainSelection = sal_False; \
        bIsInClickToEdit = false;

static SvxAutoCorrect* lcl_IsAutoCorr()
{
    SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get().GetAutoCorrect();
    if( pACorr && !pACorr->IsAutoCorrFlag( CptlSttSntnc | CptlSttWrd |
                            AddNonBrkSpace | ChgOrdinalNumber |
                            ChgToEnEmDash | SetINetAttr | Autocorrect ))
        pACorr = 0;
    return pACorr;
}

void SwWrtShell::NoEdit(bool bHideCrsr)
{
    if(bHideCrsr)
        HideCrsr();
}

void SwWrtShell::Edit()
{
    if (CanInsert())
    {
        ShowCrsr();
    }
}

sal_Bool SwWrtShell::IsEndWrd()
{
    SwMvContext aMvContext(this);
    if(IsEndPara() && !IsSttPara())
        return sal_True;

    return IsEndWord();
}

// Insert string

void SwWrtShell::InsertByWord( const String & rStr)
{
    if( rStr.Len() )
    {
        sal_Bool bDelim = GetAppCharClass().isLetterNumeric( rStr, 0 );
        xub_StrLen nPos = 0, nStt = 0;
        for( ; nPos < rStr.Len(); nPos++ )
           {
            sal_Bool bTmpDelim = GetAppCharClass().isLetterNumeric( rStr, nPos );
            if( bTmpDelim != bDelim )
            {
                Insert( rStr.Copy( nStt, nPos - nStt ));
                nStt = nPos;
            }
        }
        if( nStt != nPos )
            Insert( rStr.Copy( nStt, nPos - nStt ));
    }
}

void SwWrtShell::Insert( const String &rStr )
{
    ResetCursorStack();
    if( !CanInsert() )
        return;

    bool bStarted = false;
    sal_Bool bHasSel = HasSelection(),
        bCallIns = bIns /*|| bHasSel*/;
    bool bDeleted = false;

    if( bHasSel || ( !bIns && SelectHiddenRange() ) )
    {
            // Only here parenthesizing, because the normal
            // insert is already in parentheses at Editshell.
        StartAllAction();

        // #111827#
        SwRewriter aRewriter;

        aRewriter.AddRule(UndoArg1, GetCrsrDescr());
        aRewriter.AddRule(UndoArg2, String(SW_RES(STR_YIELDS)));
        {
            String aTmpStr;
            aTmpStr += String(SW_RES(STR_START_QUOTE));
            aTmpStr += rStr;
            aTmpStr += String(SW_RES(STR_END_QUOTE));

            aRewriter.AddRule(UndoArg3, rStr);
        }

        StartUndo(UNDO_REPLACE, &aRewriter);
        bStarted = true;
        bDeleted = DelRight() != 0;
    }

    bCallIns ?
        SwEditShell::Insert2( rStr, bDeleted ) : SwEditShell::Overwrite( rStr );

    if( bStarted )
    {
        EndAllAction();
        EndUndo();
    }
}

// Maximum height limit not possible, because the maximum height
// of the current frame can not be obtained.

void SwWrtShell::Insert( const String &rPath, const String &rFilter,
                         const Graphic &rGrf, SwFlyFrmAttrMgr *pFrmMgr,
                         sal_Bool bRule )
{
    ResetCursorStack();
    if ( !CanInsert() )
        return;

    StartAllAction();

    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, SW_RES(STR_GRAPHIC));

    StartUndo(UNDO_INSERT, &aRewriter);

    if ( HasSelection() )
        DelRight();
        // Inserted graphics in its own paragraph,
        // if at the end of a non-empty paragraph.
    //For i120928,avoid to split node

    EnterSelFrmMode();

    bool bSetGrfSize = true;
    bool bOwnMgr     = false;

    if ( !pFrmMgr )
    {
        bOwnMgr = true;
        pFrmMgr = new SwFlyFrmAttrMgr( sal_True, this, FRMMGR_TYPE_GRF );

        // CAUTION
        // GetAttrSet makes an adjustment
        // While pasting is a SwFrmSize present
        // because of the DEF-Framesize
        // These must be removed explicitly for the optimal size.
        pFrmMgr->DelAttr(RES_FRM_SIZE);
    }
    else
    {
        Size aSz( pFrmMgr->GetSize() );
        if ( !aSz.Width() || !aSz.Height() )
        {
            aSz.Width() = aSz.Height() = 567;
            pFrmMgr->SetSize( aSz );
        }
        else if ( aSz.Width() != DFLT_WIDTH && aSz.Height() != DFLT_HEIGHT )
            bSetGrfSize = false;

        pFrmMgr->SetHeightSizeType(ATT_FIX_SIZE);

    }

    // Insert the graphic
    SwFEShell::Insert(rPath, rFilter, &rGrf, &pFrmMgr->GetAttrSet());
    if ( bOwnMgr )
        pFrmMgr->UpdateAttrMgr();

    if( bSetGrfSize && !bRule )
    {
        Size aGrfSize, aBound = GetGraphicDefaultSize();
        GetGrfSize( aGrfSize );

        // Add the margin attributes to GrfSize,
        // because these counts at the margin additionaly
        aGrfSize.Width() += pFrmMgr->CalcWidthBorder();
        aGrfSize.Height()+= pFrmMgr->CalcHeightBorder();

        const BigInt aTempWidth( aGrfSize.Width() );
        const BigInt aTempHeight( aGrfSize.Height());

        // Fit width if necessary, scale down the height proportional thereafter.
        if( aGrfSize.Width() > aBound.Width() )
        {
            aGrfSize.Width()  = aBound.Width();
            aGrfSize.Height() = ((BigInt)aBound.Width()) * aTempHeight / aTempWidth;
        }
        // Fit hight if necessary, scale down the width proportional thereafter.
        if( aGrfSize.Height() > aBound.Height() )
        {
            aGrfSize.Height() = aBound.Height();
            aGrfSize.Width() =  ((BigInt)aBound.Height()) * aTempWidth / aTempHeight;
        }
        pFrmMgr->SetSize( aGrfSize );
        pFrmMgr->UpdateFlyFrm();
    }
    if ( bOwnMgr )
        delete pFrmMgr;

    EndUndo();
    EndAllAction();
}

// Insert an OLE-Objekt into the CORE.
// if no object is transfered, then one will be created.

void SwWrtShell::InsertObject( const svt::EmbeddedObjectRef& xRef, SvGlobalName *pName,
                            sal_Bool bActivate, sal_uInt16 nSlotId )
{
    ResetCursorStack();
    if( !CanInsert() )
        return;

    if( !xRef.is() )
    {
        // temporary storage
        svt::EmbeddedObjectRef xObj;
        uno::Reference < embed::XStorage > xStor = comphelper::OStorageHelper::GetTemporaryStorage();
        sal_Bool bDoVerb = sal_True;
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
                    // Intentionally no break!
                }

                // TODO/LATER: recording! Convert properties to items
                case SID_INSERT_PLUGIN:
                case SID_INSERT_FLOATINGFRAME:
                {
                    SfxSlotPool* pSlotPool = SW_MOD()->GetSlotPool();
                    const SfxSlot* pSlot = pSlotPool->GetSlot(nSlotId);
                    OString aCmd(".uno:");
                    aCmd += pSlot->GetUnoName();
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    SfxAbstractInsertObjectDialog* pDlg =
                            pFact->CreateInsertObjectDialog( GetWin(), OStringToOUString( aCmd, RTL_TEXTENCODING_UTF8 ), xStor, &aServerList );
                    if ( pDlg )
                    {
                        pDlg->Execute();
                        bDoVerb = pDlg->IsCreateNew();
                        OUString aIconMediaType;
                        uno::Reference< io::XInputStream > xIconMetaFile = pDlg->GetIconIfIconified( &aIconMediaType );
                        xObj.Assign( pDlg->GetObject(),
                                     xIconMetaFile.is() ? embed::Aspects::MSOLE_ICON : embed::Aspects::MSOLE_CONTENT );
                        if ( xIconMetaFile.is() )
                            xObj.SetGraphicStream( xIconMetaFile, aIconMediaType );

                        DELETEZ( pDlg );
                    }

                    break;
                }

                default:
                    break;
            }
        }

        if ( xObj.is() )
        {
            if( InsertOleObject( xObj ) && bActivate && bDoVerb )
            {
                SfxInPlaceClient* pClient = GetView().FindIPClient( xObj.GetObject(), &GetView().GetEditWin() );
                if ( !pClient )
                {
                    pClient = new SwOleClient( &GetView(), &GetView().GetEditWin(), xObj );
                    SetCheckForOLEInCaption( sal_True );
                }

                if ( xObj.GetViewAspect() == embed::Aspects::MSOLE_ICON )
                {
                    SwRect aArea = GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, 0, xObj.GetObject() );
                    aArea.Pos() += GetAnyCurRect( RECT_FLY_EMBEDDED, 0, xObj.GetObject() ).Pos();
                    MapMode aMapMode( MAP_TWIP );
                    Size aSize = xObj.GetSize( &aMapMode );
                    aArea.Width( aSize.Width() );
                    aArea.Height( aSize.Height() );
                    RequestObjectResize( aArea, xObj.GetObject() );
                }
                else
                    CalcAndSetScale( xObj );

                //#50270# We don't need to handle error, this is handled by the
                //DoVerb in the SfxViewShell
                pClient->DoVerb( SVVERB_SHOW );

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

sal_Bool SwWrtShell::InsertOleObject( const svt::EmbeddedObjectRef& xRef, SwFlyFrmFmt **pFlyFrmFmt )
{
    ResetCursorStack();
    StartAllAction();

    StartUndo(UNDO_INSERT);

    //Some differences between StarMath and any other objects:
    //1. Selections should be deleted. For StarMath the Text should be
    //   passed to the Object
    //2. If the cursor is at the end of an non empty paragraph a paragraph
    //   break should be insertet. StarMath objects are character bound and
    //   no break should be inserted.
    //3. If an selektion is passed to a StarMath object, this object should
    //   not be activated. sal_False should be returned then.
    bool bStarMath = true;
    sal_Bool bActivate = sal_True;

    // set parent to get correct VisArea(in case of object needing parent printer)
    uno::Reference < container::XChild > xChild( xRef.GetObject(), uno::UNO_QUERY );
    if ( xChild.is() )
        xChild->setParent( mpDoc->GetDocShell()->GetModel() );

    SvGlobalName aCLSID( xRef->getClassID() );
    bStarMath = ( SotExchange::IsMath( aCLSID ) != 0 );
    if( IsSelection() )
    {
        if( bStarMath )
        {
            String aMathData;
            GetSelectedText( aMathData, GETSELTXT_PARABRK_TO_ONLYCR );

            if( aMathData.Len() && svt::EmbeddedObjectRef::TryRunningState( xRef.GetObject() ) )
            {
                uno::Reference < beans::XPropertySet > xSet( xRef->getComponent(), uno::UNO_QUERY );
                if ( xSet.is() )
                {
                    try
                    {
                        xSet->setPropertyValue("Formula", uno::makeAny( OUString( aMathData ) ) );
                        bActivate = sal_False;
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
        SwFEShell::SplitNode( sal_False, sal_False );

    EnterSelFrmMode();

    SwFlyFrmAttrMgr aFrmMgr( sal_True, this, FRMMGR_TYPE_OLE );
    aFrmMgr.SetHeightSizeType(ATT_FIX_SIZE);

    SwRect aBound;
    CalcBoundRect( aBound, aFrmMgr.GetAnchor() );

    //The Size should be suggested by the OLE server
    MapMode aMapMode( MAP_TWIP );
    Size aSz = xRef.GetSize( &aMapMode );

    //Object size can be limited
    if ( aSz.Width() > aBound.Width() )
    {
        //Always limit proportional.
        aSz.Height() = aSz.Height() * aBound.Width() / aSz.Width();
        aSz.Width() = aBound.Width();
    }
    aFrmMgr.SetSize( aSz );
    SwFlyFrmFmt *pFmt = SwFEShell::InsertObject( xRef, &aFrmMgr.GetAttrSet() );

    // --> #i972#
    if ( bStarMath && mpDoc->get( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT ) )
        AlignFormulaToBaseline( xRef.GetObject() );

    if (pFlyFrmFmt)
        *pFlyFrmFmt = pFmt;

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
                    uno::makeAny( sal_False ) );
                xProps->setPropertyValue("DisableComplexChartTypes",
                    uno::makeAny( sal_False ) );
                uno::Reference< util::XModifiable > xModifiable( xProps, uno::UNO_QUERY );
                if ( xModifiable.is() )
                {
                    xModifiable->setModified( sal_True );
                }
            }
        }
    }

    EndAllAction();
    GetView().AutoCaption(OLE_CAP, &aCLSID);

    SwRewriter aRewriter;

    if ( bStarMath )
        aRewriter.AddRule(UndoArg1, SW_RES(STR_MATH_FORMULA));
    else if ( SotExchange::IsChart( aCLSID ) )
        aRewriter.AddRule(UndoArg1, SW_RES(STR_CHART));
    else
        aRewriter.AddRule(UndoArg1, SW_RES(STR_OLE));

    EndUndo(UNDO_INSERT, &aRewriter);

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
        SfxInPlaceClient* pCli=0;

        pCli = GetView().FindIPClient( xRef.GetObject(), &GetView().GetEditWin() );
        if ( !pCli )
            pCli = new SwOleClient( &GetView(), &GetView().GetEditWin(), xRef );

        ((SwOleClient*)pCli)->SetInDoVerb( sal_True );

        CalcAndSetScale( xRef );
        pCli->DoVerb( nVerb );

        ((SwOleClient*)pCli)->SetInDoVerb( sal_False );
        CalcAndSetScale( xRef );
    }
}

void SwWrtShell::MoveObjectIfActive( svt::EmbeddedObjectRef& xObj, const Point& rOffset )
{
    try
    {
        sal_Int32 nState = xObj->getCurrentState();
        if ( nState == ::com::sun::star::embed::EmbedStates::INPLACE_ACTIVE
          || nState == ::com::sun::star::embed::EmbedStates::UI_ACTIVE )
        {
            SfxInPlaceClient* pCli =
                GetView().FindIPClient( xObj.GetObject(), &(GetView().GetEditWin()) );
            if ( pCli )
            {
                Rectangle aArea = pCli->GetObjArea();
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
                                  const SwRect *pFlyFrmRect,
                                  const bool bNoTxtFrmPrtAreaChanged )
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
                        : GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, 0, xObj.GetObject() ));
            if( !aRect.IsEmpty() )
            {
                // TODO/LEAN: getMapUnit can switch object to running state
                // xObj.TryRunningState();

                MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );

                // TODO/LATER: needs complete VisArea?!
                Size aSize( OutputDevice::LogicToLogic( aRect.SVRect(), MAP_TWIP, aUnit ).GetSize() );
                awt::Size aSz;
                aSz.Width = aSize.Width();
                aSz.Height = aSize.Height();
                xObj->setVisualAreaSize( nAspect, aSz );
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
                        GetDoc()->GetDocShell()->EnableSetModified( sal_False );
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
                        GetDoc()->GetDocShell()->EnableSetModified( sal_True );
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
            // TODO/LATER: ResizeOnPrinterChange
             //|| SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE & xObj->GetMiscStatus()
             // --> OD #i117189# - refine condition for non-resizable objects
             // non-resizable objects need to be set the size back by this method
             || ( bNoTxtFrmPrtAreaChanged && nMisc & embed::EmbedMisc::EMBED_NEVERRESIZE ) )
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
        OSL_FAIL("Can't get visual area size!\n" );
        // the scaling will not be done
    }
    catch (const uno::Exception&)
    {
        // TODO/LATER: handle the error
        OSL_FAIL("Can't get visual area size!\n" );
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
        const MapMode aTmp( MAP_TWIP );
        MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
        _aVisArea = OutputDevice::LogicToLogic( _aVisArea, aUnit, aTmp);
        Size aObjArea;
        if ( pFlyPrtRect )
            aObjArea = pFlyPrtRect->SSize();
        else
            aObjArea = GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, 0, xObj.GetObject() ).SSize();

        // differ the aObjArea and _aVisArea by 1 Pixel then set new VisArea
        long nX, nY;
        SwSelPaintRects::Get1PixelInLogic( *this, &nX, &nY );
        if( !( _aVisArea.Width() - nX <= aObjArea.Width() &&
               _aVisArea.Width() + nX >= aObjArea.Width() &&
               _aVisArea.Height()- nY <= aObjArea.Height()&&
               _aVisArea.Height()+ nY >= aObjArea.Height() ))
        {
            // TODO/LATER: MISCSTATUS_RESIZEONPRINTERCHANGE
            /*
            if( SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE & nMisc )
            {
                //This type of objects should never be resized.
                //If this request comes from the Writer core (inaktive Object
                //ist resized), the Object should be resized too.
                //If this request comes from the Object itself, the Frame
                //in the Writer core should be resized.
                if ( pFlyPrtRect )      //Request from core?
                {
                    xObj->SetVisArea( OutputDevice::LogicToLogic(
                        pFlyPrtRect->SVRect(), MAP_TWIP, xObj->GetMapUnit() ));
                }
                else
                {
                    SwRect aTmp( Point( LONG_MIN, LONG_MIN ), _aVisArea );
                    RequestObjectResize( aTmp, xObj );
                }
                // The rest will be done, because we need to come back anyway,
                // possibly even recursively.
                return;
            }
            else*/

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
        aArea += pFlyFrmRect->Pos();
    }
    else
    {
        aArea = GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, 0, xObj.GetObject() );
        aArea.Pos() += GetAnyCurRect( RECT_FLY_EMBEDDED, 0, xObj.GetObject() ).Pos();
    }

    if ( bUseObjectSize )
    {
        // --> this moves non-resizable object so that when adding borders the baseline remains the same
        const SwFlyFrmFmt *pFlyFrmFmt = dynamic_cast< const SwFlyFrmFmt * >( GetFlyFrmFmt() );
        OSL_ENSURE( pFlyFrmFmt, "Could not find fly frame." );
        if ( pFlyFrmFmt )
        {
            const Point &rPoint = pFlyFrmFmt->GetLastFlyFrmPrtRectPos();
            SwRect aRect( pFlyPrtRect ? *pFlyPrtRect
                        : GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, 0, xObj.GetObject() ));
            aArea += rPoint - aRect.Pos(); // adjust area by diff of printing area position in order to keep baseline alignment correct.
        }
        aArea.Width ( _aVisArea.Width() );
        aArea.Height( _aVisArea.Height() );
        RequestObjectResize( aArea, xObj.GetObject() );
    }
    else
    {
        aArea.Width ( Fraction( aArea.Width()  ) / pCli->GetScaleWidth() );
        aArea.Height( Fraction( aArea.Height() ) / pCli->GetScaleHeight());
    }

    pCli->SetObjAreaAndScale( aArea.SVRect(), aScaleWidth, aScaleHeight );
}



void SwWrtShell::ConnectObj( svt::EmbeddedObjectRef& xObj, const SwRect &rPrt,
                            const SwRect &rFrm )
{
    SfxInPlaceClient* pCli = GetView().FindIPClient( xObj.GetObject(), &GetView().GetEditWin());
    if ( !pCli )
        pCli = new SwOleClient( &GetView(), &GetView().GetEditWin(), xObj );
    CalcAndSetScale( xObj, &rPrt, &rFrm );
}

// Insert hard page break;
// Selections will be overwritten

void SwWrtShell::InsertPageBreak(const String *pPageDesc, sal_uInt16 nPgNum )
{
    ResetCursorStack();
    if( CanInsert() )
    {
        SwActContext aActContext(this);
        StartUndo(UNDO_UI_INSERT_PAGE_BREAK);

        if ( !IsCrsrInTbl() )
        {
            if(HasSelection())
                DelRight();
            SwFEShell::SplitNode();
            // delete the numbered attribute of the last line if the last line is empty
            GetDoc()->ClearLineNumAttrs( *GetCrsr()->GetPoint() );
        }

        const SwPageDesc *pDesc = pPageDesc
                                ? FindPageDescByName( *pPageDesc, sal_True ) : 0;
        if( pDesc )
        {
            SwFmtPageDesc aDesc( pDesc );
            aDesc.SetNumOffset( nPgNum );
            SetAttr( aDesc );
        }
        else
            SetAttr( SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE, RES_BREAK) );
        EndUndo(UNDO_UI_INSERT_PAGE_BREAK);
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
        StartUndo(UNDO_UI_INSERT_COLUMN_BREAK);

        if ( !IsCrsrInTbl() )
        {
            if(HasSelection())
                DelRight();
            SwFEShell::SplitNode( sal_False, sal_False );
        }
        SetAttr(SvxFmtBreakItem(SVX_BREAK_COLUMN_BEFORE, RES_BREAK));

        EndUndo(UNDO_UI_INSERT_COLUMN_BREAK);
    }
}

// Insert footnote
// rStr - optional footnote mark

void SwWrtShell::InsertFootnote(const String &rStr, sal_Bool bEndNote, sal_Bool bEdit )
{
    ResetCursorStack();
    if( CanInsert() )
    {
        if(HasSelection())
        {
            //collapse cursor to the end
            if(!IsCrsrPtAtEnd())
                SwapPam();
            ClearMark();
        }
        SwPosition aPos = *GetCrsr()->GetPoint();
        SwFmtFtn aFootNote( bEndNote );
        if(rStr.Len())
            aFootNote.SetNumStr( rStr );

        SetAttr(aFootNote);

        if( bEdit )
        {
            // For editing the footnote text.
            Left(CRSR_SKIP_CHARS, sal_False, 1, sal_False );
            GotoFtnTxt();
        }
        aNavigationMgr.addEntry(aPos);
    }
}

// SplitNode; also, because
//                  - of deleting selected content;
//                  - of reset of the Cursorstack if necessary.

void SwWrtShell::SplitNode( sal_Bool bAutoFmt, sal_Bool bCheckTableStart )
{
    ResetCursorStack();
    if( CanInsert() )
    {
        SwActContext aActContext(this);

        rView.GetEditWin().FlushInBuffer();
        sal_Bool bHasSel = HasSelection();
        if( bHasSel )
        {
            StartUndo( UNDO_INSERT );
            DelRight();
        }

        SwFEShell::SplitNode( bAutoFmt, bCheckTableStart );
        if( bHasSel )
            EndUndo( UNDO_INSERT );
    }
}

// Turn on numbering
// Parameter:   Optional specification of a name for the named list;
//              this indicates a position if it is possible to convert them
//              into a number and less than nMaxRules.

// To test the CharFormats at the numbering
// external void SetNumChrFmt( SwWrtShell*, SwNumRules& );

// -> #i40041#
// Preconditions (as far as OD has figured out):
// - <SwEditShell::HasNumber()> is sal_False, if <bNum> is sal_True
// - <SwEditShell::HasBullet()> is sal_False, if <bNum> is sal_False
// Behavior of method is determined by the current situation at the current
// cursor position in the document.
void SwWrtShell::NumOrBulletOn(sal_Bool bNum)
{
    // determine numbering rule found at current cursor position in the document.
    const SwNumRule* pCurRule = GetCurNumRule();

    StartUndo(UNDO_NUMORNONUM);

    const SwNumRule * pNumRule = pCurRule;

    // - activate outline rule respectively turning on outline rule for
    //   current text node. But, only for turning on a numbering (<bNum> == sal_True).
    // - overwrite found numbering rule at current cursor position, if
    //   no numbering rule can be retrieved from the paragraph style.
    bool bContinueFoundNumRule( false );
    bool bActivateOutlineRule( false );
    int nActivateOutlineLvl( MAXLEVEL );    // only relevant, if <bActivateOutlineRule> == sal_True
    SwTxtFmtColl * pColl = GetCurTxtFmtColl();
    if ( pColl )
    {
        // retrieve numbering rule at paragraph
        // style, which is found at current cursor position in the document.
        SwNumRule* pCollRule = mpDoc->FindNumRulePtr(pColl->GetNumRule().GetValue());
        // #125993# - The outline numbering rule isn't allowed
        // to be derived from a parent paragraph style to a derived one.
        // Thus check, if the found outline numbering rule is directly
        // set at the paragraph style <pColl>. If not, set <pCollRule> to NULL
        if ( pCollRule && pCollRule == GetDoc()->GetOutlineNumRule() )
        {
            const SwNumRule* pDirectCollRule =
                    mpDoc->FindNumRulePtr(pColl->GetNumRule( sal_False ).GetValue());
            if ( !pDirectCollRule )
            {
                pCollRule = 0;
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
                SwTxtNode* pTxtNode =
                            GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();
                if ( pTxtNode && !pTxtNode->IsCountedInList() )
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
                    // it's known, that <SwEdit::HasNumber()> == sal_False
                    bActivateOutlineRule = true;
                    nActivateOutlineLvl = pColl->GetAssignedOutlineStyleLevel();//<-end,zhaojianwei
                }
            }
            else if ( !pNumRule )
            {
                // #i101234#
                // Check, if corresponding list level of the outline numbering
                // has already a numbering format set.
                nActivateOutlineLvl = pColl->GetAssignedOutlineStyleLevel();//<-end,zhaojianwei,need further consideration
                if ( pCollRule->Get( static_cast<sal_uInt16>(nActivateOutlineLvl) ).GetNumberingType()
                                == SVX_NUM_NUMBER_NONE )
                {
                    // activate outline numbering, because from the precondition
                    // it's known, that <SwEdit::HasNumber()> == sal_False
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
            pNumRule = 0;
        }
        else if ( pNumRule == GetDoc()->GetOutlineNumRule() &&
                  !bActivateOutlineRule && !bContinueFoundNumRule )
        {
            pNumRule = 0;
        }
    }

    // Search for a previous numbering/bullet rule to continue it.
    String sContinuedListId;
    if ( !pNumRule )
    {
        pNumRule = GetDoc()->SearchNumRule( *GetCrsr()->GetPoint(),
                                            false, bNum, false, 0,
                                            sContinuedListId );
        bContinueFoundNumRule = pNumRule != 0;
    }

    if (pNumRule)
    {
        SwNumRule aNumRule(*pNumRule);

        // do not change found numbering/bullet rule, if it should only be continued.
        if ( !bContinueFoundNumRule )
        {
            SwTxtNode * pTxtNode = GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();

            if (pTxtNode)
            {
                // use above retrieve outline level, if outline numbering has to be activated.
                int nLevel = bActivateOutlineRule
                              ? nActivateOutlineLvl
                              : pTxtNode->GetActualListLevel();

                if (nLevel < 0)
                    nLevel = 0;

                if (nLevel >= MAXLEVEL)
                    nLevel = MAXLEVEL - 1;

                SwNumFmt aFmt(aNumRule.Get(static_cast<sal_uInt16>(nLevel)));

                if (bNum)
                    aFmt.SetNumberingType(SVX_NUM_ARABIC);
                else
                {
                    // #i63395# Only apply user defined default bullet font
                    if ( numfunc::IsDefBulletFontUserDefined() )
                    {
                        const Font* pFnt = &numfunc::GetDefBulletFont();
                        aFmt.SetBulletFont( pFnt );
                    }
                    aFmt.SetBulletChar( numfunc::GetBulletChar(static_cast<sal_uInt8>(nLevel)));
                    aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                    // #i93908# clear suffix for bullet lists
                    aFmt.SetPrefix(OUString());
                    aFmt.SetSuffix(OUString());
                }
                aNumRule.Set(static_cast<sal_uInt16>(nLevel), aFmt);
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
        SwCharFmt* pChrFmt;
        SwDocShell* pDocSh = GetView().GetDocShell();
        // #i63395#
        // Only apply user defined default bullet font
        const Font* pFnt = numfunc::IsDefBulletFontUserDefined()
                           ? &numfunc::GetDefBulletFont()
                           : 0;

        if (bNum)
        {
            pChrFmt = GetCharFmtFromPool( RES_POOLCHR_NUM_LEVEL );
        }
        else
        {
            pChrFmt = GetCharFmtFromPool( RES_POOLCHR_BUL_LEVEL );
        }

        const SwTxtNode* pTxtNode = GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();
        const SwTwips nWidthOfTabs = pTxtNode
                                     ? pTxtNode->GetWidthOfLeadingTabs()
                                     : 0;
        GetDoc()->RemoveLeadingWhiteSpace( *GetCrsr()->GetPoint() );

        const bool bHtml = 0 != PTR_CAST(SwWebDocShell, pDocSh);
        const bool bRightToLeft = IsInRightToLeftText();
        for( sal_uInt8 nLvl = 0; nLvl < MAXLEVEL; ++nLvl )
        {
            SwNumFmt aFmt( aNumRule.Get( nLvl ) );
            aFmt.SetCharFmt( pChrFmt );

            if (! bNum)
            {
                // #i63395#
                // Only apply user defined default bullet font
                if ( pFnt )
                {
                    aFmt.SetBulletFont( pFnt );
                }
                aFmt.SetBulletChar( numfunc::GetBulletChar(nLvl) );
                aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                // #i93908# clear suffix for bullet lists
                aFmt.SetPrefix(OUString());
                aFmt.SetSuffix(OUString());
            }

            // #i95907#
            if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                if(bHtml && nLvl)
                {
                    // 1/2" for HTML
                    aFmt.SetLSpace(720);
                    aFmt.SetAbsLSpace(nLvl * 720);
                }
                else if ( nWidthOfTabs > 0 )
                {
                    aFmt.SetAbsLSpace(nWidthOfTabs + nLvl * 720);
                }
            }

            // #i38904#  Default alignment for
            // numbering/bullet should be rtl in rtl paragraph:
            if ( bRightToLeft )
            {
                aFmt.SetNumAdjust( SVX_ADJUST_RIGHT );
            }

            aNumRule.Set( nLvl, aFmt );
        }

        // #i95907#
        if ( pTxtNode &&
             ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
        {

            const SwTwips nTxtNodeIndent = pTxtNode->GetAdditionalIndentForStartingNewList();
            if ( ( nTxtNodeIndent + nWidthOfTabs ) != 0 )
            {
                // #i111172#
                // If text node is already inside a list, assure that the indents
                // are the same. Thus, adjust the indent change value by subtracting
                // indents of to be applied list style.
                SwTwips nIndentChange = nTxtNodeIndent + nWidthOfTabs;
                if ( pTxtNode->GetNumRule() )
                {
                    const SwNumFmt aFmt( aNumRule.Get( 0 ) );
                    if ( aFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
                    {
                        nIndentChange -= aFmt.GetIndentAt() + aFmt.GetFirstLineIndent();
                    }
                }
                aNumRule.ChangeIndent( nIndentChange );
            }
        }
        // reset indent attribute on applying list style
        // start new list
        SetCurNumRule( aNumRule, true, String(), true );
    }

    EndUndo(UNDO_NUMORNONUM);
}
// <- #i40041#

void SwWrtShell::NumOn()
{
    NumOrBulletOn(sal_True);
}

void SwWrtShell::NumOrBulletOff()
{
    const SwNumRule * pCurNumRule = GetCurNumRule();

    if (pCurNumRule)
    {
        if (pCurNumRule->IsOutlineRule())
        {
            SwNumRule aNumRule(*pCurNumRule);

            SwTxtNode * pTxtNode =
                GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();

            if (pTxtNode)
            {
                sal_uInt16 nLevel = sal::static_int_cast<sal_uInt16, sal_Int32>(pTxtNode->GetActualListLevel());
                SwNumFmt aFmt(aNumRule.Get(nLevel));

                aFmt.SetNumberingType(SVX_NUM_NUMBER_NONE);
                aNumRule.Set(nLevel, aFmt);

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
    NumOrBulletOn(sal_False);
}

SelectionType SwWrtShell::GetSelectionType() const
{
    // ContentType cannot be determined within a
    // Start-/Endaction parentheses.
    // Because there is no invalid value TEXT will be returned.
    // The value does not matter, it may be updated in endaction anyway.

    if ( BasicActionPend() )
        return IsSelFrmMode() ? nsSelectionType::SEL_FRM : nsSelectionType::SEL_TXT;

    SwView &_rView = ((SwView&)GetView());
    if (_rView.GetPostItMgr() && _rView.GetPostItMgr()->HasActiveSidebarWin() )
        return nsSelectionType::SEL_POSTIT;
     int nCnt;

    // Inserting a frame is not a DrawMode
    if ( !_rView.GetEditWin().IsFrmAction() &&
            (IsObjSelected() || (_rView.IsDrawMode() && !IsFrmSelected()) ))
    {
        if (GetDrawView()->IsTextEdit())
            nCnt = nsSelectionType::SEL_DRW_TXT;
        else
        {
            if (GetView().IsFormMode()) // Only Form selected
                nCnt = nsSelectionType::SEL_DRW_FORM;
            else
                nCnt = nsSelectionType::SEL_DRW;            // Any draw object

            if (_rView.IsBezierEditMode())
                nCnt |= nsSelectionType::SEL_BEZ;
            else if( GetDrawView()->GetContext() == SDRCONTEXT_MEDIA )
                nCnt |= nsSelectionType::SEL_MEDIA;

            if (svx::checkForSelectedCustomShapes(
                    const_cast<SdrView *>(GetDrawView()),
                    true /* bOnlyExtruded */ ))
            {
                nCnt |= nsSelectionType::SEL_EXTRUDED_CUSTOMSHAPE;
            }
            sal_uInt32 nCheckStatus = 0;
            if (svx::checkForSelectedFontWork(
                    const_cast<SdrView *>(GetDrawView()), nCheckStatus ))
            {
                nCnt |= nsSelectionType::SEL_FONTWORK;
            }
        }

        return nCnt;
    }

    nCnt = GetCntType();

    if ( IsFrmSelected() )
    {
        if (_rView.IsDrawMode())
            _rView.LeaveDrawCreate();   // clean up (Bug #45639)
        if ( !(nCnt & (CNT_GRF | CNT_OLE)) )
            return nsSelectionType::SEL_FRM;
    }

    if ( IsCrsrInTbl() )
        nCnt |= nsSelectionType::SEL_TBL;

    if ( IsTableMode() )
        nCnt |= (nsSelectionType::SEL_TBL | nsSelectionType::SEL_TBL_CELLS);

    // #i39855#
    // Do not pop up numbering toolbar, if the text node has a numbering
    // of type SVX_NUM_NUMBER_NONE.
    const SwNumRule* pNumRule = GetCurNumRule();
    if ( pNumRule )
    {
        const SwTxtNode* pTxtNd =
            GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();

        if ( pTxtNd && pTxtNd->IsInList() )
        {
            const SwNumFmt& rFmt = pNumRule->Get(sal::static_int_cast< sal_uInt8, sal_Int32>(pTxtNd->GetActualListLevel()));
            if ( SVX_NUM_NUMBER_NONE != rFmt.GetNumberingType() )
                nCnt |= nsSelectionType::SEL_NUM;
        }
    }

    return nCnt;
}

// Find the text collection with the name rCollname
// Returns:   Pointer at the collection or 0, if no
//            text collection with this name exists, or
//            this is a default template.

SwTxtFmtColl *SwWrtShell::GetParaStyle(const String &rCollName, GetStyle eCreate )
{
    SwTxtFmtColl* pColl = FindTxtFmtCollByName( rCollName );
    if( !pColl && GETSTYLE_NOCREATE != eCreate )
    {
        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( rCollName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
        if( USHRT_MAX != nId || GETSTYLE_CREATEANY == eCreate )
            pColl = GetTxtCollFromPool( nId );
    }
    return pColl;
}

// Find the text collection with the name rCollname
// Returns:   Pointer at the collection or 0, if no
//            character template with this name exists, or
//            this is a default template or template is automatic.

SwCharFmt *SwWrtShell::GetCharStyle(const String &rFmtName, GetStyle eCreate )
{
    SwCharFmt* pFmt = FindCharFmtByName( rFmtName );
    if( !pFmt && GETSTYLE_NOCREATE != eCreate )
    {
        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( rFmtName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
        if( USHRT_MAX != nId || GETSTYLE_CREATEANY == eCreate )
            pFmt = (SwCharFmt*)GetFmtFromPool( nId );
    }
    return pFmt;
}

// Find the table format with the name rFmtname
// Returns:   Pointer at the collection or 0, if no
//            frame format with this name exists or
//            this is a default format or the format is automatic.

SwFrmFmt *SwWrtShell::GetTblStyle(const String &rFmtName)
{
    SwFrmFmt *pFmt = 0;
    for( sal_uInt16 i = GetTblFrmFmtCount(); i; )
        if( !( pFmt = &GetTblFrmFmt( --i ) )->IsDefault() &&
            pFmt->GetName() == rFmtName && IsUsed( *pFmt ) )
            return pFmt;
    return 0;
}

SwNavigationMgr& SwWrtShell::GetNavigationMgr() {
    return aNavigationMgr;
}

void SwWrtShell::addCurrentPosition() {
    SwPaM* pPaM = GetCrsr();
    aNavigationMgr.addEntry(*pPaM->GetPoint());
}

// Applying templates

void SwWrtShell::SetPageStyle(const String &rCollName)
{
    if( !SwCrsrShell::HasSelection() && !IsSelFrmMode() && !IsObjSelected() )
    {
        SwPageDesc* pDesc = FindPageDescByName( rCollName, sal_True );
        if( pDesc )
            ChgCurPageDesc( *pDesc );
    }
}

// Access templates

String SwWrtShell::GetCurPageStyle( const sal_Bool bCalcFrm ) const
{
    return GetPageDesc(GetCurPageDesc( bCalcFrm )).GetName();
}

// Change the current template referring to the existing change.

void SwWrtShell::QuickUpdateStyle()
{
    SwTxtFmtColl *pColl = GetCurTxtFmtColl();

    // Default cannot be changed
    if(pColl && !pColl->IsDefault())
    {
        FillByEx(pColl);
            // Also apply the template to remove hard attribute assignment.
        SetTxtFmtColl(pColl);
    }
}

void SwWrtShell::AutoUpdatePara(SwTxtFmtColl* pColl, const SfxItemSet& rStyleSet, SwPaM* pPaM )
{
    SwPaM* pCrsr = pPaM ? pPaM : GetCrsr( );
    SfxItemSet aCoreSet( GetAttrPool(),
            RES_CHRATR_BEGIN,           RES_CHRATR_END - 1,
            RES_PARATR_BEGIN,           RES_PARATR_END - 1,
            RES_FRMATR_BEGIN,           RES_FRMATR_END - 1,
            SID_ATTR_TABSTOP_POS,       SID_ATTR_TABSTOP_POS,
            SID_ATTR_TABSTOP_DEFAULTS,  SID_ATTR_TABSTOP_DEFAULTS,
            SID_ATTR_TABSTOP_OFFSET,    SID_ATTR_TABSTOP_OFFSET,
            SID_ATTR_BORDER_INNER,      SID_ATTR_BORDER_INNER,
            SID_ATTR_PARA_MODEL,        SID_ATTR_PARA_KEEP,
            SID_ATTR_PARA_PAGENUM,      SID_ATTR_PARA_PAGENUM,
            0   );
    GetPaMAttr( pCrsr, aCoreSet );
    bool bReset = false;
    SfxItemIter aParaIter( aCoreSet );
    const SfxPoolItem* pParaItem = aParaIter.FirstItem();
    while( pParaItem )
    {
        if(!IsInvalidItem(pParaItem))
        {
            sal_uInt16 nWhich = pParaItem->Which();
            if(SFX_ITEM_SET == aCoreSet.GetItemState(nWhich) &&
               SFX_ITEM_SET == rStyleSet.GetItemState(nWhich))
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
        ResetAttr( std::set<sal_uInt16>(), pCrsr );
        SetAttr(aCoreSet, 0, pCrsr);
    }
    mpDoc->ChgFmt(*pColl, rStyleSet );
    EndAction();
}

void SwWrtShell::AutoUpdateFrame( SwFrmFmt* pFmt, const SfxItemSet& rStyleSet )
{
    StartAction();

    ResetFlyFrmAttr( 0, &rStyleSet );
    pFmt->SetFmtAttr( rStyleSet );

    EndAction();
}


void SwWrtShell::AutoCorrect( SvxAutoCorrect& rACorr, sal_Unicode cChar )
{
    ResetCursorStack();
    if(CanInsert())
    {
        bool bStarted = false;
        if(HasSelection())
        {
                // Only parenthese here, because the regular insert
                // is already clipped to the editshell
            StartAllAction();
            StartUndo(UNDO_INSERT);
            bStarted = true;
            DelRight();
        }
        SwEditShell::AutoCorrect( rACorr, IsInsMode(), cChar );

        if(bStarted)
        {
            EndAllAction();
            EndUndo(UNDO_INSERT);
        }
    }
}

// Some kind of controlled copy ctor

SwWrtShell::SwWrtShell( SwWrtShell& rSh, Window *_pWin, SwView &rShell )
    : SwFEShell( rSh, _pWin ),
     COMMON_INI_LIST
{
    BITFLD_INI_LIST
    SET_CURR_SHELL( this );

    SetSfxViewShell( (SfxViewShell *)&rShell );
    SetFlyMacroLnk( LINK(this, SwWrtShell, ExecFlyMac) );

    // place the cursor on the first field...
    IFieldmark *pBM = NULL;
    if ( IsFormProtected() && ( pBM = GetFieldmarkAfter( ) ) !=NULL ) {
        GotoFieldmark(pBM);
    }
}


SwWrtShell::SwWrtShell( SwDoc& rDoc, Window *_pWin, SwView &rShell,
                        const SwViewOption *pViewOpt )
    : SwFEShell( rDoc, _pWin, pViewOpt),
      COMMON_INI_LIST
{
    BITFLD_INI_LIST
    SET_CURR_SHELL( this );
    SetSfxViewShell( (SfxViewShell *)&rShell );
    SetFlyMacroLnk( LINK(this, SwWrtShell, ExecFlyMac) );

    // place the cursor on the first field...
    IFieldmark *pBM = NULL;
    if ( IsFormProtected() && ( pBM = GetFieldmarkAfter( ) ) !=NULL ) {
        GotoFieldmark(pBM);
    }
}

SwWrtShell::~SwWrtShell()
{
    SET_CURR_SHELL( this );
    while(IsModePushed())
        PopMode();
    while(PopCrsr(sal_False))
        ;
    SwTransferable::ClearSelection( *this );
}

sal_Bool SwWrtShell::Pop( sal_Bool bOldCrsr )
{
    sal_Bool bRet = SwCrsrShell::Pop( bOldCrsr );
    if( bRet && IsSelection() )
    {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
    }
    return bRet;
}

bool SwWrtShell::CanInsert()
{
    return (!(IsSelFrmMode() | IsObjSelected() | (GetView().GetDrawFuncPtr() != NULL) | (GetView().GetPostItMgr()->GetActiveSidebarWin()!= NULL)));
}

void SwWrtShell::ChgDBData(const SwDBData& aDBData)
{
    SwEditShell::ChgDBData(aDBData);
    //notify the db-beamer if available
    GetView().NotifyDBChanged();
}

String SwWrtShell::GetSelDescr() const
{
    String aResult;

    int nSelType = GetSelectionType();
    switch (nSelType)
    {
    case nsSelectionType::SEL_GRF:
        aResult = SW_RESSTR(STR_GRAPHIC);

        break;
    case nsSelectionType::SEL_FRM:
        {
            const SwFrmFmt * pFrmFmt = GetCurFrmFmt();

            if (pFrmFmt)
                aResult = pFrmFmt->GetDescription();
        }
        break;
    case nsSelectionType::SEL_DRW:
        {
            aResult = SW_RESSTR(STR_DRAWING_OBJECTS);
        }
        break;
    default:
        if (0 != mpDoc)
            aResult = GetCrsrDescr();
    }

    return aResult;
}

void SwWrtShell::ApplyViewOptions( const SwViewOption &rOpt )
{
    SwFEShell::ApplyViewOptions( rOpt );
    //#i115062# invalidate meta character slot
    GetView().GetViewFrame()->GetBindings().Invalidate( FN_VIEW_META_CHARS );
}


void SwWrtShell::SetReadonlyOption(sal_Bool bSet)
{
    GetView().GetEditWin().GetFrameControlsManager().SetReadonlyControls( bSet );
    ViewShell::SetReadonlyOption( bSet );
}

// Switch on/off header or footer of a page style - if an empty name is
// given all styles are changed

void SwWrtShell::ChangeHeaderOrFooter(
    const String& rStyleName, sal_Bool bHeader, sal_Bool bOn, sal_Bool bShowWarning)
{
    addCurrentPosition();
    StartAllAction();
    StartUndo( UNDO_HEADER_FOOTER ); // #i7983#
    bool bExecute = true;
    sal_Bool bCrsrSet = sal_False;
    for( sal_uInt16 nFrom = 0, nTo = GetPageDescCnt();
            nFrom < nTo; ++nFrom )
    {
        int bChgd = sal_False;
        SwPageDesc aDesc( GetPageDesc( nFrom ));
        String sTmp(aDesc.GetName());
        if( !rStyleName.Len() || rStyleName == sTmp )
        {
            if( bShowWarning && !bOn && GetActiveView() && GetActiveView() == &GetView() &&
                ( (bHeader && aDesc.GetMaster().GetHeader().IsActive()) ||
                  (!bHeader && aDesc.GetMaster().GetFooter().IsActive()) ) )
            {
                bShowWarning = sal_False;
                //Actions have to be closed while the dialog is showing
                EndAllAction();

                Window* pParent = &GetView().GetViewFrame()->GetWindow();
                short nResult;
                if (bHeader)
                    nResult = DeleteHeaderDialog(pParent).Execute();
                else
                    nResult = DeleteFooterDialog(pParent).Execute();
                bExecute = nResult == RET_YES;

                StartAllAction();
            }
            if( bExecute )
            {
                bChgd = sal_True;
                SwFrmFmt &rMaster = aDesc.GetMaster();
                if(bHeader)
                    rMaster.SetFmtAttr( SwFmtHeader( bOn ));
                else
                    rMaster.SetFmtAttr( SwFmtFooter( bOn ));
                if( bOn )
                {
                    SvxULSpaceItem aUL(bHeader ? 0 : MM50, bHeader ? MM50 : 0, RES_UL_SPACE );
                    SwFrmFmt* pFmt = bHeader ?
                        (SwFrmFmt*)rMaster.GetHeader().GetHeaderFmt() :
                        (SwFrmFmt*)rMaster.GetFooter().GetFooterFmt();
                    pFmt->SetFmtAttr( aUL );
                }
            }
            if( bChgd )
            {
                ChgPageDesc( nFrom, aDesc );

                if( !bCrsrSet && bOn )
                {
                    if ( !IsHeaderFooterEdit() )
                        ToggleHeaderFooterEdit();
                    bCrsrSet = SetCrsrInHdFt(
                            !rStyleName.Len() ? (sal_uInt16)0xFFFF : nFrom,
                            bHeader );
                }
            }
        }
    }
    EndUndo( UNDO_HEADER_FOOTER ); // #i7983#
    EndAllAction();
}

void SwWrtShell::SetShowHeaderFooterSeparator( FrameControlType eControl, bool bShow )
{
    ViewShell::SetShowHeaderFooterSeparator( eControl, bShow );
    if ( !bShow )
        GetView().GetEditWin().GetFrameControlsManager().HideControls( eControl );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
