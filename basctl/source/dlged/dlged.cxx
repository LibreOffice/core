/*************************************************************************
 *
 *  $RCSfile: dlged.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ab $ $Date: 2001-03-03 14:54:51 $
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

#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_CHILDWIN_HXX
#include <sfx2/childwin.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef _SBXCLASS_HXX
#include <svtools/sbx.hxx>
#endif
#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
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

#ifndef _COM_SUN_STAR_AWT_XDIALOG_HPP_
#include <com/sun/star/awt/XDialog.hpp>
#endif

#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINER_HPP_
#include <com/sun/star/script/XLibraryContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif

#pragma hdrstop

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#include <svx/svdlayer.hxx>

#include "vcsbxdef.hxx"

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

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <xmlscript/xmldlg_imexp.hxx>

using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::com::sun::star::uno;

//----------------------------------------------------------------------------

IMPL_LINK( VCDlgEditor, ClipboardCleared, Clipboard *, EMPTYARG )
{
    if( !bClipPrivate )
        return 0;

    SdrModel** ppClipPrivate = (SdrModel**)GetAppData( SHL_VCED );

    if( *ppClipPrivate )
        return 0;

    delete *ppClipPrivate;
    *ppClipPrivate = NULL;

    bClipPrivate = FALSE;
    return 0;
}

//----------------------------------------------------------------------------

void VCDlgEditor::ShowDialog()
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

BOOL VCDlgEditor::UnmarkDialog()
{
    SdrObject*      pDlgObj = pSdrModel->GetPage(0)->GetObj(0);
    SdrPageView*    pPgView = pSdrView->GetPageViewPvNum(0);

    BOOL bWasMarked = pSdrView->IsObjMarked( pDlgObj );

    if( bWasMarked )
        pSdrView->MarkObj( pDlgObj, pPgView, FALSE );

    return bWasMarked;
}

//----------------------------------------------------------------------------

void VCDlgEditor::RemarkDialog()
{
    SdrObject*      pDlgObj = pSdrModel->GetPage(0)->GetObj(0);
    SdrPageView*    pPgView = pSdrView->GetPageViewPvNum(0);

    pSdrView->MarkObj( pDlgObj, pPgView, TRUE );
}

//----------------------------------------------------------------------------

VCDlgEditor::VCDlgEditor( StarBASIC* pBas ) :
    pHScroll(NULL),
    pVScroll(NULL),
    pBasic( pBas ),
    eMode( VCDLGED_SELECT ), // eActObj( OBJ_DLG_PUSHBUTTON ),
    bFirstDraw(FALSE),
    bGridSnap(FALSE),
    bGridVisible(FALSE),
    bClipPrivate(FALSE),
    bCreateOK(TRUE),
    pSdrView(NULL)
{
    pWindow     = NULL;

    pSdrModel = new DlgEdModel;
    pSdrModel->GetItemPool().FreezeIdRanges();
    pSdrModel->SetScaleUnit( MAP_TWIP );

    SdrLayerAdmin& rAdmin = pSdrModel->GetLayerAdmin();
    rAdmin.NewStandardLayer();
    rAdmin.NewLayer( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "VCHiddenLayer" ) ) );

    pSdrPage = new DlgEdPage( *pSdrModel, pBasic, this );
    pSdrModel->InsertPage( pSdrPage );

    pObjFac = new VCDlgEditFactory( this );

    //SdrObjFactory::InsertMakeObjectHdl( LINK( pObjFac, VCDlgEditFactory, MakeObject ) );
    pFunc = new DlgEdFuncSelect( this );

    aClip.SetClearedHdl( LINK( this, VCDlgEditor, ClipboardCleared ) );

    aPaintTimer.SetTimeout( 1 );
    aPaintTimer.SetTimeoutHdl( LINK( this, VCDlgEditor, PaintTimeout ) );
}

//----------------------------------------------------------------------------

VCDlgEditor::~VCDlgEditor()
{
    ClipboardCleared( NULL );
    /*
    if( pSbxForm.Is() )
        pSbxForm->PrepareDelete();

    pSdrPage->SetDeleteSbxObject( FALSE );
    ((DlgPage*)pSdrPage)->SetSbxForm( (VCSbxDialog*)NULL );
    */

    SdrObjFactory::RemoveMakeObjectHdl( LINK( pObjFac, VCDlgEditFactory, MakeObject ) );
    delete pObjFac;

    delete pFunc;
    delete pSdrView;
    delete pSdrModel;
}

