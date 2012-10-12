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


#include "viscrs.hxx"
#include <sfx2/frame.hxx>
#include <sfx2/printer.hxx>
#include <cmdid.h>
#include <hintids.hxx>
#include <docsh.hxx>
#include <rubylist.hxx>
#include <doc.hxx>
#include <unotxvw.hxx>
#include <unodispatch.hxx>
#include <unomap.hxx>
#include <unostyle.hxx>
#include <unoprnms.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <unomod.hxx>
#include <unoframe.hxx>
#include <unocrsr.hxx>
#include <wrtsh.hxx>
#include <unotbl.hxx>
#include <svx/fmshell.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdogrp.hxx>
#include <editeng/pbinitem.hxx>
#include <pagedesc.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <frmatr.hxx>
#include <osl/mutex.hxx>
#include <IMark.hxx>
#include <unotxdoc.hxx>
#include <unodraw.hxx>
#include <svx/unoshcol.hxx>
#include <svx/unoshape.hxx>
#include <svx/svdpagv.hxx>
#include <swerror.h>
#include <shellio.hxx>
#include <ndtxt.hxx>
#include <SwStyleNameMapper.hxx>
#include <crsskip.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <editeng/outliner.hxx>
#include <editeng/editview.hxx>
#include <unobookmark.hxx>
#include <unoparagraph.hxx>
#include <unocrsrhelper.hxx>
#include <unotextrange.hxx>
#include <sfx2/docfile.hxx>
#include <switerator.hxx>
#include "swdtflvr.hxx"
#include <vcl/svapp.hxx>
#include <comphelper/servicehelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::frame;

using ::com::sun::star::util::URL;
using comphelper::HelperBaseNoState;

using ::rtl::OUString;

static SwPaM* lcl_createPamCopy(const SwPaM& rPam)
{
    SwPaM *const pRet = new SwPaM(*rPam.GetPoint());
    ::sw::DeepCopyPaM(rPam, *pRet);
    return pRet;
}

/******************************************************************
 * SwXTextView
 ******************************************************************/
SwXTextView::SwXTextView(SwView* pSwView) :
    SfxBaseController(pSwView),
    m_SelChangedListeners(m_aMutex),
    m_pView(pSwView),
    m_pPropSet( aSwMapProvider.GetPropertySet( PROPERTY_MAP_TEXT_VIEW ) ),
    pxViewSettings(0),
    pxTextViewCursor(0)
{

}

SwXTextView::~SwXTextView()
{
    Invalidate();
}

void SwXTextView::Invalidate()
{
    if(pxViewSettings)
    {
         HelperBaseNoState *pSettings = static_cast < HelperBaseNoState * > ( pxViewSettings->get() );
        static_cast < SwXViewSettings* > ( pSettings )->Invalidate();
        DELETEZ(pxViewSettings);
    }
    if(pxTextViewCursor)
    {
        text::XTextViewCursor* pCrsr = pxTextViewCursor->get();
        ((SwXTextViewCursor*)pCrsr)->Invalidate();
        DELETEZ(pxTextViewCursor);
    }

    m_refCount++; //prevent second d'tor call

    {
        uno::Reference<uno::XInterface> const xInt(static_cast<
                cppu::OWeakObject*>(static_cast<SfxBaseController*>(this)));
        lang::EventObject aEvent(xInt);
        m_SelChangedListeners.disposeAndClear(aEvent);
    }

    m_refCount--;
    m_pView = 0;
}

Sequence< uno::Type > SAL_CALL SwXTextView::getTypes(  ) throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aBaseTypes = SfxBaseController::getTypes();

    long nIndex = aBaseTypes.getLength();
    aBaseTypes.realloc(
        aBaseTypes.getLength() + 8 );

    uno::Type* pBaseTypes = aBaseTypes.getArray();
    pBaseTypes[nIndex++] = ::getCppuType((uno::Reference<XSelectionSupplier >*)0);
    pBaseTypes[nIndex++] = ::getCppuType((uno::Reference<XServiceInfo           >*)0);
    pBaseTypes[nIndex++] = ::getCppuType((uno::Reference<XFormLayerAccess   >*)0);
    pBaseTypes[nIndex++] = ::getCppuType((uno::Reference<XTextViewCursorSupplier>*)0);
    pBaseTypes[nIndex++] = ::getCppuType((uno::Reference<XViewSettingsSupplier  >*)0);
    pBaseTypes[nIndex++] = ::getCppuType((uno::Reference<XRubySelection >*)0);
    pBaseTypes[nIndex++] = ::getCppuType((uno::Reference<XPropertySet  >*)0);
    pBaseTypes[nIndex++] = ::getCppuType((uno::Reference<datatransfer::XTransferableSupplier >*)0);
    return aBaseTypes;
}

namespace
{
    class theSwXTextViewImplementationId : public rtl::Static< UnoTunnelIdInit, theSwXTextViewImplementationId > {};
}

Sequence< sal_Int8 > SAL_CALL SwXTextView::getImplementationId(  ) throw(uno::RuntimeException)
{
    return theSwXTextViewImplementationId::get().getSeq();
}

void SAL_CALL SwXTextView::acquire(  )throw()
{
    SfxBaseController::acquire();
}

void SAL_CALL SwXTextView::release(  )throw()
{
    SfxBaseController::release();
}

