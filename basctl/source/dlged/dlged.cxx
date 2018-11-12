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


#include <basidesh.hxx>
#include <dlged.hxx>
#include <dlgedclip.hxx>
#include <dlgeddef.hxx>
#include <dlgedfac.hxx>
#include <dlgedfunc.hxx>
#include <dlgedmod.hxx>
#include <dlgedobj.hxx>
#include <dlgedpage.hxx>
#include <dlgedview.hxx>
#include <iderdll.hxx>
#include <localizationmgr.hxx>
#include <baside3.hxx>

#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/UnoControlDialog.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/resource/StringResource.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/NumberFormatsSupplier.hpp>
#include <comphelper/types.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/itempool.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/svxids.hrc>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <xmlscript/xml_helper.hxx>
#include <xmlscript/xmldlg_imexp.hxx>

namespace basctl
{

using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;

static const char aResourceResolverPropName[] = "ResourceResolver";
static const char aDecorationPropName[] = "Decoration";


// DlgEdHint


DlgEdHint::DlgEdHint(Kind eHint)
    : eKind(eHint)
    , pDlgEdObj(nullptr)
{
}

DlgEdHint::DlgEdHint(Kind eHint, DlgEdObj* pObj)
    : eKind(eHint)
    , pDlgEdObj(pObj)
{
}

DlgEdHint::~DlgEdHint()
{
}


// DlgEditor


void DlgEditor::ShowDialog()
{
    uno::Reference< uno::XComponentContext >  xContext = getProcessComponentContext();

    // create a dialog
    uno::Reference< awt::XUnoControlDialog > xDlg = awt::UnoControlDialog::create( xContext );

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
        try
        {
            bool bDecoration = true;

            Any aDecorationAny = xSrcDlgModPropSet->getPropertyValue( aDecorationPropName );
            aDecorationAny >>= bDecoration;
            if( !bDecoration )
            {
                xNewDlgModPropSet->setPropertyValue( aDecorationPropName, Any( true ) );
                xNewDlgModPropSet->setPropertyValue( "Title", Any( OUString() ) );
            }
        }
        catch(const UnknownPropertyException& )
        {}
    }

    // set the model
    xDlg->setModel( xDlgMod );

    // create a peer
    uno::Reference< awt::XToolkit> xToolkit = awt::Toolkit::create( xContext );
    xDlg->createPeer( xToolkit, rWindow.GetComponentInterface() );

    xDlg->execute();

    // need to cast because of multiple inheritance
    Reference<awt::XControl>(xDlg)->dispose();
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
        pDlgEdView->MarkObj( pDlgObj, pPgView );

    return bWasMarked;
}


