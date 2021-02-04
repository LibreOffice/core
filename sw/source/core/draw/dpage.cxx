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

#include <editeng/flditem.hxx>
#include <vcl/imapobj.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/sfxhelp.hxx>
#include <vcl/help.hxx>
#include <svx/svdview.hxx>
#include <svx/unoshape.hxx>
#include <svx/scene3d.hxx>
#include <fmturl.hxx>
#include <fmtcntnt.hxx>
#include <frmfmt.hxx>
#include <ndindex.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <viewimp.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <viewsh.hxx>
#include <drawdoc.hxx>
#include <dpage.hxx>
#include <dcontact.hxx>
#include <dflyobj.hxx>
#include <docsh.hxx>
#include <flyfrm.hxx>
#include <unodraw.hxx>
#include <unoframe.hxx>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::frame;

SwDPage::SwDPage(SwDrawModel& rNewModel, bool bMasterPage)
:   FmFormPage(rNewModel, bMasterPage),
    m_pDoc(&rNewModel.GetDoc()),
    m_pPageView(nullptr)
{
}

SwDPage::~SwDPage()
{
    while (!m_vShapes.empty())
        m_vShapes.back()->dispose();
    RemovePageView();
}

void SwDPage::lateInit(const SwDPage& rSrcPage)
{
    FmFormPage::lateInit( rSrcPage );

    if ( rSrcPage.m_pGridLst )
    {
        m_pGridLst.reset( new SdrPageGridFrameList );
        for ( sal_uInt16 i = 0; i != rSrcPage.m_pGridLst->GetCount(); ++i )
            m_pGridLst->Insert( ( *rSrcPage.m_pGridLst )[ i ] );
    }
}

rtl::Reference<SdrPage> SwDPage::CloneSdrPage(SdrModel& rTargetModel) const
{
    SwDrawModel& rSwDrawModel(static_cast< SwDrawModel& >(rTargetModel));
    rtl::Reference<SwDPage> pClonedSwDPage(
        new SwDPage(
            rSwDrawModel,
            IsMasterPage()));
    pClonedSwDPage->lateInit(*this);
    return pClonedSwDPage;
}

SdrObject*  SwDPage::ReplaceObject( SdrObject* pNewObj, size_t nObjNum )
{
    SdrObject *pOld = GetObj( nObjNum );
    OSL_ENSURE( pOld, "Oups, Object not replaced" );
    SdrObjUserCall* pContact;
    if ( nullptr != ( pContact = GetUserCall(pOld) ) &&
         RES_DRAWFRMFMT == static_cast<SwContact*>(pContact)->GetFormat()->Which())
        static_cast<SwDrawContact*>(pContact)->ChangeMasterObject( pNewObj );
    return FmFormPage::ReplaceObject( pNewObj, nObjNum );
}

static void InsertGridFrame( SdrPageGridFrameList *pLst, const SwFrame *pPg )
{
    SwRect aPrt( pPg->getFramePrintArea() );
    aPrt += pPg->getFrameArea().Pos();
    const tools::Rectangle aUser( aPrt.SVRect() );
    const tools::Rectangle aPaper( pPg->getFrameArea().SVRect() );
    pLst->Insert( SdrPageGridFrame( aPaper, aUser ) );
}

const SdrPageGridFrameList*  SwDPage::GetGridFrameList(
                        const SdrPageView* pPV, const tools::Rectangle *pRect ) const
{
    SwViewShell* pSh = static_cast< SwDrawModel& >(getSdrModelFromSdrPage()).GetDoc().getIDocumentLayoutAccess().GetCurrentViewShell();
    if(pSh)
    {
        for(SwViewShell& rShell : pSh->GetRingContainer())
        {
            if(rShell.Imp()->GetPageView() == pPV)
            {
                pSh = &rShell;
                break;
            }
        }
        if ( m_pGridLst )
            const_cast<SwDPage*>(this)->m_pGridLst->Clear();
        else
            const_cast<SwDPage*>(this)->m_pGridLst.reset( new SdrPageGridFrameList );

        if ( pRect )
        {
            //The drawing demands all pages which overlap with the rest.
            const SwRect aRect( *pRect );
            const SwFrame *pPg = pSh->GetLayout()->Lower();
            do
            {   if ( pPg->getFrameArea().IsOver( aRect ) )
                    ::InsertGridFrame( const_cast<SwDPage*>(this)->m_pGridLst.get(), pPg );
                pPg = pPg->GetNext();
            } while ( pPg );
        }
        else
        {
            //The drawing demands all visible pages
            const SwFrame *pPg = pSh->Imp()->GetFirstVisPage(pSh->GetOut());
            if ( pPg )
                do
                {   ::InsertGridFrame( const_cast<SwDPage*>(this)->m_pGridLst.get(), pPg );
                    pPg = pPg->GetNext();
                } while ( pPg && pPg->getFrameArea().IsOver( pSh->VisArea() ) );
        }
    }
    return m_pGridLst.get();
}