uno::Any SAL_CALL SwXTextView::queryInterface( const uno::Type& aType )
    throw (RuntimeException)
{
    uno::Any aRet;
    if(aType == ::getCppuType((uno::Reference<view::XSelectionSupplier  >*)0))
    {
        uno::Reference<view::XSelectionSupplier> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == ::getCppuType((uno::Reference<lang::XServiceInfo           >*)0))
    {
        uno::Reference<lang::XServiceInfo> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == ::getCppuType((uno::Reference<view::XControlAccess     >*)0))
    {
        uno::Reference<view::XControlAccess> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == ::getCppuType((uno::Reference<view::XFormLayerAccess   >*)0))
    {
        uno::Reference<view::XFormLayerAccess> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == ::getCppuType((uno::Reference<text::XTextViewCursorSupplier>*)0))
    {
        uno::Reference<text::XTextViewCursorSupplier> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == ::getCppuType((uno::Reference<view::XViewSettingsSupplier  >*)0))
    {
        uno::Reference<view::XViewSettingsSupplier> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == ::getCppuType((uno::Reference<XRubySelection>*)0))
    {
        uno::Reference<XRubySelection> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == ::getCppuType((uno::Reference<XPropertySet>*)0))
    {
        uno::Reference<XPropertySet> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == ::getCppuType((uno::Reference<datatransfer::XTransferableSupplier   >*)0))
    {
        uno::Reference<datatransfer::XTransferableSupplier> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else
        aRet = SfxBaseController::queryInterface(aType);
    return aRet;
}

sal_Bool SwXTextView::select(const uno::Any& aInterface) throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< uno::XInterface >  xInterface;
    if(GetView() && (aInterface >>= xInterface))
    {
        SwWrtShell& rSh = GetView()->GetWrtShell();
        SwDoc* pDoc = GetView()->GetDocShell()->GetDoc();
        uno::Reference< lang::XUnoTunnel >  xIfcTunnel(xInterface, uno::UNO_QUERY);
        uno::Reference< text::XTextCursor >  xCrsr(xInterface, uno::UNO_QUERY);
        uno::Reference< container::XIndexAccess >   xPosN(xInterface, uno::UNO_QUERY);
        uno::Reference< text::XTextRange >  xPos(xInterface, uno::UNO_QUERY);
        SwXFrame* pFrame = xIfcTunnel.is() ? reinterpret_cast<SwXFrame*>(
            xIfcTunnel->getSomething(SwXFrame::getUnoTunnelId())) : 0;

        SwXCell* pCell = xIfcTunnel.is() ? reinterpret_cast<SwXCell*>(
            xIfcTunnel->getSomething(SwXCell::getUnoTunnelId())) : 0;

        SwPaM * pPam = 0;
        SwXTextRanges* pPosN = 0;
        if(xCrsr.is())
        {
            //
            OTextCursorHelper* pCursor =
                xIfcTunnel.is() ?
                    reinterpret_cast<OTextCursorHelper*>(xIfcTunnel->getSomething(OTextCursorHelper::getUnoTunnelId()))
                    : 0;

            if(pCursor && pCursor->GetDoc() == GetView()->GetDocShell()->GetDoc())
            {
                pPam = lcl_createPamCopy(*pCursor->GetPaM());
            }
        }
        else if(xPosN.is() &&
            xIfcTunnel.is() &&
                0 != (pPosN = reinterpret_cast<SwXTextRanges*>(xIfcTunnel->getSomething(SwXTextRanges::getUnoTunnelId()))))
        {
            const SwUnoCrsr* pUnoCrsr = pPosN->GetCursor();
            if(pUnoCrsr)
            {
                pPam = lcl_createPamCopy(*pUnoCrsr);
            }
        }
        // prevent misinterpretation of text frames that provide a XTextRange interface, too
        else if(!pFrame && !pCell && xPos.is())
        {
            SwUnoInternalPaM aPam(*pDoc);
            if (::sw::XTextRangeToSwPaM(aPam, xPos))
            {
                pPam = lcl_createPamCopy(aPam);
            }
        }
        if(pPam)
        {
            rSh.EnterStdMode();
            rSh.SetSelection(*pPam);
            while( pPam->GetNext() != pPam )
                delete pPam->GetNext();
            delete pPam;
            return sal_True;
        }
        if(pFrame)
        {

            SwFrmFmt* pFrmFmt = pFrame->GetFrmFmt();
            if(pFrmFmt && pFrmFmt->GetDoc() == pDoc)
            {
                sal_Bool bSuccess = rSh.GotoFly( pFrmFmt->GetName(), pFrame->GetFlyCntType());
                if (bSuccess)
                {
                    rSh.HideCrsr();
                    rSh.EnterSelFrmMode();
                }
                return sal_True;
            }
        }

        uno::Reference< text::XTextTable >  xTbl(xInterface, uno::UNO_QUERY);

        if(xTbl.is() && xIfcTunnel.is())
        {
            SwXTextTable* pTable = reinterpret_cast<SwXTextTable*>(
                xIfcTunnel->getSomething(SwXTextTable::getUnoTunnelId()));

            SwFrmFmt* pTblFrmFmt = pTable ? ((SwXTextTable*)pTable)->GetFrmFmt() : 0;
            if(pTblFrmFmt &&pTblFrmFmt->GetDoc() == pDoc)
            {
                rSh.EnterStdMode();
                rSh.GotoTable(pTblFrmFmt->GetName());
            }
            return sal_True;
        }

        if(pCell)
        {
            SwFrmFmt* pTblFrmFmt = pCell->GetFrmFmt();
            if(pTblFrmFmt && pTblFrmFmt->GetDoc() == pDoc)
            {
                SwTableBox* pBox = pCell->GetTblBox();
                SwTable* pTable = SwTable::FindTable( pTblFrmFmt );
                pBox = pCell->FindBox(pTable, pBox);
                if(pBox)
                {
                    const SwStartNode* pSttNd = pBox->GetSttNd();
                    SwPosition aPos(*pSttNd);
                    SwPaM aPam(aPos);
                    aPam.Move(fnMoveForward, fnGoNode);
                    rSh.EnterStdMode();
                    rSh.SetSelection(aPam);
                    return sal_True;
                }
            }
        }
        SwXCellRange* pRange = xIfcTunnel.is() ? reinterpret_cast<SwXCellRange*>(
            xIfcTunnel->getSomething(SwXCellRange::getUnoTunnelId())) : 0;
        if(pRange)
        {
           const SwUnoCrsr* pUnoCrsr = pRange->GetTblCrsr();
           if(pUnoCrsr)
           {
                UnoActionRemoveContext aContext(pDoc);
                rSh.EnterStdMode();
                rSh.SetSelection(*pUnoCrsr);
                return sal_True;
           }
        }
        uno::Reference< text::XTextContent >  xBkm(xInterface, uno::UNO_QUERY);

        if(xBkm.is() && xIfcTunnel.is())
        {
            ::sw::mark::IMark const*const pMark(
                    SwXBookmark::GetBookmarkInDoc(pDoc, xIfcTunnel) );
            if (pMark)
            {
                rSh.EnterStdMode();
                rSh.GotoMark(pMark);
                return sal_True;
            }
        }
        // IndexMark, Index, TextField, Draw, Section, Footnote, Paragraph
        //

        // detect controls

        uno::Reference< awt::XControlModel > xCtrlModel(xInterface, UNO_QUERY);
        if(xCtrlModel.is())
        {
            uno::Reference<awt::XControl> XControl;
            SdrObject* pObj = GetControl(xCtrlModel, XControl);
            if(pObj)
            {
                SdrView* pDrawView = rSh.GetDrawView();
                SdrPageView* pPV = pDrawView->GetSdrPageView();
                if ( pPV && pObj->GetPage() == pPV->GetPage() )
                {
                    pDrawView->SdrEndTextEdit();
                    pDrawView->UnmarkAll();
                    pDrawView->MarkObj( pObj, pPV );
                }
                return sal_True;
            }
        }

        uno::Reference< drawing::XShapes >  xShapeColl( xInterface, uno::UNO_QUERY );
        uno::Reference< beans::XPropertySet >  xTmpProp(xInterface, uno::UNO_QUERY);
        SwXShape* pSwXShape = 0;
        if(xIfcTunnel.is())
            pSwXShape = reinterpret_cast<SwXShape*>(xIfcTunnel->getSomething(SwXShape::getUnoTunnelId()));
        SvxShape* pSvxShape = 0;
        if(pSwXShape)
        {
            uno::Reference< uno::XAggregation >     xAgg = pSwXShape->GetAggregationInterface();
            if(xAgg.is())
            {
                pSvxShape = reinterpret_cast<SvxShape*>(xIfcTunnel->getSomething(SvxShape::getUnoTunnelId()));
            }
        }

        if ( pSvxShape || xShapeColl.is() )         // Drawing drawing::Layer
        {
            SdrView* pDrawView = rSh.GetDrawView();
            if (pDrawView)
            {
                pDrawView->SdrEndTextEdit();
                pDrawView->UnmarkAll();

                if (pSvxShape)      // einzelnes Shape
                {
                    SdrObject *pObj = pSvxShape->GetSdrObject();
                    if (pObj)
                    {
                        SdrPageView* pPV = pDrawView->GetSdrPageView();
                        if ( pPV && pObj->GetPage() == pPV->GetPage() )
                        {
                            pDrawView->MarkObj( pObj, pPV );
                            return sal_True;
                        }
                    }
                }
                else    // Shape Collection
                {
                    sal_Bool bSelected = sal_False;
                    SdrPageView* pPV = NULL;
                    long nCount = xShapeColl->getCount();
                    for ( long i = 0; i < nCount; i++ )
                    {
                        uno::Reference< drawing::XShape >  xShapeInt;
                        uno::Any aAny = xShapeColl->getByIndex(i);
                        aAny >>= xShapeInt;
                        if (xShapeInt.is())
                        {
                            uno::Reference< lang::XUnoTunnel> xShapeTunnel(xShapeInt, uno::UNO_QUERY);

                            SvxShape* pShape = xShapeTunnel.is() ?
                                reinterpret_cast<SvxShape*>(xShapeTunnel->getSomething(SvxShape::getUnoTunnelId())) : 0;

                            if (pShape)
                            {
                                SdrObject *pObj = pShape->GetSdrObject();
                                if (pObj)
                                {
                                    if (!pPV)               // erstes Objekt
                                    {
                                        pPV = pDrawView->GetSdrPageView();
                                    }
                                    if ( pPV && pObj->GetPage() == pPV->GetPage() )
                                    {
                                        pDrawView->MarkObj( pObj, pPV );
                                        bSelected = sal_True;
                                    }
                                }
                            }
                        }
                    }
                    return bSelected;
                }
            }
        }
    }
    return sal_False;

}

