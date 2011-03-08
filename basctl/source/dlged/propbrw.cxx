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
#include "precompiled_basctl.hxx"

#include <sal/macros.h>
#include "propbrw.hxx"
#include "dlgedobj.hxx"

#include "basidesh.hxx"
#include <iderid.hxx>

#include <dlgresid.hrc>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <sfx2/bindings.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/objitem.hxx>

#include <svx/svxids.hrc>
#include <tools/shl.hxx>
#include <vcl/stdtext.hxx>
#include <svx/svdview.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <sfx2/viewsh.hxx>

#include <toolkit/unohlp.hxx>
#include <comphelper/property.hxx>
#include <comphelper/composedprops.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/inspection/XObjectInspector.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/component_context.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::comphelper;


SFX_IMPL_FLOATINGWINDOW(PropBrwMgr, SID_SHOW_PROPERTYBROWSER)


PropBrwMgr::PropBrwMgr( Window* _pParent, sal_uInt16 nId,
                        SfxBindings *pBindings, SfxChildWinInfo* pInfo)
              :SfxChildWindow( _pParent, nId )
{
    // set current selection
    SfxViewShell* pShell = SfxViewShell::Current();
    pWindow = new PropBrw(
        ::comphelper::getProcessServiceFactory(),
        pBindings,
        this,
        _pParent,
        pShell ? pShell->GetCurrentDocument() : Reference< XModel >()
        );

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    ((SfxFloatingWindow*)pWindow)->Initialize( pInfo );

    ((PropBrw*)pWindow)->Update( pShell );
}


void PropBrw::Update( const SfxViewShell* _pShell )
{
    const BasicIDEShell* pBasicIDEShell = dynamic_cast< const BasicIDEShell* >( _pShell );
    OSL_ENSURE( pBasicIDEShell || !_pShell, "PropBrw::Update: invalid shell!" );
    if ( pBasicIDEShell )
    {
        ImplUpdate( pBasicIDEShell->GetCurrentDocument(), pBasicIDEShell->GetCurDlgView() );
    }
    else if ( _pShell )
    {
        ImplUpdate( NULL, _pShell->GetDrawView() );
    }
    else
    {
        ImplUpdate( NULL, NULL );
    }
}


const long STD_WIN_SIZE_X = 300;
const long STD_WIN_SIZE_Y = 350;

const long STD_MIN_SIZE_X = 250;
const long STD_MIN_SIZE_Y = 250;

const long STD_WIN_POS_X = 50;
const long STD_WIN_POS_Y = 50;
const long WIN_BORDER = 2;
const long MIN_WIN_SIZE_X = 50;
const long MIN_WIN_SIZE_Y = 50;


DBG_NAME(PropBrw)


PropBrw::PropBrw( const Reference< XMultiServiceFactory >& _xORB, SfxBindings* _pBindings, PropBrwMgr* _pMgr, Window* _pParent,
            const Reference< XModel >& _rxContextDocument )
    :SfxFloatingWindow( _pBindings, _pMgr, _pParent, WinBits( WB_STDMODELESS | WB_SIZEABLE | WB_3DLOOK | WB_ROLLABLE ) )
    ,m_bInitialStateChange(sal_True)
    ,m_xORB(_xORB)
    ,m_xContextDocument( _rxContextDocument )
    ,pView( NULL )
{
    DBG_CTOR(PropBrw,NULL);

    Size aPropWinSize(STD_WIN_SIZE_X,STD_WIN_SIZE_Y);
    SetMinOutputSizePixel(Size(STD_MIN_SIZE_X,STD_MIN_SIZE_Y));
    SetOutputSizePixel(aPropWinSize);

    try
    {
        // create a frame wrapper for myself
        m_xMeAsFrame = Reference< XFrame >(m_xORB->createInstance(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Frame" ))), UNO_QUERY);
        if (m_xMeAsFrame.is())
        {
            m_xMeAsFrame->initialize( VCLUnoHelper::GetInterface ( this ) );
            m_xMeAsFrame->setName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "form property browser" )));  // change name!
        }
    }
    catch (Exception&)
    {
        OSL_FAIL("PropBrw::PropBrw: could not create/initialize my frame!");
        m_xMeAsFrame.clear();
    }

    ImplReCreateController();
}