bool SwDPage::RequestHelp( vcl::Window* pWindow, SdrView const * pView,
                           const HelpEvent& rEvt )
{
    assert( m_pDoc );

    bool bContinue = true;

    if( rEvt.GetMode() & ( HelpEventMode::QUICK | HelpEventMode::BALLOON ))
    {
        Point aPos( rEvt.GetMousePosPixel() );
        aPos = pWindow->ScreenToOutputPixel( aPos );
        aPos = pWindow->PixelToLogic( aPos );

        SdrPageView* pPV;
        SdrObject* pObj = pView->PickObj(aPos, 0, pPV, SdrSearchOptions::PICKMACRO);
        SwVirtFlyDrawObj* pDrawObj = dynamic_cast<SwVirtFlyDrawObj*>(pObj);
        OUString sText;
        tools::Rectangle aPixRect;
        if (pDrawObj)
        {
            SwFlyFrame *pFly = pDrawObj->GetFlyFrame();

            aPixRect = pWindow->LogicToPixel(pFly->getFrameArea().SVRect());

            const SwFormatURL &rURL = pFly->GetFormat()->GetURL();
            if( rURL.GetMap() )
            {
                IMapObject *pTmpObj = pFly->GetFormat()->GetIMapObject( aPos, pFly );
                if( pTmpObj )
                {
                    sText = pTmpObj->GetAltText();
                    if ( sText.isEmpty() )
                        sText = URIHelper::removePassword( pTmpObj->GetURL(),
                                        INetURLObject::EncodeMechanism::WasEncoded,
                                           INetURLObject::DecodeMechanism::Unambiguous);
                }
            }
            else if ( !rURL.GetURL().isEmpty() )
            {
                sText = URIHelper::removePassword( rURL.GetURL(),
                                        INetURLObject::EncodeMechanism::WasEncoded,
                                           INetURLObject::DecodeMechanism::Unambiguous);

                if( rURL.IsServerMap() )
                {
                    // then append the relative pixel position!!
                    Point aPt( aPos );
                    aPt -= pFly->getFrameArea().Pos();
                    // without MapMode-Offset !!!!!
                    // without MapMode-Offset, without Offset, w ... !!!!!
                    aPt = pWindow->LogicToPixel(
                            aPt, MapMode( MapUnit::MapTwip ) );
                    sText += "?" + OUString::number( aPt.getX() )
                          + "," + OUString::number( aPt.getY() );
                }
            }
        }
        else
        {
            SdrViewEvent aVEvt;
            MouseEvent aMEvt(pWindow->ScreenToOutputPixel(rEvt.GetMousePosPixel()), 1,
                             MouseEventModifiers::NONE, MOUSE_LEFT);
            pView->PickAnything(aMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);
            if (aVEvt.eEvent == SdrEventKind::ExecuteUrl)
            {
                sText = aVEvt.pURLField->GetURL();
                aPixRect = pWindow->LogicToPixel(aVEvt.pObj->GetLogicRect());
            }
        }

        if (!sText.isEmpty())
        {
            // #i80029#
            bool bExecHyperlinks = m_pDoc->GetDocShell()->IsReadOnly();
            if (!bExecHyperlinks)
                sText = SfxHelp::GetURLHelpText(sText);

            // then display the help:
            tools::Rectangle aScreenRect(pWindow->OutputToScreenPixel(aPixRect.TopLeft()),
                                         pWindow->OutputToScreenPixel(aPixRect.BottomRight()));

            if (rEvt.GetMode() & HelpEventMode::BALLOON)
                Help::ShowBalloon(pWindow, rEvt.GetMousePosPixel(), aScreenRect, sText);
            else
                Help::ShowQuickHelp(pWindow, aScreenRect, sText);
            bContinue = false;
        }
    }

    if( bContinue )
        bContinue = !FmFormPage::RequestHelp( pWindow, pView, rEvt );

    return bContinue;
}

const SdrMarkList&  SwDPage::PreGroup(const uno::Reference< drawing::XShapes > & xShapes)
{
    SelectObjectsInView( xShapes, GetPageView() );
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
    return rMarkList;
}

void SwDPage::PreUnGroup(const uno::Reference< drawing::XShapeGroup >&  rShapeGroup)
{
    SelectObjectInView( rShapeGroup, GetPageView() );
}