uno::Any SwXTextView::getSelection(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< uno::XInterface >  aRef;
    if(GetView())
    {
        //force immediat shell update
        m_pView->StopShellTimer();
        // ein interface aus der aktuellen Selektion erzeugen
        SwWrtShell& rSh = m_pView->GetWrtShell();
        ShellModes  eSelMode = m_pView->GetShellMode();
        switch(eSelMode)
        {
            case SHELL_MODE_TABLE_TEXT      :
            {
                if(rSh.GetTableCrsr())
                {
                    OSL_ENSURE(rSh.GetTableFmt(), "kein Tabellenformat?");
                    uno::Reference< text::XTextTableCursor >  xCrsr = new SwXTextTableCursor(*rSh.GetTableFmt(),
                                                    rSh.GetTableCrsr());
                    aRef = uno::Reference< uno::XInterface >  (xCrsr, uno::UNO_QUERY);
                    break;
                }

            }
            // ohne Tabellenselektion wird der Text geliefert
            //break;
            case SHELL_MODE_LIST_TEXT       :
            case SHELL_MODE_TABLE_LIST_TEXT:
            case SHELL_MODE_TEXT            :
            {
                uno::Reference< container::XIndexAccess >  xPos = new SwXTextRanges(rSh.GetCrsr());
                aRef = uno::Reference< uno::XInterface >(xPos, uno::UNO_QUERY);
            }
            break;
            case SHELL_MODE_FRAME           :
            case SHELL_MODE_GRAPHIC         :
            case SHELL_MODE_OBJECT          :
            {
                //Get FlyFrameFormat; fuer UI Macro Anbindung an Flys
                const SwFrmFmt* pFmt = rSh.GetFlyFrmFmt();
                if (pFmt)
                {
                    SwXFrame* pxFrame = SwIterator<SwXFrame,SwFmt>::FirstElement(*pFmt);
                    if(pxFrame)                //das einzige gemeinsame interface fuer alle Frames
                    {
                        aRef = uno::Reference< uno::XInterface >((cppu::OWeakObject*)pxFrame, uno::UNO_QUERY);
                    }
                    else
                    {
                        if(SHELL_MODE_FRAME == eSelMode)
                        {
                            uno::Reference< text::XTextFrame >  xFrm =  new SwXTextFrame((SwFrmFmt&)*pFmt);
                            aRef = uno::Reference< uno::XInterface >(xFrm, uno::UNO_QUERY);
                        }
                        else if(SHELL_MODE_GRAPHIC == eSelMode)
                        {
                            uno::Reference< text::XTextContent >  xFrm = new SwXTextGraphicObject((SwFrmFmt&)*pFmt);
                            aRef = xFrm;
                        }
                        else
                        {
                            uno::Reference< text::XTextContent >  xFrm =  new SwXTextEmbeddedObject((SwFrmFmt&)*pFmt);
                            aRef = xFrm;
                        }
                    }
                }
            }
            break;
            case SHELL_MODE_DRAW            :
            case SHELL_MODE_DRAW_CTRL       :
            case SHELL_MODE_DRAW_FORM       :
            case SHELL_MODE_DRAWTEXT        :
            case SHELL_MODE_BEZIER          :
            {
                uno::Reference< drawing::XDrawPageSupplier >  xPageSupp;
                uno::Reference< frame::XModel > xModel = m_pView->GetDocShell()->GetBaseModel();
                uno::Reference< lang::XUnoTunnel > xModelTunnel(xModel, uno::UNO_QUERY);
                SwXTextDocument* pTextDoc = reinterpret_cast<SwXTextDocument*>(xModelTunnel->
                                getSomething(SwXTextDocument::getUnoTunnelId()));

                SwFmDrawPage* pSvxDrawPage =    pTextDoc->GetDrawPage()->GetSvxPage();
                uno::Reference< drawing::XShapes >  xShCol = new SvxShapeCollection();

                const SdrMarkList& rMarkList = rSh.GetDrawView()->GetMarkedObjectList();
                for(sal_uInt16 i = 0; i < rMarkList.GetMarkCount(); i++)
                {
                    SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
                    uno::Reference< uno::XInterface >  xInt = pSvxDrawPage->GetInterface( pObj );
                    uno::Reference< drawing::XShape >  xShape(xInt, uno::UNO_QUERY);
                    xShCol->add(xShape);
                }
                aRef = uno::Reference< uno::XInterface >(xShCol, uno::UNO_QUERY);
            }
            break;
            default:;//prevent warning
        }
    }
    uno::Any aRet(&aRef, ::getCppuType((uno::Reference<uno::XInterface>*)0));
    return aRet;
}

void SwXTextView::addSelectionChangeListener(
                                    const uno::Reference< view::XSelectionChangeListener > & rxListener)
                                    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    m_SelChangedListeners.addInterface(rxListener);
}

void SwXTextView::removeSelectionChangeListener(
                                        const uno::Reference< view::XSelectionChangeListener > & rxListener)
                                        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    m_SelChangedListeners.removeInterface(rxListener);
}

SdrObject* SwXTextView::GetControl(
        const uno::Reference< awt::XControlModel > & xModel,
        uno::Reference< awt::XControl >& xToFill  )
{
    SwView* pView2 = GetView();
    FmFormShell* pFormShell = pView2 ? pView2->GetFormShell() : NULL;
    SdrView* pDrawView = pView2 ? pView2->GetDrawView() : NULL;
    Window* pWindow = pView2 ? pView2->GetWrtShell().GetWin() : NULL;

    OSL_ENSURE( pFormShell && pDrawView && pWindow, "SwXTextView::GetControl: how could I?" );

    SdrObject* pControl = NULL;
    if ( pFormShell && pDrawView && pWindow )
        pControl = pFormShell->GetFormControl( xModel, *pDrawView, *pWindow, xToFill );
    return pControl;
}

uno::Reference< awt::XControl >  SwXTextView::getControl(const uno::Reference< awt::XControlModel > & xModel)
        throw( container::NoSuchElementException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< awt::XControl >  xRet;
    GetControl(xModel, xRet);
    return xRet;
}

uno::Reference< form::runtime::XFormController > SAL_CALL SwXTextView::getFormController( const uno::Reference< form::XForm >& _Form ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    SwView* pView2 = GetView();
    FmFormShell* pFormShell = pView2 ? pView2->GetFormShell() : NULL;
    SdrView* pDrawView = pView2 ? pView2->GetDrawView() : NULL;
    Window* pWindow = pView2 ? pView2->GetWrtShell().GetWin() : NULL;
    OSL_ENSURE( pFormShell && pDrawView && pWindow, "SwXTextView::getFormController: how could I?" );

    uno::Reference< form::runtime::XFormController > xController;
    if ( pFormShell && pDrawView && pWindow )
        xController = pFormShell->GetFormController( _Form, *pDrawView, *pWindow );
    return xController;
}

