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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <basic/basmgr.hxx>
#include <svtools/imapobj.hxx>
#include <svl/urihelper.hxx>
#include <unotools/securityoptions.hxx>
#include <vcl/help.hxx>
#include <svx/svdview.hxx>
#include <fmturl.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
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
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
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

/*************************************************************************
|*
|*  SwDPage::ReplaceObject()
|*
|*  Ersterstellung      MA 07. Aug. 95
|*  Letzte Aenderung    MA 07. Aug. 95
|*
*************************************************************************/

SdrObject*  SwDPage::ReplaceObject( SdrObject* pNewObj, sal_uLong nObjNum )
{
    SdrObject *pOld = GetObj( nObjNum );
    ASSERT( pOld, "Oups, Object not replaced" );
    SdrObjUserCall* pContact;
    if ( 0 != ( pContact = GetUserCall(pOld) ) &&
         RES_DRAWFRMFMT == ((SwContact*)pContact)->GetFmt()->Which())
        ((SwDrawContact*)pContact)->ChangeMasterObject( pNewObj );
    return FmFormPage::ReplaceObject( pNewObj, nObjNum );
}

/*************************************************************************
|*
|*  SwDPage::GetGridFrameList()
|*
|*  Ersterstellung      MA 04. Sep. 95
|*  Letzte Aenderung    MA 15. Feb. 96
|*
*************************************************************************/

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
                //Das Drawing verlang alle Seiten, die mit dem Rect ueberlappen.
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
                //Das Drawing verlangt alle sichbaren Seiten
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
            String sTxt;
            if( rURL.GetMap() )
            {
                IMapObject *pTmpObj = pFly->GetFmt()->GetIMapObject( aPos, pFly );
                if( pTmpObj )
                {
                    sTxt = pTmpObj->GetAltText();
                    if ( !sTxt.Len() )
                        sTxt = URIHelper::removePassword( pTmpObj->GetURL(),
                                        INetURLObject::WAS_ENCODED,
                                           INetURLObject::DECODE_UNAMBIGUOUS);
                }
            }
            else if ( rURL.GetURL().Len() )
            {
                sTxt = URIHelper::removePassword( rURL.GetURL(),
                                        INetURLObject::WAS_ENCODED,
                                           INetURLObject::DECODE_UNAMBIGUOUS);

                if( rURL.IsServerMap() )
                {
                    // dann die rel. Pixel Position anhaengen !!
                    Point aPt( aPos );
                    aPt -= pFly->Frm().Pos();
                    // ohne MapMode-Offset !!!!!
                    // ohne MapMode-Offset, ohne Offset, o ... !!!!!
                    aPt = pWindow->LogicToPixel(
                            aPt, MapMode( MAP_TWIP ) );
                    ((( sTxt += '?' ) += String::CreateFromInt32( aPt.X() ))
                             += ',' ) += String::CreateFromInt32( aPt.Y() );
                }
            }

            if ( sTxt.Len() )
            {
                // --> OD 2007-07-26 #i80029#
                sal_Bool bExecHyperlinks = rDoc.GetDocShell()->IsReadOnly();
                if ( !bExecHyperlinks )
                {
                    SvtSecurityOptions aSecOpts;
                    bExecHyperlinks = !aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK );

                    if ( !bExecHyperlinks )
                    {
                        sTxt.InsertAscii( ": ", 0 );
                        sTxt.Insert( ViewShell::GetShellRes()->aHyperlinkClick, 0 );
                    }
                }

                // <--

                if( rEvt.GetMode() & HELPMODE_BALLOON )
                {
                    Help::ShowBalloon( pWindow, rEvt.GetMousePosPixel(), sTxt );
                }
                else
                {
            // dann zeige die Hilfe mal an:
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
/* -----------------------------27.11.00 07:35--------------------------------

 ---------------------------------------------------------------------------*/
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