//----------------------------------------------------------------------------

void VCDlgEditor::SetWindow( Window* pWindow )
{
    VCDlgEditor::pWindow = pWindow;
    pWindow->SetMapMode( MapMode( MAP_100TH_MM ) );
    pSdrPage->SetSize( pWindow->PixelToLogic( Size( 1280, 1024 ) ) );

    pSdrView = new DlgEdView( pSdrModel, pWindow );
    pSdrView->ShowPagePgNum( 0, Point() );
    pSdrView->SetLayerVisible( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "VCHiddenLayer" ) ), FALSE );
    pSdrView->SetMoveSnapOnlyTopLeft( TRUE );

    //Size aGridSize( 60, 60 );  //Twips
    Size aGridSize( 100, 100 );  // 100TH_MM
    bGridSnap    = TRUE;
    bGridVisible = TRUE;
    pSdrView->SetGridCoarse( aGridSize );
    pSdrView->SetSnapGrid( aGridSize );
    pSdrView->SetGridSnap( bGridSnap );
    pSdrView->SetGridVisible( FALSE );
    pSdrView->SetDragStripes( FALSE );

    pSdrView->SetDesignMode( TRUE );  // tbe put this somewhere else
}

//----------------------------------------------------------------------------

void VCDlgEditor::SetScrollBars( ScrollBar* pHS, ScrollBar* pVS )
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

void VCDlgEditor::DoScroll( ScrollBar* pActScroll )
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

