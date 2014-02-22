/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    
    
    
    
    
    
    

    SwWrtShell &rSh  = ((SwView*)GetViewShell())->GetWrtShell();

    rSh.StartAllAction();

    
    aLogRect.SetSize( rSh.RequestObjectResize( SwRect( aLogRect ), GetObject() ) );

    
    
    if ( aLogRect.GetSize() != GetScaledObjArea().GetSize() )
    {
        
        

        
        MapMode aObjectMap( VCLUnoHelper::UnoEmbed2VCLMapUnit( GetObject()->getMapUnit( GetAspect() ) ) );
        MapMode aClientMap( GetEditWin()->GetMapMode().GetMapUnit() );

        Size aNewObjSize( Fraction( aLogRect.GetWidth() ) / GetScaleWidth(),
                          Fraction( aLogRect.GetHeight() ) / GetScaleHeight() );

        
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
        
        
        
        
        return;
    }

    SwWrtShell &rSh  = ((SwView*)GetViewShell())->GetWrtShell();

    
    
    
    

    
    awt::Size aSz;
    try
    {
        aSz = GetObject()->getVisualAreaSize( GetAspect() );
    }
    catch (const embed::NoVisualAreaSizeException&)
    {
        
    }
    catch (const uno::Exception&)
    {
        OSL_FAIL( "Something goes wrong on requesting object size!\n" );
    }

    Size aVisSize( aSz.Width, aSz.Height );

    
    
    if( !aVisSize.Width() || !aVisSize.Height() )
        return;

    
    
    
    const MapMode aMyMap ( MAP_TWIP );
    const MapMode aObjMap( VCLUnoHelper::UnoEmbed2VCLMapUnit( GetObject()->getMapUnit( GetAspect() ) ) );
    aVisSize = OutputDevice::LogicToLogic( aVisSize, aObjMap, aMyMap );

    aVisSize.Width() = Fraction( aVisSize.Width()  ) * GetScaleWidth();
    aVisSize.Height()= Fraction( aVisSize.Height() ) * GetScaleHeight();

    SwRect aRect( Point( LONG_MIN, LONG_MIN ), aVisSize );
    rSh.LockView( sal_True );   
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
