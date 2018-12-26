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

#include <querycontainerwindow.hxx>
#include <QueryDesignView.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <JoinController.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <stringconstants.hxx>
#include <strings.hxx>
#include <sfx2/sfxsids.hrc>
#include <vcl/event.hxx>
#include <vcl/fixed.hxx>
#include <UITools.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <memory>

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::beans;

    // OQueryContainerWindow
    OQueryContainerWindow::OQueryContainerWindow(vcl::Window* pParent, OQueryController& _rController,const Reference< XComponentContext >& _rxContext)
        :ODataView( pParent, _rController, _rxContext )
        ,m_pViewSwitch(nullptr)
        ,m_pBeamer(nullptr)
    {
        m_pViewSwitch = new OQueryViewSwitch( this, _rController, _rxContext );

        m_pSplitter = VclPtr<Splitter>::Create(this,WB_VSCROLL);
        m_pSplitter->Hide();
        m_pSplitter->SetSplitHdl( LINK( this, OQueryContainerWindow, SplitHdl ) );
        m_pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetDialogColor() ) );
    }
    OQueryContainerWindow::~OQueryContainerWindow()
    {
        disposeOnce();
    }
    void OQueryContainerWindow::dispose()
    {
        {
            std::unique_ptr<OQueryViewSwitch> aTemp(m_pViewSwitch);
            m_pViewSwitch = nullptr;
        }
        if ( m_pBeamer )
            ::dbaui::notifySystemWindow(this,m_pBeamer,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
        m_pBeamer.clear();
        if ( m_xBeamer.is() )
        {
            Reference< css::util::XCloseable > xCloseable(m_xBeamer,UNO_QUERY);
            m_xBeamer = nullptr;
            if(xCloseable.is())
                xCloseable->close(false); // false - holds the ownership of this frame
        }

        m_pSplitter.disposeAndClear();
        ODataView::dispose();
    }
    bool OQueryContainerWindow::switchView( ::dbtools::SQLExceptionInfo* _pErrorInfo )
    {
        return m_pViewSwitch->switchView( _pErrorInfo );
    }

    void OQueryContainerWindow::forceInitialView()
    {
        return m_pViewSwitch->forceInitialView();
    }

    void OQueryContainerWindow::resizeAll( const tools::Rectangle& _rPlayground )
    {
        tools::Rectangle aPlayground( _rPlayground );

        if ( m_pBeamer && m_pBeamer->IsVisible() )
        {
            // calc pos and size of the splitter
            Point aSplitPos     = m_pSplitter->GetPosPixel();
            Size aSplitSize     = m_pSplitter->GetOutputSizePixel();
            aSplitSize.setWidth( aPlayground.GetWidth() );

            if ( aSplitPos.Y() <= aPlayground.Top() )
                aSplitPos.setY( aPlayground.Top() + sal_Int32( aPlayground.GetHeight() * 0.2 ) );

            if ( aSplitPos.Y() + aSplitSize.Height() > aPlayground.GetHeight() )
                aSplitPos.setY( aPlayground.GetHeight() - aSplitSize.Height() );

            // set pos and size of the splitter
            m_pSplitter->SetPosSizePixel( aSplitPos, aSplitSize );
            m_pSplitter->SetDragRectPixel(  aPlayground );

            // set pos and size of the beamer
            Size aBeamerSize( aPlayground.GetWidth(), aSplitPos.Y() );
            m_pBeamer->SetPosSizePixel( aPlayground.TopLeft(), aBeamerSize );

            // shrink the playground by the size which is occupied by the beamer
            aPlayground.SetTop( aSplitPos.Y() + aSplitSize.Height() );
        }

        ODataView::resizeAll( aPlayground );
    }

    void OQueryContainerWindow::resizeDocumentView( tools::Rectangle& _rPlayground )
    {
        m_pViewSwitch->SetPosSizePixel( _rPlayground.TopLeft(), Size( _rPlayground.GetWidth(), _rPlayground.GetHeight() ) );

        ODataView::resizeDocumentView( _rPlayground );
    }

    void OQueryContainerWindow::GetFocus()
    {
        ODataView::GetFocus();
        if(m_pViewSwitch)
            m_pViewSwitch->GrabFocus();
    }
    IMPL_LINK_NOARG( OQueryContainerWindow, SplitHdl, Splitter*, void )
    {
        m_pSplitter->SetPosPixel( Point( m_pSplitter->GetPosPixel().X(),m_pSplitter->GetSplitPosPixel() ) );
        Resize();
    }

    void OQueryContainerWindow::Construct()
    {
        m_pViewSwitch->Construct();
    }

    void OQueryContainerWindow::disposingPreview()
    {
        if ( m_pBeamer )
        {
            // here I know that we will be destroyed from the frame
            ::dbaui::notifySystemWindow(this,m_pBeamer,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
            m_pBeamer = nullptr;
            m_xBeamer = nullptr;
            m_pSplitter->Hide();
            Resize();
        }
    }
    bool OQueryContainerWindow::PreNotify( NotifyEvent& rNEvt )
    {
        if (rNEvt.GetType() == MouseNotifyEvent::GETFOCUS && m_pViewSwitch)
        {
            OJoinController& rController = m_pViewSwitch->getDesignView()->getController();
            rController.InvalidateFeature(SID_CUT);
            rController.InvalidateFeature(SID_COPY);
            rController.InvalidateFeature(SID_PASTE);
        }
        return ODataView::PreNotify(rNEvt);
    }
    void OQueryContainerWindow::showPreview(const Reference<XFrame>& _xFrame)
    {
        if(!m_pBeamer)
        {
            m_pBeamer = VclPtr<OBeamer>::Create(this);

            ::dbaui::notifySystemWindow(this,m_pBeamer,::comphelper::mem_fun(&TaskPaneList::AddWindow));

            m_xBeamer = Frame::create( m_pViewSwitch->getORB() );
            m_xBeamer->initialize( VCLUnoHelper::GetInterface ( m_pBeamer ) );

            // notify layout manager to not create internal toolbars
            try
            {
                Reference < XPropertySet > xLMPropSet(m_xBeamer->getLayoutManager(), UNO_QUERY);
                if ( xLMPropSet.is() )
                {
                    const OUString aAutomaticToolbars( "AutomaticToolbars" );
                    xLMPropSet->setPropertyValue( aAutomaticToolbars, Any( false ));
                }
            }
            catch( Exception& )
            {
            }

            m_xBeamer->setName(FRAME_NAME_QUERY_PREVIEW);

            // append our frame
            Reference < XFramesSupplier > xSup(_xFrame,UNO_QUERY);
            Reference < XFrames > xFrames = xSup->getFrames();
            xFrames->append( Reference<XFrame>(m_xBeamer,UNO_QUERY_THROW) );

            Size aSize = GetOutputSizePixel();
            Size aBeamer(aSize.Width(),sal_Int32(aSize.Height()*0.33));

            const long  nFrameHeight = LogicToPixel(Size(0, 3), MapMode(MapUnit::MapAppFont)).Height();
            Point aPos(0,aBeamer.Height()+nFrameHeight);

            m_pBeamer->SetPosSizePixel(Point(0,0),aBeamer);
            m_pBeamer->Show();

            m_pSplitter->SetPosSizePixel( Point(0,aBeamer.Height()), Size(aSize.Width(),nFrameHeight) );
            // a default pos for the splitter, so that the listbox is about 80 (logical) pixels wide
            m_pSplitter->SetSplitPosPixel( aBeamer.Height() );
            m_pViewSwitch->SetPosSizePixel(aPos,Size(aBeamer.Width(),aSize.Height() - aBeamer.Height()-nFrameHeight));

            m_pSplitter->Show();

            Resize();
        }
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