DlgEditor::DlgEditor (
    vcl::Window& rWindow_, DialogWindowLayout& rLayout_,
    css::uno::Reference<css::frame::XModel> const& xModel,
    css::uno::Reference<css::container::XNameContainer> const & xDialogModel
)
    :pHScroll(nullptr)
    ,pVScroll(nullptr)
    ,pDlgEdModel(new DlgEdModel())
    ,pDlgEdPage(new DlgEdPage(*pDlgEdModel))
    ,m_ClipboardDataFlavors(1)
    ,m_ClipboardDataFlavorsResource(2)
    ,pObjFac(new DlgEdFactory(xModel))
    ,rWindow(rWindow_)
    ,pFunc(new DlgEdFuncSelect(*this))
    ,rLayout(rLayout_)
    ,eMode( DlgEditor::SELECT )
    ,eActObj( OBJ_DLG_PUSHBUTTON )
    ,bFirstDraw(false)
    ,bCreateOK(true)
    ,bDialogModelChanged(false)
    ,aMarkIdle("basctl DlgEditor Mark")
    ,mnPaintGuard(0)
    ,m_xDocument( xModel )
{
    pDlgEdModel->GetItemPool().FreezeIdRanges();
    pDlgEdView.reset(new DlgEdView(*pDlgEdModel, rWindow_, *this));
    pDlgEdModel->SetScaleUnit( MapUnit::Map100thMM );

    SdrLayerAdmin& rAdmin = pDlgEdModel->GetLayerAdmin();
    rAdmin.NewLayer( rAdmin.GetControlLayerName() );
    rAdmin.NewLayer( "HiddenLayer" );

    pDlgEdModel->InsertPage(pDlgEdPage);

    // set clipboard data flavors
    m_ClipboardDataFlavors[0].MimeType =             "application/vnd.sun.xml.dialog" ;
    m_ClipboardDataFlavors[0].HumanPresentableName = "Dialog 6.0" ;
    m_ClipboardDataFlavors[0].DataType =             cppu::UnoType<Sequence< sal_Int8 >>::get();

    m_ClipboardDataFlavorsResource[0] =                      m_ClipboardDataFlavors[0];
    m_ClipboardDataFlavorsResource[1].MimeType =             "application/vnd.sun.xml.dialogwithresource" ;
    m_ClipboardDataFlavorsResource[1].HumanPresentableName = "Dialog 8.0" ;
    m_ClipboardDataFlavorsResource[1].DataType =             cppu::UnoType<Sequence< sal_Int8 >>::get();

    aMarkIdle.SetInvokeHandler( LINK( this, DlgEditor, MarkTimeout ) );

    rWindow.SetMapMode( MapMode( MapUnit::Map100thMM ) );
    pDlgEdPage->SetSize( rWindow.PixelToLogic( Size(DLGED_PAGE_WIDTH_MIN, DLGED_PAGE_HEIGHT_MIN) ) );

    pDlgEdView->ShowSdrPage(pDlgEdView->GetModel()->GetPage(0));
    pDlgEdView->SetLayerVisible( "HiddenLayer", false );
    pDlgEdView->SetMoveSnapOnlyTopLeft(true);
    pDlgEdView->SetWorkArea( tools::Rectangle( Point( 0, 0 ), pDlgEdPage->GetSize() ) );

    Size aGridSize( 100, 100 );  // 100TH_MM
    pDlgEdView->SetGridCoarse( aGridSize );
    pDlgEdView->SetSnapGridWidth(Fraction(aGridSize.Width(), 1), Fraction(aGridSize.Height(), 1));
    pDlgEdView->SetGridSnap( true );
    pDlgEdView->SetGridVisible( false );
    pDlgEdView->SetDragStripes(false);

    pDlgEdView->SetDesignMode();

    ::comphelper::disposeComponent( m_xControlContainer );

    SetDialog(xDialogModel);
}


DlgEditor::~DlgEditor()
{
    aMarkIdle.Stop();

    ::comphelper::disposeComponent( m_xControlContainer );
}


Reference< awt::XControlContainer > const & DlgEditor::GetWindowControlContainer()
{
    if (!m_xControlContainer.is())
        m_xControlContainer = VCLUnoHelper::CreateControlContainer(&rWindow);
    return m_xControlContainer;
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

    Size aOutSize = rWindow.GetOutputSize();
    Size aPgSize  = pDlgEdPage->GetSize();

    pHScroll->SetRange( Range( 0, aPgSize.Width()  ));
    pVScroll->SetRange( Range( 0, aPgSize.Height() ));
    pHScroll->SetVisibleSize( static_cast<sal_uLong>(aOutSize.Width()) );
    pVScroll->SetVisibleSize( static_cast<sal_uLong>(aOutSize.Height()) );

    pHScroll->SetLineSize( aOutSize.Width() / 10 );
    pVScroll->SetLineSize( aOutSize.Height() / 10 );
    pHScroll->SetPageSize( aOutSize.Width() / 2 );
    pVScroll->SetPageSize( aOutSize.Height() / 2 );

    DoScroll();
}


