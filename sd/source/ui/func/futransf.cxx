/*************************************************************************
 *
 *  $RCSfile: futransf.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:09:05 $
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

#pragma hdrstop

#include "futransf.hxx"

#include <svx/dialogs.hrc>
//#include <svx/labdlg.hxx> delete by CHINA001
#include <svx/polysc3d.hxx>
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif

//CHINA001 #ifndef _SVX_TRANSFRM_HXX //autogen
//CHINA001 #include <svx/transfrm.hxx>
//CHINA001 #endif

#include "strings.hrc"
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#include "sdresid.hxx"
#include "drawdoc.hxx"
//add header of cui CHINA001
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001

namespace sd {

TYPEINIT1( FuTransform, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuTransform::FuTransform(ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
                         SdDrawDocument* pDoc, SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    if( pView->AreObjectsMarked() )
    /*  ( pView->IsResizeAllowed() ||
          pView->IsMoveAllowed() ||
          pView->IsRotateAllowed() ) ) */
    {
        // Undo
        String aString( pView->GetDescriptionOfMarkedObjects() );
        aString.Append( sal_Unicode(' ') );
        aString.Append( String( SdResId( STR_TRANSFORM ) ) );
        pView->BegUndo( aString );

        const SfxItemSet* pArgs = rReq.GetArgs();

        if( !pArgs )
        {
            // --------- Itemset fuer Groesse und Position --------
            SfxItemSet aSet( pView->GetGeoAttrFromMarked() );

            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
            SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
            if( rMarkList.GetMarkCount() == 1 &&
                pObj->GetObjInventor() == SdrInventor &&
                pObj->GetObjIdentifier() == OBJ_CAPTION )
            {
                // --------- Itemset fuer Caption --------
                SfxItemSet aNewAttr( pDoc->GetPool() );
                pView->GetAttributes( aNewAttr );

                //SvxCaptionTabDialog* pDlg = new SvxCaptionTabDialog( NULL, pView);
                //change for cui CHINA001
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if ( pFact )
                {
                    SfxAbstractTabDialog *pDlg = pFact->CreateCaptionDialog( NULL, pView, ResId( RID_SVXDLG_CAPTION ) );

                    const USHORT* pRange = pDlg->GetInputRanges( *aNewAttr.GetPool() );
                    SfxItemSet aCombSet( *aNewAttr.GetPool(), pRange );
                    aCombSet.Put( aNewAttr );
                    aCombSet.Put( aSet );
                    pDlg->SetInputSet( &aCombSet );

                    if( pDlg->Execute() == RET_OK )
                    {
                        rReq.Done( *( pDlg->GetOutputItemSet() ) );
                        pArgs = rReq.GetArgs();
                    }
                    else
                    {
                        delete pDlg;
                        pView->EndUndo();
                        return; // Abbruch
                    }
                    delete( pDlg );
                } //change for cui
            }
            else
            {
                //CHINA001 SvxTransformTabDialog* pDlg = new SvxTransformTabDialog( NULL, &aSet, pView );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    SfxAbstractTabDialog* pDlg = pFact->CreateSvxTransformTabDialog( NULL, &aSet,pView, ResId(RID_SVXDLG_TRANSFORM) );
                    DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
                    if( pDlg->Execute() == RET_OK )
                    {
                        rReq.Done( *( pDlg->GetOutputItemSet() ) );
                        pArgs = rReq.GetArgs();
                    }
                    else
                    {
                        delete pDlg;
                        pView->EndUndo();
                        return; // Abbruch
                    }
                    delete( pDlg );
                }
            }
        }
        pView->SetGeoAttrToMarked( *pArgs );
        pView->SetAttributes( *pArgs );

        /**********************************************************************
        * An der E3dView muss demnaechst SetGeoAttrToMarked() mit folgendem
        * Code ueberladen werden:
        **********************************************************************/
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        ULONG nCount = rMarkList.GetMarkCount();

        for (ULONG nMark = 0; nMark < nCount; nMark++)
        {
            SdrObject* pObj = rMarkList.GetMark(nMark)->GetObj();

            if (pObj->ISA(E3dPolyScene))
            {
                ((E3dPolyScene*) pObj)->FitSnapRectToBoundVol();
            }
        }
        //********************************************************************


        pView->EndUndo();
    }
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuTransform::Activate()
{
    FuPoor::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuTransform::Deactivate()
{
    FuPoor::Deactivate();
}


/*************************************************************************
|*
|* Function ResizeObject
|*
\************************************************************************/

Point FuTransform::GetPoint( Rectangle aRect, RECT_POINT eRP )
{

    switch( eRP )
    {
        case RP_LT: return( Point( aRect.Left(), aRect.Top() ) );
            break;
        case RP_MT: return( Point( aRect.Center().X(), aRect.Top() ) );
            break;
        case RP_RT: return( Point( aRect.Right(), aRect.Top() ) );
            break;
        case RP_LM: return( Point( aRect.Left(), aRect.Center().Y() ) );
            break;
        case RP_MM: return( Point( aRect.Center().X(), aRect.Center().Y() ) );
            break;
        case RP_RM: return( Point( aRect.Right(), aRect.Center().Y() ) );
            break;
        case RP_LB: return( Point( aRect.Left(), aRect.Bottom() ) );
            break;
        case RP_MB: return( Point( aRect.Center().X(), aRect.Bottom() ) );
            break;
        case RP_RB: return( Point( aRect.Right(), aRect.Bottom() ) );
            break;
    }
    return( Point ( 0, 0 ) ); // Sollte nicht vorkommen !
}


} // end of namespace sd
