/*************************************************************************
 *
 *  $RCSfile: dpage.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-20 13:44:04 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _BASMGR_HXX
#include <basic/basmgr.hxx>
#endif
#ifndef _GOODIES_IMAPOBJ_HXX
#include <svtools/imapobj.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
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
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
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


SwDPage::SwDPage(SwDrawDocument& rNewModel, BOOL bMasterPage) :
    FmFormPage(rNewModel, 0, bMasterPage),
    pGridLst( 0 )
{
}


SwDPage::~SwDPage()
{
    delete pGridLst;
}


Point  SwDPage::GetOffset() const
{
    return Point( DOCUMENTBORDER, DOCUMENTBORDER );
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

/*************************************************************************
|*
|*  String SwDPage::GetLinkData( const String& )
|*  void SwDPage::SetLinkData( const String&, const String& )
|*  void SwDPage::UpdateLinkData( const String&, const String& )
|*
|*  Ersterstellung      JP 04.09.95
|*  Letzte Aenderung    JP 04.09.95
|*
*************************************************************************/

String SwDPage::GetLinkData( const String& rLinkName )
{
    SwDoc& rDoc = ((SwDrawDocument*)GetModel())->GetDoc();
    SwFieldType* pFTyp = rDoc.GetFldType( RES_USERFLD, rLinkName );
    if( pFTyp )
        return ((SwUserFieldType*)pFTyp)->GetContent();
    return aEmptyStr;
}


void  SwDPage::SetLinkData( const String& rLinkName,
                                    const String& rLinkData )
{
    SwDoc& rDoc = ((SwDrawDocument*)GetModel())->GetDoc();
    SwFieldType* pFTyp = rDoc.GetFldType( RES_USERFLD, rLinkName );
    if( pFTyp )
        ((SwUserFieldType*)pFTyp)->CtrlSetContent( rLinkData );
}


void  SwDPage::RequestBasic()
{
    SwDoc& rDoc = ((SwDrawDocument*)GetModel())->GetDoc();
    if( rDoc.GetDocShell() )
    {
        BasicManager *pBasicMgr = rDoc.GetDocShell()->GetBasicManager();
        ASSERT( pBasicMgr, "wo ist mein BasicManager" )
        SetBasic( pBasicMgr->GetLib( 0 ) );
    }
    else
        ASSERT( !this, "wo ist meine DocShell" )
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
             pObj->IsWriterFlyFrame() )
        {
            SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
            const SwFmtURL &rURL = pFly->GetFmt()->GetURL();
            String sTxt;
            if( rURL.GetMap() )
            {
                IMapObject *pObj = pFly->GetFmt()->GetIMapObject( aPos, pFly );
                if( pObj )
                {
                    sTxt = pObj->GetDescription();
                    if ( !sTxt.Len() )
                        sTxt = URIHelper::removePassword( pObj->GetURL(),
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
                    aPt = (Point&)(Size&)pWindow->LogicToPixel(
                            (Size&)aPt, MapMode( MAP_TWIP ) );
                    ((( sTxt += '?' ) += String::CreateFromInt32( aPt.X() ))
                             += ',' ) += String::CreateFromInt32( aPt.Y() );
                }
            }

            if ( sTxt.Len() )
            {
                if( rEvt.GetMode() & HELPMODE_QUICK )
                {
                    // dann zeige die Hilfe mal an:
                    Rectangle aRect( rEvt.GetMousePosPixel(), Size(1,1) );
/*
Bug 29593: QuickHelp immer an der MausPosition anzeigen (besonders unter OS/2)

                    Rectangle aRect( pObj->GetSnapRect() );
                    Point aPt( pWindow->OutputToScreenPixel( pWindow->LogicToPixel( aRect.TopLeft() )));
                    aRect.Left()   = aPt.X();
                    aRect.Top()    = aPt.Y();
                    aPt = pWindow->OutputToScreenPixel( pWindow->LogicToPixel( aRect.BottomRight() ));
                    aRect.Right()  = aPt.X();
                    aRect.Bottom() = aPt.Y();
*/
                    Help::ShowQuickHelp( pWindow, aRect, sTxt );
                }
                else
                    Help::ShowBalloon( pWindow, rEvt.GetMousePosPixel(), sTxt );
                bWeiter = FALSE;
            }
        }
    }

    if( bWeiter )
        bWeiter = !FmFormPage::RequestHelp( pWindow, pView, rEvt );

    return bWeiter;
}



