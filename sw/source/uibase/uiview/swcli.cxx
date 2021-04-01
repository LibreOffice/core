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

#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <wrtsh.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <view.hxx>
#include <edtwin.hxx>
#include <swcli.hxx>
#include <svtools/embedhlp.hxx>
#include <osl/diagnose.h>

#include <toolkit/helper/vclunohelper.hxx>

using namespace com::sun::star;

SwOleClient::SwOleClient(SwView *pView, SwEditWin *pWin, const svt::EmbeddedObjectRef& xObj)
    : SfxInPlaceClient( pView, pWin, xObj.GetViewAspect() )
    , m_IsInDoVerb(false)
    , m_IsOldCheckForOLEInCaption(pView->GetWrtShell().IsCheckForOLEInCaption())
{
    SetObject( xObj.GetObject() );
}

void SwOleClient::RequestNewObjectArea( tools::Rectangle& aLogRect )
{
    // The server wants to change the client size.
    // We put the desired size in the core. The attributes of the frame
    // are set to the desired value. This value will be passed on to the
    // InPlaceClient.
    // The core accepts or formats the adjusted values not necessarily.
    // If the Ole-Frame is formatted, then the CalcAndSetScale() of the WrtShell
    // will be called. There the scaling of the SwOleClient is set if necessary.

    SwWrtShell &rSh  = static_cast<SwView*>(GetViewShell())->GetWrtShell();

    rSh.StartAllAction();

    // the aLogRect will get the preliminary size now
    aLogRect.SetSize( rSh.RequestObjectResize( SwRect( aLogRect ), GetObject() ) );

    // the EndAllAction() call will trigger CalcAndSetScale() call,
    // so the embedded object must get the correct size before
    if ( aLogRect.GetSize() != GetScaledObjArea().GetSize() )
    {
        // size has changed, so first change visual area of the object before we resize its view
        // without this the object always would be scaled - now it has the choice

        // TODO/LEAN: getMapUnit can switch object to running state
        MapMode aObjectMap( VCLUnoHelper::UnoEmbed2VCLMapUnit( GetObject()->getMapUnit( GetAspect() ) ) );
        MapMode aClientMap( GetEditWin()->GetMapMode().GetMapUnit() );

        Size aNewObjSize( tools::Long( aLogRect.GetWidth() / GetScaleWidth() ),
                          tools::Long( aLogRect.GetHeight() / GetScaleHeight() ) );

        // convert to logical coordinates of the embedded object
        Size aNewSize = GetEditWin()->LogicToLogic( aNewObjSize, &aClientMap, &aObjectMap );
        GetObject()->setVisualAreaSize( GetAspect(), awt::Size( aNewSize.Width(), aNewSize.Height() ) );
    }

    rSh.EndAllAction();

    SwRect aFrame( rSh.GetAnyCurRect( CurRectType::FlyEmbedded,     nullptr, GetObject() )),
           aPrt( rSh.GetAnyCurRect( CurRectType::FlyEmbeddedPrt, nullptr, GetObject() ));
    aLogRect.SetPos( aPrt.Pos() + aFrame.Pos() );
    aLogRect.SetSize( aPrt.SSize() );
}

void SwOleClient::ObjectAreaChanged()
{
    SwWrtShell &rSh  = static_cast<SwView*>(GetViewShell())->GetWrtShell();
    SwRect aFrame( rSh.GetAnyCurRect( CurRectType::FlyEmbedded,     nullptr, GetObject() ));
    if ( !aFrame.IsOver( rSh.VisArea() ) )
        rSh.MakeVisible( aFrame );
}

void SwOleClient::ViewChanged()
{
    if (m_IsInDoVerb)
        return;

    if ( GetAspect() == embed::Aspects::MSOLE_ICON )
    {
        // the iconified object seems not to need such a scaling handling
        // since the replacement image and the size a completely controlled by the container
        // TODO/LATER: when the icon exchange is implemented the scaling handling
        //             might be required again here
        return;
    }

    SwWrtShell &rSh  = static_cast<SwView*>(GetViewShell())->GetWrtShell();

    // Adjust the size of the object in the core. The Scaling must
    // be considered. Repercussions on the object are considered by
    // CalcAndSetScale() of the WrtShell if the size / position of
    // the frame in the core changes.

    // TODO/LEAN: getMapUnit can switch object to running state
    awt::Size aSz;
    try
    {
        aSz = GetObject()->getVisualAreaSize( GetAspect() );
    }
    catch (const embed::NoVisualAreaSizeException&)
    {
        // Nothing will be done
    }
    catch (const uno::Exception&)
    {
        OSL_FAIL( "Something goes wrong on requesting object size!" );
    }

    Size aVisSize( aSz.Width, aSz.Height );

    // As long as from the object comes no reasonable size
    // nothing can be scaled.
    if( !aVisSize.Width() || !aVisSize.Height() )
        return;

    // first convert to TWIPS before scaling, because scaling factors are calculated for
    // the TWIPS mapping and so they will produce the best results if applied to TWIPS based
    // coordinates
    const MapMode aMyMap ( MapUnit::MapTwip );
    const MapMode aObjMap( VCLUnoHelper::UnoEmbed2VCLMapUnit( GetObject()->getMapUnit( GetAspect() ) ) );
    aVisSize = OutputDevice::LogicToLogic( aVisSize, aObjMap, aMyMap );

    aVisSize.setWidth( tools::Long(aVisSize.Width() * GetScaleWidth()) );
    aVisSize.setHeight( tools::Long(aVisSize.Height() * GetScaleHeight()) );

    SwRect aRect( Point( LONG_MIN, LONG_MIN ), aVisSize );
    rSh.LockView( true );   // Prevent scrolling in the EndAction
    rSh.StartAllAction();
    rSh.RequestObjectResize( aRect, GetObject() );
    rSh.EndAllAction();
    rSh.LockView( false );
}

void SwOleClient::FormatChanged()
{
    const uno::Reference < embed::XEmbeddedObject >& xObj( GetObject() );
    SwView * pView = dynamic_cast< SwView * >( GetViewShell() );
    if ( pView && xObj.is() && SotExchange::IsMath( xObj->getClassID() ) )
    {
        SwWrtShell & rWrtSh = pView->GetWrtShell();
        if (rWrtSh.GetDoc()->getIDocumentSettingAccess().get( DocumentSettingId::MATH_BASELINE_ALIGNMENT ))
            rWrtSh.AlignFormulaToBaseline( xObj );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
