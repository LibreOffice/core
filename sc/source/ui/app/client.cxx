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

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <sfx2/objsh.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoole2.hxx>

#include <client.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <gridwin.hxx>

using namespace com::sun::star;

ScClient::ScClient( ScTabViewShell* pViewShell, vcl::Window* pDraw, SdrModel* pSdrModel, const SdrOle2Obj* pObj ) :
    SfxInPlaceClient( pViewShell, pDraw, pObj->GetAspect() ),
    pModel( pSdrModel )
{
    SetObject( pObj->GetObjRef() );
}

ScClient::~ScClient()
{
}

SdrOle2Obj* ScClient::GetDrawObj()
{
    uno::Reference < embed::XEmbeddedObject > xObj = GetObject();
    SdrOle2Obj* pOle2Obj = nullptr;
    OUString aName = GetViewShell()->GetObjectShell()->GetEmbeddedObjectContainer().GetEmbeddedObjectName( xObj );

    sal_uInt16 nPages = pModel->GetPageCount();
    for (sal_uInt16 nPNr=0; nPNr<nPages && !pOle2Obj; nPNr++)
    {
        SdrPage* pPage = pModel->GetPage(nPNr);
        SdrObjListIter aIter( pPage, SdrIterMode::DeepNoGroups );
        SdrObject* pObject = aIter.Next();
        while (pObject && !pOle2Obj)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                // name from InfoObject is PersistName
                if ( static_cast<SdrOle2Obj*>(pObject)->GetPersistName() == aName )
                    pOle2Obj = static_cast<SdrOle2Obj*>(pObject);
            }
            pObject = aIter.Next();
        }
    }
    return pOle2Obj;
}

void ScClient::RequestNewObjectArea( tools::Rectangle& aLogicRect )
{
    SfxViewShell* pSfxViewSh = GetViewShell();
    ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>( pSfxViewSh  );
    if (!pViewSh)
    {
        OSL_FAIL("Wrong ViewShell");
        return;
    }

    tools::Rectangle aOldRect = GetObjArea();
    SdrOle2Obj*  pDrawObj = GetDrawObj();
    if ( pDrawObj )
    {
        if ( pDrawObj->IsResizeProtect() )
            aLogicRect.SetSize( aOldRect.GetSize() );

        if ( pDrawObj->IsMoveProtect() )
            aLogicRect.SetPos( aOldRect.TopLeft() );
    }

    sal_uInt16 nTab = pViewSh->GetViewData().GetTabNo();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(static_cast<sal_Int16>(nTab)));
    if ( pPage && aLogicRect != aOldRect )
    {
        Point aPos;
        Size aSize = pPage->GetSize();
        if ( aSize.Width() < 0 )
        {
            aPos.setX( aSize.Width() + 1 );       // negative
            aSize.setWidth( -aSize.Width() );     // positive
        }
        tools::Rectangle aPageRect( aPos, aSize );

        if (aLogicRect.Right() > aPageRect.Right())
        {
            long nDiff = aLogicRect.Right() - aPageRect.Right();
            aLogicRect.AdjustLeft( -nDiff );
            aLogicRect.AdjustRight( -nDiff );
        }
        if (aLogicRect.Bottom() > aPageRect.Bottom())
        {
            long nDiff = aLogicRect.Bottom() - aPageRect.Bottom();
            aLogicRect.AdjustTop( -nDiff );
            aLogicRect.AdjustBottom( -nDiff );
        }

        if (aLogicRect.Left() < aPageRect.Left())
        {
            long nDiff = aLogicRect.Left() - aPageRect.Left();
            aLogicRect.AdjustRight( -nDiff );
            aLogicRect.AdjustLeft( -nDiff );
        }
        if (aLogicRect.Top() < aPageRect.Top())
        {
            long nDiff = aLogicRect.Top() - aPageRect.Top();
            aLogicRect.AdjustBottom( -nDiff );
            aLogicRect.AdjustTop( -nDiff );
        }
    }
}

void ScClient::ObjectAreaChanged()
{
    SfxViewShell* pSfxViewSh = GetViewShell();
    ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>( pSfxViewSh  );
    if (!pViewSh)
    {
        OSL_FAIL("Wrong ViewShell");
        return;
    }

    // Take over position and size into document
    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pDrawObj)
    {
        tools::Rectangle aNewRectangle(GetScaledObjArea());

        // #i118524# if sheared/rotated, center to non-rotated LogicRect
        pDrawObj->setSuppressSetVisAreaSize(true);

        if(pDrawObj->GetGeoStat().nRotationAngle || pDrawObj->GetGeoStat().nShearAngle)
        {
            pDrawObj->SetLogicRect( aNewRectangle );

            const tools::Rectangle& rBoundRect = pDrawObj->GetCurrentBoundRect();
            const Point aDelta(aNewRectangle.Center() - rBoundRect.Center());

            aNewRectangle.Move(aDelta.X(), aDelta.Y());
        }

        pDrawObj->SetLogicRect( aNewRectangle );
        pDrawObj->setSuppressSetVisAreaSize(false);

        //  set document modified (SdrModel::SetChanged is not used)
        pViewSh->GetViewData().GetDocShell()->SetDrawModified();
        pViewSh->ScrollToObject(pDrawObj);
    }
}

void ScClient::ViewChanged()
{
    if ( GetAspect() == embed::Aspects::MSOLE_ICON )
    {
        // the iconified object seems not to need such a scaling handling
        // since the replacement image and the size a completely controlled by the container
        // TODO/LATER: when the icon exchange is implemented the scaling handling might be required again here

        return;
    }

    uno::Reference < embed::XEmbeddedObject > xObj = GetObject();

    // TODO/LEAN: working with Visual Area can switch object to running state
    awt::Size aSz;
    try {
        aSz = xObj->getVisualAreaSize( GetAspect() );
    } catch ( embed::NoVisualAreaSizeException& )
    {
        OSL_FAIL("The visual area size must be available!");
    }

    MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( GetAspect() ) );
    Size aVisSize = OutputDevice::LogicToLogic(Size(aSz.Width, aSz.Height), MapMode(aMapUnit), MapMode(MapUnit::Map100thMM));

    // Take over position and size into document
    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pDrawObj)
    {
        tools::Rectangle aLogicRect = pDrawObj->GetLogicRect();
        Fraction aFractX = GetScaleWidth() * aVisSize.Width();
        Fraction aFractY = GetScaleHeight() * aVisSize.Height();
        aVisSize = Size( static_cast<long>(aFractX), static_cast<long>(aFractY) ); // Scaled for Draw model

        //  pClientData->SetObjArea before pDrawObj->SetLogicRect, so that we don't
        //  calculate wrong scalings:
        //Rectangle aObjArea = aLogicRect;
        //aObjArea.SetSize( aVisSize );          // Document size from the server
        //SetObjArea( aObjArea );

        SfxViewShell* pSfxViewSh = GetViewShell();
        ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>( pSfxViewSh  );
        if ( pViewSh )
        {
            vcl::Window* pWin = pViewSh->GetActiveWin();
            if ( pWin->LogicToPixel( aVisSize ) != pWin->LogicToPixel( aLogicRect.GetSize() ) )
            {
                aLogicRect.SetSize( aVisSize );
                pDrawObj->SetLogicRect( aLogicRect );

                // set document modified (SdrModel::SetChanged is not used)
                pViewSh->GetViewData().GetDocShell()->SetDrawModified();
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