void PropBrw::ImplReCreateController()
{
    OSL_PRECOND( m_xMeAsFrame.is(), "PropBrw::ImplCreateController: no frame for myself!" );
    if ( !m_xMeAsFrame.is() )
        return;

    if ( m_xBrowserController.is() )
        ImplDestroyController();

    try
    {
        Reference< XPropertySet > xFactoryProperties( m_xORB, UNO_QUERY_THROW );
        Reference< XComponentContext > xOwnContext(
            xFactoryProperties->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) ),
            UNO_QUERY_THROW );

        // a ComponentContext for the
        ::cppu::ContextEntry_Init aHandlerContextInfo[] =
        {
            ::cppu::ContextEntry_Init( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DialogParentWindow" ) ), makeAny( VCLUnoHelper::GetInterface ( this ) ) ),
            ::cppu::ContextEntry_Init( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ContextDocument" ) ), makeAny( m_xContextDocument ) )
        };
        Reference< XComponentContext > xInspectorContext(
            ::cppu::createComponentContext( aHandlerContextInfo, SAL_N_ELEMENTS( aHandlerContextInfo ), xOwnContext ) );

        // create a property browser controller
        Reference< XMultiComponentFactory > xFactory( xInspectorContext->getServiceManager(), UNO_QUERY_THROW );
        static const ::rtl::OUString s_sControllerServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.PropertyBrowserController" ));
        m_xBrowserController = Reference< XPropertySet >(
            xFactory->createInstanceWithContext( s_sControllerServiceName, xInspectorContext ), UNO_QUERY
        );
        if ( !m_xBrowserController.is() )
        {
            ShowServiceNotAvailableError( GetParent(), s_sControllerServiceName, sal_True );
        }
        else
        {
            Reference< XController > xAsXController( m_xBrowserController, UNO_QUERY );
            DBG_ASSERT(xAsXController.is(), "PropBrw::PropBrw: invalid controller object!");
            if (!xAsXController.is())
            {
                ::comphelper::disposeComponent(m_xBrowserController);
                m_xBrowserController.clear();
            }
            else
            {
                xAsXController->attachFrame(m_xMeAsFrame);
                m_xBrowserComponentWindow = m_xMeAsFrame->getComponentWindow();
                DBG_ASSERT(m_xBrowserComponentWindow.is(), "PropBrw::PropBrw: attached the controller, but have no component window!");
            }
        }

        Point aPropWinPos = Point( WIN_BORDER, WIN_BORDER );
        Size  aPropWinSize(STD_WIN_SIZE_X,STD_WIN_SIZE_Y);
        aPropWinSize.Width() -= (2*WIN_BORDER);
        aPropWinSize.Height() -= (2*WIN_BORDER);

        if ( m_xBrowserComponentWindow.is() )
        {
            m_xBrowserComponentWindow->setPosSize(aPropWinPos.X(), aPropWinPos.Y(), aPropWinSize.Width(), aPropWinSize.Height(),
                ::com::sun::star::awt::PosSize::WIDTH | ::com::sun::star::awt::PosSize::HEIGHT |
                ::com::sun::star::awt::PosSize::X | ::com::sun::star::awt::PosSize::Y);
            m_xBrowserComponentWindow->setVisible(sal_True);
        }
    }
    catch (Exception&)
    {
        OSL_FAIL("PropBrw::PropBrw: could not create/initialize the browser controller!");
        try
        {
            ::comphelper::disposeComponent(m_xBrowserController);
            ::comphelper::disposeComponent(m_xBrowserComponentWindow);
        }
        catch(Exception&)
        {
        }

        m_xBrowserController.clear();
        m_xBrowserComponentWindow.clear();
    }

    Resize();
}


PropBrw::~PropBrw()
{
    if ( m_xBrowserController.is() )
        ImplDestroyController();

    DBG_DTOR(PropBrw,NULL);
}


