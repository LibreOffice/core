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

#include <dbaccess/dataview.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/types.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <sfx2/app.hxx>
#include <sfx2/imgmgr.hxx>
#include <dbaccess/IController.hxx>
#include "UITools.hxx"
#include <sfx2/sfx.hrc>
#include <svtools/imgdef.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/settings.hxx>

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::frame;

    ODataView::ODataView(   vcl::Window* pParent,
                            IController& _rController,
                            const Reference< XComponentContext >& _rxContext,
                            WinBits nStyle)
        :Window(pParent,nStyle)
        ,m_xContext(_rxContext)
        ,m_xController( &_rController )
        ,m_aSeparator( VclPtr<FixedLine>::Create(this) )
    {
        m_pAccel.reset(::svt::AcceleratorExecute::createAcceleratorHelper());
        m_aSeparator->Show();
    }

    void ODataView::Construct()
    {
    }

    ODataView::~ODataView()
    {
        disposeOnce();
    }

    void ODataView::dispose()
    {
        m_xController.clear();
        m_aSeparator.disposeAndClear();
        m_pAccel.reset();
        vcl::Window::dispose();
    }

    void ODataView::resizeDocumentView( Rectangle& /*_rPlayground*/ )
    {
    }

    void ODataView::Paint(vcl::RenderContext& rRenderContext, const Rectangle& _rRect)
    {
        // draw the background
        {
            rRenderContext.Push(PushFlags::LINECOLOR | PushFlags::FILLCOLOR);
            rRenderContext.SetLineColor(COL_TRANSPARENT);
            rRenderContext.SetFillColor(GetSettings().GetStyleSettings().GetFaceColor());
            rRenderContext.DrawRect(_rRect);
            rRenderContext.Pop();
        }

        // let the base class do anything it needs
        Window::Paint(rRenderContext, _rRect);
    }

    void ODataView::resizeAll( const Rectangle& _rPlayground )
    {
        Rectangle aPlayground( _rPlayground );

        // position the separator
        const Size aSeparatorSize( aPlayground.GetWidth(), 2 );
        m_aSeparator->SetPosSizePixel( aPlayground.TopLeft(), aSeparatorSize );
        aPlayground.Top() += aSeparatorSize.Height() + 1;

        // position the controls of the document's view
        resizeDocumentView( aPlayground );
    }

    void ODataView::Resize()
    {
        Window::Resize();
        resizeAll( Rectangle( Point( 0, 0), GetSizePixel() ) );
    }
    bool ODataView::PreNotify( NotifyEvent& _rNEvt )
    {
        bool bHandled = false;
        switch ( _rNEvt.GetType() )
        {
            case MouseNotifyEvent::KEYINPUT:
            {
                const KeyEvent* pKeyEvent = _rNEvt.GetKeyEvent();
                const vcl::KeyCode& aKeyCode = pKeyEvent->GetKeyCode();
                if ( m_pAccel.get() && m_pAccel->execute( aKeyCode ) )
                    // the accelerator consumed the event
                    return true;
                SAL_FALLTHROUGH;
            }
            case MouseNotifyEvent::KEYUP:
            case MouseNotifyEvent::MOUSEBUTTONDOWN:
            case MouseNotifyEvent::MOUSEBUTTONUP:
                bHandled = m_xController->interceptUserInput( _rNEvt );
                break;
            default:
                break;
        }
        return bHandled || Window::PreNotify( _rNEvt );
    }
    void ODataView::StateChanged( StateChangedType nType )
    {
        Window::StateChanged( nType );

        if ( nType == StateChangedType::ControlBackground )
        {
            // Check if we need to get new images for normal/high contrast mode
            m_xController->notifyHiContrastChanged();
        }

        if ( nType == StateChangedType::InitShow )
        {
            // now that there's a view which is finally visible, remove the "Hidden" value from the
            // model's arguments.
            try
            {
                Reference< XController > xController( m_xController->getXController(), UNO_SET_THROW );
                Reference< XModel > xModel( xController->getModel(), UNO_QUERY );
                if ( xModel.is() )
                {
                    ::comphelper::NamedValueCollection aArgs( xModel->getArgs() );
                    aArgs.remove( "Hidden" );
                    xModel->attachResource( xModel->getURL(), aArgs.getPropertyValues() );
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }
    void ODataView::DataChanged( const DataChangedEvent& rDCEvt )
    {
        Window::DataChanged( rDCEvt );

        if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
            (rDCEvt.GetType() == DataChangedEventType::DISPLAY) ||
            (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
            ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
            (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
        {
            // Check if we need to get new images for normal/high contrast mode
            m_xController->notifyHiContrastChanged();
        }
    }
    void ODataView::attachFrame(const Reference< XFrame >& _xFrame)
    {
        m_pAccel->init(m_xContext, _xFrame);
    }
}

// namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
