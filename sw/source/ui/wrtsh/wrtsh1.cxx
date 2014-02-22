/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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


#include <SwRewriter.hxx>
#include <comcore.hrc>


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


void SwWrtShell::InsertByWord( const OUString & rStr)
{
    if( !rStr.isEmpty() )
    {
        sal_Bool bDelim = GetAppCharClass().isLetterNumeric( rStr, 0 );
        sal_Int32 nPos = 0, nStt = 0;
        for( ; nPos < rStr.getLength(); nPos++ )
           {
            sal_Bool bTmpDelim = GetAppCharClass().isLetterNumeric( rStr, nPos );
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
    sal_Bool bHasSel = HasSelection(),
        bCallIns = bIns /*|| bHasSel*/;
    bool bDeleted = false;

    if( bHasSel || ( !bIns && SelectHiddenRange() ) )
    {
            
            
        StartAllAction();

        
        SwRewriter aRewriter;

        aRewriter.AddRule(UndoArg1, GetCrsrDescr());
        aRewriter.AddRule(UndoArg2, OUString(SW_RES(STR_YIELDS)));
        {
            OUString aTmpStr;
            aTmpStr += SW_RES(STR_START_QUOTE);
            aTmpStr += rStr;
            aTmpStr += SW_RES(STR_END_QUOTE);

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




void SwWrtShell::Insert( const OUString &rPath, const OUString &rFilter,
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
        
        
    

    EnterSelFrmMode();

    bool bSetGrfSize = true;
    bool bOwnMgr     = false;

    if ( !pFrmMgr )
    {
        bOwnMgr = true;
        pFrmMgr = new SwFlyFrmAttrMgr( sal_True, this, FRMMGR_TYPE_GRF );

        
        
        
        
        
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

    
    SwFEShell::Insert(rPath, rFilter, &rGrf, &pFrmMgr->GetAttrSet());
    if ( bOwnMgr )
        pFrmMgr->UpdateAttrMgr();

    if( bSetGrfSize && !bRule )
    {
        Size aGrfSize, aBound = GetGraphicDefaultSize();
        GetGrfSize( aGrfSize );

        
        
        aGrfSize.Width() += pFrmMgr->CalcWidthBorder();
        aGrfSize.Height()+= pFrmMgr->CalcHeightBorder();

        const BigInt aTempWidth( aGrfSize.Width() );
        const BigInt aTempHeight( aGrfSize.Height());

        
        if( aGrfSize.Width() > aBound.Width() )
        {
            aGrfSize.Width()  = aBound.Width();
            aGrfSize.Height() = ((BigInt)aBound.Width()) * aTempHeight / aTempWidth;
        }
        
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




void SwWrtShell::InsertObject( const svt::EmbeddedObjectRef& xRef, SvGlobalName *pName,
                            sal_Bool bActivate, sal_uInt16 nSlotId )
{
    ResetCursorStack();
    if( !CanInsert() )
        return;

    if( !xRef.is() )
    {
        
        svt::EmbeddedObjectRef xObj;
        uno::Reference < embed::XStorage > xStor = comphelper::OStorageHelper::GetTemporaryStorage();
        sal_Bool bDoVerb = sal_True;
        if ( pName )
        {
            comphelper::EmbeddedObjectContainer aCnt( xStor );
            OUString aName;
            
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
                    
                }

                
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

                
                
                pClient->DoVerb( SVVERB_SHOW );

                
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




sal_Bool SwWrtShell::InsertOleObject( const svt::EmbeddedObjectRef& xRef, SwFlyFrmFmt **pFlyFrmFmt )
{
    ResetCursorStack();
    StartAllAction();

    StartUndo(UNDO_INSERT);

    
    
    
    
    
    
    
    
    bool bStarMath = true;
    sal_Bool bActivate = sal_True;

    
    uno::Reference < container::XChild > xChild( xRef.GetObject(), uno::UNO_QUERY );
    if ( xChild.is() )
        xChild->setParent( mpDoc->GetDocShell()->GetModel() );

    SvGlobalName aCLSID( xRef->getClassID() );
    bStarMath = ( SotExchange::IsMath( aCLSID ) != 0 );
    if( IsSelection() )
    {
        if( bStarMath )
        {
            OUString aMathData;
            GetSelectedText( aMathData, GETSELTXT_PARABRK_TO_ONLYCR );

            if( !aMathData.isEmpty() && svt::EmbeddedObjectRef::TryRunningState( xRef.GetObject() ) )
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

    
    MapMode aMapMode( MAP_TWIP );
    Size aSz = xRef.GetSize( &aMapMode );

    
    if ( aSz.Width() > aBound.Width() )
    {
        
        aSz.Height() = aSz.Height() * aBound.Width() / aSz.Width();
        aSz.Width() = aBound.Width();
    }
    aFrmMgr.SetSize( aSz );
    SwFlyFrmFmt *pFmt = SwFEShell::InsertObject( xRef, &aFrmMgr.GetAttrSet() );

    
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
    
    
    OSL_ENSURE( xObj.is(), "ObjectRef not  valid" );

    sal_Int64 nAspect = xObj.GetViewAspect();
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        return; 

    sal_Int64 nMisc = 0;
    bool bLinkingChart = false;

    try
    {
        nMisc = xObj->getStatus( nAspect );

        
        
        if( embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE & nMisc )
        {
            
            SwRect aRect( pFlyPrtRect ? *pFlyPrtRect
                        : GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, 0, xObj.GetObject() ));
            if( !aRect.IsEmpty() )
            {
                
                

                MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );

                
                Size aSize( OutputDevice::LogicToLogic( aRect.SVRect(), MAP_TWIP, aUnit ).GetSize() );
                awt::Size aSz;
                aSz.Width = aSize.Width();
                aSz.Height = aSize.Height();
                xObj->setVisualAreaSize( nAspect, aSz );
                
                
                
                
                
                
                
                
                {
                    bool bResetEnableSetModified(false);
                    if ( GetDoc()->GetDocShell()->IsEnableSetModified() )
                    {
                        GetDoc()->GetDocShell()->EnableSetModified( sal_False );
                        bResetEnableSetModified = true;
                    }

                    
                    
                    
                    
                    if ( !SotExchange::IsChart( xObj->getClassID() ) )
                        xObj.UpdateReplacement();

                    if ( bResetEnableSetModified )
                    {
                        GetDoc()->GetDocShell()->EnableSetModified( sal_True );
                    }
                }
            }

            
            uno::Reference< chart2::XChartDocument > xChartDocument( xObj->getComponent(), uno::UNO_QUERY );
            bLinkingChart = ( xChartDocument.is() && !xChartDocument->hasInternalDataProvider() );
        }
    }
    catch (const uno::Exception&)
    {
        
        return;
    }

    SfxInPlaceClient* pCli = GetView().FindIPClient( xObj.GetObject(), &GetView().GetEditWin() );
    if ( !pCli )
    {
        if ( (embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY & nMisc)
             || bLinkingChart
            
             
             
             
             || ( bNoTxtFrmPrtAreaChanged && nMisc & embed::EmbedMisc::EMBED_NEVERRESIZE ) )
        {
            pCli = new SwOleClient( &GetView(), &GetView().GetEditWin(), xObj );
        }
        else
            return;
    }

    
    

    awt::Size aSize;
    try
    {
        aSize = xObj->getVisualAreaSize( nAspect );
    }
    catch (const embed::NoVisualAreaSizeException&)
    {
        OSL_FAIL("Can't get visual area size!\n" );
        
    }
    catch (const uno::Exception&)
    {
        
        OSL_FAIL("Can't get visual area size!\n" );
        return;
    }

    Size _aVisArea( aSize.Width, aSize.Height );

    Fraction aScaleWidth( 1, 1 );
    Fraction aScaleHeight( 1, 1 );

    bool bUseObjectSize = false;

    
    
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

        
        long nX, nY;
        SwSelPaintRects::Get1PixelInLogic( *this, &nX, &nY );
        if( !( _aVisArea.Width() - nX <= aObjArea.Width() &&
               _aVisArea.Width() + nX >= aObjArea.Width() &&
               _aVisArea.Height()- nY <= aObjArea.Height()&&
               _aVisArea.Height()+ nY >= aObjArea.Height() ))
        {
            if ( nMisc & embed::EmbedMisc::EMBED_NEVERRESIZE )
            {
                
                
                bUseObjectSize = true;
            }
            else
            {
                aScaleWidth = Fraction( aObjArea.Width(),   _aVisArea.Width() );
                aScaleHeight = Fraction( aObjArea.Height(), _aVisArea.Height());
            }
        }
    }

    
    
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
        
        const SwFlyFrmFmt *pFlyFrmFmt = dynamic_cast< const SwFlyFrmFmt * >( GetFlyFrmFmt() );
        OSL_ENSURE( pFlyFrmFmt, "Could not find fly frame." );
        if ( pFlyFrmFmt )
        {
            const Point &rPoint = pFlyFrmFmt->GetLastFlyFrmPrtRectPos();
            SwRect aRect( pFlyPrtRect ? *pFlyPrtRect
                        : GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, 0, xObj.GetObject() ));
            aArea += rPoint - aRect.Pos(); 
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




void SwWrtShell::InsertPageBreak(const OUString *pPageDesc, ::boost::optional<sal_uInt16> oPgNum )
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
            
            GetDoc()->ClearLineNumAttrs( *GetCrsr()->GetPoint() );
        }

        const SwPageDesc *pDesc = pPageDesc
                                ? FindPageDescByName( *pPageDesc, sal_True ) : 0;
        if( pDesc )
        {
            SwFmtPageDesc aDesc( pDesc );
            aDesc.SetNumOffset( oPgNum );
            SetAttrItem( aDesc );
        }
        else
            SetAttrItem( SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE, RES_BREAK) );
        EndUndo(UNDO_UI_INSERT_PAGE_BREAK);
    }
}




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
        SetAttrItem(SvxFmtBreakItem(SVX_BREAK_COLUMN_BEFORE, RES_BREAK));

        EndUndo(UNDO_UI_INSERT_COLUMN_BREAK);
    }
}




void SwWrtShell::InsertFootnote(const OUString &rStr, sal_Bool bEndNote, sal_Bool bEdit )
{
    ResetCursorStack();
    if( CanInsert() )
    {
        if(HasSelection())
        {
            
            if(!IsCrsrPtAtEnd())
                SwapPam();
            ClearMark();
        }
        SwPosition aPos = *GetCrsr()->GetPoint();
        SwFmtFtn aFootNote( bEndNote );
        if(!rStr.isEmpty())
            aFootNote.SetNumStr( rStr );

        SetAttrItem(aFootNote);

        if( bEdit )
        {
            
            Left(CRSR_SKIP_CHARS, sal_False, 1, sal_False );
            GotoFtnTxt();
        }
        aNavigationMgr.addEntry(aPos);
    }
}





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















void SwWrtShell::NumOrBulletOn(sal_Bool bNum)
{
    
    const SwNumRule* pCurRule = GetCurNumRule();

    StartUndo(UNDO_NUMORNONUM);

    const SwNumRule * pNumRule = pCurRule;

    
    
    
    
    bool bContinueFoundNumRule( false );
    bool bActivateOutlineRule( false );
    int nActivateOutlineLvl( MAXLEVEL );    
    SwTxtFmtColl * pColl = GetCurTxtFmtColl();
    if ( pColl )
    {
        
        
        SwNumRule* pCollRule = mpDoc->FindNumRulePtr(pColl->GetNumRule().GetValue());
        
        
        
        
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
        
        else if ( bNum &&
                  !dynamic_cast<SwWebDocShell*>(GetDoc()->GetDocShell()) &&
                  pCollRule == GetDoc()->GetOutlineNumRule() )
        {
            if ( pNumRule == pCollRule )
            {
                
                
                
                SwTxtNode* pTxtNode =
                            GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();
                if ( pTxtNode && !pTxtNode->IsCountedInList() )
                {
                    
                    
                    nActivateOutlineLvl = pColl->GetAssignedOutlineStyleLevel();
                    OSL_ENSURE( pColl->IsAssignedToListLevelOfOutlineStyle(),
                            "<SwWrtShell::NumOrBulletOn(..)> - paragraph style with outline rule, but no outline level" );
                    if ( pColl->IsAssignedToListLevelOfOutlineStyle() &&
                         pCollRule->Get( static_cast<sal_uInt16>(nActivateOutlineLvl) ).GetNumberingType()
                            == SVX_NUM_NUMBER_NONE )
                    {
                        
                        bActivateOutlineRule = true;
                    }
                    else
                    {
                        
                        bContinueFoundNumRule = true;
                    }
                }
                else
                {
                    
                    
                    
                    bActivateOutlineRule = true;
                    nActivateOutlineLvl = pColl->GetAssignedOutlineStyleLevel();
                }
            }
            else if ( !pNumRule )
            {
                
                
                
                nActivateOutlineLvl = pColl->GetAssignedOutlineStyleLevel();
                if ( pCollRule->Get( static_cast<sal_uInt16>(nActivateOutlineLvl) ).GetNumberingType()
                                == SVX_NUM_NUMBER_NONE )
                {
                    
                    
                    bActivateOutlineRule = true;
                }
                else
                {
                    
                    bContinueFoundNumRule = true;
                }
            }
            else
            {
                
                
                nActivateOutlineLvl = pColl->GetAssignedOutlineStyleLevel();
                OSL_ENSURE( pColl->IsAssignedToListLevelOfOutlineStyle(),
                        "<SwWrtShell::NumOrBulletOn(..)> - paragraph style with outline rule, but no outline level" );
                if ( pColl->IsAssignedToListLevelOfOutlineStyle() &&
                     pCollRule->Get( static_cast<sal_uInt16>(nActivateOutlineLvl) ).GetNumberingType()
                        == SVX_NUM_NUMBER_NONE )
                {
                    
                    bActivateOutlineRule = true;
                }
                else
                {
                    
                    bContinueFoundNumRule = true;
                }
            }
            pNumRule = pCollRule;
        }
    }

    
    
    
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

    
    OUString sContinuedListId;
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

        
        if ( !bContinueFoundNumRule )
        {
            SwTxtNode * pTxtNode = GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();

            if (pTxtNode)
            {
                
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
                    
                    if ( numfunc::IsDefBulletFontUserDefined() )
                    {
                        const Font* pFnt = &numfunc::GetDefBulletFont();
                        aFmt.SetBulletFont( pFnt );
                    }
                    aFmt.SetBulletChar( numfunc::GetBulletChar(static_cast<sal_uInt8>(nLevel)));
                    aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                    
                    aFmt.SetPrefix(OUString());
                    aFmt.SetSuffix(OUString());
                }
                aNumRule.Set(static_cast<sal_uInt16>(nLevel), aFmt);
            }
        }

        
        SetCurNumRule( aNumRule, false, sContinuedListId, true );
    }
    else
    {
        
        const SvxNumberFormat::SvxNumPositionAndSpaceMode ePosAndSpaceMode(
                                    numfunc::GetDefaultPositionAndSpaceMode() );
        SwNumRule aNumRule( GetUniqueNumRuleName(), ePosAndSpaceMode );
        
        SwCharFmt* pChrFmt;
        SwDocShell* pDocSh = GetView().GetDocShell();
        
        
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
                
                
                if ( pFnt )
                {
                    aFmt.SetBulletFont( pFnt );
                }
                aFmt.SetBulletChar( numfunc::GetBulletChar(nLvl) );
                aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                
                aFmt.SetPrefix(OUString());
                aFmt.SetSuffix(OUString());
            }

            
            if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                if(bHtml && nLvl)
                {
                    
                    aFmt.SetLSpace(720);
                    aFmt.SetAbsLSpace(nLvl * 720);
                }
                else if ( nWidthOfTabs > 0 )
                {
                    aFmt.SetAbsLSpace(nWidthOfTabs + nLvl * 720);
                }
            }

            
            
            if ( bRightToLeft )
            {
                aFmt.SetNumAdjust( SVX_ADJUST_RIGHT );
            }

            aNumRule.Set( nLvl, aFmt );
        }

        
        if ( pTxtNode &&
             ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
        {

            const SwTwips nTxtNodeIndent = pTxtNode->GetAdditionalIndentForStartingNewList();
            if ( ( nTxtNodeIndent + nWidthOfTabs ) != 0 )
            {
                
                
                
                
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
        
        
        SetCurNumRule( aNumRule, true, OUString(), true );
    }

    EndUndo(UNDO_NUMORNONUM);
}


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
                int nLevel = pTxtNode->GetActualListLevel();

                if (nLevel < 0)
                    nLevel = 0;

                if (nLevel >= MAXLEVEL)
                    nLevel = MAXLEVEL - 1;

                SwNumFmt aFmt(aNumRule.Get(static_cast<sal_uInt16>(nLevel)));

                aFmt.SetNumberingType(SVX_NUM_NUMBER_NONE);
                aNumRule.Set(nLevel, aFmt);

                
                SetCurNumRule( aNumRule, false );
            }
        }
        else
        {
            DelNumRules();
        }

        
        
        SetInFrontOfLabel( false );
    }
}




