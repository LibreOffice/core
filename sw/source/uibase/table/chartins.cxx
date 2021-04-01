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

#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <IDocumentChartDataProviderAccess.hxx>
#include <osl/diagnose.h>

#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <view.hxx>
#include <chartins.hxx>
#include <tablemgr.hxx>
#include <frmfmt.hxx>
#include <unochart.hxx>

#include <edtwin.hxx>

#include <cmdid.h>
#include <anchoredobject.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/propertysequence.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <svtools/dialogclosedlistener.hxx>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

Point SwGetChartDialogPos( const vcl::Window *pParentWin, const Size& rDialogSize, const tools::Rectangle& rLogicChart )
{
    // positioning code according to spec; similar to Calc fuins2.cxx
    Point aRet;

    OSL_ENSURE( pParentWin, "Window not found" );
    if (pParentWin)
    {
        tools::Rectangle aObjPixel = pParentWin->LogicToPixel( rLogicChart, pParentWin->GetMapMode() );
        tools::Rectangle aObjAbs( pParentWin->OutputToAbsoluteScreenPixel( aObjPixel.TopLeft() ),
                           pParentWin->OutputToAbsoluteScreenPixel( aObjPixel.BottomRight() ) );

        tools::Rectangle aDesktop = pParentWin->GetDesktopRectPixel();
        Size aSpace = pParentWin->LogicToPixel(Size(8, 12), MapMode(MapUnit::MapAppFont));

        bool bLayoutRTL = ::GetActiveView()->GetWrtShell().IsTableRightToLeft();
        bool bCenterHor = false;

        if ( aDesktop.Bottom() - aObjAbs.Bottom() >= rDialogSize.Height() + aSpace.Height() )
        {
            // first preference: below the chart
            aRet.setY( aObjAbs.Bottom() + aSpace.Height() );
            bCenterHor = true;
        }
        else if ( aObjAbs.Top() - aDesktop.Top() >= rDialogSize.Height() + aSpace.Height() )
        {
            // second preference: above the chart
            aRet.setY( aObjAbs.Top() - rDialogSize.Height() - aSpace.Height() );
            bCenterHor = true;
        }
        else
        {
            bool bFitLeft = ( aObjAbs.Left() - aDesktop.Left() >= rDialogSize.Width() + aSpace.Width() );
            bool bFitRight = ( aDesktop.Right() - aObjAbs.Right() >= rDialogSize.Width() + aSpace.Width() );

            if ( bFitLeft || bFitRight )
            {
                // if both fit, prefer right in RTL mode, left otherwise
                bool bPutRight = bFitRight && ( bLayoutRTL || !bFitLeft );
                if ( bPutRight )
                    aRet.setX( aObjAbs.Right() + aSpace.Width() );
                else
                    aRet.setX( aObjAbs.Left() - rDialogSize.Width() - aSpace.Width() );

                // center vertically
                aRet.setY( aObjAbs.Top() + ( aObjAbs.GetHeight() - rDialogSize.Height() ) / 2 );
            }
            else
            {
                // doesn't fit on any edge - put at the bottom of the screen
                aRet.setY( aDesktop.Bottom() - rDialogSize.Height() );
                bCenterHor = true;
            }
        }
        if ( bCenterHor )
            aRet.setX( aObjAbs.Left() + ( aObjAbs.GetWidth() - rDialogSize.Width() ) / 2 );

        // limit to screen (centering might lead to invalid positions)
        if ( aRet.X() + rDialogSize.Width() - 1 > aDesktop.Right() )
            aRet.setX( aDesktop.Right() - rDialogSize.Width() + 1 );
        if ( aRet.X() < aDesktop.Left() )
            aRet.setX( aDesktop.Left() );
        if ( aRet.Y() + rDialogSize.Height() - 1 > aDesktop.Bottom() )
            aRet.setY( aDesktop.Bottom() - rDialogSize.Height() + 1 );
        if ( aRet.Y() < aDesktop.Top() )
            aRet.setY( aDesktop.Top() );
    }

    return aRet;
}

