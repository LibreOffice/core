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

#include "precompiled_sd.hxx"

#include "TaskPaneToolPanel.hxx"
#include "ToolPanelDeck.hxx"
#include "sdresid.hxx"

#include <tools/diagnose_ex.h>
#include <vcl/window.hxx>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::drawing::framework::XResourceId;
    /** === end UNO using === **/

    //==================================================================================================================
    //= TaskPaneToolPanel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    TaskPaneToolPanel::TaskPaneToolPanel( ToolPanelDeck& i_rPanelDeck, ::std::auto_ptr< ControlFactory >& i_rControlFactory,
                const Image& i_rImage, const USHORT i_nTitleResId, const ULONG i_nHelpId, const Reference< XResourceId >& i_rPanelResourceId )
        :m_pPanelDeck( &i_rPanelDeck )
        ,m_pControlFactory( i_rControlFactory )
        ,m_pControl()
        ,m_aImage( i_rImage )
        ,m_sTitle( SdResId( i_nTitleResId ) )
        ,m_aHelpId( i_nHelpId )
        ,m_xPanelResourceId( i_rPanelResourceId )
    {
        ENSURE_OR_THROW( m_pControlFactory.get(), "illegal control factory" );
    }

    //------------------------------------------------------------------------------------------------------------------
    TaskPaneToolPanel::~TaskPaneToolPanel()
    {
        m_pControl.reset();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString TaskPaneToolPanel::GetDisplayName() const
    {
        return m_sTitle;
    }

    //------------------------------------------------------------------------------------------------------------------
    Image TaskPaneToolPanel::GetImage() const
    {
        return m_aImage;
    }

    //------------------------------------------------------------------------------------------------------------------
    void TaskPaneToolPanel::Show()
    {
        ENSURE_OR_RETURN_VOID( impl_ensureControl(), "no control to show" );
        m_pControl->GetWindow()->Show();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TaskPaneToolPanel::Hide()
    {
        ENSURE_OR_RETURN_VOID( impl_ensureControl(), "no control to hide" );
        m_pControl->GetWindow()->Hide();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TaskPaneToolPanel::SetPosSizePixel( const Rectangle& i_rPanelPlayground )
    {
        ENSURE_OR_RETURN_VOID( impl_ensureControl(), "no control to position" );
        m_pControl->GetWindow()->SetPosSizePixel( i_rPanelPlayground.TopLeft(), i_rPanelPlayground.GetSize() );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TaskPaneToolPanel::GrabFocus()
    {
        ENSURE_OR_RETURN_VOID( impl_ensureControl(), "no control to focus" );
        m_pControl->GetWindow()->GrabFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    bool TaskPaneToolPanel::HasFocus() const
    {
        ENSURE_OR_RETURN_FALSE( const_cast< TaskPaneToolPanel* >( this )->impl_ensureControl(), "no control to ask" );
        return m_pControl->GetWindow()->HasChildPathFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TaskPaneToolPanel::Dispose()
    {
        ENSURE_OR_RETURN_VOID( m_pPanelDeck, "disposed twice" );
        m_pControl.reset();
        m_pPanelDeck = NULL;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool TaskPaneToolPanel::impl_ensureControl()
    {
        if ( m_pControl.get() )
            return true;
        if ( !m_pPanelDeck )
            return false;
        m_pControl = m_pControlFactory->CreateRootControl( *m_pPanelDeck );
        return ( m_pControl.get() != NULL );
    }

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................
