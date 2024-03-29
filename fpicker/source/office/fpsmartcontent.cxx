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

#include "fpsmartcontent.hxx"

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/ContentInfo.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/XContent.hpp>

#include <comphelper/processfactory.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <comphelper/diagnose_ex.hxx>


namespace svt
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::task;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::container;


    //= SmartContent


    SmartContent::SmartContent()
        :m_eState( NOT_BOUND )
    {
    }


    SmartContent::SmartContent( const OUString& _rInitialURL )
        :m_eState( NOT_BOUND )
    {
        bindTo( _rInitialURL );
    }


    SmartContent::~SmartContent()
    {
        /* This destructor originally contained the following blurb: "Do
           not delete the content. Because the content will be used by
           the cache." This is just plain silly, because it relies on
           the provider caching created contents (which is done by
           ucbhelper::ContentProviderImplHelper, but we do not actually
           expect all providers to use that, right?) Otherwise we are
           just leaking memory.

           TODO: If there is real need for caching the content, it must
           be done here.
        */
    }


    void SmartContent::enableOwnInteractionHandler(::svt::OFilePickerInteractionHandler::EInterceptedInteractions eInterceptions)
    {
        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        Reference< XInteractionHandler > xGlobalInteractionHandler(
            InteractionHandler::createWithParent(xContext, nullptr), UNO_QUERY_THROW );

        m_xOwnInteraction = new ::svt::OFilePickerInteractionHandler(xGlobalInteractionHandler);
        m_xOwnInteraction->enableInterceptions(eInterceptions);

        m_xCmdEnv = new ::ucbhelper::CommandEnvironment( m_xOwnInteraction, Reference< XProgressHandler >() );
    }


    void SmartContent::enableDefaultInteractionHandler()
    {
        m_xOwnInteraction.clear();

        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        Reference< XInteractionHandler > xGlobalInteractionHandler(
            InteractionHandler::createWithParent(xContext, nullptr), UNO_QUERY_THROW );
        m_xCmdEnv = new ucbhelper::CommandEnvironment( xGlobalInteractionHandler, Reference< XProgressHandler >() );
    }


    ::svt::OFilePickerInteractionHandler* SmartContent::getOwnInteractionHandler() const
    {
        return m_xOwnInteraction.get();
    }


    SmartContent::InteractionHandlerType SmartContent::queryCurrentInteractionHandler() const
    {
        if (m_xOwnInteraction.is())
            return IHT_OWN;

        if (!m_xCmdEnv.is())
            return IHT_NONE;

        return IHT_DEFAULT;
    }


    void SmartContent::disableInteractionHandler()
    {
        m_xOwnInteraction.clear();
        m_xCmdEnv.clear();
    }


    void SmartContent::bindTo( const OUString& _rURL )
    {
        if ( getURL() == _rURL )
            // nothing to do, regardless of the state
            return;

        m_oContent.reset();
        m_eState = INVALID; // default to INVALID
        m_sURL = _rURL;

        if ( !m_sURL.isEmpty() )
        {
            try
            {
                m_oContent.emplace( _rURL, m_xCmdEnv, comphelper::getProcessComponentContext() );
                m_eState = UNKNOWN;
                    // from now on, the state is unknown -> we cannot know for sure if the content
                    // is really valid (some UCP's only tell this when asking for properties, not upon
                    // creation)
            }
            catch( const ContentCreationException& )
            {
            }
            catch( const Exception& )
            {
                TOOLS_WARN_EXCEPTION( "fpicker", "SmartContent::bindTo: unexpected exception caught!" );
            }
        }
        else
        {
            m_eState = NOT_BOUND;
        }


        // don't forget to reset the may internal used interaction handler ...
        // But do it only for our own specialized interaction helper!
        ::svt::OFilePickerInteractionHandler* pHandler = getOwnInteractionHandler();
        if (pHandler)
        {
            pHandler->resetUseState();
            pHandler->forgetRequest();
        }
    }


    bool SmartContent::implIs( const OUString& _rURL, Type _eType )
    {
        // bind to this content
        bindTo( _rURL );

        // did we survive this?
        if ( isInvalid() || !isBound() )
            return false;

        assert( m_oContent && "SmartContent::implIs: inconsistence!" );
            // if, after a bindTo, we don't have a content, then we should be INVALID, or at least
            // NOT_BOUND (the latter happens, for example, if somebody tries to ask for an empty URL)

        bool bIs = false;
        try
        {
            if ( Folder == _eType )
                bIs = m_oContent->isFolder();
            else
                bIs = m_oContent->isDocument();

            // from here on, we definitely know that the content is valid
            m_eState = VALID;
        }
        catch( const Exception& )
        {
            // now we're definitely invalid
            m_eState = INVALID;
        }
        return bIs;
    }


    void SmartContent::getTitle( OUString& /* [out] */ _rTitle )
    {
        if ( !isBound() || isInvalid() )
            return;

        try
        {
            OUString sTitle;
            m_oContent->getPropertyValue("Title") >>= sTitle;
            _rTitle =  sTitle;

            // from here on, we definitely know that the content is valid
            m_eState = VALID;
        }
        catch( const css::uno::Exception& )
        {
            // now we're definitely invalid
            m_eState = INVALID;
        }
    }


    bool SmartContent::hasParentFolder( )
    {
        if ( !isBound() || isInvalid() )
            return false;

        bool bRet = false;
        try
        {
            Reference< XChild > xChild( m_oContent->get(), UNO_QUERY );
            if ( xChild.is() )
            {
                Reference< XContent > xParent( xChild->getParent(), UNO_QUERY );
                if ( xParent.is() )
                {
                    const OUString aParentURL( xParent->getIdentifier()->getContentIdentifier() );
                    bRet = ( !aParentURL.isEmpty() && aParentURL != m_oContent->getURL() );

                    // now we're definitely valid
                    m_eState = VALID;
                }
            }
        }
        catch( const Exception& )
        {
            // now we're definitely invalid
            m_eState = INVALID;
        }
        return bRet;
    }


    bool SmartContent::canCreateFolder( )
    {
        if ( !isBound() || isInvalid() )
            return false;

        bool bRet = false;
        try
        {
            const css::uno::Sequence<css::ucb::ContentInfo> aContentsInfo = m_oContent->queryCreatableContentsInfo();
            for ( auto const& rInfo : aContentsInfo )
            {
                // Simply look for the first KIND_FOLDER...
                if ( rInfo.Attributes & ContentInfoAttribute::KIND_FOLDER )
                {
                    bRet = true;
                    break;
                }
            }

            // now we're definitely valid
            m_eState = VALID;
        }
        catch( const Exception& )
        {
            // now we're definitely invalid
            m_eState = INVALID;
        }
        return bRet;
    }

    OUString SmartContent::createFolder( const OUString& _rTitle )
    {
        OUString aCreatedUrl;
        try
        {
            OUString sFolderType;

            const css::uno::Sequence<css::ucb::ContentInfo> aContentsInfo = m_oContent->queryCreatableContentsInfo();
            for ( auto const& rInfo : aContentsInfo )
            {
                // Simply look for the first KIND_FOLDER...
                if ( rInfo.Attributes & ContentInfoAttribute::KIND_FOLDER )
                {
                    sFolderType = rInfo.Type;
                    break;
                }
            }

            if ( !sFolderType.isEmpty() )
            {
                ucbhelper::Content aCreated;
                Sequence< OUString > aNames { "Title" };
                Sequence< Any > aValues { Any(_rTitle) };
                m_oContent->insertNewContent( sFolderType, aNames, aValues, aCreated );

                aCreatedUrl = aCreated.getURL();
            }
        }
        catch( const Exception& )
        {
        }
        return aCreatedUrl;
    }


} // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
