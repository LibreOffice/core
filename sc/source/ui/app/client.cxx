/*************************************************************************
 *
 *  $RCSfile: client.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 16:22:17 $
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

// INCLUDE ---------------------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif

#include <toolkit/helper/vclunohelper.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sot/sotref.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>
#include <svx/svdograf.hxx>
#include <svtools/embedhlp.hxx>

#include "client.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

ScClient::ScClient( ScTabViewShell* pViewShell, Window* pDraw, SdrModel* pSdrModel, SdrOle2Obj* pObj ) :
    SfxInPlaceClient( pViewShell, pDraw, pObj->GetAspect() ),
    pModel( pSdrModel ),
    pGrafEdit( 0 )
{
    SetObject( pObj->GetObjRef() );
}

__EXPORT ScClient::~ScClient()
{
}

SdrOle2Obj* ScClient::GetDrawObj()
{
    uno::Reference < embed::XEmbeddedObject > xObj = GetObject();
    SdrOle2Obj* pOle2Obj = NULL;
    String aName = GetViewShell()->GetObjectShell()->GetEmbeddedObjectContainer().GetEmbeddedObjectName( xObj );

    USHORT nPages = pModel->GetPageCount();
    for (USHORT nPNr=0; nPNr<nPages && !pOle2Obj; nPNr++)
    {
        SdrPage* pPage = pModel->GetPage(nPNr);
        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject && !pOle2Obj)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                // name from InfoObject is PersistName
                if ( ((SdrOle2Obj*)pObject)->GetPersistName() == aName )
                    pOle2Obj = (SdrOle2Obj*)pObject;
            }
            pObject = aIter.Next();
        }
    }
    return pOle2Obj;
}

void __EXPORT ScClient::RequestNewObjectArea( Rectangle& aLogicRect )
{
    SfxViewShell* pSfxViewSh = GetViewShell();
    ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pSfxViewSh );
    if (!pViewSh)
    {
        DBG_ERROR("Wrong ViewShell");
        return;
    }

    BOOL bChange = FALSE;
    USHORT nTab = pViewSh->GetViewData()->GetTabNo();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(static_cast<sal_Int16>(nTab)));
    if (pPage)
    {
        Point aPos;
        Size aSize = pPage->GetSize();
        if ( aSize.Width() < 0 )
        {
            aPos.X() = aSize.Width() + 1;       // negative
            aSize.Width() = -aSize.Width();     // positive
        }
        Rectangle aPageRect( aPos, aSize );

        if (aLogicRect.Right() > aPageRect.Right())
        {
            long nDiff = aLogicRect.Right() - aPageRect.Right();
            aLogicRect.Left() -= nDiff;
            aLogicRect.Right() -= nDiff;
            bChange = TRUE;
        }
        if (aLogicRect.Bottom() > aPageRect.Bottom())
        {
            long nDiff = aLogicRect.Bottom() - aPageRect.Bottom();
            aLogicRect.Top() -= nDiff;
            aLogicRect.Bottom() -= nDiff;
            bChange = TRUE;
        }

        if (aLogicRect.Left() < aPageRect.Left())
        {
            long nDiff = aLogicRect.Left() - aPageRect.Left();
            aLogicRect.Right() -= nDiff;
            aLogicRect.Left() -= nDiff;
            bChange = TRUE;
        }
        if (aLogicRect.Top() < aPageRect.Top())
        {
            long nDiff = aLogicRect.Top() - aPageRect.Top();
            aLogicRect.Bottom() -= nDiff;
            aLogicRect.Top() -= nDiff;
            bChange = TRUE;
        }
    }
}

void __EXPORT ScClient::ObjectAreaChanged()
{
    SfxViewShell* pSfxViewSh = GetViewShell();
    ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pSfxViewSh );
    if (!pViewSh)
    {
        DBG_ERROR("Wrong ViewShell");
        return;
    }

    //  Position und Groesse ins Dokument uebernehmen
    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pDrawObj)
    {
        pDrawObj->SetLogicRect( GetScaledObjArea() );

        //  set document modified (SdrModel::SetChanged is not used)
        // TODO/LATER: is there a reason that this code is not executed in Draw?
        SfxViewShell* pSfxViewSh = GetViewShell();
        ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pSfxViewSh );
        if (pViewSh)
            pViewSh->GetViewData()->GetDocShell()->SetDrawModified();
    }

    if (pDrawObj)
        pViewSh->ScrollToObject( pDrawObj );
}

void __EXPORT ScClient::ViewChanged()
{
    uno::Reference < embed::XEmbeddedObject > xObj = GetObject();

    // TODO/LEAN: working with Visual Area needs running state
    svt::EmbeddedObjectRef::TryRunningState( xObj );

    awt::Size aSz = xObj->getVisualAreaSize( GetAspect() );
    MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( GetAspect() ) );
    Size aVisSize = OutputDevice::LogicToLogic( Size( aSz.Width, aSz.Height ), aMapUnit, MAP_100TH_MM );

    //  Groesse ins Dokument uebernehmen
    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pDrawObj)
    {
        Rectangle aLogicRect = pDrawObj->GetLogicRect();
        Fraction aFractX = GetScaleWidth();
        Fraction aFractY = GetScaleHeight();
        aFractX *= aVisSize.Width();
        aFractY *= aVisSize.Height();
        aVisSize = Size( (long) aFractX, (long) aFractY );      // skaliert fuer Draw-Model

        //  pClientData->SetObjArea vor pDrawObj->SetLogicRect, damit keine
        //  falschen Skalierungen ausgerechnet werden:
        //Rectangle aObjArea = aLogicRect;
        //aObjArea.SetSize( aVisSize );          // Dokument-Groesse vom Server
        //SetObjArea( aObjArea );

        SfxViewShell* pSfxViewSh = GetViewShell();
        ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pSfxViewSh );
        if ( pViewSh )
        {
            Window* pWin = pViewSh->GetActiveWin();
            if ( pWin->LogicToPixel( aVisSize ) != pWin->LogicToPixel( aLogicRect.GetSize() ) )
            {
                aLogicRect.SetSize( aVisSize );
                pDrawObj->SetLogicRect( aLogicRect );

                //  set document modified (SdrModel::SetChanged is not used)
                pViewSh->GetViewData()->GetDocShell()->SetDrawModified();
            }
        }
    }
}

void __EXPORT ScClient::MakeVisible()
{
    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pDrawObj)
    {
        SfxViewShell* pSfxViewSh = GetViewShell();
        ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pSfxViewSh );
        if (pViewSh)
            pViewSh->ScrollToObject( pDrawObj );
    }
}

