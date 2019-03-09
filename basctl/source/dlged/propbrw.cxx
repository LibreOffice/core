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


#include <propbrw.hxx>
#include <basidesh.hxx>
#include <dlgedobj.hxx>
#include <iderid.hxx>
#include <baside3.hxx>
#include <strings.hrc>

#include <strings.hxx>
#include <svx/svxids.hrc>

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/inspection/XObjectInspector.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/types.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/component_context.hxx>
#include <svx/svditer.hxx>
#include <svx/svdview.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/stdtext.hxx>
#include <vcl/weld.hxx>

#include <memory>

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
        ImplUpdate(nullptr, pShell->GetDrawView());
    else
        ImplUpdate(nullptr, nullptr);
}


namespace
{

const long STD_WIN_SIZE_X = 300;
const long STD_WIN_SIZE_Y = 350;

const long STD_MIN_SIZE_X = 250;
const long STD_MIN_SIZE_Y = 250;

const long WIN_BORDER = 2;

} // namespace

PropBrw::PropBrw (DialogWindowLayout& rLayout_):
    DockingWindow(&rLayout_),
    m_bInitialStateChange(true),
    m_xContextDocument(SfxViewShell::Current() ? SfxViewShell::Current()->GetCurrentDocument() : Reference<XModel>()),
    pView(nullptr)
{
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
            ::cppu::ContextEntry_Init( "DialogParentWindow", Any( VCLUnoHelper::GetInterface ( this ) ) ),
            ::cppu::ContextEntry_Init( "ContextDocument", Any( m_xContextDocument ) )
        };
        Reference< XComponentContext > xInspectorContext(
            ::cppu::createComponentContext( aHandlerContextInfo, SAL_N_ELEMENTS( aHandlerContextInfo ), xOwnContext ) );

        // create a property browser controller
        Reference< XMultiComponentFactory > xFactory( xInspectorContext->getServiceManager(), UNO_QUERY_THROW );
        static const char s_sControllerServiceName[] = "com.sun.star.awt.PropertyBrowserController";
        m_xBrowserController.set( xFactory->createInstanceWithContext( s_sControllerServiceName, xInspectorContext ), UNO_QUERY );
        if ( !m_xBrowserController.is() )
        {
            vcl::Window* pWin = GetParent();
            ShowServiceNotAvailableError(pWin ? pWin->GetFrameWeld() : nullptr, s_sControllerServiceName, true);
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
        aPropWinSize.AdjustWidth( -(2*WIN_BORDER) );
        aPropWinSize.AdjustHeight( -(2*WIN_BORDER) );

        if ( m_xBrowserComponentWindow.is() )
        {
            m_xBrowserComponentWindow->setPosSize(aPropWinPos.X(), aPropWinPos.Y(), aPropWinSize.Width(), aPropWinSize.Height(),
                css::awt::PosSize::WIDTH | css::awt::PosSize::HEIGHT |
                css::awt::PosSize::X | css::awt::PosSize::Y);
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
    disposeOnce();
}

void PropBrw::dispose()
{
    if ( m_xBrowserController.is() )
        ImplDestroyController();
    DockingWindow::dispose();
}


void PropBrw::ImplDestroyController()
{
    implSetNewObject( Reference< XPropertySet >() );

    if ( m_xMeAsFrame.is() )
        m_xMeAsFrame->setComponent( nullptr, nullptr );

    Reference< XController > xAsXController( m_xBrowserController, UNO_QUERY );
    if ( xAsXController.is() )
        xAsXController->attachFrame( nullptr );

    try
    {
        ::comphelper::disposeComponent( m_xBrowserController );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("basctl");
    }

    m_xBrowserController.clear();
}


bool PropBrw::Close()
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

    const size_t nMarkCount = _rMarkList.GetMarkCount();
    for( size_t i = 0 ; i < nMarkCount ; ++i )
    {
        SdrObject* pCurrent = _rMarkList.GetMark(i)->GetMarkedSdrObj();

        std::unique_ptr<SdrObjListIter> pGroupIterator;
        if (pCurrent->IsGroupObject())
        {
            pGroupIterator.reset(new SdrObjListIter(pCurrent->GetSubList()));
            pCurrent = pGroupIterator->IsMore() ? pGroupIterator->Next() : nullptr;
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
            pCurrent = pGroupIterator && pGroupIterator->IsMore() ? pGroupIterator->Next() : nullptr;
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

        OUString aText = IDEResId(RID_STR_BRWTITLE_PROPERTIES)
                       + IDEResId(RID_STR_BRWTITLE_MULTISELECT);
        SetText( aText );
    }
}


void PropBrw::implSetNewObject( const Reference< XPropertySet >& _rxObject )
{
    if ( m_xBrowserController.is() )
    {
        m_xBrowserController->setPropertyValue( "IntrospectedObject",
            Any( _rxObject )
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
        OUString sResId;
        aName = IDEResId(RID_STR_BRWTITLE_PROPERTIES);

        if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlDialogModel" ) )
        {
            sResId = RID_STR_CLASS_DIALOG;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlButtonModel" ) )
        {
            sResId = RID_STR_CLASS_BUTTON;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlRadioButtonModel" ) )
        {
            sResId = RID_STR_CLASS_RADIOBUTTON;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlCheckBoxModel" ) )
        {
            sResId = RID_STR_CLASS_CHECKBOX;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlListBoxModel" ) )
        {
            sResId = RID_STR_CLASS_LISTBOX;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlComboBoxModel" ) )
        {
            sResId = RID_STR_CLASS_COMBOBOX;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlGroupBoxModel" ) )
        {
            sResId = RID_STR_CLASS_GROUPBOX;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlEditModel" ) )
        {
            sResId = RID_STR_CLASS_EDIT;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlFixedTextModel" ) )
        {
            sResId = RID_STR_CLASS_FIXEDTEXT;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlImageControlModel" ) )
        {
            sResId = RID_STR_CLASS_IMAGECONTROL;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlProgressBarModel" ) )
        {
            sResId = RID_STR_CLASS_PROGRESSBAR;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlScrollBarModel" ) )
        {
            sResId = RID_STR_CLASS_SCROLLBAR;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlFixedLineModel" ) )
        {
            sResId = RID_STR_CLASS_FIXEDLINE;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlDateFieldModel" ) )
        {
            sResId = RID_STR_CLASS_DATEFIELD;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlTimeFieldModel" ) )
        {
            sResId = RID_STR_CLASS_TIMEFIELD;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlNumericFieldModel" ) )
        {
            sResId = RID_STR_CLASS_NUMERICFIELD;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlCurrencyFieldModel" ) )
        {
            sResId = RID_STR_CLASS_CURRENCYFIELD;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlFormattedFieldModel" ) )
        {
            sResId = RID_STR_CLASS_FORMATTEDFIELD;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlPatternFieldModel" ) )
        {
            sResId = RID_STR_CLASS_PATTERNFIELD;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlFileControlModel" ) )
        {
            sResId = RID_STR_CLASS_FILECONTROL;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.tree.TreeControlModel" ) )
        {
            sResId = RID_STR_CLASS_TREECONTROL;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.grid.UnoControlGridModel" ) )
        {
            sResId = RID_STR_CLASS_GRIDCONTROL;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlFixedHyperlinkModel" ) )
        {
            sResId = RID_STR_CLASS_HYPERLINKCONTROL;
        }
        else
        {
            sResId = RID_STR_CLASS_CONTROL;
        }

        if (!sResId.isEmpty())
        {
            aName += sResId;
        }
    }
    else if (!_rxObject.is())    // no properties
    {
        aName = IDEResId(RID_STR_BRWTITLE_NO_PROPERTIES);
    }

    return aName;
}


void PropBrw::Resize()
{
    DockingWindow::Resize();

    // adjust size
    Size aSize_ = GetOutputSizePixel();
    Size aPropWinSize( aSize_ );
    aPropWinSize.AdjustWidth( -(2*WIN_BORDER) );
    aPropWinSize.AdjustHeight( -(2*WIN_BORDER) );

    if (m_xBrowserComponentWindow.is())
    {
        m_xBrowserComponentWindow->setPosSize(0, 0, aPropWinSize.Width(), aPropWinSize.Height(),
            css::awt::PosSize::WIDTH | css::awt::PosSize::HEIGHT);
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
            pView = nullptr;
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
        const size_t nMarkCount = rMarkList.GetMarkCount();

        if ( nMarkCount == 0 )
        {
            EndListening( *(pView->GetModel()) );
            pView = nullptr;
            implSetNewObject( nullptr );
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
                    xNewObject.set(pDlgEdObj->GetUnoControlModel(), css::uno::UNO_QUERY);
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
        DBG_UNHANDLED_EXCEPTION("basctl");
    }
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
