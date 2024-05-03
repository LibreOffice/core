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

#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/inspection/XObjectInspector.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/types.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/component_context.hxx>
#include <tools/debug.hxx>
#include <svx/svditer.hxx>
#include <svx/svdview.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/layout.hxx>
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

const tools::Long STD_WIN_SIZE_X = 300;
const tools::Long STD_WIN_SIZE_Y = 350;

const tools::Long STD_MIN_SIZE_X = 250;
const tools::Long STD_MIN_SIZE_Y = 250;

const tools::Long WIN_BORDER = 2;

} // namespace

static Reference<XModel> lclGetModel()
{
    if (SfxViewShell* pCurrent = SfxViewShell::Current())
        return pCurrent->GetCurrentDocument();
    return Reference<XModel>();
}

PropBrw::PropBrw (DialogWindowLayout& rLayout_):
    DockingWindow(&rLayout_),
    m_xContentArea(VclPtr<VclVBox>::Create(this)),
    m_bInitialStateChange(true),
    m_xContextDocument(lclGetModel()),
    pView(nullptr)
{
    Size aPropWinSize(STD_WIN_SIZE_X,STD_WIN_SIZE_Y);
    SetMinOutputSizePixel(Size(STD_MIN_SIZE_X,STD_MIN_SIZE_Y));
    SetOutputSizePixel(aPropWinSize);

    // turn off WB_CLIPCHILDREN otherwise the bg won't extend "under"
    // transparent children of the widget
    m_xContentArea->SetControlBackground(m_xContentArea->GetSettings().GetStyleSettings().GetWindowColor());
    m_xContentArea->SetBackground(m_xContentArea->GetControlBackground());
    m_xContentArea->SetStyle(m_xContentArea->GetStyle() & ~WB_CLIPCHILDREN);
    m_xContentArea->Show();

    try
    {
        // create a frame wrapper for myself
        m_xMeAsFrame = frame::Frame::create( comphelper::getProcessComponentContext() );
        m_xMeAsFrame->initialize(VCLUnoHelper::GetInterface(m_xContentArea));
        m_xMeAsFrame->setName( u"form property browser"_ustr );  // change name!
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
            ::cppu::ContextEntry_Init( u"DialogParentWindow"_ustr, Any(VCLUnoHelper::GetInterface(this))),
            ::cppu::ContextEntry_Init( u"ContextDocument"_ustr, Any( m_xContextDocument ) )
        };
        Reference< XComponentContext > xInspectorContext(
            ::cppu::createComponentContext( aHandlerContextInfo, std::size( aHandlerContextInfo ), xOwnContext ) );

        // create a property browser controller
        Reference< XMultiComponentFactory > xFactory( xInspectorContext->getServiceManager(), UNO_SET_THROW );
        static constexpr OUString s_sControllerServiceName = u"com.sun.star.awt.PropertyBrowserController"_ustr;
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
            }
        }

        Point aPropWinPos( WIN_BORDER, WIN_BORDER );
        Size  aPropWinSize(STD_WIN_SIZE_X,STD_WIN_SIZE_Y);
        aPropWinSize.AdjustWidth( -(2*WIN_BORDER) );
        aPropWinSize.AdjustHeight( -(2*WIN_BORDER) );

        VclContainer::setLayoutAllocation(*m_xContentArea, aPropWinPos, aPropWinSize);
        m_xContentArea->Show();
    }
    catch (const Exception&)
    {
        OSL_FAIL("PropBrw::PropBrw: could not create/initialize the browser controller!");
        try
        {
            ::comphelper::disposeComponent(m_xBrowserController);
        }
        catch(const Exception&)
        {
        }

        m_xBrowserController.clear();
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
    m_xContentArea.disposeAndClear();
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
        assert(pCurrent && "GetMarkedSdrObj will succeed");

        std::optional<SdrObjListIter> oGroupIterator;
        if (pCurrent->IsGroupObject())
        {
            oGroupIterator.emplace(pCurrent->GetSubList());
            pCurrent = oGroupIterator->IsMore() ? oGroupIterator->Next() : nullptr;
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
            pCurrent = oGroupIterator && oGroupIterator->IsMore() ? oGroupIterator->Next() : nullptr;
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
        m_xBrowserController->setPropertyValue( u"IntrospectedObject"_ustr,
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

        if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlDialogModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_DIALOG;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlButtonModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_BUTTON;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlRadioButtonModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_RADIOBUTTON;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlCheckBoxModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_CHECKBOX;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlListBoxModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_LISTBOX;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlComboBoxModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_COMBOBOX;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlGroupBoxModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_GROUPBOX;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlEditModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_EDIT;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlFixedTextModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_FIXEDTEXT;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlImageControlModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_IMAGECONTROL;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlProgressBarModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_PROGRESSBAR;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlScrollBarModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_SCROLLBAR;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlFixedLineModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_FIXEDLINE;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlDateFieldModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_DATEFIELD;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlTimeFieldModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_TIMEFIELD;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlNumericFieldModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_NUMERICFIELD;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlCurrencyFieldModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_CURRENCYFIELD;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlFormattedFieldModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_FORMATTEDFIELD;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlPatternFieldModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_PATTERNFIELD;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlFileControlModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_FILECONTROL;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.tree.TreeControlModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_TREECONTROL;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.grid.UnoControlGridModel"_ustr ) )
        {
            sResId = RID_STR_CLASS_GRIDCONTROL;
        }
        else if ( xServiceInfo->supportsService( u"com.sun.star.awt.UnoControlFixedHyperlinkModel"_ustr ) )
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
            EndListening(pView->GetModel());
            pView = nullptr;
        }

        if ( !pNewView )
            return;

        pView = pNewView;

        // set focus on initialization
        if ( m_bInitialStateChange )
        {
            m_xContentArea->GrabFocus();
            m_bInitialStateChange = false;
        }

        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        const size_t nMarkCount = rMarkList.GetMarkCount();

        if ( nMarkCount == 0 )
        {
            EndListening(pView->GetModel());
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

        if ( aNewObjects.hasElements() )
            implSetNewObjectSequence( aNewObjects );
        else
            implSetNewObject( xNewObject );

        StartListening(pView->GetModel());
    }
    catch ( const PropertyVetoException& ) { /* silence */ }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("basctl");
    }
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
