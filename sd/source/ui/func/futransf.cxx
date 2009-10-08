/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: futransf.cxx,v $
 * $Revision: 1.12.114.1 $
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
#include "precompiled_sd.hxx"


#include "futransf.hxx"

#include <svx/dialogs.hrc>
#include <svx/polysc3d.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>

#include "strings.hrc"
#include "ViewShell.hxx"
#include "View.hxx"
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
                    std::auto_ptr< SfxAbstractTabDialog > pDlg( pFact->CreateCaptionDialog( NULL, mpView, RID_SVXDLG_CAPTION ) );

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
                    std::auto_ptr< SfxAbstractTabDialog > pDlg( pFact->CreateSvxTransformTabDialog( NULL, &aSet,mpView, RID_SVXDLG_TRANSFORM) );
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
