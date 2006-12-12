/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: futransf.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:26:10 $
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
#include "precompiled_sd.hxx"


#include "futransf.hxx"

#include <svx/dialogs.hrc>
#include <svx/polysc3d.hxx>
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif

#include "strings.hrc"
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

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
}

FunctionReference FuTransform::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuTransform( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuTransform::DoExecute( SfxRequest& rReq )
{
    if( mpView->AreObjectsMarked() )
    {
        const SfxItemSet* pArgs = rReq.GetArgs();

        if( !pArgs )
        {
            // --------- itemset for size and position --------
            SfxItemSet aSet( mpView->GetGeoAttrFromMarked() );

            const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            if( rMarkList.GetMarkCount() == 1 &&
                pObj->GetObjInventor() == SdrInventor &&
                pObj->GetObjIdentifier() == OBJ_CAPTION )
            {
                // --------- itemset for caption --------
                SfxItemSet aNewAttr( mpDoc->GetPool() );
                mpView->GetAttributes( aNewAttr );

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if ( pFact )
                {
                    std::auto_ptr< SfxAbstractTabDialog > pDlg( pFact->CreateCaptionDialog( NULL, mpView, ResId( RID_SVXDLG_CAPTION ) ) );

                    const USHORT* pRange = pDlg->GetInputRanges( *aNewAttr.GetPool() );
                    SfxItemSet aCombSet( *aNewAttr.GetPool(), pRange );
                    aCombSet.Put( aNewAttr );
                    aCombSet.Put( aSet );
                    pDlg->SetInputSet( &aCombSet );

                    if( pDlg.get() && (pDlg->Execute() == RET_OK) )
                    {
                        rReq.Done( *( pDlg->GetOutputItemSet() ) );
                        pArgs = rReq.GetArgs();
                    }
                }
            }
            else
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if(pFact)
                {
                    std::auto_ptr< SfxAbstractTabDialog > pDlg( pFact->CreateSvxTransformTabDialog( NULL, &aSet,mpView, ResId(RID_SVXDLG_TRANSFORM) ) );
                    if( pDlg.get() && (pDlg->Execute() == RET_OK) )
                    {
                        rReq.Done( *( pDlg->GetOutputItemSet() ) );
                        pArgs = rReq.GetArgs();
                    }
                }
            }
        }

        if( pArgs )
        {
            // Undo
            String aString( mpView->GetDescriptionOfMarkedObjects() );
            aString.Append( sal_Unicode(' ') );
            aString.Append( String( SdResId( STR_TRANSFORM ) ) );
            mpView->BegUndo( aString );

            mpView->SetGeoAttrToMarked( *pArgs );
            mpView->SetAttributes( *pArgs );

            /**********************************************************************
            * An der E3dView muss demnaechst SetGeoAttrToMarked() mit folgendem
            * Code ueberladen werden:
            **********************************************************************/
            const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
            ULONG nCount = rMarkList.GetMarkCount();

            for (ULONG nMark = 0; nMark < nCount; nMark++)
            {
                SdrObject* pObj = rMarkList.GetMark(nMark)->GetMarkedSdrObj();

                if (pObj->ISA(E3dPolyScene))
                {
                    ((E3dPolyScene*) pObj)->FitSnapRectToBoundVol();
                }
            }
            //********************************************************************

            mpView->EndUndo();
        }
    }
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
        case RP_MT: return( Point( aRect.Center().X(), aRect.Top() ) );
        case RP_RT: return( Point( aRect.Right(), aRect.Top() ) );
        case RP_LM: return( Point( aRect.Left(), aRect.Center().Y() ) );
        case RP_MM: return( Point( aRect.Center().X(), aRect.Center().Y() ) );
        case RP_RM: return( Point( aRect.Right(), aRect.Center().Y() ) );
        case RP_LB: return( Point( aRect.Left(), aRect.Bottom() ) );
        case RP_MB: return( Point( aRect.Center().X(), aRect.Bottom() ) );
        case RP_RB: return( Point( aRect.Right(), aRect.Bottom() ) );
    }
    return( Point ( 0, 0 ) ); // Sollte nicht vorkommen !
}


} // end of namespace sd
