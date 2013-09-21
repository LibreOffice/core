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

SwDPage::SwDPage(SwDrawDocument& rNewModel, sal_Bool bMasterPage) :
    FmFormPage(rNewModel, 0, bMasterPage),
    pGridLst( 0 ),
    rDoc(rNewModel.GetDoc())
{
}

SwDPage::~SwDPage()
{
    delete pGridLst;
}

SdrObject*  SwDPage::ReplaceObject( SdrObject* pNewObj, sal_uLong nObjNum )
{
    SdrObject *pOld = GetObj( nObjNum );
    OSL_ENSURE( pOld, "Oups, Object not replaced" );
    SdrObjUserCall* pContact;
    if ( 0 != ( pContact = GetUserCall(pOld) ) &&
         RES_DRAWFRMFMT == ((SwContact*)pContact)->GetFmt()->Which())
        ((SwDrawContact*)pContact)->ChangeMasterObject( pNewObj );
    return FmFormPage::ReplaceObject( pNewObj, nObjNum );
}

void InsertGridFrame( SdrPageGridFrameList *pLst, const SwFrm *pPg )
{
    SwRect aPrt( pPg->Prt() );
    aPrt += pPg->Frm().Pos();
    const Rectangle aUser( aPrt.SVRect() );
    const Rectangle aPaper( pPg->Frm().SVRect() );
    pLst->Insert( SdrPageGridFrame( aPaper, aUser ) );
}

const SdrPageGridFrameList*  SwDPage::GetGridFrameList(
                        const SdrPageView* pPV, const Rectangle *pRect ) const
{
    ViewShell *pSh = ((SwDrawDocument*)GetModel())->GetDoc().GetCurrentViewShell(); //swmod 071108//swmod 071225
    if ( pSh )
    {
        while ( pSh->Imp()->GetPageView() != pPV )
            pSh = (ViewShell*)pSh->GetNext();
        if ( pSh )
        {
            if ( pGridLst )
                ((SwDPage*)this)->pGridLst->Clear();
            else
                ((SwDPage*)this)->pGridLst = new SdrPageGridFrameList;

            if ( pRect )
            {
                //The drawing demands all pages which overlap with the rest.
                const SwRect aRect( *pRect );
                const SwFrm *pPg = pSh->GetLayout()->Lower();
                do
                {   if ( pPg->Frm().IsOver( aRect ) )
                        ::InsertGridFrame( ((SwDPage*)this)->pGridLst, pPg );
                    pPg = pPg->GetNext();
                } while ( pPg );
            }
            else
            {
                //The drawing demands all visible pages
                const SwFrm *pPg = pSh->Imp()->GetFirstVisPage();
                if ( pPg )
                    do
                    {   ::InsertGridFrame( ((SwDPage*)this)->pGridLst, pPg );
                        pPg = pPg->GetNext();
                    } while ( pPg && pPg->Frm().IsOver( pSh->VisArea() ) );
            }
        }
    }
    return pGridLst;
}

sal_Bool SwDPage::RequestHelp( Window* pWindow, SdrView* pView,
                           const HelpEvent& rEvt )
{
    sal_Bool bWeiter = sal_True;

    if( rEvt.GetMode() & ( HELPMODE_QUICK | HELPMODE_BALLOON ))
    {
        Point aPos( rEvt.GetMousePosPixel() );
        aPos = pWindow->ScreenToOutputPixel( aPos );
        aPos = pWindow->PixelToLogic( aPos );

        SdrPageView* pPV;
        SdrObject* pObj;
        if( pView->PickObj( aPos, 0, pObj, pPV, SDRSEARCH_PICKMACRO ) &&
             pObj->ISA(SwVirtFlyDrawObj) )
        {
            SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
            const SwFmtURL &rURL = pFly->GetFmt()->GetURL();
            OUString sTxt;
            if( rURL.GetMap() )
            {
                IMapObject *pTmpObj = pFly->GetFmt()->GetIMapObject( aPos, pFly );
                if( pTmpObj )
                {
                    sTxt = pTmpObj->GetAltText();
                    if ( sTxt.isEmpty() )
                        sTxt = URIHelper::removePassword( pTmpObj->GetURL(),
                                        INetURLObject::WAS_ENCODED,
                                           INetURLObject::DECODE_UNAMBIGUOUS);
                }
            }
            else if ( !rURL.GetURL().isEmpty() )
            {
                sTxt = URIHelper::removePassword( rURL.GetURL(),
                                        INetURLObject::WAS_ENCODED,
                                           INetURLObject::DECODE_UNAMBIGUOUS);

                if( rURL.IsServerMap() )
                {
                    // then append the relative pixel position!!
                    Point aPt( aPos );
                    aPt -= pFly->Frm().Pos();
                    // without MapMode-Offset !!!!!
                    // without MapMode-Offset, without Offset, w ... !!!!!
                    aPt = pWindow->LogicToPixel(
                            aPt, MapMode( MAP_TWIP ) );
                    sTxt += "?" + OUString::number( aPt.getX() )
                          + "," + OUString::number( aPt.getY() );
                }
            }

            if ( !sTxt.isEmpty() )
            {
                // #i80029#
                sal_Bool bExecHyperlinks = rDoc.GetDocShell()->IsReadOnly();
                if ( !bExecHyperlinks )
                {
                    SvtSecurityOptions aSecOpts;
                    bExecHyperlinks = !aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK );

                    if ( !bExecHyperlinks )
                    {
                        sTxt = ViewShell::GetShellRes()->aHyperlinkClick + ": " + sTxt;
                    }
                }


                if( rEvt.GetMode() & HELPMODE_BALLOON )
                {
                    Help::ShowBalloon( pWindow, rEvt.GetMousePosPixel(), sTxt );
                }
                else
                {
                    // then display the help:
                    Rectangle aRect( rEvt.GetMousePosPixel(), Size(1,1) );
                    Help::ShowQuickHelp( pWindow, aRect, sTxt );
                }
                bWeiter = sal_False;
            }
        }
    }

    if( bWeiter )
        bWeiter = !FmFormPage::RequestHelp( pWindow, pView, rEvt );

    return bWeiter;
}

Reference< XInterface > SwDPage::createUnoPage()
{
    Reference < XInterface > xRet;
    SwDocShell* pDocShell = rDoc.GetDocShell();
    if ( pDocShell )
    {
        Reference<XModel> xModel = pDocShell->GetBaseModel();
        Reference<XDrawPageSupplier> xPageSupp(xModel, UNO_QUERY);
        xRet = xPageSupp->getDrawPage();
    }
    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