void VCDlgEditor::SetDialog( uno::Reference< container::XNameContainer > xUnoControlDialogModel )
{
    /* FOR TEST
    // my dialog model  --  delete this later

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xMSF = getProcessServiceFactory();


    //uno::Reference< awt::XToolkit> xToolkit( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.ExtToolkit" ) ) ), uno::UNO_QUERY );

    // Create a DialogModel
    uno::Reference< container::XNameContainer > xC( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ), uno::UNO_QUERY );
    uno::Reference< lang::XMultiServiceFactory >  xModFact( xC, uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xDlgPSet( xC, uno::UNO_QUERY );
    uno::Any aValue;
    aValue <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Test-Dialog" ) );
    xDlgPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Title" ) ), aValue );
    aValue <<= (sal_Int32) 0;
    xDlgPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ), aValue );
    aValue <<= (sal_Int32) 0;
    xDlgPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ), aValue );
    aValue <<= (sal_Int32) 0;
    xDlgPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Width" ) ), aValue );
    aValue <<= (sal_Int32) 0;
    xDlgPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Height" ) ), aValue );

    // Create a ButtonModel as a DialogModel substitute
    uno::Reference< awt::XControlModel > xDlg1( xModFact->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlButtonModel" ) ) ), uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xPSet( xDlg1, uno::UNO_QUERY );
    //aValue <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Test-Dialog" ) );
    //xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Title" ) ), aValue );
    aValue <<= (sal_Int32) 50;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ), aValue );
    aValue <<= (sal_Int32) 50;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ), aValue );
    aValue <<= (sal_Int32) 150;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Width" ) ), aValue );
    aValue <<= (sal_Int32) 150;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Height" ) ), aValue );

    // Create a ButtonModel
    uno::Reference< awt::XControlModel > xCtrl1( xModFact->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlButtonModel" ) ) ), uno::UNO_QUERY );
    xPSet = uno::Reference< beans::XPropertySet >( xCtrl1, uno::UNO_QUERY );
    aValue <<= (sal_Int32) 10;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ), aValue );
    aValue <<= (sal_Int32) 80;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ), aValue );
    aValue <<= (sal_Int32) 50;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Width" ) ), aValue );
    aValue <<= (sal_Int32) 20;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Height" ) ), aValue );
    aValue <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Button1" ) );
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), aValue );
    aValue <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Test!" ) );
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Label" ) ), aValue );
    uno::Any aAny;
    aAny <<= xCtrl1;
    xC->insertByName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Button1" ) ), aAny );

     // Create a EditModel
    uno::Reference< awt::XControlModel > xCtrl2( xModFact->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlEditModel" ) ) ), uno::UNO_QUERY );
    xPSet = uno::Reference< beans::XPropertySet >( xCtrl2, uno::UNO_QUERY );
    aValue <<= (sal_Int32) 10;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ), aValue );
    aValue <<= (sal_Int32) 50;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ), aValue );
    aValue <<= (sal_Int32) 50;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Width" ) ), aValue );
    aValue <<= (sal_Int32) 20;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Height" ) ), aValue );
    aValue <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Edit1" ) );
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), aValue );
    aValue <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Text..." ) );
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Text" ) ), aValue );
    aAny <<= xCtrl2;
    xC->insertByName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Edit1" ) ), aAny );

    // end of delete
    */

    // set dialog model
    m_xUnoControlDialogModel = xUnoControlDialogModel;


    pDlgEdForm = new DlgEdForm();
    uno::Reference< awt::XControlModel > xDlgMod( m_xUnoControlDialogModel, uno::UNO_QUERY );
    pDlgEdForm->SetUnoControlModel(xDlgMod);
    pDlgEdForm->StartPropertyListening();
    pDlgEdForm->SetDlgEditor( this );
    pDlgEdForm->SetRectFromProps();

    pSdrModel->GetPage(0)->InsertObject( pDlgEdForm );

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
            pCtrlObj->SetDlgEdForm(pDlgEdForm);
            pCtrlObj->SetUnoControlModel( xCtrlModel );
            pCtrlObj->StartPropertyListening();
            pCtrlObj->SetChanged();
            pCtrlObj->SetRectFromProps();
            pSdrModel->GetPage(0)->InsertObject( pCtrlObj );
        }
    }

    //pSdrModel->SetChanged( TRUE );

    //pNewObj->SendRepaintBroadcast();

    bFirstDraw = TRUE;

    pSdrModel->SetChanged( FALSE );

    //DBG_ASSERT(pWindow,"Window not set");
}

//----------------------------------------------------------------------------

/*
void VCDlgEditor::SetVCSbxForm( VCSbxDialogRef pForm )
{

    DBG_ASSERT( pForm.Is(), "Kein Dialog -> und tschuess..." );
    pSbxForm = pForm;

    ((DlgPage*)pSdrModel->GetPage(0))->SetName( pSbxForm->GetName() );
    ((DlgPage*)pSdrModel->GetPage(0))->SetSbxForm( (VCSbxDialog*)pSbxForm );

    // Drawobject draufklemmen und in Drawing einfuegen
    VCSbxDialogObject* pObj = new VCSbxDialogObject( pSbxForm );
    pSdrModel->GetPage(0)->InsertObject( pObj );

    // Durch das folgende Update wird SVLook am Container richtig
    // gesetzt
    pSbxForm->UpdateItem( 0 );

    pObj->SendRepaintBroadcast();

    // Childs einfuegen
    // und Zeichenobject draufklemmen
    USHORT nCount = pSbxForm->GetObjects()->Count();
    USHORT i;
    for( i=0; i < nCount; i++ )
    {
        SbxVariable* pCtl = pSbxForm->GetObjects()->Get( i );
        DBG_ASSERT( pCtl->IsA( TYPE( VCSbxControl ) ), "Is sich nix Control !!!" );
        if( (pCtl->GetSbxId() >= SBXID_FIRSTCONTROL) &&
            (pCtl->GetSbxId() <= SBXID_LASTCONTROL)      )
        {
            VCSbxDrawObject* pDrawObj = new VCSbxDrawObject( (VCSbxControl*)pCtl );
            pSdrModel->GetPage(0)->InsertObject( pDrawObj );
            pDrawObj->GetControl()->UpdateStep( pSbxForm->GetCurStep() );
            pDrawObj->SendRepaintBroadcast();
        }
    }

    bFirstDraw = TRUE;

    pSdrModel->SetChanged( FALSE );

}
*/

