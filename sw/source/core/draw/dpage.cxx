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

#include <basic/basmgr.hxx>
#include <svtools/imapobj.hxx>
#include <svl/urihelper.hxx>
#include <unotools/securityoptions.hxx>
#include <vcl/help.hxx>
#include <svx/svdview.hxx>
#include <fmturl.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <shellres.hxx>
#include <viewimp.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <rootfrm.hxx>
#include <frmatr.hxx>
#include <viewsh.hxx>
#include <drawdoc.hxx>
#include <dpage.hxx>
#include <dcontact.hxx>
#include <dflyobj.hxx>
#include <docsh.hxx>
#include <usrfld.hxx>
#include <flyfrm.hxx>
#include <ndnotxt.hxx>
#include <grfatr.hxx>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::frame;

SwDPage::SwDPage(SwDrawModel& rNewModel, bool bMasterPage) :
    FmFormPage(rNewModel, bMasterPage),
    pGridLst( nullptr ),
    pDoc(&rNewModel.GetDoc())
{
}

SwDPage::SwDPage(const SwDPage& rSrcPage) :
    FmFormPage( rSrcPage ),
    pDoc( nullptr )
{
    if ( rSrcPage.pGridLst )
    {
        pGridLst.reset( new SdrPageGridFrameList );
        for ( sal_uInt16 i = 0; i != rSrcPage.pGridLst->GetCount(); ++i )
            pGridLst->Insert( ( *rSrcPage.pGridLst )[ i ] );
    }
}

SwDPage::~SwDPage()
{
}

void SwDPage::lateInit(const SwDPage& rPage, SwDrawModel* const pNewModel)
{
    FmFormPage::lateInit( rPage, pNewModel );

    SwDrawModel* pSwDrawModel = pNewModel;
    if (!pSwDrawModel)
    {
        pSwDrawModel = &dynamic_cast<SwDrawModel&>(*GetModel());
        assert( pSwDrawModel );
    }
    pDoc = &pSwDrawModel->GetDoc();
}

SwDPage* SwDPage::Clone() const
{
    return Clone( nullptr );
}

SwDPage* SwDPage::Clone(SdrModel* const pNewModel) const
{
    SwDPage* const pNewPage = new SwDPage( *this );
    SwDrawModel* pSwDrawModel = nullptr;
    if ( pNewModel )
    {
        pSwDrawModel = &dynamic_cast<SwDrawModel&>(*pNewModel);
        assert( pSwDrawModel );
    }
    pNewPage->lateInit( *this, pSwDrawModel );
    return pNewPage;
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

void InsertGridFrame( SdrPageGridFrameList *pLst, const SwFrame *pPg )
{
    SwRect aPrt( pPg->Prt() );
    aPrt += pPg->Frame().Pos();
    const Rectangle aUser( aPrt.SVRect() );
    const Rectangle aPaper( pPg->Frame().SVRect() );
    pLst->Insert( SdrPageGridFrame( aPaper, aUser ) );
}

const SdrPageGridFrameList*  SwDPage::GetGridFrameList(
                        const SdrPageView* pPV, const Rectangle *pRect ) const
{
    SwViewShell* pSh = static_cast< SwDrawModel* >(GetModel())->GetDoc().getIDocumentLayoutAccess().GetCurrentViewShell();
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
        if ( pGridLst )
            const_cast<SwDPage*>(this)->pGridLst->Clear();
        else
            const_cast<SwDPage*>(this)->pGridLst.reset( new SdrPageGridFrameList );

        if ( pRect )
        {
            //The drawing demands all pages which overlap with the rest.
            const SwRect aRect( *pRect );
            const SwFrame *pPg = pSh->GetLayout()->Lower();
            do
            {   if ( pPg->Frame().IsOver( aRect ) )
                    ::InsertGridFrame( const_cast<SwDPage*>(this)->pGridLst.get(), pPg );
                pPg = pPg->GetNext();
            } while ( pPg );
        }
        else
        {
            //The drawing demands all visible pages
            const SwFrame *pPg = pSh->Imp()->GetFirstVisPage(pSh->GetOut());
            if ( pPg )
                do
                {   ::InsertGridFrame( const_cast<SwDPage*>(this)->pGridLst.get(), pPg );
                    pPg = pPg->GetNext();
                } while ( pPg && pPg->Frame().IsOver( pSh->VisArea() ) );
        }
    }
    return pGridLst.get();
}

bool SwDPage::RequestHelp( vcl::Window* pWindow, SdrView* pView,
                           const HelpEvent& rEvt )
{
    assert( pDoc );

    bool bContinue = true;

    if( rEvt.GetMode() & ( HelpEventMode::QUICK | HelpEventMode::BALLOON ))
    {
        Point aPos( rEvt.GetMousePosPixel() );
        aPos = pWindow->ScreenToOutputPixel( aPos );
        aPos = pWindow->PixelToLogic( aPos );

        SdrPageView* pPV;
        SdrObject* pObj = pView->PickObj(aPos, 0, pPV, SdrSearchOptions::PICKMACRO);
        SwVirtFlyDrawObj* pDrawObj = dynamic_cast<SwVirtFlyDrawObj*>(pObj);
        if (pDrawObj)
        {
            SwFlyFrame *pFly = pDrawObj->GetFlyFrame();
            const SwFormatURL &rURL = pFly->GetFormat()->GetURL();
            OUString sText;
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
                    aPt -= pFly->Frame().Pos();
                    // without MapMode-Offset !!!!!
                    // without MapMode-Offset, without Offset, w ... !!!!!
                    aPt = pWindow->LogicToPixel(
                            aPt, MapMode( MapUnit::MapTwip ) );
                    sText += "?" + OUString::number( aPt.getX() )
                          + "," + OUString::number( aPt.getY() );
                }
            }

            if ( !sText.isEmpty() )
            {
                // #i80029#
                bool bExecHyperlinks = pDoc->GetDocShell()->IsReadOnly();
                if ( !bExecHyperlinks )
                {
                    SvtSecurityOptions aSecOpts;
                    bExecHyperlinks = !aSecOpts.IsOptionSet( SvtSecurityOptions::EOption::CtrlClickHyperlink );

                    if ( !bExecHyperlinks )
                        sText = SwViewShell::GetShellRes()->aLinkCtrlClick + ": " + sText;
                    else
                        sText = SwViewShell::GetShellRes()->aLinkClick + ": " + sText;
                }

                // then display the help:
                Rectangle aRect( rEvt.GetMousePosPixel(), Size(1,1) );
                if( rEvt.GetMode() & HelpEventMode::BALLOON )
                {
                    Help::ShowBalloon( pWindow, rEvt.GetMousePosPixel(), aRect, sText );
                }
                else
                {
                    Help::ShowQuickHelp( pWindow, aRect, sText );
                }
                bContinue = false;
            }
        }
    }

    if( bContinue )
        bContinue = !FmFormPage::RequestHelp( pWindow, pView, rEvt );

    return bContinue;
}

Reference< XInterface > SwDPage::createUnoPage()
{
    assert( pDoc );

    Reference < XInterface > xRet;
    SwDocShell* pDocShell = pDoc->GetDocShell();
    if ( pDocShell )
    {
        Reference<XModel> xModel = pDocShell->GetBaseModel();
        Reference<XDrawPageSupplier> xPageSupp(xModel, UNO_QUERY);
        xRet = xPageSupp->getDrawPage();
    }
    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
