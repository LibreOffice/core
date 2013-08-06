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


#include "propbrw.hxx"
#include "basidesh.hxx"
#include "dlgedobj.hxx"
#include "iderid.hxx"
#include "baside3.hxx"

#include "dlgresid.hrc"
#include <svx/svxids.hrc>

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/inspection/XObjectInspector.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/types.hxx>
#include <cppuhelper/component_context.hxx>
#include <svx/svditer.hxx>
#include <svx/svdview.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/stdtext.hxx>

#include <boost/scoped_ptr.hpp>

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::comphelper;


void PropBrw::Update( const SfxViewShell* pShell )
{
    Shell const* pIdeShell = dynamic_cast<Shell const*>(pShell);
    OSL_ENSURE( pIdeShell || !pShell, "PropBrw::Update: invalid shell!" );
    if (pIdeShell)
        ImplUpdate(pIdeShell->GetCurrentDocument(), pIdeShell->GetCurDlgView());
    else if (pShell)
        ImplUpdate(NULL, pShell->GetDrawView());
    else
        ImplUpdate(NULL, NULL);
}


namespace
{

const long STD_WIN_SIZE_X = 300;
const long STD_WIN_SIZE_Y = 350;

const long STD_MIN_SIZE_X = 250;
const long STD_MIN_SIZE_Y = 250;

const long WIN_BORDER = 2;

} // namespace


DBG_NAME(PropBrw)


PropBrw::PropBrw (DialogWindowLayout& rLayout_):
    DockingWindow(&rLayout_),
    m_bInitialStateChange(true),
    m_xContextDocument(SfxViewShell::Current() ? SfxViewShell::Current()->GetCurrentDocument() : Reference<XModel>()),
    rLayout(rLayout_),
    pView(0)
{
    DBG_CTOR(PropBrw,NULL);

    Size aPropWinSize(STD_WIN_SIZE_X,STD_WIN_SIZE_Y);
    SetMinOutputSizePixel(Size(STD_MIN_SIZE_X,STD_MIN_SIZE_Y));
    SetOutputSizePixel(aPropWinSize);

    try
    {
        // create a frame wrapper for myself
        m_xMeAsFrame = frame::Frame::create( comphelper::getProcessComponentContext() );
        m_xMeAsFrame->initialize( VCLUnoHelper::GetInterface ( this ) );
        m_xMeAsFrame->setName( "form property browser" );  // change name!
    }
    catch (const Exception&)
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
        Reference< XComponentContext > xOwnContext = comphelper::getProcessComponentContext();

        // a ComponentContext for the
        ::cppu::ContextEntry_Init aHandlerContextInfo[] =
        {
            ::cppu::ContextEntry_Init( "DialogParentWindow", makeAny( VCLUnoHelper::GetInterface ( this ) ) ),
            ::cppu::ContextEntry_Init( "ContextDocument", makeAny( m_xContextDocument ) )
        };
        Reference< XComponentContext > xInspectorContext(
            ::cppu::createComponentContext( aHandlerContextInfo, SAL_N_ELEMENTS( aHandlerContextInfo ), xOwnContext ) );

        // create a property browser controller
        Reference< XMultiComponentFactory > xFactory( xInspectorContext->getServiceManager(), UNO_QUERY_THROW );
        static const OUString s_sControllerServiceName( "com.sun.star.awt.PropertyBrowserController" );
        m_xBrowserController = Reference< XPropertySet >(
            xFactory->createInstanceWithContext( s_sControllerServiceName, xInspectorContext ), UNO_QUERY
        );
        if ( !m_xBrowserController.is() )
        {
            ShowServiceNotAvailableError( GetParent(), s_sControllerServiceName, true );
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
                xAsXController->attachFrame( Reference<XFrame>(m_xMeAsFrame,UNO_QUERY_THROW) );
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
            m_xBrowserComponentWindow->setVisible(true);
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("PropBrw::PropBrw: could not create/initialize the browser controller!");
        try
        {
            ::comphelper::disposeComponent(m_xBrowserController);
            ::comphelper::disposeComponent(m_xBrowserComponentWindow);
        }
        catch(const Exception&)
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

    return DockingWindow::Close();
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

        boost::scoped_ptr<SdrObjListIter> pGroupIterator;
        if (pCurrent->IsGroupObject())
        {
            pGroupIterator.reset(new SdrObjListIter(*pCurrent->GetSubList()));
            pCurrent = pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
        }

        while (pCurrent)
        {
            if (DlgEdObj* pDlgEdObj = dynamic_cast<DlgEdObj*>(pCurrent))
            {
                Reference< XInterface > xControlInterface(pDlgEdObj->GetUnoControlModel(), UNO_QUERY);
                if (xControlInterface.is())
                    aInterfaces.push_back(xControlInterface);
            }

            // next element
            pCurrent = pGroupIterator && pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
        }
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

        OUString aText = IDE_RESSTR(RID_STR_BRWTITLE_PROPERTIES);
        aText += IDE_RESSTR(RID_STR_BRWTITLE_MULTISELECT);
        SetText( aText );
    }
}


