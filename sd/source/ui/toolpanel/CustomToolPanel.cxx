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
    /** === end UNO using === **/

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
        ,m_xPanel()
        ,m_bAttemptedPanelCreation( false )
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
    void CustomToolPanel::Dispose()
    {
        if ( m_xPanel.is() )
        {
            try
            {
                // TODO: obtain the factory for our panel, and invoke its destroyResource method
            }
            catch ( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        TaskPaneToolPanel::Dispose();
    }

    //------------------------------------------------------------------------------------------------------------------
    const Reference< XResourceId >& CustomToolPanel::getResourceId() const
    {
        return m_xPanelResourceId;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::Window* CustomToolPanel::getPanelWindow() const
    {
        ENSURE_OR_RETURN( impl_ensurePanel(), "could not create the panel", NULL );
        // TODO
        return NULL;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool CustomToolPanel::impl_ensurePanel() const
    {
        ENSURE_OR_RETURN_FALSE( !isDisposed(), "already disposed" );

        if ( !m_bAttemptedPanelCreation )
        {
            const_cast< CustomToolPanel* >( this )->m_bAttemptedPanelCreation = true;

            try
            {
                Reference< XConfigurationController > xConfigController( m_pFrameworkHelper->GetConfigurationController(), UNO_QUERY_THROW );
                Reference< XResource > xToolPanelResource( xConfigController->getResource( m_xPanelResourceId ) );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return m_xPanel.is();
    }

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