void PropBrw::ImplDestroyController()
{
    implSetNewObject( Reference< XPropertySet >() );

    if ( m_xMeAsFrame.is() )
        m_xMeAsFrame->setComponent( NULL, NULL );

    Reference< XController > xAsXController( m_xBrowserController, UNO_QUERY );
    if ( xAsXController.is() )
        xAsXController->attachFrame( NULL );

    try
    {
        ::comphelper::disposeComponent( m_xBrowserController );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    m_xBrowserController.clear();
}


sal_Bool PropBrw::Close()
{
    ImplDestroyController();

    if( IsRollUp() )
        RollDown();

    sal_Bool bClose = SfxFloatingWindow::Close();

    return bClose;
}


Sequence< Reference< XInterface > >
    PropBrw::CreateMultiSelectionSequence( const SdrMarkList& _rMarkList )
{
    Sequence< Reference< XInterface > > aSeq;
    InterfaceArray aInterfaces;

    sal_uInt32 nMarkCount = _rMarkList.GetMarkCount();
    for( sal_uInt32 i = 0 ; i < nMarkCount ; i++ )
    {
        SdrObject* pCurrent = _rMarkList.GetMark(i)->GetMarkedSdrObj();

        SdrObjListIter* pGroupIterator = NULL;
        if (pCurrent->IsGroupObject())
        {
            pGroupIterator = new SdrObjListIter(*pCurrent->GetSubList());
            pCurrent = pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
        }

        while (pCurrent)
        {
            DlgEdObj* pDlgEdObj = PTR_CAST(DlgEdObj, pCurrent);
            if (pDlgEdObj)
            {
                Reference< XInterface > xControlInterface(pDlgEdObj->GetUnoControlModel(), UNO_QUERY);
                if (xControlInterface.is())
                    aInterfaces.push_back(xControlInterface);
            }

            // next element
            pCurrent = pGroupIterator && pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
        }
        if (pGroupIterator)
            delete pGroupIterator;
    }

    sal_Int32 nCount = aInterfaces.size();
    aSeq.realloc( nCount );
    Reference< XInterface >* pInterfaces = aSeq.getArray();
    for( sal_Int32 i = 0 ; i < nCount ; i++ )
        pInterfaces[i] = aInterfaces[i];

    return aSeq;
}


void PropBrw::implSetNewObjectSequence
    ( const Sequence< Reference< XInterface > >& _rObjectSeq )
{
    Reference< inspection::XObjectInspector > xObjectInspector(m_xBrowserController, UNO_QUERY);
    if ( xObjectInspector.is() )
    {
        xObjectInspector->inspect( _rObjectSeq );

        ::rtl::OUString aText = ::rtl::OUString(String(IDEResId(RID_STR_BRWTITLE_PROPERTIES)));
        aText += ::rtl::OUString(String(IDEResId(RID_STR_BRWTITLE_MULTISELECT)));
        SetText( aText );
    }
}


void PropBrw::implSetNewObject( const Reference< XPropertySet >& _rxObject )
{
    if ( m_xBrowserController.is() )
    {
        m_xBrowserController->setPropertyValue(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IntrospectedObject" )),
            makeAny( _rxObject )
        );

        // set the new title according to the selected object
        SetText( GetHeadlineName( _rxObject ) );
    }
}


::rtl::OUString PropBrw::GetHeadlineName( const Reference< XPropertySet >& _rxObject )
{
    ::rtl::OUString aName;
    Reference< lang::XServiceInfo > xServiceInfo( _rxObject, UNO_QUERY );

    if (xServiceInfo.is())    // single selection
    {
        sal_uInt16 nResId = 0;
        aName = ::rtl::OUString(String(IDEResId(RID_STR_BRWTITLE_PROPERTIES)));

        if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_DIALOG;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlButtonModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_BUTTON;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlRadioButtonModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_RADIOBUTTON;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlCheckBoxModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_CHECKBOX;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlListBoxModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_LISTBOX;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlComboBoxModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_COMBOBOX;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlGroupBoxModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_GROUPBOX;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlEditModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_EDIT;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFixedTextModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_FIXEDTEXT;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlImageControlModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_IMAGECONTROL;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlProgressBarModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_PROGRESSBAR;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlScrollBarModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_SCROLLBAR;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFixedLineModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_FIXEDLINE;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDateFieldModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_DATEFIELD;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlTimeFieldModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_TIMEFIELD;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlNumericFieldModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_NUMERICFIELD;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlCurrencyFieldModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_CURRENCYFIELD;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFormattedFieldModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_FORMATTEDFIELD;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlPatternFieldModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_PATTERNFIELD;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFileControlModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_FILECONTROL;
        }
        else if ( xServiceInfo->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.tree.TreeControlModel" ) ) ) )
        {
            nResId = RID_STR_CLASS_TREECONTROL;
        }
        else
        {
            nResId = RID_STR_CLASS_CONTROL;
        }

        if (nResId)
        {
            aName += ::rtl::OUString( String(IDEResId(nResId)) );
        }
    }
    else if (!_rxObject.is())    // no properties
    {
        aName = ::rtl::OUString(String(IDEResId(RID_STR_BRWTITLE_NO_PROPERTIES)));
    }

    return aName;
}


void PropBrw::FillInfo( SfxChildWinInfo& rInfo ) const
{
    rInfo.bVisible = sal_False;
}


void PropBrw::Resize()
{
    SfxFloatingWindow::Resize();

    // adjust size
    Size aSize_ = GetOutputSizePixel();
    Size aPropWinSize( aSize_ );
    aPropWinSize.Width() -= (2*WIN_BORDER);
    aPropWinSize.Height() -= (2*WIN_BORDER);

    if (m_xBrowserComponentWindow.is())
    {
        m_xBrowserComponentWindow->setPosSize(0, 0, aPropWinSize.Width(), aPropWinSize.Height(),
            ::com::sun::star::awt::PosSize::WIDTH | ::com::sun::star::awt::PosSize::HEIGHT);
    }
}


void PropBrw::ImplUpdate( const Reference< XModel >& _rxContextDocument, SdrView* pNewView )
{
    Reference< XModel > xContextDocument( _rxContextDocument );

    // if we should simply "empty" ourself, assume the context document didn't change
    if ( !pNewView )
    {
        OSL_ENSURE( !_rxContextDocument.is(), "PropBrw::ImplUpdate: no view, but a document?!" );
        xContextDocument = m_xContextDocument;
    }

    if ( xContextDocument != m_xContextDocument )
    {
        m_xContextDocument = xContextDocument;
        ImplReCreateController();
    }

    try
    {
        if ( pView )
        {
            EndListening( *(pView->GetModel()) );
            pView = NULL;
        }

        if ( !pNewView )
            return;

        pView = pNewView;

        // set focus on initialization
        if ( m_bInitialStateChange )
        {
            if ( m_xBrowserComponentWindow.is() )
                m_xBrowserComponentWindow->setFocus();
            m_bInitialStateChange = sal_False;
        }

        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        sal_uInt32 nMarkCount = rMarkList.GetMarkCount();

        if ( nMarkCount == 0 )
        {
            EndListening( *(pView->GetModel()) );
            pView = NULL;
            implSetNewObject( NULL );
            return;
        }

        Reference< XPropertySet > xNewObject;
        Sequence< Reference< XInterface > > aNewObjects;
        if ( nMarkCount == 1 )
        {
            DlgEdObj* pDlgEdObj = PTR_CAST( DlgEdObj, rMarkList.GetMark(0)->GetMarkedSdrObj() );
            if ( pDlgEdObj )
            {
                if ( pDlgEdObj->IsGroupObject() ) // group object
                    aNewObjects = CreateMultiSelectionSequence( rMarkList );
                else // single selection
                    xNewObject = xNewObject.query( pDlgEdObj->GetUnoControlModel() );
            }
        }
        else if ( nMarkCount > 1 ) // multiple selection
        {
            aNewObjects = CreateMultiSelectionSequence( rMarkList );
        }

        if ( aNewObjects.getLength() )
            implSetNewObjectSequence( aNewObjects );
        else
            implSetNewObject( xNewObject );

        StartListening( *(pView->GetModel()) );
    }
    catch ( const PropertyVetoException& ) { /* silence */ }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