void PropBrw::implSetNewObject( const Reference< XPropertySet >& _rxObject )
{
    if ( m_xBrowserController.is() )
    {
        m_xBrowserController->setPropertyValue( "IntrospectedObject",
            makeAny( _rxObject )
        );

        // set the new title according to the selected object
        SetText( GetHeadlineName( _rxObject ) );
    }
}


OUString PropBrw::GetHeadlineName( const Reference< XPropertySet >& _rxObject )
{
    OUString aName;
    Reference< lang::XServiceInfo > xServiceInfo( _rxObject, UNO_QUERY );

    if (xServiceInfo.is())    // single selection
    {
        sal_uInt16 nResId = 0;
        aName = IDE_RESSTR(RID_STR_BRWTITLE_PROPERTIES);

        if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlDialogModel" ) )
        {
            nResId = RID_STR_CLASS_DIALOG;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlButtonModel" ) )
        {
            nResId = RID_STR_CLASS_BUTTON;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlRadioButtonModel" ) )
        {
            nResId = RID_STR_CLASS_RADIOBUTTON;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlCheckBoxModel" ) )
        {
            nResId = RID_STR_CLASS_CHECKBOX;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlListBoxModel" ) )
        {
            nResId = RID_STR_CLASS_LISTBOX;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlComboBoxModel" ) )
        {
            nResId = RID_STR_CLASS_COMBOBOX;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlGroupBoxModel" ) )
        {
            nResId = RID_STR_CLASS_GROUPBOX;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlEditModel" ) )
        {
            nResId = RID_STR_CLASS_EDIT;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlFixedTextModel" ) )
        {
            nResId = RID_STR_CLASS_FIXEDTEXT;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlImageControlModel" ) )
        {
            nResId = RID_STR_CLASS_IMAGECONTROL;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlProgressBarModel" ) )
        {
            nResId = RID_STR_CLASS_PROGRESSBAR;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlScrollBarModel" ) )
        {
            nResId = RID_STR_CLASS_SCROLLBAR;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlFixedLineModel" ) )
        {
            nResId = RID_STR_CLASS_FIXEDLINE;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlDateFieldModel" ) )
        {
            nResId = RID_STR_CLASS_DATEFIELD;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlTimeFieldModel" ) )
        {
            nResId = RID_STR_CLASS_TIMEFIELD;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlNumericFieldModel" ) )
        {
            nResId = RID_STR_CLASS_NUMERICFIELD;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlCurrencyFieldModel" ) )
        {
            nResId = RID_STR_CLASS_CURRENCYFIELD;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlFormattedFieldModel" ) )
        {
            nResId = RID_STR_CLASS_FORMATTEDFIELD;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlPatternFieldModel" ) )
        {
            nResId = RID_STR_CLASS_PATTERNFIELD;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlFileControlModel" ) )
        {
            nResId = RID_STR_CLASS_FILECONTROL;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.tree.TreeControlModel" ) )
        {
            nResId = RID_STR_CLASS_TREECONTROL;
        }
        else
        {
            nResId = RID_STR_CLASS_CONTROL;
        }

        if (nResId)
        {
            aName += IDE_RESSTR(nResId);
        }
    }
    else if (!_rxObject.is())    // no properties
    {
        aName = IDE_RESSTR(RID_STR_BRWTITLE_NO_PROPERTIES);
    }

    return aName;
}


void PropBrw::Resize()
{
    DockingWindow::Resize();

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
            m_bInitialStateChange = false;
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
            if (DlgEdObj* pDlgEdObj = dynamic_cast<DlgEdObj*>(rMarkList.GetMark(0)->GetMarkedSdrObj()))
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

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
