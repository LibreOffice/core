/*************************************************************************
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

#include "precompiled_sfx2.hxx"

#include "sfx2/taskpane.hxx"
#include "sfx2/sfxsids.hrc"
#include "sfxresid.hxx"
#include "helpid.hrc"

#include <tools/diagnose_ex.h>
#include <svtools/toolpanel/toolpaneldeck.hxx>

#include <boost/noncopyable.hpp>

//......................................................................................................................
namespace sfx2
{
//......................................................................................................................

#define USE_DUMMY_PANEL
#if OSL_DEBUG_LEVEL > 0
    //==================================================================================================================
    //= DummyPanel - declaration
    //==================================================================================================================
    class DummyPanel : public ::svt::ToolPanelBase
    {
    public:
        DummyPanel( Window& i_rParent );
        virtual ~DummyPanel();

        // IToolPanel
        virtual ::rtl::OUString GetDisplayName() const;
        virtual Image GetImage() const;
        virtual void Activate( Window& i_rParentWindow );
        virtual void Deactivate();
        virtual void SetSizePixel( const Size& i_rPanelWindowSize );
        virtual void GrabFocus();
        virtual void Dispose();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                    CreatePanelAccessible(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rParentAccessible
                    );

        class GreenWindow : public Window
        {
        public:
            GreenWindow( Window& i_rParent )
                :Window( &i_rParent, 0 )
            {
                SetLineColor();
                SetFillColor( COL_GREEN );
            }
            virtual void Paint( const Rectangle& i_rRect )
            {
                DrawRect( i_rRect );
            }
        };

    private:
        ::boost::scoped_ptr< Window >  m_pWindow;
    };

    //==================================================================================================================
    //= DummyPanel - implementation
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    DummyPanel::DummyPanel( Window& i_rParent )
        :m_pWindow( new GreenWindow( i_rParent ) )
    {
        m_pWindow->SetPosSizePixel( Point(), i_rParent.GetOutputSizePixel() );
    }

    //------------------------------------------------------------------------------------------------------------------
    DummyPanel::~DummyPanel()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString DummyPanel::GetDisplayName() const
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Soylent Green" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    Image DummyPanel::GetImage() const
    {
        return Image();
    }

    //------------------------------------------------------------------------------------------------------------------
    void DummyPanel::Activate( Window& i_rParentWindow )
    {
        OSL_ENSURE( &i_rParentWindow == m_pWindow->GetParent(), "DummyPanel::Activate: reparenting not supported (and not expected to be needed)!" );
        (void)i_rParentWindow;

        m_pWindow->Show();
    }

    //------------------------------------------------------------------------------------------------------------------
    void DummyPanel::Deactivate()
    {
        m_pWindow->Hide();
    }

    //------------------------------------------------------------------------------------------------------------------
    void DummyPanel::SetSizePixel( const Size& i_rPanelWindowSize )
    {
        m_pWindow->SetPosSizePixel( Point(), i_rPanelWindowSize );
    }

    //------------------------------------------------------------------------------------------------------------------
    void DummyPanel::GrabFocus()
    {
        m_pWindow->GrabFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    void DummyPanel::Dispose()
    {
        m_pWindow.reset();
    }

    //------------------------------------------------------------------------------------------------------------------
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::accessibility::XAccessible;
    Reference< XAccessible > DummyPanel::CreatePanelAccessible( const Reference< XAccessible >& i_rParentAccessible )
    {
        (void)i_rParentAccessible;
        return NULL;
    }

#endif

    //==================================================================================================================
    //= TaskPane_Impl
    //==================================================================================================================
    class TaskPane_Impl : public ::boost::noncopyable
    {
    public:
        TaskPane_Impl( TaskPane& i_rAntiImpl )
            :m_rAntiImpl( i_rAntiImpl )
            ,m_aToolPanels( i_rAntiImpl, 0 )
        {
            m_aToolPanels.Show();
#if ( OSL_DEBUG_LEVEL > 0 ) && defined ( USE_DUMMY_PANEL )
            m_aToolPanels.InsertPanel( ::svt::PToolPanel( new DummyPanel( m_aToolPanels.GetPanelWindowAnchor() ) ), m_aToolPanels.GetPanelCount() );
#endif
            OnResize();
        }

        ~TaskPane_Impl()
        {
        }

        void    OnResize();
        void    OnGetFocus();

    private:
        TaskPane&               m_rAntiImpl;
        ::svt::ToolPanelDeck    m_aToolPanels;
    };

    //------------------------------------------------------------------------------------------------------------------
    void TaskPane_Impl::OnResize()
    {
        m_aToolPanels.SetPosSizePixel( Point(), m_rAntiImpl.GetOutputSizePixel() );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TaskPane_Impl::OnGetFocus()
    {
        m_aToolPanels.GrabFocus();
    }

    //==================================================================================================================
    //= TaskPane
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    TaskPane::TaskPane( SfxBindings* i_pBindings, TaskPaneWrapper& i_rWrapper, Window* i_pParent, WinBits i_nBits )
        :SfxDockingWindow( i_pBindings, &i_rWrapper, i_pParent, i_nBits )
        ,m_pImpl( new TaskPane_Impl( *this ) )
    {
        SetText( String( SfxResId( SID_TASKPANE ) ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TaskPane::GetFocus()
    {
        SfxDockingWindow::GetFocus();
        m_pImpl->OnGetFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TaskPane::Resize()
    {
        SfxDockingWindow::Resize();
        m_pImpl->OnResize();
    }

    //==================================================================================================================
    //= TaskPaneWrapper
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    SFX_IMPL_DOCKINGWINDOW( TaskPaneWrapper, SID_TASKPANE );

    //------------------------------------------------------------------------------------------------------------------
    TaskPaneWrapper::TaskPaneWrapper( Window* i_pParent, USHORT i_nId, SfxBindings* i_pBindings, SfxChildWinInfo* i_pInfo )
        :SfxChildWindow( i_pParent, i_nId )
    {
        pWindow = new TaskPane( i_pBindings, *this, i_pParent,
            WB_STDDOCKWIN | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK | WB_ROLLABLE);
        eChildAlignment = SFX_ALIGN_RIGHT;

        pWindow->SetHelpId( HID_TASKPANE_WINDOW );
        pWindow->SetOutputSizePixel( Size( 300, 450 ) );
        pWindow->Show();

        dynamic_cast< SfxDockingWindow* >( pWindow )->Initialize( i_pInfo );
        SetHideNotDelete( TRUE );
    }

//......................................................................................................................
} // namespace sfx2
//......................................................................................................................