::sal_Bool SAL_CALL SwXTextView::isFormDesignMode(  ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SwView* pView2 = GetView();
    FmFormShell* pFormShell = pView2 ? pView2->GetFormShell() : NULL;
    return pFormShell ? pFormShell->IsDesignMode() : sal_True;
}

void SAL_CALL SwXTextView::setFormDesignMode( ::sal_Bool _DesignMode ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    SwView* pView2 = GetView();
    FmFormShell* pFormShell = pView2 ? pView2->GetFormShell() : NULL;
    if ( pFormShell )
        pFormShell->SetDesignMode( _DesignMode );
}

uno::Reference< text::XTextViewCursor >  SwXTextView::getViewCursor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(GetView())
    {
        if(!pxTextViewCursor)
        {
            ((SwXTextView*)this)->pxTextViewCursor = new uno::Reference< text::XTextViewCursor > ;
            *pxTextViewCursor = new  SwXTextViewCursor(GetView());
        }
        return *pxTextViewCursor;
    }
    else
        throw uno::RuntimeException();
}

uno::Reference< beans::XPropertySet >  SwXTextView::getViewSettings(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if(!pxViewSettings)
        {
            ((SwXTextView*)this)->pxViewSettings = new uno::Reference< beans::XPropertySet > ;
            *pxViewSettings = static_cast < HelperBaseNoState * > ( new SwXViewSettings( sal_False, m_pView ) );
        }
    }
    else
        throw uno::RuntimeException();
    return *pxViewSettings;
}

Sequence< Sequence< PropertyValue > > SwXTextView::getRubyList( sal_Bool /*bAutomatic*/ ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    if(!GetView())
        throw RuntimeException();
    SwWrtShell& rSh = m_pView->GetWrtShell();
    ShellModes  eSelMode = m_pView->GetShellMode();
    if (eSelMode != SHELL_MODE_LIST_TEXT      &&
        eSelMode != SHELL_MODE_TABLE_LIST_TEXT &&
        eSelMode != SHELL_MODE_TABLE_TEXT      &&
        eSelMode != SHELL_MODE_TEXT           )
        return Sequence< Sequence< PropertyValue > > ();

    SwDoc* pDoc = m_pView->GetDocShell()->GetDoc();
    SwRubyList aList;

    sal_uInt16 nCount = pDoc->FillRubyList( *rSh.GetCrsr(), aList, 0 );
    Sequence< Sequence< PropertyValue > > aRet(nCount);
    Sequence< PropertyValue >* pRet = aRet.getArray();
    String aString;
    for(sal_uInt16 n = 0; n < nCount; n++)
    {
        const SwRubyListEntry* pEntry = &aList[n];

        const String& rEntryText = pEntry->GetText();
        const SwFmtRuby& rAttr = pEntry->GetRubyAttr();

        pRet[n].realloc(5);
        PropertyValue* pValues = pRet[n].getArray();
        pValues[0].Name = rtl::OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_RUBY_BASE_TEXT));
        pValues[0].Value <<= OUString(rEntryText);
        pValues[1].Name = rtl::OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_RUBY_TEXT));
        pValues[1].Value <<= OUString(rAttr.GetText());
        pValues[2].Name = rtl::OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_RUBY_CHAR_STYLE_NAME));
        SwStyleNameMapper::FillProgName(rAttr.GetCharFmtName(), aString, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
        pValues[2].Value <<= OUString( aString );
        pValues[3].Name = rtl::OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_RUBY_ADJUST));
        pValues[3].Value <<= (sal_Int16)rAttr.GetAdjustment();
        pValues[4].Name = rtl::OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_RUBY_IS_ABOVE));
        sal_Bool bVal = !rAttr.GetPosition();
        pValues[4].Value.setValue(&bVal, ::getBooleanCppuType());
    }
    return aRet;
}

void SAL_CALL SwXTextView::setRubyList(
    const Sequence< Sequence< PropertyValue > >& rRubyList, sal_Bool /*bAutomatic*/ )
        throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    if(!GetView() || !rRubyList.getLength())
        throw RuntimeException();
    SwWrtShell& rSh = m_pView->GetWrtShell();
    ShellModes  eSelMode = m_pView->GetShellMode();
    if (eSelMode != SHELL_MODE_LIST_TEXT      &&
        eSelMode != SHELL_MODE_TABLE_LIST_TEXT &&
        eSelMode != SHELL_MODE_TABLE_TEXT      &&
        eSelMode != SHELL_MODE_TEXT           )
        throw RuntimeException();

    SwRubyList aList;

    const Sequence<PropertyValue>* pRubyList = rRubyList.getConstArray();
    for(sal_Int32 nPos = 0; nPos < rRubyList.getLength(); nPos++)
    {
        SwRubyListEntry* pEntry = new SwRubyListEntry;
        const PropertyValue* pProperties = pRubyList[nPos].getConstArray();
        OUString sTmp;
        for(sal_Int32 nProp = 0; nProp < pRubyList[nPos].getLength(); nProp++)
        {
            if(pProperties[nProp].Name.equalsAsciiL(
                                    SW_PROP_NAME(UNO_NAME_RUBY_BASE_TEXT)))
            {
                pProperties[nProp].Value >>= sTmp;
                pEntry->SetText(sTmp);
            }
            else if(pProperties[nProp].Name.equalsAsciiL(
                                    SW_PROP_NAME(UNO_NAME_RUBY_TEXT)))
            {
                pProperties[nProp].Value >>= sTmp;
                pEntry->GetRubyAttr().SetText(sTmp);
            }
            else if(pProperties[nProp].Name.equalsAsciiL(
                                    SW_PROP_NAME(UNO_NAME_RUBY_CHAR_STYLE_NAME)))
            {
                if((pProperties[nProp].Value >>= sTmp))
                {
                    String sName;
                    SwStyleNameMapper::FillUIName(sTmp, sName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
                    sal_uInt16 nPoolId = sName.Len() ?
                        SwStyleNameMapper::GetPoolIdFromUIName( sName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT ) : 0;

                    pEntry->GetRubyAttr().SetCharFmtName( sName );
                    pEntry->GetRubyAttr().SetCharFmtId( nPoolId );
                }
            }
            else if(pProperties[nProp].Name.equalsAsciiL(
                                    SW_PROP_NAME(UNO_NAME_RUBY_ADJUST)))
            {
                sal_Int16 nTmp = 0;
                if((pProperties[nProp].Value >>= nTmp))
                    pEntry->GetRubyAttr().SetAdjustment(nTmp);
            }
            else if(pProperties[nProp].Name.equalsAsciiL(
                                    SW_PROP_NAME(UNO_NAME_RUBY_IS_ABOVE)))
            {
                sal_Bool bValue = pProperties[nProp].Value.hasValue() ?
                    *(sal_Bool*)pProperties[nProp].Value.getValue() : sal_True;
                pEntry->GetRubyAttr().SetPosition(bValue ? 0 : 1);
            }
        }
        aList.insert(aList.begin() + nPos, pEntry);
    }
    SwDoc* pDoc = m_pView->GetDocShell()->GetDoc();
    pDoc->SetRubyList( *rSh.GetCrsr(), aList, 0 );
}

