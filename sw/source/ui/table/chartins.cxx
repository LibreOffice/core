/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_sw.hxx"


#include <sfx2/viewfrm.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/basedlgs.hxx>
#include <IDocumentUndoRedo.hxx>

#include <sfx2/app.hxx>
#include <swtypes.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <view.hxx>
#include <chartins.hxx>
#include <tablemgr.hxx>
#include <frmfmt.hxx>
#include <swtable.hxx>
#include <tblsel.hxx>
#include <unochart.hxx>
#include <autoedit.hxx>
#include <doc.hxx>

#include <edtwin.hxx>

#include <cmdid.h>
#include <chartins.hrc>
#include <anchoredobject.hxx>

#include <sot/clsids.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/component_context.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;



Point SwGetChartDialogPos( const Window *pParentWin, const Size& rDialogSize, const Rectangle& rLogicChart )
{
    // !! positioning code according to spepc; similar to Calc fuins2.cxx

    Point aRet;

    OSL_ENSURE( pParentWin, "Window not found" );
    if (pParentWin)
    {
        Rectangle aObjPixel = pParentWin->LogicToPixel( rLogicChart, pParentWin->GetMapMode() );
        Rectangle aObjAbs( pParentWin->OutputToAbsoluteScreenPixel( aObjPixel.TopLeft() ),
                           pParentWin->OutputToAbsoluteScreenPixel( aObjPixel.BottomRight() ) );

        Rectangle aDesktop = pParentWin->GetDesktopRectPixel();
        Size aSpace = pParentWin->LogicToPixel( Size( 8, 12 ), MAP_APPFONT );

        sal_Bool bLayoutRTL = ::GetActiveView()->GetWrtShell().IsTableRightToLeft();
        bool bCenterHor = false;

        if ( aDesktop.Bottom() - aObjAbs.Bottom() >= rDialogSize.Height() + aSpace.Height() )
        {
            // first preference: below the chart
            aRet.Y() = aObjAbs.Bottom() + aSpace.Height();
            bCenterHor = true;
        }
        else if ( aObjAbs.Top() - aDesktop.Top() >= rDialogSize.Height() + aSpace.Height() )
        {
            // second preference: above the chart
            aRet.Y() = aObjAbs.Top() - rDialogSize.Height() - aSpace.Height();
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
                    aRet.X() = aObjAbs.Right() + aSpace.Width();
                else
                    aRet.X() = aObjAbs.Left() - rDialogSize.Width() - aSpace.Width();

                // center vertically
                aRet.Y() = aObjAbs.Top() + ( aObjAbs.GetHeight() - rDialogSize.Height() ) / 2;
            }
            else
            {
                // doesn't fit on any edge - put at the bottom of the screen
                aRet.Y() = aDesktop.Bottom() - rDialogSize.Height();
                bCenterHor = true;
            }
        }
        if ( bCenterHor )
            aRet.X() = aObjAbs.Left() + ( aObjAbs.GetWidth() - rDialogSize.Width() ) / 2;

        // limit to screen (centering might lead to invalid positions)
        if ( aRet.X() + rDialogSize.Width() - 1 > aDesktop.Right() )
            aRet.X() = aDesktop.Right() - rDialogSize.Width() + 1;
        if ( aRet.X() < aDesktop.Left() )
            aRet.X() = aDesktop.Left();
        if ( aRet.Y() + rDialogSize.Height() - 1 > aDesktop.Bottom() )
            aRet.Y() = aDesktop.Bottom() - rDialogSize.Height() + 1;
        if ( aRet.Y() < aDesktop.Top() )
            aRet.Y() = aDesktop.Top();
    }

    return aRet;
}

