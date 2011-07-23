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
#include "precompiled_dbaccess.hxx"

#include "querycontainerwindow.hxx"
#include "QueryDesignView.hxx"
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include "JoinController.hxx"
#include <toolkit/unohlp.hxx>
#include "dbustrings.hrc"
#include <sfx2/sfxsids.hrc>
#include <vcl/fixed.hxx>
#include "UITools.hxx"
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::beans;

    //=====================================================================
    //= OQueryContainerWindow
    //=====================================================================
    DBG_NAME(OQueryContainerWindow)
    OQueryContainerWindow::OQueryContainerWindow(Window* pParent, OQueryController& _rController,const Reference< XMultiServiceFactory >& _rFactory)
        :ODataView( pParent, _rController, _rFactory )
        ,m_pViewSwitch(NULL)
        ,m_pBeamer(NULL)
    {
        DBG_CTOR(OQueryContainerWindow,NULL);
        m_pViewSwitch = new OQueryViewSwitch( this, _rController, _rFactory );

        m_pSplitter = new Splitter(this,WB_VSCROLL);
        m_pSplitter->Hide();
        m_pSplitter->SetSplitHdl( LINK( this, OQueryContainerWindow, SplitHdl ) );
        m_pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetDialogColor() ) );
    }
    // -----------------------------------------------------------------------------
    OQueryContainerWindow::~OQueryContainerWindow()
    {
        DBG_DTOR(OQueryContainerWindow,NULL);
        {
            ::std::auto_ptr<OQueryViewSwitch> aTemp(m_pViewSwitch);
            m_pViewSwitch = NULL;
        }
        if ( m_pBeamer )
            ::dbaui::notifySystemWindow(this,m_pBeamer,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
        m_pBeamer = NULL;
        if ( m_xBeamer.is() )
        {
            Reference< ::com::sun::star::util::XCloseable > xCloseable(m_xBeamer,UNO_QUERY);
            m_xBeamer = NULL;
            if(xCloseable.is())
                xCloseable->close(sal_False); // false - holds the owner ship of this frame
        }

        ::std::auto_ptr<Window> aTemp(m_pSplitter);
        m_pSplitter = NULL;

    }
    // -----------------------------------------------------------------------------
    bool OQueryContainerWindow::switchView( ::dbtools::SQLExceptionInfo* _pErrorInfo )
    {
        return m_pViewSwitch->switchView( _pErrorInfo );
    }

    // -----------------------------------------------------------------------------
    void OQueryContainerWindow::forceInitialView()
    {
        return m_pViewSwitch->forceInitialView();
    }

    // -----------------------------------------------------------------------------
    void OQueryContainerWindow::resizeAll( const Rectangle& _rPlayground )
    {
        Rectangle aPlayground( _rPlayground );

        if ( m_pBeamer && m_pBeamer->IsVisible() )
        {
            // calc pos and size of the splitter
            Point aSplitPos     = m_pSplitter->GetPosPixel();
            Size aSplitSize     = m_pSplitter->GetOutputSizePixel();
            aSplitSize.Width() = aPlayground.GetWidth();

            if ( aSplitPos.Y() <= aPlayground.Top() )
                aSplitPos.Y() = aPlayground.Top() + sal_Int32( aPlayground.GetHeight() * 0.2 );

            if ( aSplitPos.Y() + aSplitSize.Height() > aPlayground.GetHeight() )
                aSplitPos.Y() = aPlayground.GetHeight() - aSplitSize.Height();

            // set pos and size of the splitter
            m_pSplitter->SetPosSizePixel( aSplitPos, aSplitSize );
            m_pSplitter->SetDragRectPixel(  aPlayground );

            // set pos and size of the beamer
            Size aBeamerSize( aPlayground.GetWidth(), aSplitPos.Y() );
            m_pBeamer->SetPosSizePixel( aPlayground.TopLeft(), aBeamerSize );

            // shrink the playground by the size which is occupied by the beamer
            aPlayground.Top() = aSplitPos.Y() + aSplitSize.Height();
        }

        ODataView::resizeAll( aPlayground );
    }

    // -----------------------------------------------------------------------------
    void OQueryContainerWindow::resizeDocumentView( Rectangle& _rPlayground )
    {
        m_pViewSwitch->SetPosSizePixel( _rPlayground.TopLeft(), Size( _rPlayground.GetWidth(), _rPlayground.GetHeight() ) );

        ODataView::resizeDocumentView( _rPlayground );
    }

    // -----------------------------------------------------------------------------
    void OQueryContainerWindow::GetFocus()
    {
        ODataView::GetFocus();
        if(m_pViewSwitch)
            m_pViewSwitch->GrabFocus();
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK( OQueryContainerWindow, SplitHdl, void*, /*p*/ )
    {
        m_pSplitter->SetPosPixel( Point( m_pSplitter->GetPosPixel().X(),m_pSplitter->GetSplitPosPixel() ) );
        Resize();

        return 0L;
    }

    // -----------------------------------------------------------------------------
    void OQueryContainerWindow::Construct()
    {
        m_pViewSwitch->Construct();
    }

    // -----------------------------------------------------------------------------
    void OQueryContainerWindow::disposingPreview()
    {
        if ( m_pBeamer )
        {
            // here I know that we will be destroyed from the frame
            ::dbaui::notifySystemWindow(this,m_pBeamer,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
            m_pBeamer = NULL;
            m_xBeamer = NULL;
            m_pSplitter->Hide();
            Resize();
        }
    }
    // -----------------------------------------------------------------------------
    long OQueryContainerWindow::PreNotify( NotifyEvent& rNEvt )
    {
        sal_Bool bHandled = sal_False;
        switch (rNEvt.GetType())
        {
            case  EVENT_GETFOCUS:
                if ( m_pViewSwitch )
                {
                    OJoinController& rController = m_pViewSwitch->getDesignView()->getController();
                    rController.InvalidateFeature(SID_CUT);
                    rController.InvalidateFeature(SID_COPY);
                    rController.InvalidateFeature(SID_PASTE);
                }
        }
        return bHandled ? 1L : ODataView::PreNotify(rNEvt);
    }
    // -----------------------------------------------------------------------------
    void OQueryContainerWindow::showPreview(const Reference<XFrame>& _xFrame)
    {
        if(!m_pBeamer)
        {
            m_pBeamer = new OBeamer(this);

            ::dbaui::notifySystemWindow(this,m_pBeamer,::comphelper::mem_fun(&TaskPaneList::AddWindow));

            Reference < XFrame > xBeamerFrame( m_pViewSwitch->getORB()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Frame"))),UNO_QUERY );
            m_xBeamer.set( xBeamerFrame );
            OSL_ENSURE(m_xBeamer.is(),"No frame created!");
            m_xBeamer->initialize( VCLUnoHelper::GetInterface ( m_pBeamer ) );

            // notify layout manager to not create internal toolbars
            Reference < XPropertySet > xPropSet( xBeamerFrame, UNO_QUERY );
            try
            {
                const ::rtl::OUString aLayoutManager( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ));
                Reference < XPropertySet > xLMPropSet(xPropSet->getPropertyValue( aLayoutManager ),UNO_QUERY);

                if ( xLMPropSet.is() )
                {
                    const ::rtl::OUString aAutomaticToolbars( RTL_CONSTASCII_USTRINGPARAM( "AutomaticToolbars" ));
                    xLMPropSet->setPropertyValue( aAutomaticToolbars, Any( sal_False ));
                }
            }
            catch( Exception& )
            {
            }

            m_xBeamer->setName(FRAME_NAME_QUERY_PREVIEW);

            // append our frame
            Reference < XFramesSupplier > xSup(_xFrame,UNO_QUERY);
            Reference < XFrames > xFrames = xSup->getFrames();
            xFrames->append( m_xBeamer );

            Size aSize = GetOutputSizePixel();
            Size aBeamer(aSize.Width(),sal_Int32(aSize.Height()*0.33));

            const long  nFrameHeight = LogicToPixel( Size( 0, 3 ), MAP_APPFONT ).Height();
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
    // -----------------------------------------------------------------------------


//.........................................................................
}   // namespace dbaui
//.........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