void SwWrtShell::BulletOn()
{
    NumOrBulletOn(sal_False);
}

SelectionType SwWrtShell::GetSelectionType() const
{
    
    
    
    

    if ( BasicActionPend() )
        return IsSelFrmMode() ? nsSelectionType::SEL_FRM : nsSelectionType::SEL_TXT;

    SwView &_rView = ((SwView&)GetView());
    if (_rView.GetPostItMgr() && _rView.GetPostItMgr()->HasActiveSidebarWin() )
        return nsSelectionType::SEL_POSTIT;

    
    int nCnt;
    if ( !_rView.GetEditWin().IsFrmAction() &&
        (IsObjSelected() || (_rView.IsDrawMode() && !IsFrmSelected()) ))
    {
        if (GetDrawView()->IsTextEdit())
            nCnt = nsSelectionType::SEL_DRW_TXT;
        else
        {
            if (GetView().IsFormMode()) 
                nCnt = nsSelectionType::SEL_DRW_FORM;
            else
                nCnt = nsSelectionType::SEL_DRW;            

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
            _rView.LeaveDrawCreate();   
        if ( !(nCnt & (CNT_GRF | CNT_OLE)) )
            return nsSelectionType::SEL_FRM;
    }

    if ( IsCrsrInTbl() )
        nCnt |= nsSelectionType::SEL_TBL;

    if ( IsTableMode() )
        nCnt |= (nsSelectionType::SEL_TBL | nsSelectionType::SEL_TBL_CELLS);

    
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






SwTxtFmtColl *SwWrtShell::GetParaStyle(const OUString &rCollName, GetStyle eCreate )
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






SwCharFmt *SwWrtShell::GetCharStyle(const OUString &rFmtName, GetStyle eCreate )
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






SwFrmFmt *SwWrtShell::GetTblStyle(const OUString &rFmtName)
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



void SwWrtShell::SetPageStyle(const OUString &rCollName)
{
    if( !SwCrsrShell::HasSelection() && !IsSelFrmMode() && !IsObjSelected() )
    {
        SwPageDesc* pDesc = FindPageDescByName( rCollName, sal_True );
        if( pDesc )
            ChgCurPageDesc( *pDesc );
    }
}



OUString SwWrtShell::GetCurPageStyle( const sal_Bool bCalcFrm ) const
{
    return GetPageDesc(GetCurPageDesc( bCalcFrm )).GetName();
}



void SwWrtShell::QuickUpdateStyle()
{
    SwTxtFmtColl *pColl = GetCurTxtFmtColl();

    
    if(pColl && !pColl->IsDefault())
    {
        FillByEx(pColl);
            
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
        SetAttrSet(aCoreSet, 0, pCrsr);
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



SwWrtShell::SwWrtShell( SwWrtShell& rSh, Window *_pWin, SwView &rShell )
    : SwFEShell( rSh, _pWin ),
     COMMON_INI_LIST
{
    BITFLD_INI_LIST
    SET_CURR_SHELL( this );

    SetSfxViewShell( (SfxViewShell *)&rShell );
    SetFlyMacroLnk( LINK(this, SwWrtShell, ExecFlyMac) );

    
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
    if(IsSelFrmMode())
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
    
    GetView().NotifyDBChanged();
}

OUString SwWrtShell::GetSelDescr() const
{
    OUString aResult;

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
    
    GetView().GetViewFrame()->GetBindings().Invalidate( FN_VIEW_META_CHARS );
}

void SwWrtShell::SetReadonlyOption(sal_Bool bSet)
{
    GetView().GetEditWin().GetFrameControlsManager().SetReadonlyControls( bSet );
    SwViewShell::SetReadonlyOption( bSet );
}




void SwWrtShell::ChangeHeaderOrFooter(
    const OUString& rStyleName, sal_Bool bHeader, sal_Bool bOn, sal_Bool bShowWarning)
{
    addCurrentPosition();
    StartAllAction();
    StartUndo( UNDO_HEADER_FOOTER ); 
    bool bExecute = true;
    sal_Bool bCrsrSet = sal_False;
    for( sal_uInt16 nFrom = 0, nTo = GetPageDescCnt();
            nFrom < nTo; ++nFrom )
    {
        int bChgd = sal_False;
        SwPageDesc aDesc( GetPageDesc( nFrom ));
        OUString sTmp(aDesc.GetName());
        if( rStyleName.isEmpty() || rStyleName == sTmp )
        {
            if( bShowWarning && !bOn && GetActiveView() && GetActiveView() == &GetView() &&
                ( (bHeader && aDesc.GetMaster().GetHeader().IsActive()) ||
                  (!bHeader && aDesc.GetMaster().GetFooter().IsActive()) ) )
            {
                bShowWarning = sal_False;
                
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
                            rStyleName.isEmpty() ? (sal_uInt16)0xFFFF : nFrom,
                            bHeader );
                }
            }
        }
    }
    EndUndo( UNDO_HEADER_FOOTER ); 
    EndAllAction();
}

void SwWrtShell::SetShowHeaderFooterSeparator( FrameControlType eControl, bool bShow )
{
    SwViewShell::SetShowHeaderFooterSeparator( eControl, bShow );
    if ( !bShow )
        GetView().GetEditWin().GetFrameControlsManager().HideControls( eControl );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