SfxObjectShellLock SwXTextView::BuildTmpSelectionDoc()
{
    SwWrtShell& rOldSh = m_pView->GetWrtShell();
    SfxPrinter *pPrt = rOldSh.getIDocumentDeviceAccess()->getPrinter( false );
    SwDocShell* pDocSh;
    SfxObjectShellLock xDocSh( pDocSh = new SwDocShell( /*pPrtDoc, */SFX_CREATE_MODE_STANDARD ) );
    xDocSh->DoInitNew( 0 );
    SwDoc *const pTempDoc( pDocSh->GetDoc() );
    // #i103634#, #i112425#: do not expand numbering and fields on PDF export
    pTempDoc->SetClipBoard(true);
    rOldSh.FillPrtDoc(pTempDoc,  pPrt);
    SfxViewFrame* pDocFrame = SfxViewFrame::LoadHiddenDocument( *xDocSh, 0 );
    SwView* pDocView = (SwView*) pDocFrame->GetViewShell();
    pDocView->AttrChangedNotify( &pDocView->GetWrtShell() );//Damit SelectShell gerufen wird.
    SwWrtShell* pSh = pDocView->GetWrtShellPtr();

    IDocumentDeviceAccess* pIDDA = pSh->getIDocumentDeviceAccess();
    SfxPrinter* pTempPrinter = pIDDA->getPrinter( true );

    const SwPageDesc& rCurPageDesc = rOldSh.GetPageDesc(rOldSh.GetCurPageDesc());

    IDocumentDeviceAccess* pIDDA_old = rOldSh.getIDocumentDeviceAccess();

    if( pIDDA_old->getPrinter( false ) )
    {
        pIDDA->setJobsetup( *pIDDA_old->getJobsetup() );
        //#69563# if it isn't the same printer then the pointer has been invalidated!
        pTempPrinter = pIDDA->getPrinter( true );
    }

    pTempPrinter->SetPaperBin(rCurPageDesc.GetMaster().GetPaperBin().GetValue());

    return xDocSh;
}

void SwXTextView::NotifySelChanged()
{
    OSL_ENSURE( m_pView, "view is missing" );

    uno::Reference<uno::XInterface> const xInt(
        static_cast<cppu::OWeakObject*>(static_cast<SfxBaseController*>(this)));

    lang::EventObject const aEvent(xInt);
    m_SelChangedListeners.notifyEach(
            &view::XSelectionChangeListener::selectionChanged, aEvent);
}

namespace {
    struct DispatchListener
    {
        URL const & m_rURL;
        Sequence<PropertyValue> const& m_rSeq;
        explicit DispatchListener(URL const& rURL,
                Sequence<PropertyValue> const& rSeq)
            : m_rURL(rURL), m_rSeq(rSeq) { }
        void operator()(uno::Reference<XDispatch> const & xListener) const
        {
            xListener->dispatch(m_rURL, m_rSeq);
        }
    };
}

void SwXTextView::NotifyDBChanged()
{
    URL aURL;
    aURL.Complete = rtl::OUString::createFromAscii(SwXDispatch::GetDBChangeURL());

    m_SelChangedListeners.forEach<XDispatch>(
            DispatchListener(aURL, Sequence<PropertyValue>(0)));
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL SwXTextView::getPropertySetInfo(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Reference< XPropertySetInfo > aRef = m_pPropSet->getPropertySetInfo();
    return aRef;
}

void SAL_CALL SwXTextView::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pEntry)
        throw UnknownPropertyException();
    else if (pEntry->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException();
    else
    {
        switch (pEntry->nWID)
        {
            case WID_IS_HIDE_SPELL_MARKS :
                // deprecated #i91949
            break;
            case WID_IS_CONSTANT_SPELLCHECK :
            {
                sal_Bool bVal = sal_False;
                const SwViewOption *pOpt = m_pView->GetWrtShell().GetViewOptions();
                if (!pOpt || !(rValue >>= bVal))
                    throw RuntimeException();
                SwViewOption aNewOpt( *pOpt );
                if (pEntry->nWID == WID_IS_CONSTANT_SPELLCHECK)
                    aNewOpt.SetOnlineSpell(bVal);
                m_pView->GetWrtShell().ApplyViewOptions( aNewOpt );
            }
            break;
            default :
                OSL_FAIL("unknown WID");
        }
    }
}

uno::Any SAL_CALL SwXTextView::getPropertyValue(
        const OUString& rPropertyName )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Any aRet;

    const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pEntry)
        throw UnknownPropertyException();
    else
    {
        sal_Int16 nWID = pEntry->nWID;
        switch (nWID)
        {
            case WID_PAGE_COUNT :
            case WID_LINE_COUNT :
            {
                // format document completely in order to get meaningful
                // values for page count and line count
                m_pView->GetWrtShell().CalcLayout();

                sal_Int32 nCount = -1;
                if (nWID == WID_PAGE_COUNT)
                    nCount = m_pView->GetWrtShell().GetPageCount();
                else // WID_LINE_COUNT
                    nCount = m_pView->GetWrtShell().GetLineCount( sal_False /*of whole document*/ );
                aRet <<= nCount;
            }
            break;
            case WID_IS_HIDE_SPELL_MARKS :
                // deprecated #i91949
            break;
            case WID_IS_CONSTANT_SPELLCHECK :
            {
                const SwViewOption *pOpt = m_pView->GetWrtShell().GetViewOptions();
                if (!pOpt)
                    throw RuntimeException();
                sal_uInt32 nFlag = VIEWOPT_1_ONLINESPELL;
                sal_Bool bVal = 0 != (pOpt->GetCoreOptions() & nFlag);
                aRet <<= bVal;
            }
            break;
            default :
                OSL_FAIL("unknown WID");
        }
    }

    return aRet;
}

void SAL_CALL SwXTextView::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*rxListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SwXTextView::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*rxListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SwXTextView::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*rxListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SwXTextView::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*rxListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

OUString SwXTextView::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextView");
}

sal_Bool SwXTextView::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return rServiceName == "com.sun.star.text.TextDocumentView" || rServiceName == "com.sun.star.view.OfficeDocumentView";
}

Sequence< OUString > SwXTextView::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextDocumentView");
    pArray[1] = C2U("com.sun.star.view.OfficeDocumentView");
    return aRet;
}

/******************************************************************
 * SwXTextViewCursor
 ******************************************************************/
SwXTextViewCursor::SwXTextViewCursor(SwView* pVw) :
    m_pView(pVw),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR))
{
}

SwXTextViewCursor::~SwXTextViewCursor()
{
}

// used to determine if there is a text selction or not.
// If there is no text selection the functions that need a working
// cursor will be disabled (throw RuntimeException). This will be the case
// for the following interfaces:
// - XViewCursor
// - XTextCursor
// - XTextRange
// - XLineCursor
sal_Bool SwXTextViewCursor::IsTextSelection( sal_Bool bAllowTables ) const
{

    sal_Bool bRes = sal_False;
    OSL_ENSURE(m_pView, "m_pView is NULL ???");
    if(m_pView)
    {
        //! m_pView->GetShellMode() will only work after the shell
        //! has already changed and thus can not be used here!
        SelectionType eSelType = m_pView->GetWrtShell().GetSelectionType();
        bRes =  ( (nsSelectionType::SEL_TXT & eSelType) ||
                  (nsSelectionType::SEL_NUM & eSelType) )  &&
                (!(nsSelectionType::SEL_TBL_CELLS & eSelType) || bAllowTables);
    }
    return bRes;
}

