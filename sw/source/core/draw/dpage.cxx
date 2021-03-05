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
#include <fmturl.hxx>
#include <frmfmt.hxx>
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
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::frame;

SwDPage::SwDPage(SwDrawModel& rNewModel, bool bMasterPage)
:   FmFormPage(rNewModel, bMasterPage),
    m_pDoc(&rNewModel.GetDoc())
{
}

SwDPage::~SwDPage()
{
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

Reference< XInterface > SwDPage::createUnoPage()
{
    assert( m_pDoc );

    Reference < XInterface > xRet;
    SwDocShell* pDocShell = m_pDoc->GetDocShell();
    if ( pDocShell )
    {
        Reference<XModel> xModel = pDocShell->GetBaseModel();
        Reference<XDrawPageSupplier> xPageSupp(xModel, UNO_QUERY);
        xRet = xPageSupp->getDrawPage();
    }
    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