void DlgEditor::DoScroll()
{
    if( !pHScroll || !pVScroll )
        return;

    MapMode aMap = rWindow.GetMapMode();
    Point aOrg = aMap.GetOrigin();

    Size  aScrollPos( pHScroll->GetThumbPos(), pVScroll->GetThumbPos() );
    aScrollPos = rWindow.LogicToPixel( aScrollPos );
    aScrollPos = rWindow.PixelToLogic( aScrollPos );

    long  nX   = aScrollPos.Width() + aOrg.X();
    long  nY   = aScrollPos.Height() + aOrg.Y();

    if( !nX && !nY )
        return;

    rWindow.Update();

    // #i31562#
    // When scrolling, someone was rescuing the Wallpaper and forced the window scroll to
    // be done without background refresh. I do not know why, but that causes the repaint
    // problems. Taking that out.
    //  Wallpaper aOldBackground = rWindow.GetBackground();
    //  rWindow.SetBackground();

    // #i74769# children should be scrolled
    rWindow.Scroll( -nX, -nY, ScrollFlags::Children);
    aMap.SetOrigin( Point( -aScrollPos.Width(), -aScrollPos.Height() ) );
    rWindow.SetMapMode( aMap );
    rWindow.Update();

    DlgEdHint aHint( DlgEdHint::WINDOWSCROLLED );
    Broadcast( aHint );
}


void DlgEditor::UpdateScrollBars()
{
    MapMode aMap = rWindow.GetMapMode();
    Point aOrg = aMap.GetOrigin();

    if ( pHScroll )
        pHScroll->SetThumbPos( -aOrg.X() );

    if ( pVScroll )
        pVScroll->SetThumbPos( -aOrg.Y() );
}