//----------------------------------------------------------------------------

StarBASIC* VCDlgEditor::GetBasic() const
{
    return pBasic;
}

//----------------------------------------------------------------------------

void VCDlgEditor::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( pWindow )
        pWindow->GrabFocus();
    pFunc->MouseButtonDown( rMEvt );
}

//----------------------------------------------------------------------------

void VCDlgEditor::MouseButtonUp( const MouseEvent& rMEvt )
{
    BOOL bRet = pFunc->MouseButtonUp( rMEvt );

    if( (eMode == VCDLGED_INSERT) )
        bCreateOK = bRet;
}

//----------------------------------------------------------------------------

void VCDlgEditor::MouseMove( const MouseEvent& rMEvt )
{
    pFunc->MouseMove( rMEvt );
}

//----------------------------------------------------------------------------

BOOL VCDlgEditor::KeyInput( const KeyEvent& rKEvt )
{
    return pFunc->KeyInput( rKEvt );
}

//----------------------------------------------------------------------------

void VCDlgEditor::Paint( const Rectangle& rRect )
{
    aPaintRect = rRect;
    PaintTimeout( &aPaintTimer );
}

//----------------------------------------------------------------------------

IMPL_LINK( VCDlgEditor, PaintTimeout, Timer *, EMPTYARG )
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

        if( pDlgEdForm->GetSnapRect().GetSize() == aMacSize )
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
            pDlgEdForm->SetPropsFromRect();

            // set position and size of controls
            ULONG nObjCount;
            if ( pSdrPage && ( ( nObjCount = pSdrPage->GetObjCount() ) > 0 ) )
            {
                for ( ULONG i = 1 ; i < nObjCount ; i++ )
                {
                    SdrObject* pObj = pSdrPage->GetObj(i);
                    DlgEdObj* pDlgEdObj = PTR_CAST(DlgEdObj, pObj);
                    if (pDlgEdObj)
                        pDlgEdObj->SetRectFromProps();
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

void VCDlgEditor::SetMode( VCDlgMode eNewMode )
{
    if( eMode != eNewMode )
    {
        if( pFunc )
            delete pFunc;
    }

    eMode = eNewMode;
    if( eMode == VCDLGED_INSERT )
        pFunc = new DlgEdFuncInsert( this );
    else
        pFunc = new DlgEdFuncSelect( this );

    if( eMode == VCDLGED_TEST )
        ShowDialog();
}

//----------------------------------------------------------------------------

void VCDlgEditor::SetInsertObj( USHORT eObj )
{
    eActObj = eObj;

    if( pSdrView )
        pSdrView->SetCurrentObj( eActObj, VCSbxInventor );
}

//----------------------------------------------------------------------------

USHORT VCDlgEditor::GetInsertObj() const
{
    return eActObj;
}

//----------------------------------------------------------------------------

void VCDlgEditor::Cut()
{
    if( !pSdrView->HasMarkedObj() )
        return;

    pSdrView->BrkAction();

    BOOL bDlgMarked = UnmarkDialog();

    SdrModel* pMarked = pSdrView->GetAllMarkedModel();

    aClip.Clear();
    aClip.CopyPrivateData( pMarked );

    SdrModel** ppClipPrivate = (SdrModel**)GetAppData( SHL_VCED );
    *ppClipPrivate = pMarked;
    bClipPrivate = TRUE;

    Delete();

    if( bDlgMarked )
        RemarkDialog();
}

//----------------------------------------------------------------------------

void VCDlgEditor::Copy()
{
    if( !pSdrView->HasMarkedObj() )
        return;

    pSdrView->BrkAction();

    BOOL bDlgMarked = UnmarkDialog();

    SdrModel* pMarked = pSdrView->GetAllMarkedModel();

    aClip.Clear();
    aClip.CopyPrivateData( pMarked );

    SdrModel** ppClipPrivate = (SdrModel**)GetAppData( SHL_VCED );
    *ppClipPrivate = pMarked;
    bClipPrivate   = TRUE;

    if( bDlgMarked )
        RemarkDialog();
}

//----------------------------------------------------------------------------

void VCDlgEditor::Paste()
{
    SdrModel** ppClipPrivate = (SdrModel**)GetAppData( SHL_VCED );

    pSdrView->BrkAction();

    if( !*ppClipPrivate )
        return;

    static Point aDefPoint;
    /*
    Rectangle aRect( aDefPoint,
                     pSbxForm->GetSize() );
    pSdrView->Paste( **ppClipPrivate, aRect.Center() );
    */
}

//----------------------------------------------------------------------------

void VCDlgEditor::Delete()
{
    if( !pSdrView->HasMarkedObj() )
        return;

    // Sicherstellen, dass nicht ein Dialog geloescht wird. So machts man
    // richtig (inkompat.,ab 369): SdrView::CheckPossibilities() ueberladen
    ULONG nMark = pSdrView->GetMarkList().GetMarkCount();

    for( ULONG i = 0; i < nMark; i++ )
    {
        if(pSdrView->GetMarkList().GetMark(i)->GetObj()->ISA(DlgEdForm))
            return;
    }

    // remove control models of marked objects from dialog model
    for( i = 0; i < nMark; i++ )
    {
        SdrObject* pObj = pSdrView->GetMarkList().GetMark(i)->GetObj();
        DlgEdObj* pDlgEdObj = PTR_CAST(DlgEdObj, pObj);

        if (pDlgEdObj)
        {
            // get name from property
            ::rtl::OUString aName;
            uno::Reference< beans::XPropertySet >  xPSet(pDlgEdObj->GetUnoControlModel(), uno::UNO_QUERY);
            if (xPSet.is())
            {
                xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ) ) >>= aName;
            }

            Reference< ::com::sun::star::container::XNameAccess > xNameAcc(pDlgEdObj->GetDlgEdForm()->GetUnoControlModel(), UNO_QUERY );
            if ( xNameAcc.is() && xNameAcc->hasByName(aName) )
            {
                Reference< ::com::sun::star::container::XNameContainer > xCont(xNameAcc, UNO_QUERY );
                if ( xCont.is() )
                {
                    xCont->removeByName( aName );
                }
            }
        }
    }

    pSdrView->BrkAction();

    BOOL bDlgMarked = UnmarkDialog();

    pSdrView->DeleteMarked();

    if( bDlgMarked )
        RemarkDialog();
}

//----------------------------------------------------------------------------

BOOL VCDlgEditor::IsModified() const
{
    return pSdrModel->IsChanged();
}

//----------------------------------------------------------------------------

void VCDlgEditor::ClearModifyFlag()
{
    pSdrModel->SetChanged( FALSE );
}

//----------------------------------------------------------------------------

#define LMARGPRN        1700
#define RMARGPRN         900
#define TMARGPRN        2000
#define BMARGPRN        1000
#define BORDERPRN       300

//----------------------------------------------------------------------------

void lcl_PrintHeader( Printer* pPrinter, const String& rTitle )
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

void VCDlgEditor::PrintData( Printer* pPrinter, const String& rTitle )
{
    if( pSdrView )
    {
        MapMode aOldMap( pPrinter->GetMapMode());
        Font aOldFont( pPrinter->GetFont() );

        MapMode aMap( MAP_100TH_MM );
        pPrinter->SetMapMode( aMap );
        Font aFont;
#ifdef OS2
        aFont.SetName( System::GetStandardFont( STDFONT_SWISS ).GetName() );
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
