/*************************************************************************
 *
 *  $RCSfile: unotxvw.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: os $ $Date: 2001-03-09 14:58:16 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "viscrs.hxx"
#include <sfx2/frame.hxx>
#include <cmdid.h>
#include <hintids.hxx>
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _RUBYLIST_HXX
#include <rubylist.hxx>
#endif
#ifndef _SWDOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _UNOTXVW_HXX
#include <unotxvw.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _UNOSTYLE_HXX
#include <unostyle.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _SWVIEW_HXX
#include <view.hxx>
#endif
#ifndef _UNOMOD_HXX
#include <unomod.hxx>
#endif
#ifndef _UNOFRAME_HXX
#include <unoframe.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _WRTSH_HXX //autogen
#include <wrtsh.hxx>
#endif
#ifndef _UNOTBL_HXX
#include <unotbl.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDOUNO_HXX //autogen
#include <svx/svdouno.hxx>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _BOOKMRK_HXX //autogen
#include <bookmrk.hxx>
#endif
#ifndef _UNOTXDOC_HXX //autogen
#include <unotxdoc.hxx>
#endif
#ifndef _UNODRAW_HXX //autogen
#include <unodraw.hxx>
#endif
#ifndef _SVX_UNOSHGRP_HXX //autogen
#include <svx/unoshcol.hxx>
#endif
#ifndef _SVX_UNOSHAPE_HXX //autogen wg. SvxShape
#include <svx/unoshape.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen wg. SdrPageView
#include <svx/svdpagv.hxx>
#endif
#include <swerror.h>
#ifndef _CACHESTR_HXX
#include <tools/cachestr.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FMTRUBY_HXX
#include <fmtruby.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::view;
using namespace rtl;

SV_IMPL_PTRARR( SelectionChangeListenerArr, XSelectionChangeListenerPtr );

/* -----------------22.05.98 12:20-------------------
 *
 * --------------------------------------------------*/
SwPaM* lcl_createPamCopy(const SwPaM& rPam)
{
    SwPaM* pRet = new SwPaM(*rPam.GetPoint());
    if(rPam.HasMark())
    {
        pRet->SetMark();
        *pRet->GetMark() = *rPam.GetMark();
    }
    if(rPam.GetNext() != (const Ring*)&rPam)
    {
        SwPaM *_pStartCrsr = (SwPaM *)rPam.GetNext();
        do
        {
            //neuen PaM erzeugen
            SwPaM* pPaM = new SwPaM(*_pStartCrsr);
            //und in den Ring einfuegen
            pPaM->MoveTo(pRet);

        } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != rPam.GetNext() );
    }
    return pRet;
}
/* -----------------04.11.98 15:32-------------------
 *
 * --------------------------------------------------*/
sal_Bool lcl_FindObjInGroup(uno::Reference< awt::XControl > & xRet, SdrObjGroup* pGroup, const Reference< awt::XControlModel > & xModel, Window* pWin)
{
    SdrObjList* pList = pGroup->GetSubList();
    sal_uInt32 nCount = pList->GetObjCount();
    for( sal_uInt32 i=0; i< nCount; i++ )
    {
        SdrObject* pObj = pList->GetObj(i);
        SdrUnoObj *pFormObj = PTR_CAST( SdrUnoObj, pObj );
        SdrObjGroup* pGroup;
        if( pFormObj )
        {
            Reference< awt::XControlModel >  xCM = pFormObj->GetUnoControlModel();
            if( xCM.is() && xModel == xCM )
            {
                xRet = pFormObj->GetUnoControl( pWin );
                break;
            }
        }
        else if(0 != (pGroup = PTR_CAST( SdrObjGroup, pObj )))
        {
            if(lcl_FindObjInGroup(xRet, pGroup, xModel, pWin))
                break;
        }
    }
    return xRet.is();
}

/******************************************************************
 * SwXTextView
 ******************************************************************/
/*-- 17.12.98 09:34:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextView::SwXTextView(SwView* pSwView) :
    SfxBaseController(pSwView),
    pView(pSwView),
    pxViewSettings(0),
    pxTextViewCursor(0)
{

}
/*-- 17.12.98 09:34:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextView::~SwXTextView()
{
    Invalidate();
}
/* -----------------------------09.03.01 15:47--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextView::Invalidate()
{
    if(pxViewSettings)
    {
         beans::XPropertySet* pSettings = pxViewSettings->get();
        ((SwXViewSettings*)pSettings)->Invalidate();
        DELETEZ(pxViewSettings);
    }
    if(pxTextViewCursor)
    {
        text::XTextViewCursor* pCrsr = pxTextViewCursor->get();
        ((SwXTextViewCursor*)pCrsr)->Invalidate();
        DELETEZ(pxTextViewCursor);
    }
    sal_uInt16 nCount = aSelChangedListeners.Count();
    m_refCount++; //prevent second d'tor call
    if(nCount)
    {
        Reference< uno::XInterface >  xInt = (cppu::OWeakObject*)(SfxBaseController*)this;
        lang::EventObject aEvent(xInt);
        for ( sal_uInt16 i = nCount; i--; )
        {
            Reference< view::XSelectionChangeListener >  *pObj = aSelChangedListeners[i];
            (*pObj)->disposing(aEvent);
        }
    }
    m_refCount--;
    pView = 0;
}

/* -----------------------------18.05.00 10:18--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< uno::Type > SAL_CALL SwXTextView::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
//  uno::Sequence< uno::Type > aViewTypes = SwXTextViewBaseClass::getTypes();
    uno::Sequence< uno::Type > aBaseTypes = SfxBaseController::getTypes();

    long nIndex = aBaseTypes.getLength();
    aBaseTypes.realloc(
        aBaseTypes.getLength() + 6 );

    uno::Type* pBaseTypes = aBaseTypes.getArray();
    pBaseTypes[nIndex++] = ::getCppuType((Reference<XSelectionSupplier  >*)0);
    pBaseTypes[nIndex++] = ::getCppuType((Reference<XServiceInfo            >*)0);
    pBaseTypes[nIndex++] = ::getCppuType((Reference<XControlAccess      >*)0);
    pBaseTypes[nIndex++] = ::getCppuType((Reference<XTextViewCursorSupplier>*)0);
    pBaseTypes[nIndex++] = ::getCppuType((Reference<XViewSettingsSupplier   >*)0);
    pBaseTypes[nIndex++] = ::getCppuType((Reference<XRubySelection  >*)0);
    return aBaseTypes;
}
/* -----------------------------18.05.00 10:18--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< sal_Int8 > SAL_CALL SwXTextView::getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId( 16 );
    static BOOL bInit = FALSE;
    if(!bInit)
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    return aId;
}
/* -----------------------------18.05.00 10:18--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextView::acquire(  )throw()
{
    SfxBaseController::acquire();
}
/* -----------------------------18.05.00 10:18--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextView::release(  )throw()
{
    SfxBaseController::release();
}
/* -----------------------------18.05.00 10:23--------------------------------

 ---------------------------------------------------------------------------*/
