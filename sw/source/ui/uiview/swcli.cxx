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
#include <wrtsh.hxx>
#include <doc.hxx>
#include <swtypes.hxx>
#include <view.hxx>
#include <edtwin.hxx>
#include <swcli.hxx>
#include <cmdid.h>
#include <cfgitems.hxx>

#include <toolkit/helper/vclunohelper.hxx>

using namespace com::sun::star;

SwOleClient::SwOleClient( SwView *pView, SwEditWin *pWin, const svt::EmbeddedObjectRef& xObj ) :
    SfxInPlaceClient( pView, pWin, xObj.GetViewAspect() ), bInDoVerb( sal_False ),
    bOldCheckForOLEInCaption( pView->GetWrtShell().IsCheckForOLEInCaption() )
{
    SetObject( xObj.GetObject() );
}

void SwOleClient::RequestNewObjectArea( Rectangle& aLogRect )
{
    //Der Server moechte die Clientgrosse verandern.
    //Wir stecken die Wunschgroesse in die Core. Die Attribute des Rahmens
    //werden auf den Wunschwert eingestellt. Dieser Wert wird also auch an
    //den InPlaceClient weitergegeben.
    //Die Core aktzeptiert bzw. formatiert die eingestellten Werte nicht
    //zwangslaeufig. Wenn der Ole-Frm formatiert wurde wird das CalcAndSetScale()
    //der WrtShell gerufen. Dort wird ggf. die Scalierung des SwOleClient
    //eingestellt.

    SwWrtShell &rSh  = ((SwView*)GetViewShell())->GetWrtShell();

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

        Size aNewObjSize( Fraction( aLogRect.GetWidth() ) / GetScaleWidth(),
                          Fraction( aLogRect.GetHeight() ) / GetScaleHeight() );

        // convert to logical coordinates of the embedded object
        Size aNewSize = GetEditWin()->LogicToLogic( aNewObjSize, &aClientMap, &aObjectMap );
        GetObject()->setVisualAreaSize( GetAspect(), awt::Size( aNewSize.Width(), aNewSize.Height() ) );
    }

    rSh.EndAllAction();

    SwRect aFrm( rSh.GetAnyCurRect( RECT_FLY_EMBEDDED,     0, GetObject() )),
           aPrt( rSh.GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, 0, GetObject() ));
    aLogRect.SetPos( aPrt.Pos() + aFrm.Pos() );
    aLogRect.SetSize( aPrt.SSize() );
}

void SwOleClient::ObjectAreaChanged()
{
    SwWrtShell &rSh  = ((SwView*)GetViewShell())->GetWrtShell();
    SwRect aFrm( rSh.GetAnyCurRect( RECT_FLY_EMBEDDED,     0, GetObject() ));
    if ( !aFrm.IsOver( rSh.VisArea() ) )
        rSh.MakeVisible( aFrm );
}

void SwOleClient::ViewChanged()
{
    if ( bInDoVerb )
        return;

    if ( GetAspect() == embed::Aspects::MSOLE_ICON )
    {
        // the iconified object seems not to need such a scaling handling
        // since the replacement image and the size a completely controlled by the container
        // TODO/LATER: when the icon exchange is implemented the scaling handling might be required again here
        return;
    }

    SwWrtShell &rSh  = ((SwView*)GetViewShell())->GetWrtShell();

    //Einstellen der Groesse des Objektes in der Core. Die Scalierung muss
    //beruecksichtigt werden. Rueckwirkung auf das Objekt werden von
    //CalcAndSetScale() der WrtShell beruecksichtig, wenn die Groesse/Pos des
    //Rahmens in der Core sich veraendert.

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
        // this is an error
        OSL_FAIL( "Something goes wrong on requesting object size!\n" );
    }

    Size aVisSize( aSz.Width, aSz.Height );

    // solange keine vernuenftige Size vom Object kommt,
    // kann nichts skaliert werden
    if( !aVisSize.Width() || !aVisSize.Height() )
        return;

    // first convert to TWIPS before scaling, because scaling factors are calculated for
    // the TWIPS mapping and so they will produce the best results if applied to TWIPS based
    // coordinates
    const MapMode aMyMap ( MAP_TWIP );
    const MapMode aObjMap( VCLUnoHelper::UnoEmbed2VCLMapUnit( GetObject()->getMapUnit( GetAspect() ) ) );
    aVisSize = OutputDevice::LogicToLogic( aVisSize, aObjMap, aMyMap );

    aVisSize.Width() = Fraction( aVisSize.Width()  ) * GetScaleWidth();
    aVisSize.Height()= Fraction( aVisSize.Height() ) * GetScaleHeight();

    SwRect aRect( Point( LONG_MIN, LONG_MIN ), aVisSize );
    rSh.LockView( sal_True );   //Scrollen im EndAction verhindern
    rSh.StartAllAction();
    rSh.RequestObjectResize( aRect, GetObject() );
    rSh.EndAllAction();
    rSh.LockView( sal_False );
}

void SwOleClient::MakeVisible()
{
    const SwWrtShell &rSh  = ((SwView*)GetViewShell())->GetWrtShell();
    rSh.MakeObjVisible( GetObject() );
}

void SwOleClient::FormatChanged()
{
    const uno::Reference < embed::XEmbeddedObject >& xObj( GetObject() );
    SwView * pView = dynamic_cast< SwView * >( GetViewShell() );
    if ( pView && xObj.is() && SotExchange::IsMath( xObj->getClassID() ) )
    {
        SwWrtShell & rWrtSh = pView->GetWrtShell();
        if (rWrtSh.GetDoc()->get( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT ))
            rWrtSh.AlignFormulaToBaseline( xObj );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
