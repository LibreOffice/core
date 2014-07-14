/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

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
#include <docsh.hxx>
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
#include <dview.hxx>
#include <drawdoc.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::frame;

SwDPage::SwDPage(SwDrawModel& rNewModel, bool bMasterPage) :
    FmFormPage(rNewModel, 0, bMasterPage),
    pGridLst( 0 ),
    rDoc(rNewModel.GetDoc())
{
}


SwDPage::~SwDPage()
{
    delete pGridLst;
}

void SwDPage::copyDataFromSdrPage(const SdrPage& rSource)
{
    if(this != &rSource)
    {
        const SwDPage* pSource = dynamic_cast< const SwDPage* >(&rSource);

        if(pSource)
        {
            // call parent
            FmFormPage::copyDataFromSdrPage(rSource);

            // no local data to copy
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrPage* SwDPage::CloneSdrPage(SdrModel* pTargetModel) const
{
    SwDrawModel* pSwDrawModel = static_cast< SwDrawModel* >(pTargetModel ? pTargetModel : &getSdrModelFromSdrPage());
    OSL_ENSURE(dynamic_cast< SwDrawModel* >(pSwDrawModel), "Wrong SdrModel type in SwDPage clone (!)");
    SwDPage* pClone = new SwDPage(
        *pSwDrawModel,
        IsMasterPage());
    OSL_ENSURE(pClone, "CloneSdrPage error (!)");
    pClone->copyDataFromSdrPage(*this);

    return pClone;
}

/*************************************************************************
|*
|*  SwDPage::ReplaceObjectInSdrObjList()
|*
|*  Ersterstellung      MA 07. Aug. 95
|*  Letzte Aenderung    MA 07. Aug. 95
|*
*************************************************************************/

SdrObject*  SwDPage::ReplaceObjectInSdrObjList( SdrObject& rNewObj, sal_uInt32 nObjNum )
{
    SdrObject *pOld = GetObj( nObjNum );
    ASSERT( pOld, "Oups, Object not replaced" );
    SwContact* pContact;
    if ( 0 != ( pContact = findConnectionToSdrObject(pOld) ) &&
         RES_DRAWFRMFMT == pContact->GetFmt()->Which())
        ((SwDrawContact*)pContact)->ChangeMasterObject( &rNewObj );

    // call parent
    return FmFormPage::ReplaceObjectInSdrObjList( rNewObj, nObjNum );
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
    pLst->Insert(
        SdrPageGridFrame(
            basegfx::B2DRange(aPaper.Left(), aPaper.Top(), aPaper.Right(), aPaper.Bottom()),
            basegfx::B2DRange(aUser.Left(), aUser.Top(), aUser.Right(), aUser.Bottom())));
}


const SdrPageGridFrameList*  SwDPage::GetGridFrameList(
    const SdrView& rSdrView, const Rectangle *pRect ) const
{
    ViewShell *pSh = static_cast< SwDrawModel& >(getSdrModelFromSdrPage()).GetDoc().GetCurrentViewShell();  //swmod 071108//swmod 071225
    if ( pSh )
    {
        while ( pSh->Imp()->GetDrawView() != &rSdrView )
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
        const Point aOutputPixel(pWindow->ScreenToOutputPixel(rEvt.GetMousePosPixel()));
        const basegfx::B2DPoint aB2DPos(pWindow->GetInverseViewTransformation() * basegfx::B2DPoint(aOutputPixel.X(), aOutputPixel.Y()));
        const Point aPos(basegfx::fround(aB2DPos.getX()), basegfx::fround(aB2DPos.getY()));

        SdrObject* pObj;
        if( pView->PickObj( aB2DPos, 0, pObj, SDRSEARCH_PICKMACRO ) &&
             dynamic_cast< SwVirtFlyDrawObj* >(pObj) )
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