uno::Any SAL_CALL SwXTextView::queryInterface( const uno::Type& aType )
    throw (RuntimeException)
{
    uno::Any aRet;
    if(aType == ::getCppuType((Reference<view::XSelectionSupplier   >*)0))
    {
        Reference<view::XSelectionSupplier> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == ::getCppuType((Reference<lang::XServiceInfo            >*)0))
    {
        Reference<lang::XServiceInfo> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == ::getCppuType((Reference<view::XControlAccess      >*)0))
    {
        Reference<view::XControlAccess> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == ::getCppuType((Reference<text::XTextViewCursorSupplier>*)0))
    {
        Reference<text::XTextViewCursorSupplier> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == ::getCppuType((Reference<view::XViewSettingsSupplier   >*)0))
    {
        Reference<view::XViewSettingsSupplier> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == ::getCppuType((Reference<XRubySelection>*)0))
    {
        Reference<XRubySelection> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else
        aRet = SfxBaseController::queryInterface(aType);
    return aRet;
}
/*-- 17.12.98 09:34:26---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextView::select(const uno::Any& aInterface) throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if(GetView() && aInterface.getValueType().getTypeClass() == uno::TypeClass_INTERFACE)
    {
        Reference< uno::XInterface >  xInterface = *(Reference< uno::XInterface > *)aInterface.getValue();
        SwWrtShell& rSh = GetView()->GetWrtShell();
        SwDoc* pDoc = GetView()->GetDocShell()->GetDoc();
        Reference< lang::XUnoTunnel >  xIfcTunnel(xInterface, uno::UNO_QUERY);
        Reference< text::XTextCursor >  xCrsr(xInterface, uno::UNO_QUERY);
        Reference< container::XIndexAccess >    xPosN(xInterface, uno::UNO_QUERY);
        Reference< text::XTextRange >   xPos(xInterface, uno::UNO_QUERY);

        SwPaM * pPam = 0;
        SwXTextRanges* pPosN = 0;
        if(xCrsr.is())
        {
            //
            SwXTextCursor* pCursor =
                xIfcTunnel.is() ?
                    (SwXTextCursor*)xIfcTunnel->getSomething(SwXTextCursor::getUnoTunnelId())
                    : 0;

            if(pCursor && pCursor->GetDoc() == GetView()->GetDocShell()->GetDoc())
                pPam = lcl_createPamCopy(*((SwXTextCursor*)pCursor)->GetPaM());

        }
        else if(xPosN.is() &&
            xIfcTunnel.is() &&
                0 != (pPosN = (SwXTextRanges*)xIfcTunnel->getSomething(SwXTextRanges::getUnoTunnelId())))
        {
            const SwUnoCrsr* pUnoCrsr = pPosN->GetCursor();
            if(pUnoCrsr)
            {
                pPam = lcl_createPamCopy(*pUnoCrsr);
            }
        }
        else if(xPos.is())
        {
            SwUnoInternalPaM aPam(*pDoc);
            if(SwXTextRange::XTextRangeToSwPaM(aPam, xPos))
            {
                pPam = lcl_createPamCopy(aPam);
            }
        }
        if(pPam)
        {
            rSh.EnterStdMode();
            rSh.SetSelection(*pPam);
            delete pPam;
            return sal_True;
        }
        Reference< uno::XInterface >  xFrm(xInterface, uno::UNO_QUERY);;

        if(xFrm.is() && xIfcTunnel.is())
        {
            //
            SwXFrame* pFrame = (SwXFrame*)
                xIfcTunnel->getSomething(SwXFrame::getUnoTunnelId());

            SwFrmFmt* pFrmFmt = pFrame ? pFrame->GetFrmFmt() : 0;
            if(pFrmFmt && pFrmFmt->GetDoc() == pDoc)
            {
                sal_Bool bSuccess = rSh.GotoFly( pFrmFmt->GetName(), pFrame->GetFlyCntType());
                if (bSuccess)
                {
                    rSh.HideCrsr();
                    rSh.EnterSelFrmMode();
                }
            }
            return sal_True;
        }

        Reference< text::XTextTable >  xTbl(xInterface, uno::UNO_QUERY);;

        if(xTbl.is() && xIfcTunnel.is())
        {
            SwXTextTable* pTable = (SwXTextTable*)
                xIfcTunnel->getSomething(SwXTextTable::getUnoTunnelId());

            SwFrmFmt* pTblFrmFmt = pTable ? ((SwXTextTable*)pTable)->GetFrmFmt() : 0;
            if(pTblFrmFmt &&pTblFrmFmt->GetDoc() == pDoc)
            {
                rSh.EnterStdMode();
                rSh.GotoTable(pTblFrmFmt->GetName());
            }
            return sal_True;
        }

        Reference< text::XTextContent >  xBkm(xInterface, uno::UNO_QUERY);;

        if(xBkm.is() && xIfcTunnel.is())
        {
            SwXBookmark* pBkm = (SwXBookmark*)
                    xIfcTunnel->getSomething(SwXBookmark::getUnoTunnelId());
            if(pBkm && pBkm->GetDoc() == pDoc)
            {

                sal_uInt16 nFndPos = rSh.FindBookmark(pBkm->getName());

                if( USHRT_MAX != nFndPos )
                {
                    rSh.EnterStdMode();
                    rSh.GotoBookmark( nFndPos );
                }
                return sal_True;
            }
        }
        // IndexMark, sdbcx::Index, TextField, Draw, Section, text::Footnote, text::Paragraph
        //

        Reference< drawing::XShapes >  xShapeColl( xInterface, uno::UNO_QUERY );
        Reference< beans::XPropertySet >  xTmpProp(xInterface, uno::UNO_QUERY);
        SwXShape* pSwXShape = 0;
        if(xIfcTunnel.is())
            pSwXShape = (SwXShape*)xIfcTunnel->getSomething(SwXShape::getUnoTunnelId());
        SvxShape* pSvxShape = 0;
        if(pSwXShape)
        {
            Reference< uno::XAggregation >  xAgg = pSwXShape->GetAggregationInterface();
            if(xAgg.is())
            {
                pSvxShape = (SvxShape*)xIfcTunnel->getSomething(SvxShape::getUnoTunnelId());
            }
        }

        if ( pSvxShape || xShapeColl.is() )         // Drawing drawing::Layer
        {
            SdrView* pDrawView = rSh.GetDrawView();
            if (pDrawView)
            {
                pDrawView->EndTextEdit();
                pDrawView->UnmarkAll();

                if (pSvxShape)      // einzelnes Shape
                {
                    SdrObject *pObj = pSvxShape->GetSdrObject();
                    if (pObj)
                    {
//                      lcl_ShowObject( *pViewSh, *pDrawView, pObj );
                        SdrPageView* pPV = pDrawView->GetPageViewPvNum(0);
                        if ( pPV && pObj->GetPage() == pPV->GetPage() )
                        {
                            pDrawView->MarkObj( pObj, pPV );
                        }
                    }
                }
                else    // Shape Collection
                {
                    SdrPageView* pPV = NULL;
                    long nCount = xShapeColl->getCount();
                    for ( long i = 0; i < nCount; i++ )
                    {
                        Reference< drawing::XShape >  xShapeInt;
                        uno::Any aAny = xShapeColl->getByIndex(i);
                        if ( aAny.getValueType() == ::getCppuType((Reference<drawing::XShape>*)0))
                            xShapeInt = *((Reference< drawing::XShape > *) aAny.getValue());
                        if (xShapeInt.is())
                        {
                            Reference< lang::XUnoTunnel> xShapeTunnel(xShapeInt, uno::UNO_QUERY);

                            SvxShape* pShape = xShapeTunnel.is() ?
                                (SvxShape*)xShapeTunnel->getSomething(SvxShape::getUnoTunnelId()) : 0;

                            if (pShape)
                            {
                                SdrObject *pObj = pShape->GetSdrObject();
                                if (pObj)
                                {
                                    if (!pPV)               // erstes Objekt
                                    {
//                                      lcl_ShowObject( *pViewSh, *pDrawView, pObj );
                                        pPV = pDrawView->GetPageViewPvNum(0);
                                    }
                                    if ( pPV && pObj->GetPage() == pPV->GetPage() )
                                    {
                                        pDrawView->MarkObj( pObj, pPV );
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return sal_False;

}
/*-- 17.12.98 09:34:26---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextView::getSelection(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    Reference< uno::XInterface >  aRef;
    SwView* pView = ((SwXTextView*)this)->GetView();
    if(pView)
    {
        // ein interface aus der aktuellen Selektion erzeugen
        SwWrtShell& rSh = pView->GetWrtShell();
        ShellModes  eSelMode = pView->GetShellMode();
        switch(eSelMode)
        {
            case SEL_TABLE_TEXT      :
            {
                if(rSh.GetTableCrsr())
                {
                    DBG_ASSERT(rSh.GetTableFmt(), "kein Tabellenformat?");
                    Reference< text::XTextTableCursor >  xCrsr = new SwXTextTableCursor(*rSh.GetTableFmt(),
                                                    rSh.GetTableCrsr());
                    aRef = Reference< uno::XInterface >  (xCrsr, uno::UNO_QUERY);;
                    break;
                }

            }
            // ohne Tabellenselektion wird der Text geliefert
            //break;
            case SEL_LIST_TEXT       :
            case SEL_TABLE_LIST_TEXT:
            case SEL_TEXT            :
            {
                Reference< container::XIndexAccess >  xPos = new SwXTextRanges(rSh.GetCrsr());
                aRef = Reference< uno::XInterface >(xPos, uno::UNO_QUERY);
            }
            break;
            case SEL_FRAME           :
            case SEL_GRAPHIC         :
            case SEL_OBJECT          :
            {
                //Get FlyFrameFormat; fuer UI Macro Anbindung an Flys
                const SwFrmFmt* pFmt = rSh.GetFlyFrmFmt();
                SwXFrame* pxFrame = (SwXFrame*)SwClientIter((SwFrmFmt&)*pFmt).
                                                First(TYPE(SwXFrame));

                if(pxFrame)                //das einzige gemeinsame interface fuer alle Frames
                {
                    aRef = Reference< uno::XInterface >((cppu::OWeakObject*)pxFrame, uno::UNO_QUERY);
                }
                else
                {
                    if(SEL_FRAME == eSelMode)
                    {
                        Reference< text::XTextFrame >  xFrm =  new SwXTextFrame((SwFrmFmt&)*pFmt);
                        aRef = Reference< uno::XInterface >(xFrm, uno::UNO_QUERY);
                    }
                    else if(SEL_GRAPHIC == eSelMode)
                    {
                        Reference< text::XTextContent >  xFrm = new SwXTextGraphicObject((SwFrmFmt&)*pFmt);
                        aRef = xFrm;
                    }
                    else
                    {
                        Reference< text::XTextContent >  xFrm =  new SwXTextEmbeddedObject((SwFrmFmt&)*pFmt);
                        aRef = xFrm;
                    }
                }
            }
            break;
            case SEL_DRAW            :
            case SEL_DRAW_CTRL       :
            case SEL_DRAW_FORM       :
            case SEL_DRAWTEXT        :
            case SEL_BEZIER          :
            {
                Reference< drawing::XDrawPageSupplier >  xPageSupp;
                Reference< frame::XModel > xModel = pView->GetDocShell()->GetBaseModel();
                Reference< lang::XUnoTunnel > xModelTunnel(xModel, uno::UNO_QUERY);
                SwXTextDocument* pTextDoc = (SwXTextDocument*)xModelTunnel->
                                getSomething(SwXTextDocument::getUnoTunnelId());

                SwFmDrawPage* pSvxDrawPage =    pTextDoc->GetDrawPage()->GetSvxPage();
                Reference< drawing::XShapes >  xShCol = new SvxShapeCollection();

                const SdrMarkList& rMarkList = rSh.GetDrawView()->GetMarkList();
                for(sal_uInt16 i = 0; i < rMarkList.GetMarkCount(); i++)
                {
                    SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
                    Reference< uno::XInterface >  xInt = pSvxDrawPage->GetInterface( pObj );
                    Reference< drawing::XShape >  xShape(xInt, uno::UNO_QUERY);;
                    xShCol->add(xShape);
                }
                aRef = Reference< uno::XInterface >(xShCol, uno::UNO_QUERY);
            }
            break;
        }
    }
    uno::Any aRet(&aRef, ::getCppuType((Reference<uno::XInterface>*)0));
    return aRet;
}
/*-- 17.12.98 09:34:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextView::addSelectionChangeListener(
                                    const Reference< view::XSelectionChangeListener > & rxListener)
                                    throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    DBG_WARNING("not implemented")

    Reference< view::XSelectionChangeListener > * pInsert = new Reference< view::XSelectionChangeListener > ;
    *pInsert = rxListener;
    aSelChangedListeners.Insert(pInsert, aSelChangedListeners.Count());
}
/*-- 17.12.98 09:34:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextView::removeSelectionChangeListener(
                                        const Reference< view::XSelectionChangeListener > & rxListener)
                                        throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRemoved = sal_False;
    view::XSelectionChangeListener* pLeft = rxListener.get();
    for(sal_uInt16 i = 0; i < aSelChangedListeners.Count(); i++)
    {
        Reference< view::XSelectionChangeListener > * pElem = aSelChangedListeners.GetObject(i);
         view::XSelectionChangeListener* pRight = pElem->get();
        if(pLeft == pRight)
        {
            aSelChangedListeners.Remove(i);
            bRemoved = sal_True;
            delete pElem;
        }
    }
    if(!bRemoved)
        throw uno::RuntimeException();
}
/*-- 17.12.98 09:34:27---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< awt::XControl >  SwXTextView::getControl(const Reference< awt::XControlModel > & xModel)
        throw( container::NoSuchElementException, uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    Reference< awt::XControl >  xRet;

    ViewShell *pVSh = 0;
    SwView* pView = ((SwXTextView*)this)->GetView();
    if(pView)
    {
        SdrModel* pModel = pView->GetDocShell()->GetDoc()->GetDrawModel();
        if(pModel)
        {
            SdrPage* pPage = pModel->GetPage( 0 );
            Window *pWin = pView->GetWrtShell().GetWin();
            sal_uInt32 nCount = pPage->GetObjCount();
            for( sal_uInt32 i=0; i< nCount; i++ )
            {
                SdrObject* pObj = pPage->GetObj(i);
                SdrUnoObj *pFormObj = PTR_CAST( SdrUnoObj, pObj );
                SdrObjGroup* pGroup;
                if( pFormObj )
                {
                    Reference< awt::XControlModel >  xCM = pFormObj->GetUnoControlModel();
                    if( xCM.is() && xModel == xCM )
                    {
                        xRet = pFormObj->GetUnoControl( pWin );
                        break;
                    }
                }
                else if(0 != (pGroup = PTR_CAST( SdrObjGroup, pObj )))
                {
                    if(lcl_FindObjInGroup(xRet, pGroup, xModel, pWin))
                        break;
                }
            }
        }
    }
    return xRet;
}
/*-- 17.12.98 09:34:28---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< text::XTextViewCursor >  SwXTextView::getViewCursor(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    SwView* pView = ((SwXTextView*)this)->GetView();
    if(pView)
    {
        if(!pxTextViewCursor)
        {
            ((SwXTextView*)this)->pxTextViewCursor = new Reference< text::XTextViewCursor > ;
            *pxTextViewCursor = new  SwXTextViewCursor(pView);
        }
        return *pxTextViewCursor;
    }
    else
        throw uno::RuntimeException();
    return Reference< text::XTextViewCursor > ();
}
/*-- 17.12.98 09:34:28---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< beans::XPropertySet >  SwXTextView::getViewSettings(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if(pView)
    {
        if(!pxViewSettings)
        {
            ((SwXTextView*)this)->pxViewSettings = new Reference< beans::XPropertySet > ;
            *pxViewSettings = new SwXViewSettings(sal_False, pView);
        }
    }
    else
        throw uno::RuntimeException();
    return *pxViewSettings;
}
/* -----------------------------30.01.01 15:01--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< Sequence< PropertyValue > > SwXTextView::getRubyList( sal_Bool bAutomatic ) throw(RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());

    SwView* pView = GetView();
    if(!pView)
        throw RuntimeException();
    SwWrtShell& rSh = pView->GetWrtShell();
    ShellModes  eSelMode = pView->GetShellMode();
    if(eSelMode != SEL_LIST_TEXT      &&
        eSelMode != SEL_TABLE_LIST_TEXT &&
        eSelMode != SEL_TEXT           )
        return Sequence< Sequence< PropertyValue > > ();

    SwDoc* pDoc = pView->GetDocShell()->GetDoc();
    SwRubyList aList;

    USHORT nCount = pDoc->FillRubyList( *rSh.GetCrsr(), aList, 0 );
    Sequence< Sequence< PropertyValue > > aRet(nCount);
    Sequence< PropertyValue >* pRet = aRet.getArray();
    for(USHORT n = 0; n < nCount; n++)
    {
        const SwRubyListEntryPtr pEntry = aList[n];

        const String& rEntryText = pEntry->GetText();
        const SwFmtRuby& rAttr = pEntry->GetRubyAttr();

        pRet[n].realloc(4);
        PropertyValue* pValues = pRet[n].getArray();
        pValues[0].Name = C2U(UNO_NAME_RUBY_BASE_TEXT);
        pValues[0].Value <<= OUString(rEntryText);
        pValues[1].Name = C2U(UNO_NAME_RUBY_TEXT);
        pValues[1].Value <<= OUString(rAttr.GetText());
        pValues[2].Name = C2U(UNO_NAME_RUBY_CHAR_STYLE_NAME);
        pValues[2].Value <<= OUString(
                SwXStyleFamilies::GetProgrammaticName(rAttr.GetCharFmtName(),SFX_STYLE_FAMILY_CHAR));
        pValues[3].Name = C2U(UNO_NAME_RUBY_ADJUST);
        pValues[3].Value <<= (sal_Int16)rAttr.GetAdjustment();
    }
    return aRet;
}
/* -----------------------------30.01.01 15:02--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextView::setRubyList(
    const Sequence< Sequence< PropertyValue > >& rRubyList, sal_Bool bAutomatic )
        throw(RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());

    SwView* pView = GetView();
    if(!pView || !rRubyList.getLength())
        throw RuntimeException();
    SwWrtShell& rSh = pView->GetWrtShell();
    ShellModes  eSelMode = pView->GetShellMode();
    if(eSelMode != SEL_LIST_TEXT      &&
        eSelMode != SEL_TABLE_LIST_TEXT &&
        eSelMode != SEL_TEXT           )
        throw RuntimeException();

    SwRubyList aList;

    const Sequence<PropertyValue>* pRubyList = rRubyList.getConstArray();
    for(sal_Int32 nPos = 0; nPos < rRubyList.getLength(); nPos++)
    {
        SwRubyListEntryPtr pEntry = new SwRubyListEntry;
        const PropertyValue* pProperties = pRubyList[nPos].getConstArray();
        OUString sTmp;
        for(sal_Int32 nProp = 0; nProp < pRubyList[nPos].getLength(); nProp++)
        {
            if(!pProperties[nProp].Name.compareToAscii(UNO_NAME_RUBY_BASE_TEXT.pName))
            {
                pProperties[nProp].Value >>= sTmp;
                pEntry->SetText(sTmp);
            }
            else if(!pProperties[nProp].Name.compareToAscii(UNO_NAME_RUBY_TEXT.pName))
            {
                pProperties[nProp].Value >>= sTmp;
                pEntry->GetRubyAttr().SetText(sTmp);
            }
            else if(!pProperties[nProp].Name.compareToAscii(UNO_NAME_RUBY_CHAR_STYLE_NAME.pName))
            {
                pProperties[nProp].Value >>= sTmp;
                String sName(SwXStyleFamilies::GetUIName(sTmp, SFX_STYLE_FAMILY_CHAR));
                sal_uInt16 nPoolId = sName.Len() ?
                    SwDoc::GetPoolId( sName, GET_POOLID_CHRFMT ) : -1;

                pEntry->GetRubyAttr().SetCharFmtName( sName );
                pEntry->GetRubyAttr().SetCharFmtId( nPoolId );
            }
            else if(!pProperties[nProp].Name.compareToAscii(UNO_NAME_RUBY_ADJUST.pName))
            {
                sal_Int16 nTmp;
                pProperties[nProp].Value >>= nTmp;
                pEntry->GetRubyAttr().SetAdjustment(nTmp);
            }
        }
        aList.Insert(pEntry, (USHORT)nPos);
    }
    SwDoc* pDoc = pView->GetDocShell()->GetDoc();
    pDoc->SetRubyList( *rSh.GetCrsr(), aList, 0 );
}
/*-- 17.12.98 09:34:29---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextView::NotifySelChanged()
{
    Reference< uno::XInterface >  xInt = (cppu::OWeakObject*)(SfxBaseController*)this;

     lang::EventObject aEvent(xInt);

    sal_uInt16 nCount = aSelChangedListeners.Count();
    for ( sal_uInt16 i = nCount; i--; )
    {
        Reference< view::XSelectionChangeListener >  *pObj = aSelChangedListeners[i];
        (*pObj)->selectionChanged(aEvent);
    }
}
/* -----------------------------06.04.00 11:07--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTextView::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextView");
}
/* -----------------------------06.04.00 11:07--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTextView::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextView") == rServiceName;
}
/* -----------------------------06.04.00 11:07--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXTextView::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextView");
    return aRet;
}

/******************************************************************
 * SwXTextViewCursor
 ******************************************************************/
/*-- 17.12.98 09:36:23---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextViewCursor::SwXTextViewCursor(SwView* pVw) :
    pView(pVw),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR))
{

}
/*-- 17.12.98 09:36:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextViewCursor::~SwXTextViewCursor()
{
}
/*-- 17.12.98 09:36:25---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::isVisible(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    DBG_WARNING("not implemented")
    return sal_True;
}
/*-- 17.12.98 09:36:25---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextViewCursor::setVisible(sal_Bool bVisible) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    DBG_WARNING("not implemented")
}
/*-- 17.12.98 09:36:26---------------------------------------------------

  -----------------------------------------------------------------------*/
awt::Point SwXTextViewCursor::getPosition(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    awt::Point aRet;
    if(pView)
    {
        const SwWrtShell& rSh = pView->GetWrtShell();
        const SvxULSpaceItem& rUL = rSh.GetPageDesc( rSh.GetCurPageDesc()
                                                    ).GetMaster().GetULSpace();
        SwRect aCharRect(rSh.GetCharRect());
        long nY = aCharRect.Top() -
                                (rUL.GetUpper() + DOCUMENTBORDER);
        aRet.Y = TWIP_TO_MM100(nY);
        const SvxLRSpaceItem& rLR = rSh.GetPageDesc( rSh.GetCurPageDesc()
                                                    ).GetMaster().GetLRSpace();
        long nX = aCharRect.Left() -
                                (rLR.GetLeft() + DOCUMENTBORDER);
        aRet.X = TWIP_TO_MM100(nX);
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 17.12.98 09:36:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextViewCursor::collapseToStart(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if(pView)
    {
        SwWrtShell& rSh = pView->GetWrtShell();
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
/*-- 17.12.98 09:36:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextViewCursor::collapseToEnd(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if(pView)
    {
        SwWrtShell& rSh = pView->GetWrtShell();
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
/*-- 17.12.98 09:36:27---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::isCollapsed(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
    {
        const SwWrtShell& rSh = pView->GetWrtShell();
        bRet = rSh.HasSelection();
    }
    else
        throw uno::RuntimeException();
    return bRet;

}
/*-- 17.12.98 09:36:27---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::goLeft(sal_Int16 nCount, sal_Bool bExpand) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
    {
        for( sal_uInt16 i = 0; i < nCount; i++ )
            bRet = pView->GetWrtShell().Left( bExpand, 1, sal_True );
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 17.12.98 09:36:27---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::goRight(sal_Int16 nCount, sal_Bool bExpand) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
    {
        for( sal_uInt16 i = 0; i < nCount; i++ )
            bRet = pView->GetWrtShell().Right( bExpand, 1, sal_True );
    }
    else
        throw uno::RuntimeException();
    return bRet;

}
/* -----------------08.03.99 11:18-------------------
 *
 * --------------------------------------------------*/
void SwXTextViewCursor::gotoRange(
    const Reference< text::XTextRange > & xRange,
    sal_Bool bExpand)
        throw(RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if(pView && xRange.is())
    {
        ShellModes  eSelMode = pView->GetShellMode();
        // ein interface aus der aktuellen Selektion erzeugen
        SwWrtShell& rSh = pView->GetWrtShell();
        rSh.EnterStdMode();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        SwPaM aOwnPaM(*pShellCrsr->GetPoint());
        if(pShellCrsr->HasMark())
        {
            aOwnPaM.SetMark();
            *aOwnPaM.GetMark() = *pShellCrsr->GetMark();
        }

        Reference<lang::XUnoTunnel> xRangeTunnel( xRange, uno::UNO_QUERY);
        SwXTextRange* pRange = 0;
        SwXTextCursor* pCursor = 0;
        if(xRangeTunnel.is())
        {
            pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                    SwXTextRange::getUnoTunnelId());
            pCursor = (SwXTextCursor*)xRangeTunnel->getSomething(
                                    SwXTextCursor::getUnoTunnelId());
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
        if(pCursor && pCursor->GetCrsr())
        {
            pSrcNode = pCursor->GetCrsr()->GetNode();
        }
        else if(pRange && pRange->GetBookmark())
        {
            SwBookmark* pBkm = pRange->GetBookmark();
            pSrcNode = &pBkm->GetPos().nNode.GetNode();
        }
        else
            throw RuntimeException();
        const SwStartNode* pTmp = pSrcNode ? pSrcNode->FindSttNodeByType(eSearchNodeType) : 0;

        //SectionNodes ueberspringen
        while(pTmp && pTmp->IsSectionNode())
        {
            pTmp = pTmp->FindStartNode();
        }
        //ohne Expand darf mit dem ViewCursor ueberall hingesprungen werden
        //mit Expand nur in der gleichen Umgebung
        if(bExpand &&
            (pOwnStartNode != pTmp ||
            (eSelMode != SEL_LIST_TEXT &&
                eSelMode != SEL_TABLE_LIST_TEXT &&
                    eSelMode != SEL_TEXT)))
            throw uno::RuntimeException();

        //jetzt muss die Selektion erweitert werden
        if(bExpand)
        {
            // der Cursor soll alles einschliessen, was bisher von ihm und dem uebergebenen
            // Range eingeschlossen wurde
            SwPosition aOwnLeft(*aOwnPaM.GetPoint());
            SwPosition aOwnRight(aOwnPaM.HasMark() ? *aOwnPaM.GetMark() : aOwnLeft);
            if(aOwnRight < aOwnLeft)
            {
                SwPosition aTmp = aOwnLeft;
                aOwnLeft = aOwnRight;
                aOwnRight = aTmp;
            }
            SwPosition* pParamLeft;
            SwPosition* pParamRight;
            if(pCursor)
            {
                const SwUnoCrsr* pTmp = pCursor->GetCrsr();
                pParamLeft = new SwPosition(*pTmp->GetPoint());
                pParamRight = new SwPosition(pTmp->HasMark() ? *pTmp->GetMark() : *pParamLeft);
            }
            else if(pRange)
            {
                SwBookmark* pBkm = pRange->GetBookmark();
                pParamLeft = new SwPosition(pBkm->GetPos());
                pParamRight = new SwPosition(pBkm->GetOtherPos() ? *pBkm->GetOtherPos() : *pParamLeft);
            }
            if(*pParamRight < *pParamLeft)
            {
                SwPosition* pTmp = pParamLeft;
                pParamLeft = pParamRight;
                pParamRight = pTmp;
            }
            // jetzt sind vier SwPositions da, zwei davon werden gebraucht, also welche?
            if(aOwnRight < *pParamRight)
                *aOwnPaM.GetPoint() = aOwnRight;
            else
                *aOwnPaM.GetPoint() = *pParamRight;
            aOwnPaM.SetMark();
            if(aOwnLeft < *pParamLeft)
                *aOwnPaM.GetMark() = *pParamLeft;
            else
                *aOwnPaM.GetMark() = aOwnLeft;

            delete pParamLeft;
            delete pParamRight;
        }
        else
        {
            //der Cursor soll dem uebergebenen Range entsprechen
            if(pCursor)
            {
                const SwUnoCrsr* pTmp = pCursor->GetCrsr();
                *aOwnPaM.GetPoint() = *pTmp->GetPoint();
                if(pTmp->HasMark())
                {
                    aOwnPaM.SetMark();
                    *aOwnPaM.GetMark() = *pTmp->GetMark();
                }
                else
                    aOwnPaM.DeleteMark();
            }
            else
            {
                SwBookmark* pBkm = pRange->GetBookmark();
                *aOwnPaM.GetPoint() = pBkm->GetPos();
                if(pBkm->GetOtherPos())
                {
                    aOwnPaM.SetMark();
                    *aOwnPaM.GetMark() = *pBkm->GetOtherPos();
                }
                else
                    aOwnPaM.DeleteMark();
            }
        }
        rSh.SetSelection(aOwnPaM);
    }
    else
        throw uno::RuntimeException();

}
/*-- 17.12.98 09:36:28---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextViewCursor::gotoStart(sal_Bool bExpand) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if(pView)
        pView->GetWrtShell().SttDoc( bExpand );
    else
        throw uno::RuntimeException();
}
/*-- 17.12.98 09:36:28---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextViewCursor::gotoEnd(sal_Bool bExpand) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if(pView)
        pView->GetWrtShell().EndDoc( bExpand );
    else
        throw uno::RuntimeException();
}
/*-- 17.12.98 09:36:28---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::jumpToFirstPage(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
        bRet = pView->GetWrtShell().SttDoc();
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 17.12.98 09:36:29---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::jumpToLastPage(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
    {
        SwWrtShell& rSh = pView->GetWrtShell();
        bRet = rSh.EndDoc();
        rSh.SttPg();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 17.12.98 09:36:30---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::jumpToPage(sal_Int16 nPage) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
        bRet = pView->GetWrtShell().GotoPage(nPage);
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 17.12.98 09:36:30---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::jumpToNextPage(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
        pView->GetWrtShell().SttNxtPg();
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 17.12.98 09:36:31---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::jumpToPreviousPage(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
        bRet = pView->GetWrtShell().EndPrvPg();
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 17.12.98 09:36:32---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::jumpToEndOfPage(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
        bRet = pView->GetWrtShell().EndPg();
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 17.12.98 09:36:32---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::jumpToStartOfPage(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
        bRet = pView->GetWrtShell().SttPg();
    else
        throw uno::RuntimeException();
    return bRet;
}
/* -----------------04.10.99 14:21-------------------

 --------------------------------------------------*/
sal_Int16 SwXTextViewCursor::getPage(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    short nRet = 0;
    if(pView)
    {
        SwWrtShell& rSh = pView->GetWrtShell();
        sal_uInt16 nPage, nLogPage;
        rSh.GetPageNum( nPage, nLogPage,
                           rSh.IsCrsrVisible(), sal_False);
        nRet = (short)nPage;
    }
    else
        throw uno::RuntimeException();
    return nRet;
}
/*-- 17.12.98 09:36:33---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::screenDown(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
    {
        SfxRequest aReq(FN_PAGEDOWN, SFX_CALLMODE_SLOT, pView->GetPool());
        pView->Execute(aReq);
        const SfxPoolItem* pRet = aReq.GetReturnValue();
        bRet = pRet && ((const SfxBoolItem*)pRet)->GetValue();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 17.12.98 09:36:33---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::screenUp(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
    {
        SfxRequest aReq(FN_PAGEUP, SFX_CALLMODE_SLOT, pView->GetPool());
        pView->Execute(aReq);
        const SfxPoolItem* pRet = aReq.GetReturnValue();
        bRet = pRet && ((const SfxBoolItem*)pRet)->GetValue();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 17.12.98 11:59:05---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< text::XText >  SwXTextViewCursor::getText(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    Reference< text::XText >  xRet;
    if(pView)
    {
        SwWrtShell& rSh = pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        SwDoc* pDoc = pView->GetDocShell()->GetDoc();
        Reference< text::XTextRange >  xRg = SwXTextRange::CreateTextRangeFromPosition(pDoc,
                                    *pShellCrsr->Start(), 0);
        xRet = xRg->getText();
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 17.12.98 11:59:05---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< text::XTextRange >  SwXTextViewCursor::getStart(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    Reference< text::XTextRange >  xRet;
    if(pView)
    {
        SwWrtShell& rSh = pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        SwDoc* pDoc = pView->GetDocShell()->GetDoc();
        xRet = SwXTextRange::CreateTextRangeFromPosition(pDoc,
                                    *pShellCrsr->Start(), 0);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 17.12.98 11:59:06---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< text::XTextRange >  SwXTextViewCursor::getEnd(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    Reference< text::XTextRange >  xRet;
    if(pView)
    {
        SwWrtShell& rSh = pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        SwDoc* pDoc = pView->GetDocShell()->GetDoc();
        xRet = SwXTextRange::CreateTextRangeFromPosition(pDoc,
                                        *pShellCrsr->End(), 0);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/* -----------------12.10.99 09:03-------------------

 --------------------------------------------------*/
OUString SwXTextViewCursor::getString(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    OUString uRet;
    if(pView)
    {
        ShellModes  eSelMode = pView->GetShellMode();
        switch(eSelMode)

            case SEL_LIST_TEXT       :
            case SEL_TABLE_LIST_TEXT:
            case SEL_TEXT            :
            {
                SwWrtShell& rSh = pView->GetWrtShell();
                SwPaM* pShellCrsr = rSh.GetCrsr();
                SwXTextCursor::getTextFromPam(*pShellCrsr, uRet);
            }
    }
    return uRet;
}
/*-- 17.12.98 11:59:06---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextViewCursor::setString(const OUString& aString) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    DBG_WARNING("not implemented")
}

/*-- 29.06.00 17:33:38---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySetInfo >  SwXTextViewCursor::getPropertySetInfo(  ) throw(RuntimeException)
{
    static Reference< XPropertySetInfo >  xRef = aPropSet.getPropertySetInfo();
    return xRef;
}
/*-- 29.06.00 17:33:39---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SwXTextViewCursor::setPropertyValue( const OUString& rPropertyName, const Any& aValue )
                            throw(UnknownPropertyException, PropertyVetoException,
                                IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if(pView)
    {
        SwWrtShell& rSh = pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        SwXTextCursor::SetPropertyValue(*pShellCrsr, aPropSet, rPropertyName, aValue );
    }
    else
        throw RuntimeException();
}
/*-- 29.06.00 17:33:39---------------------------------------------------

  -----------------------------------------------------------------------*/
Any  SwXTextViewCursor::getPropertyValue( const OUString& rPropertyName )
                throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    Any aRet;
    if(pView)
    {
        SwWrtShell& rSh = pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        aRet = SwXTextCursor::GetPropertyValue(  *pShellCrsr, aPropSet, rPropertyName);
    }
    else
        throw RuntimeException();
    return aRet;
}
/*-- 29.06.00 17:33:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SwXTextViewCursor::addPropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 29.06.00 17:33:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SwXTextViewCursor::removePropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 29.06.00 17:33:41---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SwXTextViewCursor::addVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 29.06.00 17:33:41---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SwXTextViewCursor::removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 29.06.00 17:33:41---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyState  SwXTextViewCursor::getPropertyState( const OUString& rPropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    PropertyState eState;
    if(pView)
    {
        SwWrtShell& rSh = pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        eState = SwXTextCursor::GetPropertyState( *pShellCrsr, aPropSet, rPropertyName);
    }
    else
        throw RuntimeException();
    return eState;
}
/*-- 29.06.00 17:33:42---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< PropertyState >  SwXTextViewCursor::getPropertyStates(
    const Sequence< OUString >& rPropertyNames ) throw(UnknownPropertyException, RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< PropertyState >  aRet;
    if(pView)
    {
        SwWrtShell& rSh = pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        aRet = SwXTextCursor::GetPropertyStates(*pShellCrsr, aPropSet,  rPropertyNames);
    }
    return aRet;
}
/*-- 29.06.00 17:33:42---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SwXTextViewCursor::setPropertyToDefault( const OUString& rPropertyName )
                                        throw(UnknownPropertyException, RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if(pView)
    {
        SwWrtShell& rSh = pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        SwXTextCursor::SetPropertyToDefault( *pShellCrsr, aPropSet, rPropertyName);
    }
}
/*-- 29.06.00 17:33:43---------------------------------------------------

  -----------------------------------------------------------------------*/
Any  SwXTextViewCursor::getPropertyDefault( const OUString& rPropertyName )
                        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Any aRet;
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if(pView)
    {
        SwWrtShell& rSh = pView->GetWrtShell();
        SwPaM* pShellCrsr = rSh.GetCrsr();
        aRet = SwXTextCursor::GetPropertyDefault( *pShellCrsr, aPropSet, rPropertyName);
    }
    return aRet;
}
/*-- 28.09.99 08:31:19---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::goDown(sal_Int16 nCount, sal_Bool bExpand) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
    {
        for( sal_uInt16 i = 0; i < nCount; i++ )
            bRet = pView->GetWrtShell().Down( bExpand, 1, sal_True );
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 28.09.99 08:31:20---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::goUp(sal_Int16 nCount, sal_Bool bExpand) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
    {
        for( sal_uInt16 i = 0; i < nCount; i++ )
            bRet = pView->GetWrtShell().Up( bExpand, 1, sal_True );
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 28.09.99 08:31:20---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::isAtStartOfLine(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
    {
        bRet = pView->GetWrtShell().IsAtLeftMargin();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 28.09.99 08:31:21---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextViewCursor::isAtEndOfLine(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pView)
    {
        bRet = pView->GetWrtShell().IsAtRightMargin(sal_True);
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 28.09.99 08:31:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextViewCursor::gotoEndOfLine(sal_Bool bExpand) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if(pView)
        pView->GetWrtShell().RightMargin(bExpand, sal_True);
    else
        throw uno::RuntimeException();
}
/*-- 28.09.99 08:31:22---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextViewCursor::gotoStartOfLine(sal_Bool bExpand) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if(pView)
        pView->GetWrtShell().LeftMargin(bExpand, sal_True);
    else
        throw uno::RuntimeException();
}
/* -----------------------------06.04.00 11:07--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTextViewCursor::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextViewCursor");
}
/* -----------------------------06.04.00 11:07--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTextViewCursor::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextViewCursor") == rServiceName;
}
/* -----------------------------06.04.00 11:07--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXTextViewCursor::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextViewCursor");
    return aRet;
}

