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


#include "basidesh.hxx"
#include "dlged.hxx"
#include "dlgedclip.hxx"
#include "dlgeddef.hxx"
#include "dlgedfac.hxx"
#include "dlgedfunc.hxx"
#include "dlgedmod.hxx"
#include "dlgedobj.hxx"
#include "dlgedpage.hxx"
#include "dlgedview.hxx"
#include "iderdll.hxx"
#include "localizationmgr.hxx"
#include "propbrw.hxx"

#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/resource/XStringResourcePersistence.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <comphelper/types.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/itempool.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/svxids.hrc>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <xmlscript/xml_helper.hxx>
#include <xmlscript/xmldlg_imexp.hxx>

using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using ::rtl::OUString;

static ::rtl::OUString aResourceResolverPropName( RTL_CONSTASCII_USTRINGPARAM( "ResourceResolver" ));
static ::rtl::OUString aDecorationPropName( RTL_CONSTASCII_USTRINGPARAM( "Decoration" ));
static ::rtl::OUString aTitlePropName( RTL_CONSTASCII_USTRINGPARAM( "Title" ));


//============================================================================
// DlgEdHint
//============================================================================

TYPEINIT1( DlgEdHint, SfxHint );

DlgEdHint::DlgEdHint( DlgEdHintKind eHint )
    :eHintKind( eHint )
{
}


DlgEdHint::DlgEdHint( DlgEdHintKind eHint, DlgEdObj* pObj )
    :eHintKind( eHint )
    ,pDlgEdObj( pObj )
{
}


DlgEdHint::~DlgEdHint()
{
}


//============================================================================
// DlgEditor
//============================================================================