void SwInsertChart(Window* pParent, SfxBindings* pBindings )
{
    (void) pParent;
    (void) pBindings;
    SwView *pView = ::GetActiveView();

    // get range string of marked data
    SwWrtShell &rWrtShell = pView->GetWrtShell();
    uno::Reference< chart2::data::XDataProvider > xDataProvider;
    uno::Reference< frame::XModel > xChartModel;
    OUString aRangeString;

    if( rWrtShell.IsCrsrInTbl())
    {
        if (!rWrtShell.IsTableMode())
        {
            // select whole table
            rWrtShell.GetView().GetViewFrame()->GetDispatcher()->
                Execute(FN_TABLE_SELECT_ALL, SFX_CALLMODE_SYNCHRON);
        }
        if( ! rWrtShell.IsTblComplexForChart())
        {
            SwFrmFmt* pTblFmt = rWrtShell.GetTableFmt();
            String aCurrentTblName = pTblFmt->GetName();
            aRangeString = aCurrentTblName;
            aRangeString += OUString::valueOf( sal_Unicode('.') );
            aRangeString += rWrtShell.GetBoxNms();

            // get table data provider
            xDataProvider.set( pView->GetDocShell()->getIDocumentChartDataProviderAccess()->GetChartDataProvider( true ) );
        }
    }

    SwFlyFrmFmt *pFlyFrmFmt = 0;
    xChartModel.set( SwTableFUNC( &rWrtShell, sal_False ).InsertChart( xDataProvider, (sal_True == xDataProvider.is()), aRangeString, &pFlyFrmFmt ));

    //open wizard
    //@todo get context from writer if that has one
    uno::Reference< uno::XComponentContext > xContext(
        ::cppu::defaultBootstrap_InitialComponentContext() );
    if( xContext.is() && xChartModel.is() && xDataProvider.is())
    {
        uno::Reference< lang::XMultiComponentFactory > xMCF( xContext->getServiceManager() );
        if(xMCF.is())
        {
            uno::Reference< ui::dialogs::XExecutableDialog > xDialog(
                xMCF->createInstanceWithContext(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.chart2.WizardDialog"))
                    , xContext), uno::UNO_QUERY);
            uno::Reference< lang::XInitialization > xInit( xDialog, uno::UNO_QUERY );
            if( xInit.is() )
            {
                uno::Reference< awt::XWindow > xDialogParentWindow(0);
                //  initialize dialog
                uno::Sequence<uno::Any> aSeq(2);
                uno::Any* pArray = aSeq.getArray();
                beans::PropertyValue aParam1;
                aParam1.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParentWindow"));
                aParam1.Value <<= uno::makeAny(xDialogParentWindow);
                beans::PropertyValue aParam2;
                aParam2.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ChartModel"));
                aParam2.Value <<= uno::makeAny(xChartModel);
                pArray[0] <<= uno::makeAny(aParam1);
                pArray[1] <<= uno::makeAny(aParam2);
                xInit->initialize( aSeq );

                // try to set the dialog's position so it doesn't hide the chart
                uno::Reference < beans::XPropertySet > xDialogProps( xDialog, uno::UNO_QUERY );
                if ( xDialogProps.is() )
                {
                    try
                    {
                        //get dialog size:
                        awt::Size aDialogAWTSize;
                        if( xDialogProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Size")) )
                            >>= aDialogAWTSize )
                        {
                            Size aDialogSize( aDialogAWTSize.Width, aDialogAWTSize.Height );
                            if ( aDialogSize.Width() > 0 && aDialogSize.Height() > 0 )
                            {
                                //calculate and set new position
                                SwRect aSwRect;
                                if (pFlyFrmFmt)
                                    aSwRect = pFlyFrmFmt->GetAnchoredObj()->GetObjRectWithSpaces();
                                Rectangle aRect( aSwRect.SVRect() );
                                Point aDialogPos = SwGetChartDialogPos( &rWrtShell.GetView().GetEditWin(), aDialogSize, aRect );
                                xDialogProps->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Position")),
                                    uno::makeAny( awt::Point(aDialogPos.getX(),aDialogPos.getY()) ) );
                            }
                        }
                    }
                    catch( uno::Exception& )
                    {
                        OSL_FAIL("Chart wizard couldn't be positioned automatically\n" );
                    }
                }

                sal_Int16 nDialogRet = xDialog->execute();
                if( nDialogRet == ui::dialogs::ExecutableDialogResults::CANCEL )
                {
                    rWrtShell.Undo();
                    rWrtShell.GetIDocumentUndoRedo().ClearRedo();
                }
                else
                {
                    OSL_ENSURE( nDialogRet == ui::dialogs::ExecutableDialogResults::OK,
                        "dialog execution failed" );
                }
            }
            uno::Reference< lang::XComponent > xComponent( xDialog, uno::UNO_QUERY );
            if( xComponent.is())
                xComponent->dispose();
        }
    }
}


void AutoEdit::KeyInput( const KeyEvent& rEvt )
{
    sal_uInt16 nCode = rEvt.GetKeyCode().GetCode();
    if( nCode != KEY_SPACE )
        Edit::KeyInput( rEvt );
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