SwInsertChart::SwInsertChart( const Link<css::ui::dialogs::DialogClosedEvent*, void>& rLink )
{
    SwView *pView = ::GetActiveView();

    // get range string of marked data
    SwWrtShell &rWrtShell = pView->GetWrtShell();
    uno::Reference< chart2::data::XDataProvider > xDataProvider;
    uno::Reference< frame::XModel > xChartModel;
    OUString aRangeString;

    if( rWrtShell.IsCursorInTable())
    {
        if (!rWrtShell.IsTableMode())
        {
            // select whole table
            rWrtShell.GetView().GetViewFrame()->GetDispatcher()->
                Execute(FN_TABLE_SELECT_ALL, SfxCallMode::SYNCHRON);
        }
        if( ! rWrtShell.IsTableComplexForChart())
        {
            SwFrameFormat* pTableFormat = rWrtShell.GetTableFormat();
            aRangeString = pTableFormat->GetName() + "." + rWrtShell.GetBoxNms();

            // get table data provider
            xDataProvider.set( pView->GetDocShell()->getIDocumentChartDataProviderAccess().GetChartDataProvider( true ) );
        }
    }

    SwFlyFrameFormat *pFlyFrameFormat = nullptr;
    xChartModel.set( SwTableFUNC( &rWrtShell ).InsertChart( xDataProvider, xDataProvider.is(), aRangeString, &pFlyFrameFormat ));

    //open wizard
    //@todo get context from writer if that has one
    uno::Reference< uno::XComponentContext > xContext(
        ::cppu::defaultBootstrap_InitialComponentContext() );
    if( !(xContext.is() && xChartModel.is() && xDataProvider.is()))
        return;

    uno::Reference< lang::XMultiComponentFactory > xMCF( xContext->getServiceManager() );
    if(!xMCF.is())
        return;

    uno::Reference< ui::dialogs::XAsynchronousExecutableDialog > xDialog(
        xMCF->createInstanceWithContext(
            "com.sun.star.comp.chart2.WizardDialog", xContext),
        uno::UNO_QUERY);
    uno::Reference< lang::XInitialization > xInit( xDialog, uno::UNO_QUERY );
    if( xInit.is() )
    {
        //  initialize dialog
        uno::Sequence<uno::Any> aSeq(comphelper::InitAnyPropertySequence(
        {
            {"ParentWindow", uno::Any(uno::Reference< awt::XWindow >())},
            {"ChartModel", uno::Any(xChartModel)}
        }));
        xInit->initialize( aSeq );

        // try to set the dialog's position so it doesn't hide the chart
        uno::Reference < beans::XPropertySet > xDialogProps( xDialog, uno::UNO_QUERY );
        if ( xDialogProps.is() )
        {
            try
            {
                //get dialog size:
                awt::Size aDialogAWTSize;
                if( xDialogProps->getPropertyValue("Size")
                    >>= aDialogAWTSize )
                {
                    Size aDialogSize( aDialogAWTSize.Width, aDialogAWTSize.Height );
                    if ( !aDialogSize.IsEmpty() )
                    {
                        //calculate and set new position
                        SwRect aSwRect;
                        if (pFlyFrameFormat)
                            aSwRect = pFlyFrameFormat->GetAnchoredObj()->GetObjRectWithSpaces();
                        tools::Rectangle aRect( aSwRect.SVRect() );
                        Point aDialogPos = SwGetChartDialogPos( &rWrtShell.GetView().GetEditWin(), aDialogSize, aRect );
                        xDialogProps->setPropertyValue("Position",
                            uno::makeAny( awt::Point(aDialogPos.getX(),aDialogPos.getY()) ) );
                    }
                }
            }
            catch (const uno::Exception&)
            {
                OSL_FAIL("Chart wizard couldn't be positioned automatically" );
            }
        }

        rtl::Reference<::svt::DialogClosedListener> pListener = new ::svt::DialogClosedListener();
        pListener->SetDialogClosedLink( rLink );

        xDialog->startExecuteModal( pListener );
    }
    else
    {
        uno::Reference< lang::XComponent > xComponent( xDialog, uno::UNO_QUERY );
        if( xComponent.is())
            xComponent->dispose();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
