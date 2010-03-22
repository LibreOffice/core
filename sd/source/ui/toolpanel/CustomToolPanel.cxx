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

#include "CustomToolPanel.hxx"
#include "framework/FrameworkHelper.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/awt/PosSize.hpp>
/** === end UNO includes === **/

#include <comphelper/processfactory.hxx>
#include <unotools/confignode.hxx>
#include <tools/diagnose_ex.h>

#if OSL_DEBUG_LEVEL > 0
#include <rtl/strbuf.hxx>
#endif

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
    using ::com::sun::star::drawing::framework::XConfigurationController;
    using ::com::sun::star::drawing::framework::XResourceId;
    using ::com::sun::star::drawing::framework::ResourceId;
    using ::com::sun::star::drawing::framework::XResource;
    using ::com::sun::star::awt::XWindow;
    /** === end UNO using === **/
    namespace PosSize = ::com::sun::star::awt::PosSize;

    //==================================================================================================================
    //= helper
    //==================================================================================================================
    namespace
    {
        ::rtl::OUString lcl_getPanelConfig( const ::utl::OConfigurationNode& i_rPanelConfig, const sal_Char* i_pAsciiPropName )
        {
            ::rtl::OUString sConfigValue;
            OSL_VERIFY( i_rPanelConfig.getNodeValue( i_pAsciiPropName ) >>= sConfigValue );
            if ( sConfigValue.getLength() == 0 )
            {
#if OSL_DEBUG_LEVEL > 0
                ::rtl::OStringBuffer aMessage;
                aMessage.append( "lcl_getPanelConfig: invalid config value (" );
                aMessage.append( i_pAsciiPropName );
                aMessage.append( ") for panel config '" );
                aMessage.append( ::rtl::OUStringToOString( i_rPanelConfig.getLocalName(), RTL_TEXTENCODING_UTF8 ) );
                aMessage.append( "'" );
                OSL_ENSURE( false, aMessage.makeStringAndClear().getStr() );
#endif
            }
            return sConfigValue;
        }
    }

    //==================================================================================================================
    //= CustomToolPanel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    CustomToolPanel::CustomToolPanel( ToolPanelDeck& i_rPanelDeck, const ::utl::OConfigurationNode& i_rPanelConfig,
            const Reference< XResourceId >& i_rPaneResourceId, const ::boost::shared_ptr< framework::FrameworkHelper >& i_pFrameworkHelper )
        :TaskPaneToolPanel( i_rPanelDeck, lcl_getPanelConfig( i_rPanelConfig, "DisplayTitle" ), Image() /* TODO */, SmartId() /* TODO */ )
        ,m_pFrameworkHelper( i_pFrameworkHelper )
        ,m_xPanelResourceId()
        ,m_xResource()
        ,m_xToolPanel()
        ,m_bAttemptedPanelCreation( false )
        ,m_nResourceAccessLock( 0 )
    {
        ENSURE_OR_THROW( m_pFrameworkHelper.get() != NULL, "invalid framework helper" );
        ENSURE_OR_THROW( i_rPaneResourceId.is(), "invalid pane resource id" );
        try
        {
            ::rtl::OUString sPanelResourceURL( lcl_getPanelConfig( i_rPanelConfig, "ResourceURL" ) );
            m_xPanelResourceId.set(
                ResourceId::createWithAnchor(
                    ::comphelper::getProcessComponentContext(),
                    sPanelResourceURL,
                    i_rPaneResourceId ),
                UNO_SET_THROW
            );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    CustomToolPanel::~CustomToolPanel()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void CustomToolPanel::Activate( ::Window& i_rParentWindow )
    {
        OSL_ENSURE( &getPanelWindowAnchor() == &i_rParentWindow,
            "CustomToolPanel::Activate: invalid new parent window" );
        // getPanelWindowAnchor() is what is returned in the TaskPane's XPane::getWindow method. So,
        // any custom panel which is loaded into the TaskPane will use this window as parent window.
        // Consequently, this is the window which shall be passed here, since this method is intended to
        // re-create the panel with the given parent.

        // we do not need to do anything here. The XResourceFactory::createResource method of the custom
        // tool panel already created and showed the window.
    }

    //------------------------------------------------------------------------------------------------------------------
    void CustomToolPanel::Deactivate()
    {
        // When a certain tool panel is activated, this is routed through the drawing framework, which ensures
        // that the resource associated with the previously active panel is deactivated, which calls the
        // XResourceFactory::destroyResource at our custom panel's factory.

        // So, we do not have to do anything here - except forgetting the XResource, at it might (or might not,
        // if cached by the factory) be re-created next time.
        m_xResource.clear();
        m_xToolPanel.clear();
        m_bAttemptedPanelCreation = false;
    }

    //------------------------------------------------------------------------------------------------------------------
    void CustomToolPanel::SetSizePixel( const Size& i_rPanelWindowSize )
    {
        impl_ensurePanel();
        if ( !m_xToolPanel.is() )
            // if the custom panel does not support XPanel, this just means it is its own responsibility
            // to resize/layout everything within the pane window.
            return;

        try
        {
            Reference< XWindow > xPanelWindow( m_xToolPanel->getWindow(), UNO_SET_THROW );
            xPanelWindow->setPosSize( 0, 0, i_rPanelWindowSize.Width(), i_rPanelWindowSize.Height(),
                PosSize::POSSIZE );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void CustomToolPanel::GrabFocus()
    {
        impl_ensurePanel();
        if ( !m_xToolPanel.is() )
            // if the custom panel does not support XPanel, this just means it is its own responsibility
            // to care for focus handling
            return;

        try
        {
            Reference< XWindow > xPanelWindow( m_xToolPanel->getWindow(), UNO_SET_THROW );
            xPanelWindow->setFocus();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void CustomToolPanel::Dispose()
    {
        // nothing to do here. Lifetime handling of the XResource which this panel represents is done by
        // the drawing framework, we ourself do not have resources to release.
        TaskPaneToolPanel::Dispose();
    }

    //------------------------------------------------------------------------------------------------------------------
    void CustomToolPanel::LockResourceAccess()
    {
        ++m_nResourceAccessLock;
    }

    //------------------------------------------------------------------------------------------------------------------
    void CustomToolPanel::UnlockResourceAccess()
    {
        OSL_ENSURE( m_nResourceAccessLock > 0, "CustomToolPanel::UnlockResourceAccess: not locked!" );
        --m_nResourceAccessLock;
    }

    //------------------------------------------------------------------------------------------------------------------
    const Reference< XResourceId >& CustomToolPanel::getResourceId() const
    {
        return m_xPanelResourceId;
    }

    //------------------------------------------------------------------------------------------------------------------
    void CustomToolPanel::impl_ensurePanel()
    {
        ENSURE_OR_RETURN_VOID( !isDisposed(), "already disposed" );

        if ( ( m_nResourceAccessLock == 0 ) && !m_bAttemptedPanelCreation )
        {
            m_bAttemptedPanelCreation = true;

            try
            {
                Reference< XConfigurationController > xConfigController( m_pFrameworkHelper->GetConfigurationController(), UNO_QUERY_THROW );
                m_xResource.set( xConfigController->getResource( m_xPanelResourceId ), UNO_QUERY_THROW );
                m_xToolPanel.set( m_xResource, UNO_QUERY );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