sal_Bool SwXTextViewCursor::isVisible(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    OSL_FAIL("not implemented");
    return sal_True;
}

void SwXTextViewCursor::setVisible(sal_Bool /*bVisible*/) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    OSL_FAIL("not implemented");
}

awt::Point SwXTextViewCursor::getPosition(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    awt::Point aRet;
    if(m_pView)
    {
        const SwWrtShell& rSh = m_pView->GetWrtShell();
        const SwRect aCharRect(rSh.GetCharRect());

        const SwFrmFmt& rMaster = rSh.GetPageDesc( rSh.GetCurPageDesc() ).GetMaster();

        const SvxULSpaceItem& rUL = rMaster.GetULSpace();
        const long nY = aCharRect.Top() - (rUL.GetUpper() + DOCUMENTBORDER);
        aRet.Y = TWIP_TO_MM100(nY);

        const SvxLRSpaceItem& rLR = rMaster.GetLRSpace();
        const long nX = aCharRect.Left() - (rLR.GetLeft() + DOCUMENTBORDER);
        aRet.X = TWIP_TO_MM100(nX);
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXTextViewCursor::collapseToStart(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        SwWrtShell& rSh = m_pView->GetWrtShell();
        if(rSh.HasSelection())
        {
            SwPaM* pShellCrsr = rSh.GetCrsr();
            if(*pShellCrsr->GetPoint() > *pShellCrsr->GetMark())
                pShellCrsr->Exchange();
            pShellCrsr->DeleteMark();
            rSh.EnterStdMode();
            rSh.SetSelection(*pShellCrsr);
        }
    }
    else
        throw uno::RuntimeException();
}

void SwXTextViewCursor::collapseToEnd(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        SwWrtShell& rSh = m_pView->GetWrtShell();
        if(rSh.HasSelection())
        {
            SwPaM* pShellCrsr = rSh.GetCrsr();
            if(*pShellCrsr->GetPoint() < *pShellCrsr->GetMark())
                pShellCrsr->Exchange();
            pShellCrsr->DeleteMark();
            rSh.EnterStdMode();
            rSh.SetSelection(*pShellCrsr);
        }
    }
    else
        throw uno::RuntimeException();
}

sal_Bool SwXTextViewCursor::isCollapsed(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        const SwWrtShell& rSh = m_pView->GetWrtShell();
        bRet = !rSh.HasSelection();
    }
    else
        throw uno::RuntimeException();
    return bRet;

}

sal_Bool SwXTextViewCursor::goLeft(sal_Int16 nCount, sal_Bool bExpand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        for( sal_uInt16 i = 0; i < nCount; i++ )
            bRet = m_pView->GetWrtShell().Left( CRSR_SKIP_CHARS, bExpand, 1, sal_True );
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::goRight(sal_Int16 nCount, sal_Bool bExpand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        for( sal_uInt16 i = 0; i < nCount; i++ )
            bRet = m_pView->GetWrtShell().Right( CRSR_SKIP_CHARS, bExpand, 1, sal_True );
    }
    else
        throw uno::RuntimeException();
    return bRet;

}

void SwXTextViewCursor::gotoRange(
    const uno::Reference< text::XTextRange > & xRange,
    sal_Bool bExpand)
        throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if(m_pView && xRange.is())
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        SwUnoInternalPaM rDestPam(*m_pView->GetDocShell()->GetDoc());
        if (!::sw::XTextRangeToSwPaM(rDestPam, xRange))
        {
            throw uno::RuntimeException();
        }

        ShellModes  eSelMode = m_pView->GetShellMode();
        SwWrtShell& rSh = m_pView->GetWrtShell();
        // call EnterStdMode in non-text selections only
        if(!bExpand ||
           (eSelMode != SHELL_MODE_TABLE_TEXT &&
            eSelMode != SHELL_MODE_LIST_TEXT &&
            eSelMode != SHELL_MODE_TABLE_LIST_TEXT &&
            eSelMode != SHELL_MODE_TEXT ))
                rSh.EnterStdMode();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        SwPaM aOwnPaM(*pShellCrsr->GetPoint());
        if(pShellCrsr->HasMark())
        {
            aOwnPaM.SetMark();
            *aOwnPaM.GetMark() = *pShellCrsr->GetMark();
        }

        uno::Reference<lang::XUnoTunnel> xRangeTunnel( xRange, uno::UNO_QUERY);
        SwXTextRange* pRange = 0;
        SwXParagraph* pPara = 0;
        OTextCursorHelper* pCursor = 0;
        if(xRangeTunnel.is())
        {
            pRange = reinterpret_cast<SwXTextRange*>(xRangeTunnel->getSomething(
                                    SwXTextRange::getUnoTunnelId()));
            pCursor = reinterpret_cast<OTextCursorHelper*>(xRangeTunnel->getSomething(
                                    OTextCursorHelper::getUnoTunnelId()));
            pPara = reinterpret_cast<SwXParagraph*>(xRangeTunnel->getSomething(
                                    SwXParagraph::getUnoTunnelId()));
        }

        const sal_uInt16 nFrmType = rSh.GetFrmType(0,sal_True);

        SwStartNodeType eSearchNodeType = SwNormalStartNode;
        if(nFrmType & FRMTYPE_FLY_ANY)
            eSearchNodeType = SwFlyStartNode;
        else if(nFrmType &FRMTYPE_HEADER)
            eSearchNodeType = SwHeaderStartNode;
        else if(nFrmType & FRMTYPE_FOOTER)
            eSearchNodeType = SwFooterStartNode;
        else if(nFrmType & FRMTYPE_TABLE)
            eSearchNodeType = SwTableBoxStartNode;
        else if(nFrmType & FRMTYPE_FOOTNOTE)
            eSearchNodeType = SwFootnoteStartNode;

        const SwStartNode* pOwnStartNode = aOwnPaM.GetNode()->
                                                FindSttNodeByType(eSearchNodeType);

        const SwNode* pSrcNode = 0;
        if(pCursor && pCursor->GetPaM())
        {
            pSrcNode = pCursor->GetPaM()->GetNode();
        }
        else if (pRange)
        {
            SwPaM aPam(pRange->GetDoc()->GetNodes());
            if (pRange->GetPositions(aPam))
            {
                pSrcNode = aPam.GetNode();
            }
        }
        else if (pPara && pPara->GetTxtNode())
        {
            pSrcNode = pPara->GetTxtNode();
        }
        const SwStartNode* pTmp = pSrcNode ? pSrcNode->FindSttNodeByType(eSearchNodeType) : 0;

        //SectionNodes ueberspringen
        while(pTmp && pTmp->IsSectionNode())
        {
            pTmp = pTmp->StartOfSectionNode();
        }
        while(pOwnStartNode && pOwnStartNode->IsSectionNode())
        {
            pOwnStartNode = pOwnStartNode->StartOfSectionNode();
        }
        //ohne Expand darf mit dem ViewCursor ueberall hingesprungen werden
        //mit Expand nur in der gleichen Umgebung
        if(bExpand &&
            (pOwnStartNode != pTmp ||
            (eSelMode != SHELL_MODE_TABLE_TEXT &&
                eSelMode != SHELL_MODE_LIST_TEXT &&
                eSelMode != SHELL_MODE_TABLE_LIST_TEXT &&
                eSelMode != SHELL_MODE_TEXT)))
            throw uno::RuntimeException();

        //jetzt muss die Selektion erweitert werden
        if(bExpand)
        {
            // der Cursor soll alles einschliessen, was bisher von ihm und dem uebergebenen
            // Range eingeschlossen wurde
            SwPosition aOwnLeft(*aOwnPaM.Start());
            SwPosition aOwnRight(*aOwnPaM.End());
            SwPosition* pParamLeft = rDestPam.Start();
            SwPosition* pParamRight = rDestPam.End();
            // jetzt sind vier SwPositions da, zwei davon werden gebraucht, also welche?
            if(aOwnRight > *pParamRight)
                *aOwnPaM.GetPoint() = aOwnRight;
            else
                *aOwnPaM.GetPoint() = *pParamRight;
            aOwnPaM.SetMark();
            if(aOwnLeft < *pParamLeft)
                *aOwnPaM.GetMark() = aOwnLeft;
            else
                *aOwnPaM.GetMark() = *pParamLeft;
        }
        else
        {
            //der Cursor soll dem uebergebenen Range entsprechen
            *aOwnPaM.GetPoint() = *rDestPam.GetPoint();
            if(rDestPam.HasMark())
            {
                aOwnPaM.SetMark();
                *aOwnPaM.GetMark() = *rDestPam.GetMark();
            }
            else
                aOwnPaM.DeleteMark();
        }
        rSh.SetSelection(aOwnPaM);
    }
    else
        throw uno::RuntimeException();

}

