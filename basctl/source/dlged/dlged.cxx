/*************************************************************************
 *
 *  $RCSfile: dlged.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: tbe $ $Date: 2001-09-20 09:05:27 $
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

#ifndef _BASCTL_DLGED_HXX
#include "dlged.hxx"
#endif

#ifndef _BASCTL_DLGEDFUNC_HXX
#include "dlgedfunc.hxx"
#endif

#ifndef _BASCTL_DLGEDFAC_HXX
#include "dlgedfac.hxx"
#endif

#ifndef _BASCTL_DLGEDVIEW_HXX
#include "dlgedview.hxx"
#endif

#ifndef _BASCTL_DLGEDOBJ_HXX
#include "dlgedobj.hxx"
#endif

#ifndef _BASCTL_DLGEDCLIP_HXX
#include "dlgedclip.hxx"
#endif

#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XDIALOG_HPP_
#include <com/sun/star/awt/XDialog.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _XMLSCRIPT_XML_HELPER_HXX_
#include <xmlscript/xml_helper.hxx>
#endif

#ifndef _XMLSCRIPT_XMLDLG_IMEXP_HXX_
#include <xmlscript/xmldlg_imexp.hxx>
#endif

#ifndef _BASCTL_DLGEDDEF_HXX
#include <dlgeddef.hxx>
#endif


using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::rtl;


//----------------------------------------------------------------------------

void DlgEditor::ShowDialog()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xMSF = getProcessServiceFactory();

    // create a dialog
    uno::Reference< awt::XControl > xDlg( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialog" ) ) ), uno::UNO_QUERY );

    // clone the dialog model
    uno::Reference< util::XCloneable > xC( m_xUnoControlDialogModel, uno::UNO_QUERY );
    uno::Reference< util::XCloneable > xNew = xC->createClone();
    uno::Reference< awt::XControlModel > xDlgMod( xNew, uno::UNO_QUERY );

    // set the model
    xDlg->setModel( xDlgMod );

    // create a peer
    uno::Reference< awt::XToolkit> xToolkit( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.ExtToolkit" ) ) ), uno::UNO_QUERY );
    xDlg->createPeer( xToolkit, pWindow->GetComponentInterface() );

    uno::Reference< awt::XDialog > xD( xDlg, uno::UNO_QUERY );
    xD->execute();
}

//----------------------------------------------------------------------------

BOOL DlgEditor::UnmarkDialog()
{
    SdrObject*      pDlgObj = pSdrModel->GetPage(0)->GetObj(0);
    SdrPageView*    pPgView = pSdrView->GetPageViewPvNum(0);

    BOOL bWasMarked = pSdrView->IsObjMarked( pDlgObj );

    if( bWasMarked )
        pSdrView->MarkObj( pDlgObj, pPgView, TRUE );

    return bWasMarked;
}

//----------------------------------------------------------------------------

BOOL DlgEditor::RemarkDialog()
{
    SdrObject*      pDlgObj = pSdrModel->GetPage(0)->GetObj(0);
    SdrPageView*    pPgView = pSdrView->GetPageViewPvNum(0);

    BOOL bWasMarked = pSdrView->IsObjMarked( pDlgObj );

    if( !bWasMarked )
        pSdrView->MarkObj( pDlgObj, pPgView, FALSE );

    return bWasMarked;
}

//----------------------------------------------------------------------------

DlgEditor::DlgEditor()
    :pHScroll(NULL)
    ,pVScroll(NULL)
    ,pSdrModel(NULL)
    ,pSdrPage(NULL)
    ,pSdrView(NULL)
    ,pDlgEdForm(NULL)
    ,m_xUnoControlDialogModel(NULL)
    ,m_ClipboardDataFlavors(1)
    ,pObjFac(NULL)
    ,pWindow(NULL)
    ,pFunc(NULL)
    ,eMode( DLGED_SELECT )
    ,eActObj( OBJ_DLG_PUSHBUTTON )
    ,bFirstDraw(FALSE)
    ,aGridSize( 100, 100 )  // 100TH_MM
    ,bGridVisible(FALSE)
    ,bGridSnap(TRUE)
    ,bCreateOK(TRUE)
    ,bDialogModelChanged(FALSE)
{
    pSdrModel = new DlgEdModel();
    pSdrModel->GetItemPool().FreezeIdRanges();
    pSdrModel->SetScaleUnit( MAP_100TH_MM );

    SdrLayerAdmin& rAdmin = pSdrModel->GetLayerAdmin();
    rAdmin.NewStandardLayer();
    rAdmin.NewLayer( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "HiddenLayer" ) ) );

    pSdrPage = new DlgEdPage( *pSdrModel );
    pSdrModel->InsertPage( pSdrPage );

    pObjFac = new DlgEdFactory();

    pFunc = new DlgEdFuncSelect( this );

    // set clipboard data flavor
    m_ClipboardDataFlavors[0].MimeType =                ::rtl::OUString::createFromAscii("application/vnd.sun.xml.dialog");
    m_ClipboardDataFlavors[0].HumanPresentableName =    ::rtl::OUString::createFromAscii("Dialog 6.0");
    m_ClipboardDataFlavors[0].DataType =                ::getCppuType( (const Sequence< sal_Int8 >*) 0 );

    aPaintTimer.SetTimeout( 1 );
    aPaintTimer.SetTimeoutHdl( LINK( this, DlgEditor, PaintTimeout ) );
}

//----------------------------------------------------------------------------

DlgEditor::~DlgEditor()
{
    delete pObjFac;
    delete pFunc;
    delete pSdrView;
    delete pSdrModel;
}

//----------------------------------------------------------------------------

void DlgEditor::SetWindow( Window* pWindow )
{
    DlgEditor::pWindow = pWindow;
    pWindow->SetMapMode( MapMode( MAP_100TH_MM ) );
    pSdrPage->SetSize( pWindow->PixelToLogic( Size( 1280, 1024 ) ) );

    pSdrView = new DlgEdView( pSdrModel, pWindow );
    pSdrView->ShowPagePgNum( 0, Point() );
    pSdrView->SetLayerVisible( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "HiddenLayer" ) ), FALSE );
    pSdrView->SetMoveSnapOnlyTopLeft( TRUE );

    pSdrView->SetGridCoarse( aGridSize );
    pSdrView->SetSnapGrid( aGridSize );
    pSdrView->SetGridSnap( bGridSnap );
    pSdrView->SetGridVisible( bGridVisible );
    pSdrView->SetDragStripes( FALSE );

    pSdrView->SetDesignMode( TRUE );
}

//----------------------------------------------------------------------------

void DlgEditor::SetScrollBars( ScrollBar* pHS, ScrollBar* pVS )
{
    pHScroll = pHS;
    pVScroll = pVS;

    Size aOutSize = pWindow->GetOutputSize();
    Size aPgSize  = pSdrPage->GetSize();

    pHScroll->SetRange( Range( 0, aPgSize.Width()  ));
    pVScroll->SetRange( Range( 0, aPgSize.Height() ));
    pHScroll->SetVisibleSize( (ULONG)aOutSize.Width() );
    pVScroll->SetVisibleSize( (ULONG)aOutSize.Height() );

    pHScroll->SetLineSize( aOutSize.Width() / 10 );
    pVScroll->SetLineSize( aOutSize.Height() / 10 );
    pHScroll->SetPageSize( aOutSize.Width() / 2 );
    pVScroll->SetPageSize( aOutSize.Height() / 2 );

    DoScroll( pHScroll );
    DoScroll( pVScroll );
}

//----------------------------------------------------------------------------

void DlgEditor::DoScroll( ScrollBar* pActScroll )
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

    Brush aOldBrush( pWindow->GetBackgroundBrush() );
    Brush aBrush( BRUSH_NULL );
    pWindow->SetBackgroundBrush( aBrush );
    pWindow->Scroll( -nX, -nY, SCROLL_NOCHILDREN );
    aMap.SetOrigin( Point( -aScrollPos.Width(), -aScrollPos.Height() ) );
    pWindow->SetMapMode( aMap );
    pWindow->Update();

    pWindow->SetBackgroundBrush( aOldBrush );
}

//----------------------------------------------------------------------------

void DlgEditor::SetDialog( uno::Reference< container::XNameContainer > xUnoControlDialogModel )
{
    // set dialog model
    m_xUnoControlDialogModel = xUnoControlDialogModel;

    // create dialog form
    pDlgEdForm = new DlgEdForm();
    uno::Reference< awt::XControlModel > xDlgMod( m_xUnoControlDialogModel , uno::UNO_QUERY );
    pDlgEdForm->SetUnoControlModel(xDlgMod);
    pDlgEdForm->SetDlgEditor( this );
    ((DlgEdPage*)pSdrModel->GetPage(0))->SetDlgEdForm( pDlgEdForm );
    pSdrModel->GetPage(0)->InsertObject( pDlgEdForm );
    pDlgEdForm->SetRectFromProps();
    pDlgEdForm->SortByTabIndex();       // for backward compatibility
    pDlgEdForm->StartListening();

    // create controls
    Reference< ::com::sun::star::container::XNameAccess > xNameAcc( m_xUnoControlDialogModel, UNO_QUERY );
    if ( xNameAcc.is() )
    {
           Sequence< OUString > aNames = xNameAcc->getElementNames();
           const OUString* pNames = aNames.getConstArray();
        sal_uInt32 nCtrls = aNames.getLength();

        for( sal_uInt32 n = 0; n < nCtrls; n++ )
        {
               Any aA = xNameAcc->getByName( pNames[n] );
            Reference< ::com::sun::star::awt::XControlModel > xCtrlModel;
               aA >>= xCtrlModel;
            DlgEdObj* pCtrlObj = new DlgEdObj();
            pCtrlObj->SetUnoControlModel( xCtrlModel );
            pCtrlObj->SetDlgEdForm(pDlgEdForm);
            pDlgEdForm->AddChild(pCtrlObj);                     // add child to parent form
            pSdrModel->GetPage(0)->InsertObject( pCtrlObj );    // insert object into drawing page
            pCtrlObj->SetRectFromProps();
            pCtrlObj->UpdateStep();
            pCtrlObj->StartListening();
        }
    }

    bFirstDraw = TRUE;

    pSdrModel->SetChanged( FALSE );
}

//----------------------------------------------------------------------------

void DlgEditor::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( pWindow )
        pWindow->GrabFocus();
    pFunc->MouseButtonDown( rMEvt );
}

//----------------------------------------------------------------------------

void DlgEditor::MouseButtonUp( const MouseEvent& rMEvt )
{
    BOOL bRet = pFunc->MouseButtonUp( rMEvt );

    if( (eMode == DLGED_INSERT) )
        bCreateOK = bRet;
}

//----------------------------------------------------------------------------

void DlgEditor::MouseMove( const MouseEvent& rMEvt )
{
    pFunc->MouseMove( rMEvt );
}

//----------------------------------------------------------------------------

BOOL DlgEditor::KeyInput( const KeyEvent& rKEvt )
{
    return pFunc->KeyInput( rKEvt );
}

//----------------------------------------------------------------------------

void DlgEditor::Paint( const Rectangle& rRect )
{
    aPaintRect = rRect;
    PaintTimeout( &aPaintTimer );
}

//----------------------------------------------------------------------------

IMPL_LINK( DlgEditor, PaintTimeout, Timer *, EMPTYARG )
{
    static int nInPaint = FALSE;
    if( !pSdrView )
        return 0;

    nInPaint = TRUE;

    Size aMacSize;
    if( bFirstDraw &&
        pWindow->IsVisible() &&
        (pWindow->GetOutputSize() != aMacSize) )
    {
        bFirstDraw = FALSE;

        // get property set
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSet(pDlgEdForm->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);

        if ( xPSet.is() )
        {
            // get dialog size from properties
            sal_Int32 nWidth, nHeight;
            xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" ) ) ) >>= nWidth;
            xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) ) >>= nHeight;

            if ( nWidth == 0 && nHeight == 0 )
            {
                Size   aSize = pWindow->PixelToLogic( Size( 400, 300 ) );

                // align with grid
                Size  aGridSize = pSdrView->GetSnapGrid();
                aSize.Width()  -= aSize.Width()  % aGridSize.Width();
                aSize.Height() -= aSize.Height() % aGridSize.Height();

                Point  aPos;
                Size   aOutSize = pWindow->GetOutputSize();
                aPos.X() = (aOutSize.Width()>>1)  -  (aSize.Width()>>1);
                aPos.Y() = (aOutSize.Height()>>1) -  (aSize.Height()>>1);

                // align with grid
                aPos.X() -= aPos.X() % aGridSize.Width();
                aPos.Y() -= aPos.Y() % aGridSize.Height();

                // don't put in the corner
                Point aMinPos = pWindow->PixelToLogic( Point( 30, 20 ) );
                if( (aPos.X() < aMinPos.X()) || (aPos.Y() < aMinPos.Y()) )
                {
                    aPos = aMinPos;
                    aPos.X() -= aPos.X() % aGridSize.Width();
                    aPos.Y() -= aPos.Y() % aGridSize.Height();
                }

                // set dialog position and size
                pDlgEdForm->SetSnapRect( Rectangle( aPos, aSize ) );
                pDlgEdForm->EndListening(sal_False);
                pDlgEdForm->SetPropsFromRect();
                pDlgEdForm->GetDlgEditor()->SetDialogModelChanged(TRUE);
                pDlgEdForm->StartListening();

                // set position and size of controls
                ULONG nObjCount;
                if ( pSdrPage && ( ( nObjCount = pSdrPage->GetObjCount() ) > 0 ) )
                {
                    for ( ULONG i = 0 ; i < nObjCount ; i++ )
                    {
                        SdrObject* pObj = pSdrPage->GetObj(i);
                        DlgEdObj* pDlgEdObj = PTR_CAST(DlgEdObj, pObj);
                        if ( pDlgEdObj && !pDlgEdObj->ISA(DlgEdForm) )
                            pDlgEdObj->SetRectFromProps();
                    }
                }
            }
        }
    }

    pSdrView->InitRedraw( pWindow, Region( aPaintRect ) );

    nInPaint = FALSE;

    DBG_ASSERT(pWindow,"Window not set");
    return 0;
}

//----------------------------------------------------------------------------

void DlgEditor::SetMode( DlgEdMode eNewMode )
{
    if( eMode != eNewMode )
    {
        if( pFunc )
            delete pFunc;
    }

    eMode = eNewMode;
    if( eMode == DLGED_INSERT )
        pFunc = new DlgEdFuncInsert( this );
    else
        pFunc = new DlgEdFuncSelect( this );

    if( eMode == DLGED_TEST )
        ShowDialog();
}

//----------------------------------------------------------------------------

void DlgEditor::SetInsertObj( USHORT eObj )
{
    eActObj = eObj;

    if( pSdrView )
        pSdrView->SetCurrentObj( eActObj, DlgInventor );
}

//----------------------------------------------------------------------------

USHORT DlgEditor::GetInsertObj() const
{
    return eActObj;
}

//----------------------------------------------------------------------------

void DlgEditor::Cut()
{
    Copy();
    Delete();
}

//----------------------------------------------------------------------------

void DlgEditor::Copy()
{
    if( !pSdrView->HasMarkedObj() )
        return;

    // stop all drawing actions
    pSdrView->BrkAction();

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
    ULONG nMark = pSdrView->GetMarkList().GetMarkCount();
    for( ULONG i = 0; i < nMark; i++ )
    {
        SdrObject* pObj = pSdrView->GetMarkList().GetMark(i)->GetObj();
        DlgEdObj* pDlgEdObj = PTR_CAST(DlgEdObj, pObj);

        if (pDlgEdObj && !pDlgEdObj->ISA(DlgEdForm) )
        {
            ::rtl::OUString aName;
            Reference< beans::XPropertySet >  xMarkPSet(pDlgEdObj->GetUnoControlModel(), uno::UNO_QUERY);
            if (xMarkPSet.is())
            {
                xMarkPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ) ) >>= aName;
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
    Reference< XInputStreamProvider > xISP = ::xmlscript::exportDialogModel( xClipDialogModel, xContext );
    Reference< XInputStream > xStream( xISP->createInputStream() );
    Sequence< sal_Int8 > bytes;
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
    xStream->closeInput();

    // set clipboard content
    Reference< datatransfer::clipboard::XClipboard > xClipboard = GetWindow()->GetClipboard();
    if ( xClipboard.is() )
    {
        Any aBytes;
        aBytes <<= bytes;
        Sequence< Any > aSeqData(1);
        aSeqData[0] = aBytes;
        DlgEdTransferableImpl* pTrans = new DlgEdTransferableImpl( m_ClipboardDataFlavors , aSeqData );
        const sal_uInt32 nRef = Application::ReleaseSolarMutex();
        xClipboard->setContents( pTrans , pTrans );
        Application::AcquireSolarMutex( nRef );
    }
}

//----------------------------------------------------------------------------

void DlgEditor::Paste()
{
    // stop all drawing actions
    pSdrView->BrkAction();

    // unmark all objects
    pSdrView->UnmarkAll();

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
            if ( xTransf->isDataFlavorSupported( m_ClipboardDataFlavors[0] ) )
            {
                // create clipboard dialog model from xml
                Reference< lang::XMultiServiceFactory > xMSF = getProcessServiceFactory();
                Reference< container::XNameContainer > xClipDialogModel( xMSF->createInstance
                    ( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ),
                        uno::UNO_QUERY );

                Any aAny = xTransf->getTransferData( m_ClipboardDataFlavors[0] );
                Sequence< sal_Int8 > bytes;
                aAny >>= bytes;

                if ( xClipDialogModel.is() )
                {
                    Reference< XComponentContext > xContext;
                    Reference< beans::XPropertySet > xProps( xMSF, UNO_QUERY );
                    OSL_ASSERT( xProps.is() );
                    OSL_VERIFY( xProps->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext );
                    ::xmlscript::importDialogModel( ::xmlscript::createInputStream( *((::rtl::ByteSequence*)(&bytes)) ) , xClipDialogModel, xContext );
                }

                // get control models from clipboard dialog model
                Reference< ::com::sun::star::container::XNameAccess > xNameAcc( xClipDialogModel, UNO_QUERY );
                if ( xNameAcc.is() )
                {
                       Sequence< OUString > aNames = xNameAcc->getElementNames();
                       const OUString* pNames = aNames.getConstArray();
                    sal_uInt32 nCtrls = aNames.getLength();

                    for( sal_uInt32 n = 0; n < nCtrls; n++ )
                    {
                           Any aA = xNameAcc->getByName( pNames[n] );
                        Reference< ::com::sun::star::awt::XControlModel > xCtrlModel;
                           aA >>= xCtrlModel;

                        DlgEdObj* pCtrlObj = new DlgEdObj();
                        pCtrlObj->SetDlgEdForm(pDlgEdForm);         // set parent form
                        pDlgEdForm->AddChild(pCtrlObj);             // add child to parent form
                        pCtrlObj->SetUnoControlModel( xCtrlModel ); // set control model

                        // set new name
                        ::rtl::OUString aOUniqueName( pCtrlObj->GetUniqueName() );
                        Reference< beans::XPropertySet > xPSet( xCtrlModel , UNO_QUERY );
                        Any aUniqueName;
                        aUniqueName <<= aOUniqueName;
                        xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), aUniqueName );

                        // set tabindex
                        Reference< container::XNameAccess > xNA( m_xUnoControlDialogModel , UNO_QUERY );
                           Sequence< OUString > aNames = xNA->getElementNames();
                        Any aTabIndex;
                        aTabIndex <<= (sal_Int16) aNames.getLength();
                        xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "TabIndex" ) ), aTabIndex );

                        // insert control model in editor dialog model
                        Any aCtrlModel;
                        aCtrlModel <<= xCtrlModel;
                        m_xUnoControlDialogModel->insertByName( aOUniqueName , aCtrlModel );

                        // insert object into drawing page
                        pSdrModel->GetPage(0)->InsertObject( pCtrlObj );
                        pCtrlObj->SetRectFromProps();
                        pCtrlObj->UpdateStep();
                        pCtrlObj->StartListening();                         // start listening

                        // mark object
                        SdrPageView* pPgView = pSdrView->GetPageViewPvNum(0);
                        pSdrView->MarkObj( pCtrlObj, pPgView, FALSE, TRUE);
                    }

                    // center marked objects in dialog editor form
                    Point aMarkCenter = (pSdrView->GetMarkedObjRect()).Center();
                    Point aFormCenter = (pDlgEdForm->GetSnapRect()).Center();
                    Point aPoint = aFormCenter - aMarkCenter;
                    Size  aSize( aPoint.X() , aPoint.Y() );
                    pSdrView->MoveMarkedObj( aSize );                       // update of control model properties (position + size) in NbcMove
                    pSdrView->MarkListHasChanged();

                    // dialog model changed
                    SetDialogModelChanged(TRUE);
                }
            }
        }
    }
}

//----------------------------------------------------------------------------

void DlgEditor::Delete()
{
    if( !pSdrView->HasMarkedObj() )
        return;

    // remove control models of marked objects from dialog model
    ULONG nMark = pSdrView->GetMarkList().GetMarkCount();

    for( ULONG i = 0; i < nMark; i++ )
    {
        SdrObject* pObj = pSdrView->GetMarkList().GetMark(i)->GetObj();
        DlgEdObj* pDlgEdObj = PTR_CAST(DlgEdObj, pObj);

        if ( pDlgEdObj && !pDlgEdObj->ISA(DlgEdForm) )
        {
            // get name from property
            ::rtl::OUString aName;
            uno::Reference< beans::XPropertySet >  xPSet(pDlgEdObj->GetUnoControlModel(), uno::UNO_QUERY);
            if (xPSet.is())
            {
                xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ) ) >>= aName;
            }

            // remove control from dialog model
            Reference< ::com::sun::star::container::XNameAccess > xNameAcc(pDlgEdObj->GetDlgEdForm()->GetUnoControlModel(), UNO_QUERY );
            if ( xNameAcc.is() && xNameAcc->hasByName(aName) )
            {
                Reference< ::com::sun::star::container::XNameContainer > xCont(xNameAcc, UNO_QUERY );
                if ( xCont.is() )
                {
                    xCont->removeByName( aName );
                }
            }

            // remove child from parent form
            pDlgEdForm->RemoveChild( pDlgEdObj );
        }
    }

    // update tabindex
    pDlgEdForm->SortByTabIndex();

    pSdrView->BrkAction();

    BOOL bDlgMarked = UnmarkDialog();
    pSdrView->DeleteMarked();
    if( bDlgMarked )
        RemarkDialog();
}

//----------------------------------------------------------------------------

BOOL DlgEditor::IsModified() const
{
    return pSdrModel->IsChanged() || bDialogModelChanged;
}

//----------------------------------------------------------------------------

void DlgEditor::ClearModifyFlag()
{
    pSdrModel->SetChanged( FALSE );
    bDialogModelChanged = FALSE;
}

//----------------------------------------------------------------------------

#define LMARGPRN        1700
#define RMARGPRN         900
#define TMARGPRN        2000
#define BMARGPRN        1000
#define BORDERPRN       300

//----------------------------------------------------------------------------

void lcl_PrintHeader( Printer* pPrinter, const String& rTitle ) // not working yet
{
    short nLeftMargin   = LMARGPRN;
    Size aSz = pPrinter->GetOutputSize();
    short nBorder = BORDERPRN;

    Pen aOldPen( pPrinter->GetPen() );
    Brush aOldBrush( pPrinter->GetFillInBrush() );
    Font aOldFont( pPrinter->GetFont() );

    pPrinter->SetPen( Pen( (PenStyle)PEN_SOLID ) );
    pPrinter->SetFillInBrush( Brush( BRUSH_NULL ) );

    Font aFont( aOldFont );
    aFont.SetWeight( WEIGHT_BOLD );
    aFont.SetAlign( ALIGN_BOTTOM );
    pPrinter->SetFont( aFont );

    long nFontHeight = pPrinter->GetTextHeight();

    // 1.Border => Strich, 2+3 Border = Freiraum.
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

    pPrinter->SetPen( aOldPen );
    pPrinter->SetFont( aOldFont );
    pPrinter->SetFillInBrush( aOldBrush );
}

//----------------------------------------------------------------------------

void DlgEditor::PrintData( Printer* pPrinter, const String& rTitle )    // not working yet
{
    if( pSdrView )
    {
        MapMode aOldMap( pPrinter->GetMapMode());
        Font aOldFont( pPrinter->GetFont() );

        MapMode aMap( MAP_100TH_MM );
        pPrinter->SetMapMode( aMap );
        Font aFont;
#ifdef OS2
        //aFont.SetName( System::GetStandardFont( STDFONT_SWISS ).GetName() );
#endif
        aFont.SetAlign( ALIGN_BOTTOM );
        aFont.SetSize( Size( 0, 360 ));
        pPrinter->SetFont( aFont );

        Size aPaperSz = pPrinter->GetOutputSize();
        aPaperSz.Width() -= (LMARGPRN+RMARGPRN);
        aPaperSz.Height() -= (TMARGPRN+BMARGPRN);

        pPrinter->StartPage();

        lcl_PrintHeader( pPrinter, rTitle );

        Bitmap aDlg;
#ifdef OS2
        Bitmap* pDlg = new Bitmap;
        //pSbxForm->SnapShot( *pDlg );
        SvMemoryStream* pStrm = new SvMemoryStream;
        *pStrm << *pDlg;
        delete pDlg;
        pStrm->Seek(0);
        *pStrm >> aDlg;
        delete pStrm;
#else
        //pSbxForm->SnapShot( aDlg );
#endif
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

        pPrinter->EndPage();

        pPrinter->SetMapMode( aOldMap );
        pPrinter->SetFont( aOldFont );
    }
}

//----------------------------------------------------------------------------
