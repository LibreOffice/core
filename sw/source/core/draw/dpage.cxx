/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dpage.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 14:52:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifndef _BASMGR_HXX
#include <basic/basmgr.hxx>
#endif
#ifndef _GOODIES_IMAPOBJ_HXX
#include <svtools/imapobj.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SECURITYOPTIONS_HXX
#include <svtools/securityoptions.hxx>
#endif
#ifndef _SV_HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _DRAWDOC_HXX
#include <drawdoc.hxx>
#endif
#ifndef _DPAGE_HXX
#include <dpage.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _USRFLD_HXX
#include <usrfld.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _NDNOTXT_HXX
#include <ndnotxt.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::frame;

SwDPage::SwDPage(SwDrawDocument& rNewModel, BOOL bMasterPage) :
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

SdrObject*  SwDPage::ReplaceObject( SdrObject* pNewObj, ULONG nObjNum )
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
    ViewShell *pSh = ((SwDrawDocument*)GetModel())->GetDoc().GetRootFrm()->GetCurrShell();
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

BOOL SwDPage::RequestHelp( Window* pWindow, SdrView* pView,
                           const HelpEvent& rEvt )
{
    BOOL bWeiter = TRUE;

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
                    aPt = (Point&)(const Size&)pWindow->LogicToPixel(
                            (const Size&)aPt, MapMode( MAP_TWIP ) );
                    ((( sTxt += '?' ) += String::CreateFromInt32( aPt.X() ))
                             += ',' ) += String::CreateFromInt32( aPt.Y() );
                }
            }

            if ( sTxt.Len() )
            {
                // --> OD 2007-07-26 #i80029#
                BOOL bExecHyperlinks = rDoc.GetDocShell()->IsReadOnly();
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
                bWeiter = FALSE;
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