void SwXTextViewCursor::gotoStart(sal_Bool bExpand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        m_pView->GetWrtShell().SttDoc( bExpand );
    }
    else
        throw uno::RuntimeException();
}

void SwXTextViewCursor::gotoEnd(sal_Bool bExpand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        m_pView->GetWrtShell().EndDoc( bExpand );
    }
    else
        throw uno::RuntimeException();
}

sal_Bool SwXTextViewCursor::jumpToFirstPage(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        if (rSh.IsSelFrmMode())
        {
            rSh.UnSelectFrm();
            rSh.LeaveSelFrmMode();
        }
        rSh.EnterStdMode();
        bRet = rSh.SttEndDoc(sal_True);
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::jumpToLastPage(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        if (rSh.IsSelFrmMode())
        {
            rSh.UnSelectFrm();
            rSh.LeaveSelFrmMode();
        }
        rSh.EnterStdMode();
        bRet = rSh.SttEndDoc(sal_False);
        rSh.SttPg();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::jumpToPage(sal_Int16 nPage) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
        bRet = m_pView->GetWrtShell().GotoPage(nPage, sal_True);
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::jumpToNextPage(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
        bRet = m_pView->GetWrtShell().SttNxtPg();
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::jumpToPreviousPage(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
        bRet = m_pView->GetWrtShell().EndPrvPg();
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::jumpToEndOfPage(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
        bRet = m_pView->GetWrtShell().EndPg();
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::jumpToStartOfPage(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
        bRet = m_pView->GetWrtShell().SttPg();
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Int16 SwXTextViewCursor::getPage(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    short nRet = 0;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        nRet = (short)pShellCrsr->GetPageNum( sal_True, 0 );
    }
    else
        throw uno::RuntimeException();
    return nRet;
}

sal_Bool SwXTextViewCursor::screenDown(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
    {
        SfxRequest aReq(FN_PAGEDOWN, SFX_CALLMODE_SLOT, m_pView->GetPool());
        m_pView->Execute(aReq);
        const SfxPoolItem* pRet = aReq.GetReturnValue();
        bRet = pRet && ((const SfxBoolItem*)pRet)->GetValue();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::screenUp(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
    {
        SfxRequest aReq(FN_PAGEUP, SFX_CALLMODE_SLOT, m_pView->GetPool());
        m_pView->Execute(aReq);
        const SfxPoolItem* pRet = aReq.GetReturnValue();
        bRet = pRet && ((const SfxBoolItem*)pRet)->GetValue();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

uno::Reference< text::XText >  SwXTextViewCursor::getText(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XText >  xRet;
    if(m_pView)
    {
        if (!IsTextSelection( sal_False ))
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        SwDoc* pDoc = m_pView->GetDocShell()->GetDoc();
        xRet = ::sw::CreateParentXText(*pDoc, *pShellCrsr->Start());
    }
    else
        throw uno::RuntimeException();
    return xRet;
}

uno::Reference< text::XTextRange >  SwXTextViewCursor::getStart(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  xRet;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        SwDoc* pDoc = m_pView->GetDocShell()->GetDoc();
        xRet = SwXTextRange::CreateXTextRange(*pDoc, *pShellCrsr->Start(), 0);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}

uno::Reference< text::XTextRange >  SwXTextViewCursor::getEnd(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  xRet;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        SwDoc* pDoc = m_pView->GetDocShell()->GetDoc();
        xRet = SwXTextRange::CreateXTextRange(*pDoc, *pShellCrsr->End(), 0);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}

OUString SwXTextViewCursor::getString(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    OUString uRet;
    if(m_pView)
    {
        if (!IsTextSelection( sal_False ))
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        ShellModes  eSelMode = m_pView->GetShellMode();
        switch(eSelMode)
        {
            //! since setString for SEL_TABLE_TEXT (with possible
            //! multi selection of cells) would not work properly we
            //! will ignore this case for both
            //! functions (setString AND getString) because of symmetrie.

            case SHELL_MODE_LIST_TEXT       :
            case SHELL_MODE_TABLE_LIST_TEXT:
            case SHELL_MODE_TEXT            :
            {
                SwWrtShell& rSh = m_pView->GetWrtShell();
                SwPaM* pShellCrsr = rSh.GetCrsr();
                SwUnoCursorHelper::GetTextFromPam(*pShellCrsr, uRet);
            }
            default:;//prevent warning
        }
    }
    return uRet;
}

void SwXTextViewCursor::setString(const OUString& aString) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if (!IsTextSelection( sal_False ))
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        ShellModes  eSelMode = m_pView->GetShellMode();
        switch(eSelMode)
        {
            //! since setString for SEL_TABLE_TEXT (with possible
            //! multi selection of cells) would not work properly we
            //! will ignore this case for both
            //! functions (setString AND getString) because of symmetrie.

            case SHELL_MODE_LIST_TEXT       :
            case SHELL_MODE_TABLE_LIST_TEXT :
            case SHELL_MODE_TEXT            :
            {
                SwWrtShell& rSh = m_pView->GetWrtShell();
                SwCursor* pShellCrsr = rSh.GetSwCrsr();
                SwUnoCursorHelper::SetString(*pShellCrsr, aString);
            }
            default:;//prevent warning
        }
    }
}

uno::Reference< XPropertySetInfo >  SwXTextViewCursor::getPropertySetInfo(  ) throw(RuntimeException)
{
    static uno::Reference< XPropertySetInfo >  xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}

void  SwXTextViewCursor::setPropertyValue( const OUString& rPropertyName, const Any& aValue )
                            throw(UnknownPropertyException, PropertyVetoException,
                                IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        SwNode *pNode = pShellCrsr->GetNode();
        if (pNode && pNode->IsTxtNode())
        {
            SwUnoCursorHelper::SetPropertyValue(
                *pShellCrsr, *m_pPropSet, rPropertyName, aValue );
        }
        else
            throw RuntimeException();
    }
    else
        throw RuntimeException();
}

Any  SwXTextViewCursor::getPropertyValue( const OUString& rPropertyName )
                throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    Any aRet;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        aRet = SwUnoCursorHelper::GetPropertyValue(
                *pShellCrsr, *m_pPropSet, rPropertyName);
    }
    else
        throw RuntimeException();
    return aRet;
}

void  SwXTextViewCursor::addPropertyChangeListener(
    const OUString& /*aPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void  SwXTextViewCursor::removePropertyChangeListener(
    const OUString& /*aPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*aListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void  SwXTextViewCursor::addVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< XVetoableChangeListener >& /*aListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void  SwXTextViewCursor::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< XVetoableChangeListener >& /*aListener*/ ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

PropertyState  SwXTextViewCursor::getPropertyState( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;
    PropertyState eState;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        eState = SwUnoCursorHelper::GetPropertyState(
                *pShellCrsr, *m_pPropSet, rPropertyName);
    }
    else
        throw RuntimeException();
    return eState;
}

Sequence< PropertyState >  SwXTextViewCursor::getPropertyStates(
    const Sequence< OUString >& rPropertyNames ) throw(UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;
    Sequence< PropertyState >  aRet;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        aRet = SwUnoCursorHelper::GetPropertyStates(
                *pShellCrsr, *m_pPropSet,  rPropertyNames);
    }
    return aRet;
}

void  SwXTextViewCursor::setPropertyToDefault( const OUString& rPropertyName )
                                        throw(UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        SwUnoCursorHelper::SetPropertyToDefault(
                *pShellCrsr, *m_pPropSet, rPropertyName);
    }
}

Any  SwXTextViewCursor::getPropertyDefault( const OUString& rPropertyName )
                        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Any aRet;
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        aRet = SwUnoCursorHelper::GetPropertyDefault(
                *pShellCrsr, *m_pPropSet, rPropertyName);
    }
    return aRet;
}