void DlgEditor::SetDialog( const uno::Reference< container::XNameContainer >& xUnoControlDialogModel )
{
    // set dialog model
    m_xUnoControlDialogModel = xUnoControlDialogModel;

    // create dialog form
    pDlgEdForm = new DlgEdForm(*pDlgEdModel, *this);
    uno::Reference< awt::XControlModel > xDlgMod( m_xUnoControlDialogModel , uno::UNO_QUERY );
    pDlgEdForm->SetUnoControlModel(xDlgMod);
    static_cast<DlgEdPage*>(pDlgEdModel->GetPage(0))->SetDlgEdForm( pDlgEdForm );
    pDlgEdModel->GetPage(0)->InsertObject( pDlgEdForm );
    AdjustPageSize();
    pDlgEdForm->SetRectFromProps();
    pDlgEdForm->UpdateTabIndices();     // for backward compatibility
    pDlgEdForm->StartListening();

    // create controls
    Reference< css::container::XNameAccess > xNameAcc( m_xUnoControlDialogModel, UNO_QUERY );
    if ( xNameAcc.is() )
    {
        // get sequence of control names
        Sequence< OUString > aNames = xNameAcc->getElementNames();
        const OUString* pNames = aNames.getConstArray();
        sal_Int32 nCtrls = aNames.getLength();

        // create a map of tab indices and control names, sorted by tab index
        IndexToNameMap aIndexToNameMap;
        for ( sal_Int32 i = 0; i < nCtrls; ++i )
        {
            // get name
            OUString aName( pNames[i] );

            // get tab index
            sal_Int16 nTabIndex = -1;
            Any aCtrl = xNameAcc->getByName( aName );
            Reference< css::beans::XPropertySet > xPSet;
            aCtrl >>= xPSet;
            if ( xPSet.is() )
                xPSet->getPropertyValue( DLGED_PROP_TABINDEX ) >>= nTabIndex;

            // insert into map
            aIndexToNameMap.emplace( nTabIndex, aName );
        }

        // create controls and insert them into drawing page
        for (auto const& indexToName : aIndexToNameMap)
        {
            Any aCtrl = xNameAcc->getByName( indexToName.second );
            Reference< css::awt::XControlModel > xCtrlModel;
            aCtrl >>= xCtrlModel;
            DlgEdObj* pCtrlObj = new DlgEdObj(*pDlgEdModel);
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
    DlgEdPage* pPage = static_cast<DlgEdPage*>(pDlgEdModel->GetPage(0));
    SdrPageView* pPgView = pDlgEdView->GetSdrPageView();
    bool bWasMarked = pDlgEdView->IsObjMarked( pOldDlgEdForm );
    pDlgEdView->UnmarkAll();

    // clear SdrObjects with broadcasting
    pPage->ClearSdrObjList();

    pPage->SetDlgEdForm( nullptr );
    SetDialog( m_xUnoControlDialogModel );
    if( bWasMarked )
        pDlgEdView->MarkObj( pDlgEdForm, pPgView );
}


Reference< util::XNumberFormatsSupplier > const & DlgEditor::GetNumberFormatsSupplier()
{
    if ( !m_xSupplier.is() )
    {
        Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        Reference< util::XNumberFormatsSupplier > xSupplier( util::NumberFormatsSupplier::createWithDefaultLocale(xContext) );

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
    rWindow.GrabFocus();
    pFunc->MouseButtonDown( rMEvt );
}


void DlgEditor::MouseButtonUp( const MouseEvent& rMEvt )
{
    bool bRet = pFunc->MouseButtonUp( rMEvt );

    if( eMode == DlgEditor::INSERT )
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


void DlgEditor::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    aPaintRect = rRect;
    mnPaintGuard++;

    if (bFirstDraw && rWindow.IsVisible() && (rRenderContext.GetOutputSize() != Size()))
    {
        bFirstDraw = false;

        // get property set
        css::uno::Reference<css::beans::XPropertySet> xPSet(pDlgEdForm->GetUnoControlModel(), css::uno::UNO_QUERY);

        if (xPSet.is())
        {
            // get dialog size from properties
            sal_Int32 nWidth = 0, nHeight = 0;
            xPSet->getPropertyValue( DLGED_PROP_WIDTH ) >>= nWidth;
            xPSet->getPropertyValue( DLGED_PROP_HEIGHT ) >>= nHeight;

            if (nWidth == 0 && nHeight == 0)
            {
                Size   aSize = rRenderContext.PixelToLogic( Size( 400, 300 ) );

                // align with grid
                Size aGridSize_(long(pDlgEdView->GetSnapGridWidthX()), long(pDlgEdView->GetSnapGridWidthY()));
                aSize.AdjustWidth( -(aSize.Width()  % aGridSize_.Width()) );
                aSize.AdjustHeight( -(aSize.Height() % aGridSize_.Height()) );

                Point  aPos;
                Size   aOutSize = rRenderContext.GetOutputSize();
                aPos.setX( (aOutSize.Width()>>1)  -  (aSize.Width()>>1) );
                aPos.setY( (aOutSize.Height()>>1) -  (aSize.Height()>>1) );

                // align with grid
                aPos.AdjustX( -(aPos.X() % aGridSize_.Width()) );
                aPos.AdjustY( -(aPos.Y() % aGridSize_.Height()) );

                // don't put in the corner
                Point aMinPos = rRenderContext.PixelToLogic( Point( 30, 20 ) );
                if( (aPos.X() < aMinPos.X()) || (aPos.Y() < aMinPos.Y()) )
                {
                    aPos = aMinPos;
                    aPos.AdjustX( -(aPos.X() % aGridSize_.Width()) );
                    aPos.AdjustY( -(aPos.Y() % aGridSize_.Height()) );
                }

                // set dialog position and size
                pDlgEdForm->SetSnapRect( tools::Rectangle( aPos, aSize ) );
                pDlgEdForm->EndListening(false);
                pDlgEdForm->SetPropsFromRect();
                pDlgEdForm->GetDlgEditor().SetDialogModelChanged();
                pDlgEdForm->StartListening();

                // set position and size of controls
                if (const size_t nObjCount = pDlgEdPage->GetObjCount())
                {
                    for (size_t i = 0 ; i < nObjCount ; ++i)
                    {
                        if (DlgEdObj* pDlgEdObj = dynamic_cast<DlgEdObj*>(pDlgEdPage->GetObj(i)))
                        {
                            if (!dynamic_cast<DlgEdForm*>(pDlgEdObj))
                            {
                                pDlgEdObj->SetRectFromProps();
                            }
                        }
                    }
                }
            }
        }
    }

    // repaint, get PageView and prepare Region
    SdrPageView* pPgView = pDlgEdView->GetSdrPageView();
    const vcl::Region aPaintRectRegion(aPaintRect);

    // #i74769#
    SdrPaintWindow* pTargetPaintWindow = nullptr;

    // mark repaint start
    if (pPgView)
    {
        pTargetPaintWindow = pPgView->GetView().BeginDrawLayers(&rRenderContext, aPaintRectRegion);
        OSL_ENSURE(pTargetPaintWindow, "BeginDrawLayers: Got no SdrPaintWindow (!)");
    }

    // draw background self using wallpaper
    // #i79128# ...and use correct OutDev for that
    if (pTargetPaintWindow)
    {
        OutputDevice& rTargetOutDev = pTargetPaintWindow->GetTargetOutputDevice();
        rTargetOutDev.DrawWallpaper(aPaintRect, Wallpaper(COL_WHITE));
    }

    // do paint (unbuffered) and mark repaint end
    if (pPgView)
    {
        // paint of control layer is done in EndDrawLayers anyway...
        pPgView->GetView().EndDrawLayers(*pTargetPaintWindow, true);
    }

    mnPaintGuard--;
}


IMPL_LINK_NOARG(DlgEditor, MarkTimeout, Timer *, void)
{
    rLayout.UpdatePropertyBrowser();
}


void DlgEditor::SetMode (Mode eNewMode )
{
    if ( eNewMode != eMode )
    {
        if ( eNewMode == INSERT )
            pFunc.reset(new DlgEdFuncInsert(*this));
        else
            pFunc.reset(new DlgEdFuncSelect(*this));

        if ( eNewMode == READONLY )
            pDlgEdModel->SetReadOnly( true );
        else
            pDlgEdModel->SetReadOnly( false );
    }

    if ( eNewMode == TEST )
        ShowDialog();

    eMode = eNewMode;
}


void DlgEditor::SetInsertObj( sal_uInt16 eObj )
{
    eActObj = eObj;

    pDlgEdView->SetCurrentObj( eActObj, SdrInventor::BasicDialog );
}

void DlgEditor::CreateDefaultObject()
{
    // create object by factory
    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        *pDlgEdModel,
        pDlgEdView->GetCurrentObjInventor(),
        pDlgEdView->GetCurrentObjIdentifier());

    if (DlgEdObj* pDlgEdObj = dynamic_cast<DlgEdObj*>(pObj))
    {
        // set position and size
        Size aSize = rWindow.PixelToLogic( Size( 96, 24 ) );
        Point aPoint = pDlgEdForm->GetSnapRect().Center();
        aPoint.AdjustX( -(aSize.Width() / 2) );
        aPoint.AdjustY( -(aSize.Height() / 2) );
        pDlgEdObj->SetSnapRect( tools::Rectangle( aPoint, aSize ) );

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


static void implCopyStreamToByteSequence( const Reference< XInputStream >& xStream,
    Sequence< sal_Int8 >& bytes )
{
    xStream->readBytes( bytes, xStream->available() );
    for (;;)
    {
        Sequence< sal_Int8 > readBytes;
        sal_Int32 nRead = xStream->readBytes( readBytes, 1024 );
        if (! nRead)
            break;

        sal_Int32 nPos = bytes.getLength();
        bytes.realloc( nPos + nRead );
        memcpy( bytes.getArray() + nPos, readBytes.getConstArray(), static_cast<sal_uInt32>(nRead) );
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
    const size_t nMark = pDlgEdView->GetMarkedObjectList().GetMarkCount();
    for( size_t i = 0; i < nMark; ++i )
    {
        SdrObject* pObj = pDlgEdView->GetMarkedObjectList().GetMark(i)->GetMarkedSdrObj();
        DlgEdObj* pDlgEdObj = dynamic_cast<DlgEdObj*>(pObj);

        if (pDlgEdObj && !dynamic_cast<DlgEdForm*>(pDlgEdObj))
        {
            OUString aName;
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
    Reference< XComponentContext > xContext(
        comphelper::getProcessComponentContext() );
    Reference< XInputStreamProvider > xISP = ::xmlscript::exportDialogModel( xClipDialogModel, xContext, m_xDocument );
    Reference< XInputStream > xStream( xISP->createInputStream() );
    Sequence< sal_Int8 > DialogModelBytes;
    implCopyStreamToByteSequence( xStream, DialogModelBytes );
    xStream->closeInput();

    // set clipboard content
    Reference< datatransfer::clipboard::XClipboard > xClipboard = GetWindow().GetClipboard();
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

        DlgEdTransferableImpl* pTrans = nullptr;
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
            memcpy( pCombinedData + 4, DialogModelBytes.getConstArray(), nDialogDataLen );
            memcpy( pCombinedData + nResOffset, aResData.getConstArray(), nResDataLen );

            aSeqData[1] <<= aCombinedData;

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
        SolarMutexReleaser aReleaser;
        xClipboard->setContents( pTrans , pTrans );
    }
}


void DlgEditor::Paste()
{
    // stop all drawing actions
    pDlgEdView->BrkAction();

    // unmark all objects
    pDlgEdView->UnmarkAll();

    // get clipboard
    Reference< datatransfer::clipboard::XClipboard > xClipboard = GetWindow().GetClipboard();
    if ( xClipboard.is() )
    {
        Reference< datatransfer::XTransferable > xTransf;
        {
            SolarMutexReleaser aReleaser;
            // get clipboard content
            xTransf = xClipboard->getContents();
        }
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
                Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
                Reference< container::XNameContainer > xClipDialogModel( xContext->getServiceManager()->createInstanceWithContext(
                    "com.sun.star.awt.UnoControlDialogModel", xContext ), uno::UNO_QUERY );

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
                    memcpy( DialogModelBytes.getArray(), pCombinedData + 4, nDialogDataLen );

                    aResData.realloc( nResDataLen );
                    memcpy( aResData.getArray(), pCombinedData + nResOffset, nResDataLen );
                }
                else
                {
                    Any aAny = xTransf->getTransferData( m_ClipboardDataFlavors[0] );
                    aAny >>= DialogModelBytes;
                }

                if ( xClipDialogModel.is() )
                {
                    Reference<XInputStream> xIn = ::xmlscript::createInputStream( DialogModelBytes.getConstArray(), DialogModelBytes.getLength() );
                    ::xmlscript::importDialogModel( xIn , xClipDialogModel, xContext, m_xDocument );
                }

                // get control models from clipboard dialog model
                Reference< css::container::XNameAccess > xNameAcc( xClipDialogModel, UNO_QUERY );
                if ( xNameAcc.is() )
                {
                    Sequence< OUString > aNames = xNameAcc->getElementNames();
                    const OUString* pNames = aNames.getConstArray();
                    sal_uInt32 nCtrls = aNames.getLength();

                    Reference< resource::XStringResourcePersistence > xStringResourcePersistence;
                    if( nCtrls > 0 && bSourceIsLocalized )
                    {
                        xStringResourcePersistence = css::resource::StringResource::create( getProcessComponentContext() );
                        xStringResourcePersistence->importBinary( aResData );
                    }
                    for( sal_uInt32 n = 0; n < nCtrls; n++ )
                    {
                        Any aA = xNameAcc->getByName( pNames[n] );
                        Reference< css::awt::XControlModel > xCM;
                        aA >>= xCM;

                        // clone the control model
                        Reference< util::XCloneable > xClone( xCM, uno::UNO_QUERY );
                        Reference< awt::XControlModel > xCtrlModel( xClone->createClone(), uno::UNO_QUERY );

                        DlgEdObj* pCtrlObj = new DlgEdObj(*pDlgEdModel);
                        pCtrlObj->SetDlgEdForm(pDlgEdForm);         // set parent form
                        pDlgEdForm->AddChild(pCtrlObj);             // add child to parent form
                        pCtrlObj->SetUnoControlModel( xCtrlModel ); // set control model

                        // set new name
                        OUString aOUniqueName( pCtrlObj->GetUniqueName() );
                        Reference< beans::XPropertySet > xPSet( xCtrlModel , UNO_QUERY );
                        xPSet->setPropertyValue( DLGED_PROP_NAME, Any(aOUniqueName) );

                        // set tabindex
                        Reference< container::XNameAccess > xNA( m_xUnoControlDialogModel , UNO_QUERY );
                        Sequence< OUString > aNames_ = xNA->getElementNames();
                        xPSet->setPropertyValue( DLGED_PROP_TABINDEX, Any(static_cast<sal_Int16>(aNames_.getLength())) );

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
                    Point aMarkCenter = pDlgEdView->GetMarkedObjRect().Center();
                    Point aFormCenter = pDlgEdForm->GetSnapRect().Center();
                    Point aPoint = aFormCenter - aMarkCenter;
                    Size  aSize( aPoint.X() , aPoint.Y() );
                    pDlgEdView->MoveMarkedObj( aSize );                     // update of control model properties (position + size) in NbcMove
                    pDlgEdView->MarkListHasChanged();

                    // dialog model changed
                    SetDialogModelChanged();
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
    const size_t nMark = pDlgEdView->GetMarkedObjectList().GetMarkCount();

    for( size_t i = 0; i < nMark; ++i )
    {
        SdrObject* pObj = pDlgEdView->GetMarkedObjectList().GetMark(i)->GetMarkedSdrObj();
        DlgEdObj* pDlgEdObj = dynamic_cast<DlgEdObj*>(pObj);

        if ( pDlgEdObj && !dynamic_cast<DlgEdForm*>(pDlgEdObj) )
        {
            // get name from property
            OUString aName;
            uno::Reference< beans::XPropertySet >  xPSet(pDlgEdObj->GetUnoControlModel(), uno::UNO_QUERY);
            if (xPSet.is())
            {
                xPSet->getPropertyValue( DLGED_PROP_NAME ) >>= aName;
            }

            // remove control from dialog model
            Reference< css::container::XNameAccess > xNameAcc(pDlgEdObj->GetDlgEdForm()->GetUnoControlModel(), UNO_QUERY );
            if ( xNameAcc.is() && xNameAcc->hasByName(aName) )
            {
                Reference< css::container::XNameContainer > xCont(xNameAcc, UNO_QUERY );
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
    Reference< datatransfer::clipboard::XClipboard > xClipboard = GetWindow().GetClipboard();
    if ( xClipboard.is() )
    {
        Reference< datatransfer::XTransferable > xTransf;
        {
            SolarMutexReleaser aReleaser;
            // get clipboard content
            xTransf = xClipboard->getContents();
        }
        if (xTransf.is())
            return xTransf->isDataFlavorSupported(m_ClipboardDataFlavors[0]);
    }
    return false;
}


void DlgEditor::ShowProperties()
{
    rLayout.ShowPropertyBrowser();
}


void DlgEditor::UpdatePropertyBrowserDelayed()
{
    aMarkIdle.Start();
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


namespace Print
{
    long const nLeftMargin = 1700;
    long const nRightMargin = 900;
    long const nTopMargin = 2000;
    long const nBottomMargin = 1000;
    long const nBorder = 300;
}

static void lcl_PrintHeader( Printer* pPrinter, const OUString& rTitle ) // not working yet
{

    pPrinter->Push();

    Size const aSz = pPrinter->GetOutputSize();

    pPrinter->SetLineColor( COL_BLACK );
    pPrinter->SetFillColor();

    vcl::Font aFont( pPrinter->GetFont() );
    aFont.SetWeight( WEIGHT_BOLD );
    aFont.SetAlignment( ALIGN_BOTTOM );
    pPrinter->SetFont( aFont );

    long const nFontHeight = pPrinter->GetTextHeight();

    // 1st border => line, 2+3 border = free space
    long const nYTop = Print::nTopMargin - 3*Print::nBorder - nFontHeight;

    long const nXLeft = Print::nLeftMargin - Print::nBorder;
    long const nXRight = aSz.Width() - Print::nRightMargin + Print::nBorder;

    pPrinter->DrawRect(tools::Rectangle(
        Point(nXLeft, nYTop),
        Size(nXRight - nXLeft, aSz.Height() - nYTop - Print::nBottomMargin + Print::nBorder)
    ));

    long nY = Print::nTopMargin - 2*Print::nBorder;
    Point aPos(Print::nLeftMargin, nY);
    pPrinter->DrawText( aPos, rTitle );

    nY = Print::nTopMargin - Print::nBorder;
    pPrinter->DrawLine( Point( nXLeft, nY ), Point( nXRight, nY ) );

    pPrinter->Pop();
}


void DlgEditor::printPage( sal_Int32 nPage, Printer* pPrinter, const OUString& rTitle )
{
    if( nPage == 0 )
        Print( pPrinter, rTitle );
}


void DlgEditor::Print( Printer* pPrinter, const OUString& rTitle )    // not working yet
{
    MapMode aOldMap( pPrinter->GetMapMode());
    vcl::Font aOldFont( pPrinter->GetFont() );

    MapMode aMap( MapUnit::Map100thMM );
    pPrinter->SetMapMode( aMap );
    vcl::Font aFont;
    aFont.SetAlignment( ALIGN_BOTTOM );
    aFont.SetFontSize( Size( 0, 360 ));
    pPrinter->SetFont( aFont );

    Size aPaperSz = pPrinter->GetOutputSize();
    aPaperSz.AdjustWidth( -(Print::nLeftMargin + Print::nRightMargin) );
    aPaperSz.AdjustHeight( -(Print::nTopMargin + Print::nBottomMargin) );

    lcl_PrintHeader( pPrinter, rTitle );

    Bitmap aDlg;
    Size aBmpSz( pPrinter->PixelToLogic( aDlg.GetSizePixel() ) );
    double nPaperSzWidth = aPaperSz.Width();
    double nPaperSzHeight = aPaperSz.Height();
    double nBmpSzWidth = aBmpSz.Width();
    double nBmpSzHeight = aBmpSz.Height();
    double nScaleX = nPaperSzWidth / nBmpSzWidth;
    double nScaleY = nPaperSzHeight / nBmpSzHeight;

    Size aOutputSz;
    if( nBmpSzHeight * nScaleX <= nPaperSzHeight )
    {
        aOutputSz.setWidth( static_cast<long>(nBmpSzWidth * nScaleX) );
        aOutputSz.setHeight( static_cast<long>(nBmpSzHeight * nScaleX) );
    }
    else
    {
        aOutputSz.setWidth( static_cast<long>(nBmpSzWidth * nScaleY) );
        aOutputSz.setHeight( static_cast<long>(nBmpSzHeight * nScaleY) );
    }

    Point aPosOffs(
        (aPaperSz.Width() / 2) - (aOutputSz.Width() / 2),
        (aPaperSz.Height()/ 2) - (aOutputSz.Height() / 2));

    aPosOffs.AdjustX(Print::nLeftMargin );
    aPosOffs.AdjustY(Print::nTopMargin );

    pPrinter->DrawBitmap( aPosOffs, aOutputSz, aDlg );

    pPrinter->SetMapMode( aOldMap );
    pPrinter->SetFont( aOldFont );
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
            aPageSizeDelta = rWindow.PixelToLogic( aPageSizeDelta, MapMode( MapUnit::Map100thMM ) );

            sal_Int32 nNewPageWidth = nFormX + nFormWidth + aPageSizeDelta.Width();
            sal_Int32 nNewPageHeight = nFormY + nFormHeight + aPageSizeDelta.Height();

            Size aPageSizeMin( DLGED_PAGE_WIDTH_MIN, DLGED_PAGE_HEIGHT_MIN );
            aPageSizeMin = rWindow.PixelToLogic( aPageSizeMin, MapMode( MapUnit::Map100thMM ) );
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
                    pDlgEdView->SetWorkArea( tools::Rectangle( Point( 0, 0 ), aNewPageSize ) );
                    bAdjustedPageSize = true;
                }
            }
        }
    }

    return bAdjustedPageSize;
}


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