void DlgEditor::ShowDialog()
{
    uno::Reference< lang::XMultiServiceFactory >  xMSF = getProcessServiceFactory();

    // create a dialog
    uno::Reference< awt::XControl > xDlg( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialog" ) ) ), uno::UNO_QUERY );

    // clone the dialog model
    uno::Reference< util::XCloneable > xC( m_xUnoControlDialogModel, uno::UNO_QUERY );
    uno::Reference< util::XCloneable > xNew = xC->createClone();
    uno::Reference< awt::XControlModel > xDlgMod( xNew, uno::UNO_QUERY );

    uno::Reference< beans::XPropertySet > xSrcDlgModPropSet( m_xUnoControlDialogModel, uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xNewDlgModPropSet( xDlgMod, uno::UNO_QUERY );
    if( xNewDlgModPropSet.is() )
    {
        if( xSrcDlgModPropSet.is() )
        {
            try
            {
                Any aResourceResolver = xSrcDlgModPropSet->getPropertyValue( aResourceResolverPropName );
                xNewDlgModPropSet->setPropertyValue( aResourceResolverPropName, aResourceResolver );
            }
            catch(const UnknownPropertyException& )
            {
                OSL_FAIL( "DlgEditor::ShowDialog(): No ResourceResolver property" );
            }
        }

        // Disable decoration
        bool bDecoration = true;
        try
        {
            Any aDecorationAny = xSrcDlgModPropSet->getPropertyValue( aDecorationPropName );
            aDecorationAny >>= bDecoration;
            if( !bDecoration )
            {
                xNewDlgModPropSet->setPropertyValue( aDecorationPropName, makeAny( true ) );
                xNewDlgModPropSet->setPropertyValue( aTitlePropName, makeAny( ::rtl::OUString() ) );
            }
        }
        catch(const UnknownPropertyException& )
        {}
    }

    // set the model
    xDlg->setModel( xDlgMod );

    // create a peer
    uno::Reference< awt::XToolkit> xToolkit( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.ExtToolkit" ) ) ), uno::UNO_QUERY );
    xDlg->createPeer( xToolkit, pWindow->GetComponentInterface() );

    uno::Reference< awt::XDialog > xD( xDlg, uno::UNO_QUERY );
    xD->execute();

    uno::Reference< lang::XComponent > xComponent(xDlg, uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
}


bool DlgEditor::UnmarkDialog()
{
    SdrObject*      pDlgObj = pDlgEdModel->GetPage(0)->GetObj(0);
    SdrPageView*    pPgView = pDlgEdView->GetSdrPageView();

    bool bWasMarked = pDlgEdView->IsObjMarked( pDlgObj );

    if( bWasMarked )
        pDlgEdView->MarkObj( pDlgObj, pPgView, true );

    return bWasMarked;
}


bool DlgEditor::RemarkDialog()
{
    SdrObject*      pDlgObj = pDlgEdModel->GetPage(0)->GetObj(0);
    SdrPageView*    pPgView = pDlgEdView->GetSdrPageView();

    bool bWasMarked = pDlgEdView->IsObjMarked( pDlgObj );

    if( !bWasMarked )
        pDlgEdView->MarkObj( pDlgObj, pPgView, false );

    return bWasMarked;
}


DlgEditor::DlgEditor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel )
    :pHScroll(NULL)
    ,pVScroll(NULL)
    ,pDlgEdModel(NULL)
    ,pDlgEdPage(NULL)
    ,pDlgEdView(NULL)
    ,pDlgEdForm(NULL)
    ,m_xUnoControlDialogModel(NULL)
    ,m_ClipboardDataFlavors(1)
    ,m_ClipboardDataFlavorsResource(2)
    ,pObjFac(NULL)
    ,pWindow(NULL)
    ,pFunc(NULL)
    ,eMode( DLGED_SELECT )
    ,eActObj( OBJ_DLG_PUSHBUTTON )
    ,bFirstDraw(false)
    ,aGridSize( 100, 100 )  // 100TH_MM
    ,bGridVisible(false)
    ,bGridSnap(true)
    ,bCreateOK(true)
    ,bDialogModelChanged(false)
    ,mnPaintGuard(0)
    ,m_xDocument( xModel )
{
    pDlgEdModel = new DlgEdModel();
    pDlgEdModel->GetItemPool().FreezeIdRanges();
    pDlgEdModel->SetScaleUnit( MAP_100TH_MM );

    SdrLayerAdmin& rAdmin = pDlgEdModel->GetLayerAdmin();
    rAdmin.NewLayer( rAdmin.GetControlLayerName() );
    rAdmin.NewLayer( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HiddenLayer")) );

    pDlgEdPage = new DlgEdPage( *pDlgEdModel );
    pDlgEdModel->InsertPage( pDlgEdPage );

    pObjFac = new DlgEdFactory(xModel);

    pFunc = new DlgEdFuncSelect( this );

    // set clipboard data flavors
    m_ClipboardDataFlavors[0].MimeType =             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.sun.xml.dialog" ));
    m_ClipboardDataFlavors[0].HumanPresentableName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Dialog 6.0" ));
    m_ClipboardDataFlavors[0].DataType =             ::getCppuType( (const Sequence< sal_Int8 >*) 0 );

    m_ClipboardDataFlavorsResource[0] =                      m_ClipboardDataFlavors[0];
    m_ClipboardDataFlavorsResource[1].MimeType =             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.sun.xml.dialogwithresource" ));
    m_ClipboardDataFlavorsResource[1].HumanPresentableName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Dialog 8.0" ));
    m_ClipboardDataFlavorsResource[1].DataType =             ::getCppuType( (const Sequence< sal_Int8 >*) 0 );

    aPaintTimer.SetTimeout( 1 );
    aPaintTimer.SetTimeoutHdl( LINK( this, DlgEditor, PaintTimeout ) );

    aMarkTimer.SetTimeout( 100 );
    aMarkTimer.SetTimeoutHdl( LINK( this, DlgEditor, MarkTimeout ) );
}


DlgEditor::~DlgEditor()
{
    aPaintTimer.Stop();
    aMarkTimer.Stop();

    ::comphelper::disposeComponent( m_xControlContainer );

    delete pObjFac;
    delete pFunc;
    delete pDlgEdView;
    delete pDlgEdModel;
}


Reference< awt::XControlContainer > DlgEditor::GetWindowControlContainer()
{
    if ( !m_xControlContainer.is() && pWindow )
        m_xControlContainer = VCLUnoHelper::CreateControlContainer( pWindow );
    return m_xControlContainer;
}


void DlgEditor::SetWindow( Window* pWindow_ )
{
    DlgEditor::pWindow = pWindow_;
    pWindow_->SetMapMode( MapMode( MAP_100TH_MM ) );
    pDlgEdPage->SetSize( pWindow_->PixelToLogic( Size( DLGED_PAGE_WIDTH_MIN, DLGED_PAGE_HEIGHT_MIN ) ) );

    pDlgEdView = new DlgEdView( pDlgEdModel, pWindow_, this );
    pDlgEdView->ShowSdrPage(pDlgEdView->GetModel()->GetPage(0));
    pDlgEdView->SetLayerVisible( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HiddenLayer")), false );
    pDlgEdView->SetMoveSnapOnlyTopLeft(true);
    pDlgEdView->SetWorkArea( Rectangle( Point( 0, 0 ), pDlgEdPage->GetSize() ) );

    pDlgEdView->SetGridCoarse( aGridSize );
    pDlgEdView->SetSnapGridWidth(Fraction(aGridSize.Width(), 1), Fraction(aGridSize.Height(), 1));
    pDlgEdView->SetGridSnap( bGridSnap );
    pDlgEdView->SetGridVisible( bGridVisible );
    pDlgEdView->SetDragStripes(false);

    pDlgEdView->SetDesignMode(true);

    ::comphelper::disposeComponent( m_xControlContainer );
}


void DlgEditor::SetScrollBars( ScrollBar* pHS, ScrollBar* pVS )
{
    pHScroll = pHS;
    pVScroll = pVS;

    InitScrollBars();
}


void DlgEditor::InitScrollBars()
{
    DBG_ASSERT( pHScroll, "DlgEditor::InitScrollBars: no horizontal scroll bar!" );
    DBG_ASSERT( pVScroll, "DlgEditor::InitScrollBars: no vertical scroll bar!" );
    if ( !pHScroll || !pVScroll )
        return;

    Size aOutSize = pWindow->GetOutputSize();
    Size aPgSize  = pDlgEdPage->GetSize();

    pHScroll->SetRange( Range( 0, aPgSize.Width()  ));
    pVScroll->SetRange( Range( 0, aPgSize.Height() ));
    pHScroll->SetVisibleSize( (sal_uLong)aOutSize.Width() );
    pVScroll->SetVisibleSize( (sal_uLong)aOutSize.Height() );

    pHScroll->SetLineSize( aOutSize.Width() / 10 );
    pVScroll->SetLineSize( aOutSize.Height() / 10 );
    pHScroll->SetPageSize( aOutSize.Width() / 2 );
    pVScroll->SetPageSize( aOutSize.Height() / 2 );

    DoScroll( pHScroll );
    DoScroll( pVScroll );
}


void DlgEditor::DoScroll( ScrollBar* )
{
    if( !pHScroll || !pVScroll )
        return;

    MapMode aMap = pWindow->GetMapMode();
    Point aOrg = aMap.GetOrigin();

    Size  aScrollPos( pHScroll->GetThumbPos(), pVScroll->GetThumbPos() );
    aScrollPos = pWindow->LogicToPixel( aScrollPos );
    aScrollPos = pWindow->PixelToLogic( aScrollPos );

    long  nX   = aScrollPos.Width() + aOrg.X();
    long  nY   = aScrollPos.Height() + aOrg.Y();

    if( !nX && !nY )
        return;

    pWindow->Update();

    // #i31562#
    // When scrolling, someone was rescuing the Wallpaper and forced the window scroll to
    // be done without background refresh. I do not know why, but that causes the repaint
    // problems. Taking that out.
    //  Wallpaper aOldBackground = pWindow->GetBackground();
    //  pWindow->SetBackground();

    // #i74769# children should be scrolled
    pWindow->Scroll( -nX, -nY, SCROLL_CHILDREN);
    aMap.SetOrigin( Point( -aScrollPos.Width(), -aScrollPos.Height() ) );
    pWindow->SetMapMode( aMap );
    pWindow->Update();

    DlgEdHint aHint( DLGED_HINT_WINDOWSCROLLED );
    Broadcast( aHint );
}


void DlgEditor::UpdateScrollBars()
{
    MapMode aMap = pWindow->GetMapMode();
    Point aOrg = aMap.GetOrigin();

    if ( pHScroll )
        pHScroll->SetThumbPos( -aOrg.X() );

    if ( pVScroll )
        pVScroll->SetThumbPos( -aOrg.Y() );
}


void DlgEditor::SetDialog( uno::Reference< container::XNameContainer > xUnoControlDialogModel )
{
    // set dialog model
    m_xUnoControlDialogModel = xUnoControlDialogModel;

    // create dialog form
    pDlgEdForm = new DlgEdForm();
    uno::Reference< awt::XControlModel > xDlgMod( m_xUnoControlDialogModel , uno::UNO_QUERY );
    pDlgEdForm->SetUnoControlModel(xDlgMod);
    pDlgEdForm->SetDlgEditor( this );
    ((DlgEdPage*)pDlgEdModel->GetPage(0))->SetDlgEdForm( pDlgEdForm );
    pDlgEdModel->GetPage(0)->InsertObject( pDlgEdForm );
    AdjustPageSize();
    pDlgEdForm->SetRectFromProps();
    pDlgEdForm->UpdateTabIndices();     // for backward compatibility
    pDlgEdForm->StartListening();

    // create controls
    Reference< ::com::sun::star::container::XNameAccess > xNameAcc( m_xUnoControlDialogModel, UNO_QUERY );
    if ( xNameAcc.is() )
    {
        // get sequence of control names
        Sequence< ::rtl::OUString > aNames = xNameAcc->getElementNames();
        const ::rtl::OUString* pNames = aNames.getConstArray();
        sal_Int32 nCtrls = aNames.getLength();

        // create a map of tab indices and control names, sorted by tab index
        IndexToNameMap aIndexToNameMap;
        for ( sal_Int32 i = 0; i < nCtrls; ++i )
        {
            // get name
            ::rtl::OUString aName( pNames[i] );

            // get tab index
            sal_Int16 nTabIndex = -1;
            Any aCtrl = xNameAcc->getByName( aName );
            Reference< ::com::sun::star::beans::XPropertySet > xPSet;
               aCtrl >>= xPSet;
            if ( xPSet.is() )
                xPSet->getPropertyValue( DLGED_PROP_TABINDEX ) >>= nTabIndex;

            // insert into map
            aIndexToNameMap.insert( IndexToNameMap::value_type( nTabIndex, aName ) );
        }

        // create controls and insert them into drawing page
        for ( IndexToNameMap::iterator aIt = aIndexToNameMap.begin(); aIt != aIndexToNameMap.end(); ++aIt )
        {
            Any aCtrl = xNameAcc->getByName( aIt->second );
            Reference< ::com::sun::star::awt::XControlModel > xCtrlModel;
            aCtrl >>= xCtrlModel;
            DlgEdObj* pCtrlObj = new DlgEdObj();
            pCtrlObj->SetUnoControlModel( xCtrlModel );
            pCtrlObj->SetDlgEdForm( pDlgEdForm );
            pDlgEdForm->AddChild( pCtrlObj );
            pDlgEdModel->GetPage(0)->InsertObject( pCtrlObj );
            pCtrlObj->SetRectFromProps();
            pCtrlObj->UpdateStep();
            pCtrlObj->StartListening();
        }
    }

    bFirstDraw = true;

    pDlgEdModel->SetChanged(false);
}

void DlgEditor::ResetDialog ()
{
    DlgEdForm* pOldDlgEdForm = pDlgEdForm;
    DlgEdPage* pPage = (DlgEdPage*)pDlgEdModel->GetPage(0);
    SdrPageView* pPgView = pDlgEdView->GetSdrPageView();
    bool bWasMarked = pDlgEdView->IsObjMarked( pOldDlgEdForm );
    pDlgEdView->UnmarkAll();
    pPage->Clear();
    pPage->SetDlgEdForm( NULL );
    SetDialog( m_xUnoControlDialogModel );
    if( bWasMarked )
        pDlgEdView->MarkObj( pDlgEdForm, pPgView, false );
}


Reference< util::XNumberFormatsSupplier > const & DlgEditor::GetNumberFormatsSupplier()
{
    if ( !m_xSupplier.is() )
    {
        Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        Reference< util::XNumberFormatsSupplier > xSupplier( xMSF->createInstance(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.NumberFormatsSupplier") ) ), UNO_QUERY );

        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if ( !m_xSupplier.is() )
        {
            m_xSupplier = xSupplier;
        }
    }
    return m_xSupplier;
}


void DlgEditor::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( pWindow )
        pWindow->GrabFocus();
    pFunc->MouseButtonDown( rMEvt );
}


void DlgEditor::MouseButtonUp( const MouseEvent& rMEvt )
{
    bool bRet = pFunc->MouseButtonUp( rMEvt );

    if( eMode == DLGED_INSERT )
        bCreateOK = bRet;
}


void DlgEditor::MouseMove( const MouseEvent& rMEvt )
{
    pFunc->MouseMove( rMEvt );
}


bool DlgEditor::KeyInput( const KeyEvent& rKEvt )
{
    return pFunc->KeyInput( rKEvt );
}


void DlgEditor::Paint( const Rectangle& rRect )
{
    aPaintRect = rRect;
    PaintTimeout( &aPaintTimer );
}


IMPL_LINK_NOARG(DlgEditor, PaintTimeout)
{
    if( !pDlgEdView )
        return 0;

    mnPaintGuard++;

    Size aMacSize;
    if( bFirstDraw &&
        pWindow->IsVisible() &&
        (pWindow->GetOutputSize() != aMacSize) )
    {
        bFirstDraw = false;

        // get property set
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSet(pDlgEdForm->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);

        if ( xPSet.is() )
        {
            // get dialog size from properties
            sal_Int32 nWidth = 0, nHeight = 0;
            xPSet->getPropertyValue( DLGED_PROP_WIDTH ) >>= nWidth;
            xPSet->getPropertyValue( DLGED_PROP_HEIGHT ) >>= nHeight;

            if ( nWidth == 0 && nHeight == 0 )
            {
                Size   aSize = pWindow->PixelToLogic( Size( 400, 300 ) );

                // align with grid
                Size aGridSize_(long(pDlgEdView->GetSnapGridWidthX()), long(pDlgEdView->GetSnapGridWidthY()));
                aSize.Width()  -= aSize.Width()  % aGridSize_.Width();
                aSize.Height() -= aSize.Height() % aGridSize_.Height();

                Point  aPos;
                Size   aOutSize = pWindow->GetOutputSize();
                aPos.X() = (aOutSize.Width()>>1)  -  (aSize.Width()>>1);
                aPos.Y() = (aOutSize.Height()>>1) -  (aSize.Height()>>1);

                // align with grid
                aPos.X() -= aPos.X() % aGridSize_.Width();
                aPos.Y() -= aPos.Y() % aGridSize_.Height();

                // don't put in the corner
                Point aMinPos = pWindow->PixelToLogic( Point( 30, 20 ) );
                if( (aPos.X() < aMinPos.X()) || (aPos.Y() < aMinPos.Y()) )
                {
                    aPos = aMinPos;
                    aPos.X() -= aPos.X() % aGridSize_.Width();
                    aPos.Y() -= aPos.Y() % aGridSize_.Height();
                }

                // set dialog position and size
                pDlgEdForm->SetSnapRect( Rectangle( aPos, aSize ) );
                pDlgEdForm->EndListening(false);
                pDlgEdForm->SetPropsFromRect();
                pDlgEdForm->GetDlgEditor()->SetDialogModelChanged(true);
                pDlgEdForm->StartListening();

                // set position and size of controls
                sal_uLong nObjCount;
                if ( pDlgEdPage && ( ( nObjCount = pDlgEdPage->GetObjCount() ) > 0 ) )
                {
                    for ( sal_uLong i = 0 ; i < nObjCount ; i++ )
                    {
                        SdrObject* pObj = pDlgEdPage->GetObj(i);
                        DlgEdObj* pDlgEdObj = PTR_CAST(DlgEdObj, pObj);
                        if ( pDlgEdObj && !pDlgEdObj->ISA(DlgEdForm) )
                            pDlgEdObj->SetRectFromProps();
                    }
                }
            }
        }
    }

    // repaint, get PageView and prepare Region
    SdrPageView* pPgView = pDlgEdView->GetSdrPageView();
    const Region aPaintRectRegion(aPaintRect);


    // #i74769#
    SdrPaintWindow* pTargetPaintWindow = 0;

    // mark repaint start
    if(pPgView)
    {
        pTargetPaintWindow = pPgView->GetView().BeginDrawLayers(pWindow, aPaintRectRegion);
        OSL_ENSURE(pTargetPaintWindow, "BeginDrawLayers: Got no SdrPaintWindow (!)");
    }

    // draw background self using wallpaper
    // #i79128# ...and use correct OutDev for that
    if(pTargetPaintWindow)
    {
        OutputDevice& rTargetOutDev = pTargetPaintWindow->GetTargetOutputDevice();
        rTargetOutDev.DrawWallpaper(aPaintRect, Wallpaper(Color(COL_WHITE)));
    }

    // do paint (unbuffered) and mark repaint end
    if(pPgView)
    {
        // paint of control layer is done in EndDrawLayers anyway...
        pPgView->GetView().EndDrawLayers(*pTargetPaintWindow, true);
    }

    mnPaintGuard--;

    DBG_ASSERT(pWindow,"Window not set");
    return 0;
}


IMPL_LINK_NOARG(DlgEditor, MarkTimeout)
{
    BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();

    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
    SfxChildWindow* pChildWin = pViewFrame ? pViewFrame->GetChildWindow( SID_SHOW_PROPERTYBROWSER ) : NULL;
    if ( !pChildWin )
        return 0L;

    ((PropBrw*)(pChildWin->GetWindow()))->Update( pIDEShell );

    return 1;
}


void DlgEditor::SetMode( DlgEdMode eNewMode )
{
    if ( eNewMode != eMode )
    {
        delete pFunc;

        if ( eNewMode == DLGED_INSERT )
            pFunc = new DlgEdFuncInsert( this );
        else
            pFunc = new DlgEdFuncSelect( this );

        if ( eNewMode == DLGED_READONLY )
            pDlgEdModel->SetReadOnly( true );
        else
            pDlgEdModel->SetReadOnly( false );
    }

    if ( eNewMode == DLGED_TEST )
        ShowDialog();

    eMode = eNewMode;
}


void DlgEditor::SetInsertObj( sal_uInt16 eObj )
{
    eActObj = eObj;

    if( pDlgEdView )
        pDlgEdView->SetCurrentObj( eActObj, DlgInventor );
}


sal_uInt16 DlgEditor::GetInsertObj() const
{
    return eActObj;
}


void DlgEditor::CreateDefaultObject()
{
    // create object by factory
    SdrObject* pObj = SdrObjFactory::MakeNewObject( pDlgEdView->GetCurrentObjInventor(), pDlgEdView->GetCurrentObjIdentifier(), pDlgEdPage );

    DlgEdObj* pDlgEdObj = PTR_CAST( DlgEdObj, pObj );
    if ( pDlgEdObj )
    {
        // set position and size
        Size aSize = pWindow->PixelToLogic( Size( 96, 24 ) );
        Point aPoint = (pDlgEdForm->GetSnapRect()).Center();
        aPoint.X() -= aSize.Width() / 2;
        aPoint.Y() -= aSize.Height() / 2;
        pDlgEdObj->SetSnapRect( Rectangle( aPoint, aSize ) );

        // set default property values
        pDlgEdObj->SetDefaults();

        // insert object into drawing page
        SdrPageView* pPageView = pDlgEdView->GetSdrPageView();
        pDlgEdView->InsertObjectAtView( pDlgEdObj, *pPageView);

        // start listening
        pDlgEdObj->StartListening();
    }
}


void DlgEditor::Cut()
{
    Copy();
    Delete();
}


void implCopyStreamToByteSequence( Reference< XInputStream > xStream,
    Sequence< sal_Int8 >& bytes )
{
    sal_Int32 nRead = xStream->readBytes( bytes, xStream->available() );
    for (;;)
    {
        Sequence< sal_Int8 > readBytes;
        nRead = xStream->readBytes( readBytes, 1024 );
        if (! nRead)
            break;

        sal_Int32 nPos = bytes.getLength();
        bytes.realloc( nPos + nRead );
        ::rtl_copyMemory( bytes.getArray() + nPos, readBytes.getConstArray(), (sal_uInt32)nRead );
    }
}

void DlgEditor::Copy()
{
    if( !pDlgEdView->AreObjectsMarked() )
        return;

    // stop all drawing actions
    pDlgEdView->BrkAction();

    // create an empty clipboard dialog model
    Reference< util::XCloneable > xClone( m_xUnoControlDialogModel, UNO_QUERY );
    Reference< util::XCloneable > xNewClone = xClone->createClone();
    Reference< container::XNameContainer > xClipDialogModel( xNewClone, UNO_QUERY );

    Reference< container::XNameAccess > xNAcc( xClipDialogModel, UNO_QUERY );
    if ( xNAcc.is() )
    {
           Sequence< OUString > aNames = xNAcc->getElementNames();
           const OUString* pNames = aNames.getConstArray();
        sal_uInt32 nCtrls = aNames.getLength();

        for ( sal_uInt32 n = 0; n < nCtrls; n++ )
        {
               xClipDialogModel->removeByName( pNames[n] );
        }
    }

    // insert control models of marked objects into clipboard dialog model
    sal_uLong nMark = pDlgEdView->GetMarkedObjectList().GetMarkCount();
    for( sal_uLong i = 0; i < nMark; i++ )
    {
        SdrObject* pObj = pDlgEdView->GetMarkedObjectList().GetMark(i)->GetMarkedSdrObj();
        DlgEdObj* pDlgEdObj = PTR_CAST(DlgEdObj, pObj);

        if (pDlgEdObj && !pDlgEdObj->ISA(DlgEdForm) )
        {
            ::rtl::OUString aName;
            Reference< beans::XPropertySet >  xMarkPSet(pDlgEdObj->GetUnoControlModel(), uno::UNO_QUERY);
            if (xMarkPSet.is())
            {
                xMarkPSet->getPropertyValue( DLGED_PROP_NAME ) >>= aName;
            }

            Reference< container::XNameAccess > xNameAcc(m_xUnoControlDialogModel, UNO_QUERY );
            if ( xNameAcc.is() && xNameAcc->hasByName(aName) )
            {
                Any aCtrl = xNameAcc->getByName( aName );

                // clone control model
                Reference< util::XCloneable > xCtrl;
                   aCtrl >>= xCtrl;
                Reference< util::XCloneable > xNewCtrl = xCtrl->createClone();
                Any aNewCtrl;
                aNewCtrl <<= xNewCtrl;

                if (xClipDialogModel.is())
                    xClipDialogModel->insertByName( aName , aNewCtrl );
            }
        }
    }

    // export clipboard dialog model to xml
    Reference< XComponentContext > xContext;
    Reference< beans::XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    OSL_VERIFY( xProps->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext );
    Reference< XInputStreamProvider > xISP = ::xmlscript::exportDialogModel( xClipDialogModel, xContext, m_xDocument );
    Reference< XInputStream > xStream( xISP->createInputStream() );
    Sequence< sal_Int8 > DialogModelBytes;
    implCopyStreamToByteSequence( xStream, DialogModelBytes );
    xStream->closeInput();

    // set clipboard content
    Reference< datatransfer::clipboard::XClipboard > xClipboard = GetWindow()->GetClipboard();
    if ( xClipboard.is() )
    {
        // With resource?
        uno::Reference< beans::XPropertySet > xDialogModelPropSet( m_xUnoControlDialogModel, uno::UNO_QUERY );
        uno::Reference< resource::XStringResourcePersistence > xStringResourcePersistence;
        if( xDialogModelPropSet.is() )
        {
            try
            {
                Any aResourceResolver = xDialogModelPropSet->getPropertyValue( aResourceResolverPropName );
                aResourceResolver >>= xStringResourcePersistence;
            }
            catch(const UnknownPropertyException& )
            {}
        }

        DlgEdTransferableImpl* pTrans = NULL;
        if( xStringResourcePersistence.is() )
        {
            // With resource, support old and new format

            // Export xClipDialogModel another time with ids replaced by current language string
            uno::Reference< resource::XStringResourceManager >
                xStringResourceManager( xStringResourcePersistence, uno::UNO_QUERY );
            LocalizationMgr::resetResourceForDialog( xClipDialogModel, xStringResourceManager );
            Reference< XInputStreamProvider > xISP2 = ::xmlscript::exportDialogModel( xClipDialogModel, xContext, m_xDocument );
            Reference< XInputStream > xStream2( xISP2->createInputStream() );
            Sequence< sal_Int8 > NoResourceDialogModelBytes;
            implCopyStreamToByteSequence( xStream2, NoResourceDialogModelBytes );
            xStream2->closeInput();

            // Old format contains dialog with replaced ids
            Sequence< Any > aSeqData(2);
            Any aNoResourceDialogModelBytesAny;
            aNoResourceDialogModelBytesAny <<= NoResourceDialogModelBytes;
            aSeqData[0] = aNoResourceDialogModelBytesAny;

            // New format contains dialog and resource
            Sequence< sal_Int8 > aResData = xStringResourcePersistence->exportBinary();

            // Create sequence for combined dialog and resource
            sal_Int32 nDialogDataLen = DialogModelBytes.getLength();
            sal_Int32 nResDataLen = aResData.getLength();

            // Combined data = 4 Bytes 32Bit Offset to begin of resource data, lowest byte first
            // + nDialogDataLen bytes dialog data + nResDataLen resource data
            sal_Int32 nTotalLen = 4 + nDialogDataLen + nResDataLen;
            sal_Int32 nResOffset = 4 + nDialogDataLen;
            Sequence< sal_Int8 > aCombinedData( nTotalLen );
            sal_Int8* pCombinedData = aCombinedData.getArray();

            // Write offset
            sal_Int32 n = nResOffset;
            for( sal_Int16 i = 0 ; i < 4 ; i++ )
            {
                pCombinedData[i] = sal_Int8( n & 0xff );
                n >>= 8;
            }
            ::rtl_copyMemory( pCombinedData + 4, DialogModelBytes.getConstArray(), nDialogDataLen );
            ::rtl_copyMemory( pCombinedData + nResOffset, aResData.getConstArray(), nResDataLen );

            Any aCombinedDataAny;
            aCombinedDataAny <<= aCombinedData;
            aSeqData[1] = aCombinedDataAny;

            pTrans = new DlgEdTransferableImpl( m_ClipboardDataFlavorsResource, aSeqData );
        }
        else
        {
            // No resource, support only old format
            Sequence< Any > aSeqData(1);
            Any aDialogModelBytesAny;
            aDialogModelBytesAny <<= DialogModelBytes;
            aSeqData[0] = aDialogModelBytesAny;
            pTrans = new DlgEdTransferableImpl( m_ClipboardDataFlavors , aSeqData );
        }
        const sal_uInt32 nRef = Application::ReleaseSolarMutex();
        xClipboard->setContents( pTrans , pTrans );
        Application::AcquireSolarMutex( nRef );
    }
}


void DlgEditor::Paste()
{
    // stop all drawing actions
    pDlgEdView->BrkAction();

    // unmark all objects
    pDlgEdView->UnmarkAll();

    // get clipboard
    Reference< datatransfer::clipboard::XClipboard > xClipboard = GetWindow()->GetClipboard();
    if ( xClipboard.is() )
    {
        // get clipboard content
        const sal_uInt32 nRef = Application::ReleaseSolarMutex();
        Reference< datatransfer::XTransferable > xTransf = xClipboard->getContents();
        Application::AcquireSolarMutex( nRef );
        if ( xTransf.is() )
        {
            // Is target dialog (library) localized?
            uno::Reference< beans::XPropertySet > xDialogModelPropSet( m_xUnoControlDialogModel, uno::UNO_QUERY );
            uno::Reference< resource::XStringResourceManager > xStringResourceManager;
            if( xDialogModelPropSet.is() )
            {
                try
                {
                    Any aResourceResolver = xDialogModelPropSet->getPropertyValue( aResourceResolverPropName );
                    aResourceResolver >>= xStringResourceManager;
                }
                catch(const UnknownPropertyException& )
                {}
            }
            bool bLocalized = false;
            if( xStringResourceManager.is() )
                bLocalized = ( xStringResourceManager->getLocales().getLength() > 0 );

            if ( xTransf->isDataFlavorSupported( m_ClipboardDataFlavors[0] ) )
            {
                // create clipboard dialog model from xml
                Reference< lang::XMultiServiceFactory > xMSF = getProcessServiceFactory();
                Reference< container::XNameContainer > xClipDialogModel( xMSF->createInstance
                    ( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ),
                        uno::UNO_QUERY );

                bool bSourceIsLocalized = false;
                Sequence< sal_Int8 > DialogModelBytes;
                Sequence< sal_Int8 > aResData;
                if( bLocalized && xTransf->isDataFlavorSupported( m_ClipboardDataFlavorsResource[1] ) )
                {
                    bSourceIsLocalized = true;

                    Any aCombinedDataAny = xTransf->getTransferData( m_ClipboardDataFlavorsResource[1] );
                    Sequence< sal_Int8 > aCombinedData;
                    aCombinedDataAny >>= aCombinedData;
                    const sal_Int8* pCombinedData = aCombinedData.getConstArray();

                    sal_Int32 nTotalLen = aCombinedData.getLength();

                    // Reading offset
                    sal_Int32 nResOffset = 0;
                    sal_Int32 nFactor = 1;
                    for( sal_Int16 i = 0; i < 4; i++ )
                    {
                        nResOffset += nFactor * sal_uInt8( pCombinedData[i] );
                        nFactor *= 256;
                    }

                    sal_Int32 nResDataLen = nTotalLen - nResOffset;
                    sal_Int32 nDialogDataLen = nTotalLen - nResDataLen - 4;

                    DialogModelBytes.realloc( nDialogDataLen );
                    ::rtl_copyMemory( DialogModelBytes.getArray(), pCombinedData + 4, nDialogDataLen );

                    aResData.realloc( nResDataLen );
                    ::rtl_copyMemory( aResData.getArray(), pCombinedData + nResOffset, nResDataLen );
                }
                else
                {
                    Any aAny = xTransf->getTransferData( m_ClipboardDataFlavors[0] );
                    aAny >>= DialogModelBytes;
                }

                if ( xClipDialogModel.is() )
                {
                    Reference< XComponentContext > xContext;
                    Reference< beans::XPropertySet > xProps( xMSF, UNO_QUERY );
                    OSL_ASSERT( xProps.is() );
                    OSL_VERIFY( xProps->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext );
                    ::xmlscript::importDialogModel( ::xmlscript::createInputStream( rtl::ByteSequence(DialogModelBytes.getArray(), DialogModelBytes.getLength()) ) , xClipDialogModel, xContext, m_xDocument );
                }

                // get control models from clipboard dialog model
                Reference< ::com::sun::star::container::XNameAccess > xNameAcc( xClipDialogModel, UNO_QUERY );
                if ( xNameAcc.is() )
                {
                       Sequence< OUString > aNames = xNameAcc->getElementNames();
                       const OUString* pNames = aNames.getConstArray();
                    sal_uInt32 nCtrls = aNames.getLength();

                    Reference< resource::XStringResourcePersistence > xStringResourcePersistence;
                    if( nCtrls > 0 && bSourceIsLocalized )
                    {
                        Reference< lang::XMultiServiceFactory > xSMgr = getProcessServiceFactory();
                        xStringResourcePersistence = Reference< resource::XStringResourcePersistence >( xSMgr->createInstance
                            ( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.resource.StringResource" )) ), UNO_QUERY );
                        if( xStringResourcePersistence.is() )
                            xStringResourcePersistence->importBinary( aResData );
                    }
                    for( sal_uInt32 n = 0; n < nCtrls; n++ )
                    {
                           Any aA = xNameAcc->getByName( pNames[n] );
                        Reference< ::com::sun::star::awt::XControlModel > xCM;
                           aA >>= xCM;

                        // clone the control model
                        Reference< util::XCloneable > xClone( xCM, uno::UNO_QUERY );
                        Reference< awt::XControlModel > xCtrlModel( xClone->createClone(), uno::UNO_QUERY );

                        DlgEdObj* pCtrlObj = new DlgEdObj();
                        pCtrlObj->SetDlgEdForm(pDlgEdForm);         // set parent form
                        pDlgEdForm->AddChild(pCtrlObj);             // add child to parent form
                        pCtrlObj->SetUnoControlModel( xCtrlModel ); // set control model

                        // set new name
                        ::rtl::OUString aOUniqueName( pCtrlObj->GetUniqueName() );
                        Reference< beans::XPropertySet > xPSet( xCtrlModel , UNO_QUERY );
                        Any aUniqueName;
                        aUniqueName <<= aOUniqueName;
                        xPSet->setPropertyValue( DLGED_PROP_NAME, aUniqueName );

                        // set tabindex
                        Reference< container::XNameAccess > xNA( m_xUnoControlDialogModel , UNO_QUERY );
                           Sequence< OUString > aNames_ = xNA->getElementNames();
                        Any aTabIndex;
                        aTabIndex <<= (sal_Int16) aNames_.getLength();
                        xPSet->setPropertyValue( DLGED_PROP_TABINDEX, aTabIndex );

                        if( bLocalized )
                        {
                            Any aControlAny;
                            aControlAny <<= xCtrlModel;
                            if( bSourceIsLocalized && xStringResourcePersistence.is() )
                            {
                                Reference< resource::XStringResourceResolver >
                                    xSourceStringResolver( xStringResourcePersistence, UNO_QUERY );
                                LocalizationMgr::copyResourcesForPastedEditorObject( this,
                                    aControlAny, aOUniqueName, xSourceStringResolver );
                            }
                            else
                            {
                                LocalizationMgr::setControlResourceIDsForNewEditorObject
                                    ( this, aControlAny, aOUniqueName );
                            }
                        }

                        // insert control model in editor dialog model
                        Any aCtrlModel;
                        aCtrlModel <<= xCtrlModel;
                        m_xUnoControlDialogModel->insertByName( aOUniqueName , aCtrlModel );

                        // insert object into drawing page
                        pDlgEdModel->GetPage(0)->InsertObject( pCtrlObj );
                        pCtrlObj->SetRectFromProps();
                        pCtrlObj->UpdateStep();
                        pDlgEdForm->UpdateTabOrderAndGroups();
                        pCtrlObj->StartListening();                         // start listening

                        // mark object
                        SdrPageView* pPgView = pDlgEdView->GetSdrPageView();
                        pDlgEdView->MarkObj( pCtrlObj, pPgView, false, true);
                    }

                    // center marked objects in dialog editor form
                    Point aMarkCenter = (pDlgEdView->GetMarkedObjRect()).Center();
                    Point aFormCenter = (pDlgEdForm->GetSnapRect()).Center();
                    Point aPoint = aFormCenter - aMarkCenter;
                    Size  aSize( aPoint.X() , aPoint.Y() );
                    pDlgEdView->MoveMarkedObj( aSize );                     // update of control model properties (position + size) in NbcMove
                    pDlgEdView->MarkListHasChanged();

                    // dialog model changed
                    SetDialogModelChanged(true);
                }
            }
        }
    }
}


void DlgEditor::Delete()
{
    if( !pDlgEdView->AreObjectsMarked() )
        return;

    // remove control models of marked objects from dialog model
    sal_uLong nMark = pDlgEdView->GetMarkedObjectList().GetMarkCount();

    for( sal_uLong i = 0; i < nMark; i++ )
    {
        SdrObject* pObj = pDlgEdView->GetMarkedObjectList().GetMark(i)->GetMarkedSdrObj();
        DlgEdObj* pDlgEdObj = PTR_CAST(DlgEdObj, pObj);

        if ( pDlgEdObj && !pDlgEdObj->ISA(DlgEdForm) )
        {
            // get name from property
            ::rtl::OUString aName;
            uno::Reference< beans::XPropertySet >  xPSet(pDlgEdObj->GetUnoControlModel(), uno::UNO_QUERY);
            if (xPSet.is())
            {
                xPSet->getPropertyValue( DLGED_PROP_NAME ) >>= aName;
            }

            // remove control from dialog model
            Reference< ::com::sun::star::container::XNameAccess > xNameAcc(pDlgEdObj->GetDlgEdForm()->GetUnoControlModel(), UNO_QUERY );
            if ( xNameAcc.is() && xNameAcc->hasByName(aName) )
            {
                Reference< ::com::sun::star::container::XNameContainer > xCont(xNameAcc, UNO_QUERY );
                if ( xCont.is() )
                {
                    if( xCont->hasByName( aName ) )
                    {
                        Any aAny = xCont->getByName( aName );
                        LocalizationMgr::deleteControlResourceIDsForDeletedEditorObject( this, aAny, aName );
                    }
                    xCont->removeByName( aName );
                }
            }

            // remove child from parent form
            pDlgEdForm->RemoveChild( pDlgEdObj );
        }
    }

    // update tab indices
    pDlgEdForm->UpdateTabIndices();

    pDlgEdView->BrkAction();

    bool const bDlgMarked = UnmarkDialog();
    pDlgEdView->DeleteMarked();
    if( bDlgMarked )
        RemarkDialog();
}


bool DlgEditor::IsPasteAllowed()
{
    // get clipboard
    Reference< datatransfer::clipboard::XClipboard > xClipboard = GetWindow()->GetClipboard();
    if ( xClipboard.is() )
    {
        // get clipboard content
        const sal_uInt32 nRef = Application::ReleaseSolarMutex();
        Reference< datatransfer::XTransferable > xTransf = xClipboard->getContents();
        Application::AcquireSolarMutex( nRef );
        return xTransf.is() && xTransf->isDataFlavorSupported( m_ClipboardDataFlavors[0] );
    }
    return false;
}


void DlgEditor::ShowProperties()
{
    BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
    if ( pViewFrame && !pViewFrame->HasChildWindow( SID_SHOW_PROPERTYBROWSER ) )
        pViewFrame->ToggleChildWindow( SID_SHOW_PROPERTYBROWSER );
}


void DlgEditor::UpdatePropertyBrowserDelayed()
{
    aMarkTimer.Start();
}


bool DlgEditor::IsModified() const
{
    return pDlgEdModel->IsChanged() || bDialogModelChanged;
}


void DlgEditor::ClearModifyFlag()
{
    pDlgEdModel->SetChanged(false);
    bDialogModelChanged = false;
}


#define LMARGPRN        1700
#define RMARGPRN         900
#define TMARGPRN        2000
#define BMARGPRN        1000
#define BORDERPRN       300


void lcl_PrintHeader( Printer* pPrinter, const ::rtl::OUString& rTitle ) // not working yet
{
    pPrinter->Push();

    short nLeftMargin = LMARGPRN;
    Size aSz = pPrinter->GetOutputSize();
    short nBorder = BORDERPRN;

    pPrinter->SetLineColor( COL_BLACK );
    pPrinter->SetFillColor();

    Font aFont( pPrinter->GetFont() );
    aFont.SetWeight( WEIGHT_BOLD );
    aFont.SetAlign( ALIGN_BOTTOM );
    pPrinter->SetFont( aFont );

    long nFontHeight = pPrinter->GetTextHeight();

    // 1st border => line, 2+3 border = free space
    long nYTop = TMARGPRN-3*nBorder-nFontHeight;

    long nXLeft = nLeftMargin-nBorder;
    long nXRight = aSz.Width()-RMARGPRN+nBorder;

    pPrinter->DrawRect( Rectangle(
        Point( nXLeft, nYTop ),
        Size( nXRight-nXLeft, aSz.Height() - nYTop - BMARGPRN + nBorder ) ) );

    long nY = TMARGPRN-2*nBorder;
    Point aPos( nLeftMargin, nY );
    pPrinter->DrawText( aPos, rTitle );

    nY = TMARGPRN-nBorder;

    pPrinter->DrawLine( Point( nXLeft, nY ), Point( nXRight, nY ) );

    pPrinter->Pop();
}


sal_Int32 DlgEditor::countPages( Printer* )
{
    return 1;
}

void DlgEditor::printPage( sal_Int32 nPage, Printer* pPrinter, const ::rtl::OUString& rTitle )
{
    if( nPage == 0 )
        Print( pPrinter, rTitle );
}


void DlgEditor::Print( Printer* pPrinter, const ::rtl::OUString& rTitle )    // not working yet
{
    if( pDlgEdView )
    {
        MapMode aOldMap( pPrinter->GetMapMode());
        Font aOldFont( pPrinter->GetFont() );

        MapMode aMap( MAP_100TH_MM );
        pPrinter->SetMapMode( aMap );
        Font aFont;
        aFont.SetAlign( ALIGN_BOTTOM );
        aFont.SetSize( Size( 0, 360 ));
        pPrinter->SetFont( aFont );

        Size aPaperSz = pPrinter->GetOutputSize();
        aPaperSz.Width() -= (LMARGPRN+RMARGPRN);
        aPaperSz.Height() -= (TMARGPRN+BMARGPRN);

        lcl_PrintHeader( pPrinter, rTitle );

        Bitmap aDlg;
        Size aBmpSz( pPrinter->PixelToLogic( aDlg.GetSizePixel() ) );
        double nPaperSzWidth = aPaperSz.Width();
        double nPaperSzHeight = aPaperSz.Height();
        double nBmpSzWidth = aBmpSz.Width();
        double nBmpSzHeight = aBmpSz.Height();
        double nScaleX = (nPaperSzWidth / nBmpSzWidth );
        double nScaleY = (nPaperSzHeight / nBmpSzHeight );

        Size aOutputSz;
        if( nBmpSzHeight * nScaleX <= nPaperSzHeight )
        {
            aOutputSz.Width() = (long)(((double)nBmpSzWidth) * nScaleX);
            aOutputSz.Height() = (long)(((double)nBmpSzHeight) * nScaleX);
        }
        else
        {
            aOutputSz.Width() = (long)(((double)nBmpSzWidth) * nScaleY);
            aOutputSz.Height() = (long)(((double)nBmpSzHeight) * nScaleY);
        }

        Point aPosOffs(
            (aPaperSz.Width() / 2) - (aOutputSz.Width() / 2),
            (aPaperSz.Height()/ 2) - (aOutputSz.Height() / 2));

        aPosOffs.X() += LMARGPRN;
        aPosOffs.Y() += TMARGPRN;

        pPrinter->DrawBitmap( aPosOffs, aOutputSz, aDlg );

        pPrinter->SetMapMode( aOldMap );
        pPrinter->SetFont( aOldFont );
    }
}


bool DlgEditor::AdjustPageSize()
{
    bool bAdjustedPageSize = false;
    Reference< beans::XPropertySet > xPSet( m_xUnoControlDialogModel, UNO_QUERY );
    if ( xPSet.is() )
    {
        sal_Int32 nFormXIn = 0, nFormYIn = 0, nFormWidthIn = 0, nFormHeightIn = 0;
        xPSet->getPropertyValue( DLGED_PROP_POSITIONX ) >>= nFormXIn;
        xPSet->getPropertyValue( DLGED_PROP_POSITIONY ) >>= nFormYIn;
        xPSet->getPropertyValue( DLGED_PROP_WIDTH ) >>= nFormWidthIn;
        xPSet->getPropertyValue( DLGED_PROP_HEIGHT ) >>= nFormHeightIn;

        sal_Int32 nFormX, nFormY, nFormWidth, nFormHeight;
        if ( pDlgEdForm && pDlgEdForm->TransformFormToSdrCoordinates( nFormXIn, nFormYIn, nFormWidthIn, nFormHeightIn, nFormX, nFormY, nFormWidth, nFormHeight ) )
        {
            Size aPageSizeDelta( 400, 300 );
            DBG_ASSERT( pWindow, "DlgEditor::AdjustPageSize: no window!" );
            if ( pWindow )
                aPageSizeDelta = pWindow->PixelToLogic( aPageSizeDelta, MapMode( MAP_100TH_MM ) );

            sal_Int32 nNewPageWidth = nFormX + nFormWidth + aPageSizeDelta.Width();
            sal_Int32 nNewPageHeight = nFormY + nFormHeight + aPageSizeDelta.Height();

            Size aPageSizeMin( DLGED_PAGE_WIDTH_MIN, DLGED_PAGE_HEIGHT_MIN );
            DBG_ASSERT( pWindow, "DlgEditor::AdjustPageSize: no window!" );
            if ( pWindow )
                aPageSizeMin = pWindow->PixelToLogic( aPageSizeMin, MapMode( MAP_100TH_MM ) );
            sal_Int32 nPageWidthMin = aPageSizeMin.Width();
            sal_Int32 nPageHeightMin = aPageSizeMin.Height();

            if ( nNewPageWidth < nPageWidthMin )
                nNewPageWidth = nPageWidthMin;

            if ( nNewPageHeight < nPageHeightMin )
                nNewPageHeight = nPageHeightMin;

            if ( pDlgEdPage )
            {
                Size aPageSize = pDlgEdPage->GetSize();
                if ( nNewPageWidth != aPageSize.Width() || nNewPageHeight != aPageSize.Height() )
                {
                    Size aNewPageSize( nNewPageWidth, nNewPageHeight );
                    pDlgEdPage->SetSize( aNewPageSize );
                    DBG_ASSERT( pDlgEdView, "DlgEditor::AdjustPageSize: no view!" );
                    if ( pDlgEdView )
                        pDlgEdView->SetWorkArea( Rectangle( Point( 0, 0 ), aNewPageSize ) );
                    bAdjustedPageSize = true;
                }
            }
        }
    }

    return bAdjustedPageSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