SdrPageView*    SwDPage::GetPageView()
{
    if(!m_pPageView)
        m_pPageView = mpView->ShowSdrPage( this );
    return m_pPageView;
}

void    SwDPage::RemovePageView()
{
    if(m_pPageView && mpView)
        mpView->HideSdrPage();
    m_pPageView = nullptr;
}

uno::Reference<drawing::XShape> SwDPage::GetShape(SdrObject* pObj)
{
    if(!pObj)
        return nullptr;
    SwFrameFormat* pFormat = ::FindFrameFormat( pObj );
    SwDPage* pPage = dynamic_cast<SwDPage*>(pFormat);
    if(!pPage || pPage->m_vShapes.empty())
        return uno::Reference<drawing::XShape>(pObj->getUnoShape(), uno::UNO_QUERY);
    for(auto pShape : pPage->m_vShapes)
    {
        SvxShape* pSvxShape = pShape->GetSvxShape();
        if (pSvxShape && pSvxShape->GetSdrObject() == pObj)
            return uno::Reference<drawing::XShape>(static_cast<::cppu::OWeakObject*>(pShape), uno::UNO_QUERY);
    }
    return nullptr;
}

uno::Reference<drawing::XShapeGroup> SwDPage::GetShapeGroup(SdrObject* pObj)
{
    return uno::Reference<drawing::XShapeGroup>(GetShape(pObj), uno::UNO_QUERY);
}

uno::Reference< drawing::XShape > SwDPage::CreateShape( SdrObject *pObj ) const
{
    uno::Reference< drawing::XShape >  xRet;
    if(dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr || pObj->GetObjInventor() == SdrInventor::Swg)
    {
        SwFlyDrawContact* pFlyContact = static_cast<SwFlyDrawContact*>(pObj->GetUserCall());
        if(pFlyContact)
        {
            SwFrameFormat* pFlyFormat = pFlyContact->GetFormat();
            SwDoc* pDoc = pFlyFormat->GetDoc();
            const SwNodeIndex* pIdx;
            if( RES_FLYFRMFMT == pFlyFormat->Which()
                && nullptr != ( pIdx = pFlyFormat->GetContent().GetContentIdx() )
                && pIdx->GetNodes().IsDocNodes()
                )
            {
                const SwNode* pNd = pDoc->GetNodes()[ pIdx->GetIndex() + 1 ];
                if(!pNd->IsNoTextNode())
                {
                    xRet.set(SwXTextFrame::CreateXTextFrame(*pDoc, pFlyFormat),
                            uno::UNO_QUERY);
                }
                else if( pNd->IsGrfNode() )
                {
                    xRet.set(SwXTextGraphicObject::CreateXTextGraphicObject(
                                *pDoc, pFlyFormat), uno::UNO_QUERY);
                }
                else if( pNd->IsOLENode() )
                {
                    xRet.set(SwXTextEmbeddedObject::CreateXTextEmbeddedObject(
                                *pDoc, pFlyFormat), uno::UNO_QUERY);
                }
            }
            else
            {
                OSL_FAIL( "<SwDPage::CreateShape(..)> - could not retrieve type. Thus, no shape created." );
                return xRet;
            }
        }
     }
    else
    {
        // own block - temporary object has to be destroyed before
        // the delegator is set #81670#
        {
            xRet = FmFormPage::CreateShape( pObj );
        }
        uno::Reference< XUnoTunnel > xShapeTunnel(xRet, uno::UNO_QUERY);
        //don't create an SwXShape if it already exists
        SwXShape* pShape = nullptr;
        if(xShapeTunnel.is())
            pShape = reinterpret_cast< SwXShape * >(
                    sal::static_int_cast< sal_IntPtr >( xShapeTunnel->getSomething(SwXShape::getUnoTunnelId()) ));
        if(!pShape)
        {
            xShapeTunnel = nullptr;
            uno::Reference< uno::XInterface > xCreate(xRet, uno::UNO_QUERY);
            xRet = nullptr;
            if ( pObj->IsGroupObject() && (!pObj->Is3DObj() || (dynamic_cast<const E3dScene*>( pObj) !=  nullptr)) )
                pShape = new SwXGroupShape(xCreate, nullptr);
            else
                pShape = new SwXShape(xCreate, nullptr);
            uno::Reference<beans::XPropertySet> xPrSet = pShape;
            xRet.set(xPrSet, uno::UNO_QUERY);
        }
        const_cast<std::vector<SwXShape*>*>(&m_vShapes)->push_back(pShape);
        pShape->m_pPage = this;
    }
    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