sal_Bool SwXTextViewCursor::goDown(sal_Int16 nCount, sal_Bool bExpand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        for( sal_uInt16 i = 0; i < nCount; i++ )
            bRet = m_pView->GetWrtShell().Down( bExpand, 1, sal_True );
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::goUp(sal_Int16 nCount, sal_Bool bExpand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        for( sal_uInt16 i = 0; i < nCount; i++ )
            bRet = m_pView->GetWrtShell().Up( bExpand, 1, sal_True );
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::isAtStartOfLine(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
    {
        if (!IsTextSelection( sal_False ))
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        bRet = m_pView->GetWrtShell().IsAtLeftMargin();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::isAtEndOfLine(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(m_pView)
    {
        if (!IsTextSelection( sal_False ))
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        bRet = m_pView->GetWrtShell().IsAtRightMargin(sal_True);
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

void SwXTextViewCursor::gotoEndOfLine(sal_Bool bExpand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if (!IsTextSelection( sal_False ))
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        m_pView->GetWrtShell().RightMargin(bExpand, sal_True);
    }
    else
        throw uno::RuntimeException();
}

void SwXTextViewCursor::gotoStartOfLine(sal_Bool bExpand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if (!IsTextSelection( sal_False ))
            throw  uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), static_cast < cppu::OWeakObject * > ( this ) );

        m_pView->GetWrtShell().LeftMargin(bExpand, sal_True);
    }
    else
        throw uno::RuntimeException();
}

OUString SwXTextViewCursor::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextViewCursor");
}

sal_Bool SwXTextViewCursor::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii("com.sun.star.text.TextViewCursor") ||
            !rServiceName.compareToAscii("com.sun.star.style.CharacterProperties") ||
            !rServiceName.compareToAscii("com.sun.star.style.CharacterPropertiesAsian") ||
            !rServiceName.compareToAscii("com.sun.star.style.CharacterPropertiesComplex") ||
            !rServiceName.compareToAscii("com.sun.star.style.ParagraphProperties") ||
            !rServiceName.compareToAscii("com.sun.star.style.ParagraphPropertiesAsian") ||
            !rServiceName.compareToAscii("com.sun.star.style.ParagraphPropertiesComplex");
}

Sequence< OUString > SwXTextViewCursor::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(7);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextViewCursor");
    pArray[1] = C2U("com.sun.star.style.CharacterProperties");
    pArray[2] = C2U("com.sun.star.style.CharacterPropertiesAsian");
    pArray[3] = C2U("com.sun.star.style.CharacterPropertiesComplex");
    pArray[4] = C2U("com.sun.star.style.ParagraphProperties");
    pArray[5] = C2U("com.sun.star.style.ParagraphPropertiesAsian");
    pArray[6] = C2U("com.sun.star.style.ParagraphPropertiesComplex");
    return aRet;
}

namespace
{
    class theSwXTextViewCursorUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextViewCursorUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXTextViewCursor::getUnoTunnelId()
{
    return theSwXTextViewCursorUnoTunnelId::get().getSeq();
}

//XUnoTunnel
sal_Int64 SAL_CALL SwXTextViewCursor::getSomething(
    const uno::Sequence< sal_Int8 >& rId )
        throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
        {
                return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ));
        }
    return OTextCursorHelper::getSomething(rId);
}

IMPLEMENT_FORWARD_XINTERFACE2(SwXTextViewCursor,SwXTextViewCursor_Base,OTextCursorHelper)
const SwDoc*        SwXTextViewCursor::GetDoc() const
{
    SwWrtShell& rSh = m_pView->GetWrtShell();
    return   rSh.GetCrsr() ? rSh.GetCrsr()->GetDoc() : 0;
}

SwDoc*  SwXTextViewCursor::GetDoc()
{
    SwWrtShell& rSh = m_pView->GetWrtShell();
    return   rSh.GetCrsr() ? rSh.GetCrsr()->GetDoc() : 0;
}

const SwPaM*    SwXTextViewCursor::GetPaM() const
{
    SwWrtShell& rSh = m_pView->GetWrtShell();
    return rSh.GetCrsr();
}

SwPaM*  SwXTextViewCursor::GetPaM()
{
    SwWrtShell& rSh = m_pView->GetWrtShell();
    return rSh.GetCrsr();
}

uno::Reference< datatransfer::XTransferable > SAL_CALL SwXTextView::getTransferable(  ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //force immediat shell update
    GetView()->StopShellTimer();
    SwWrtShell& rSh = GetView()->GetWrtShell();
    if ( GetView()->GetShellMode() == SHELL_MODE_DRAWTEXT )
    {
        SdrView *pSdrView = rSh.GetDrawView();
        OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
        return pOLV->GetEditView().GetTransferable();
    }
    else
    {
        SwTransferable* pTransfer = new SwTransferable( rSh );
        const sal_Bool bLockedView = rSh.IsViewLocked();
        rSh.LockView( sal_True );    //lock visible section
        pTransfer->PrepareForCopy();
        rSh.LockView( bLockedView );
        return uno::Reference< datatransfer::XTransferable >( pTransfer );
    }
}

void SAL_CALL SwXTextView::insertTransferable( const uno::Reference< datatransfer::XTransferable >& xTrans ) throw (datatransfer::UnsupportedFlavorException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //force immediat shell update
    GetView()->StopShellTimer();
    SwWrtShell& rSh = GetView()->GetWrtShell();
    if ( GetView()->GetShellMode() == SHELL_MODE_DRAWTEXT )
    {
        SdrView *pSdrView = rSh.GetDrawView();
        OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
        pOLV->GetEditView().InsertText( xTrans, GetView()->GetDocShell()->GetMedium()->GetBaseURL(), sal_False );
    }
    else
    {
        TransferableDataHelper aDataHelper( xTrans );
        if ( SwTransferable::IsPaste( rSh, aDataHelper ) )
        {
            SwTransferable::Paste( rSh, aDataHelper );
            if( rSh.IsFrmSelected() || rSh.IsObjSelected() )
                rSh.EnterSelFrmMode();
            GetView()->AttrChangedNotify( &rSh );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
