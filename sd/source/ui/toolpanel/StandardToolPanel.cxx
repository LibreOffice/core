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

#include "StandardToolPanel.hxx"
#include "ToolPanelDeck.hxx"
#include "sdresid.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>

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
    using ::com::sun::star::accessibility::XAccessible;
    /** === end UNO using === **/

    //==================================================================================================================
    //= StandardToolPanel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    StandardToolPanel::StandardToolPanel( ToolPanelDeck& i_rPanelDeck, ::std::auto_ptr< ControlFactory >& i_rControlFactory,
            const USHORT i_nTitleResId, const Image& i_rImage, const ULONG i_nHelpId,
            const Reference< XResourceId >& i_rPanelResourceId )
        :TaskPaneToolPanel( i_rPanelDeck, SdResId( i_nTitleResId ), i_rImage, SmartId( i_nHelpId ) )
        ,m_pControlFactory( i_rControlFactory )
        ,m_xPanelResourceId( i_rPanelResourceId )
    {
        ENSURE_OR_THROW( m_pControlFactory.get(), "illegal control factory" );
    }

    //------------------------------------------------------------------------------------------------------------------
    StandardToolPanel::~StandardToolPanel()
    {
        m_pControl.reset();
    }

    //------------------------------------------------------------------------------------------------------------------
    void StandardToolPanel::Activate( ::Window& i_rParentWindow )
    {
        Window* pPanelWindow( impl_getPanelWindow() );
        ENSURE_OR_RETURN_VOID( pPanelWindow, "no window to show" );
        pPanelWindow->SetPosSizePixel( Point(), i_rParentWindow.GetSizePixel() );
        pPanelWindow->Show();
    }

    //------------------------------------------------------------------------------------------------------------------
    void StandardToolPanel::Deactivate()
    {
        Window* pPanelWindow( impl_getPanelWindow() );
        ENSURE_OR_RETURN_VOID( pPanelWindow, "no window to hide" );
        pPanelWindow->Hide();
    }

    //------------------------------------------------------------------------------------------------------------------
    void StandardToolPanel::SetSizePixel( const Size& i_rPanelWindowSize )
    {
        Window* pPanelWindow( impl_getPanelWindow() );
        ENSURE_OR_RETURN_VOID( pPanelWindow, "no window to resize" );
        pPanelWindow->SetSizePixel( i_rPanelWindowSize );
    }

    //------------------------------------------------------------------------------------------------------------------
    void StandardToolPanel::GrabFocus()
    {
        Window* pPanelWindow( impl_getPanelWindow() );
        ENSURE_OR_RETURN_VOID( pPanelWindow, "no window to focus" );
        pPanelWindow->GrabFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    void StandardToolPanel::Dispose()
    {
        m_pControl.reset();
        TaskPaneToolPanel::Dispose();
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > StandardToolPanel::CreatePanelAccessible( const Reference< XAccessible >& i_rParentAccessible )
    {
        ENSURE_OR_RETURN( !isDisposed(), "already disposed!", NULL );

        Window* pControlWindow( m_pControl->GetWindow() );

        Reference< XAccessible > xControlAccessible( pControlWindow->GetAccessible( FALSE ) );
        if ( !xControlAccessible.is() )
        {
            xControlAccessible = m_pControl->CreateAccessibleObject( i_rParentAccessible );
            OSL_ENSURE( xControlAccessible.is(), "StandardToolPanel::CreatePanelAccessible: invalid XAccessible returned by CreateAccessibleObject!" );
            pControlWindow->SetAccessible( xControlAccessible );
        }
        return xControlAccessible;
    }

    //------------------------------------------------------------------------------------------------------------------
    const Reference< XResourceId >& StandardToolPanel::getResourceId() const
    {
        return m_xPanelResourceId;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::Window* StandardToolPanel::impl_getPanelWindow() const
    {
        if ( const_cast< StandardToolPanel* >( this )->impl_ensureControl() )
            return m_pControl->GetWindow();
        return NULL;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool StandardToolPanel::impl_ensureControl()
    {
        if ( m_pControl.get() )
            return true;
        if ( isDisposed() )
            return false;
        m_pControl = m_pControlFactory->CreateControl( getPanelWindowAnchor() );
        return ( m_pControl.get() != NULL );
    }

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................
