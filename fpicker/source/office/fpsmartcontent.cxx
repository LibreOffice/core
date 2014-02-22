/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "fpsmartcontent.hxx"

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/ContentInfo.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/XContent.hpp>

#include <comphelper/processfactory.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <tools/solar.h>
#include <tools/debug.hxx>


namespace svt
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::task;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;

    
    
    
    
    SmartContent::SmartContent()
        :m_pContent( NULL )
        ,m_eState( NOT_BOUND )
        ,m_pOwnInteraction( NULL )
    {
    }

    
    SmartContent::SmartContent( const OUString& _rInitialURL )
        :m_pContent( NULL )
        ,m_eState( NOT_BOUND )
        ,m_pOwnInteraction( NULL )
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
        delete m_pContent;
    }

    
    void SmartContent::enableOwnInteractionHandler(::svt::OFilePickerInteractionHandler::EInterceptedInteractions eInterceptions)
    {
        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        Reference< XInteractionHandler > xGlobalInteractionHandler(
            InteractionHandler::createWithParent(xContext, 0), UNO_QUERY_THROW );

        m_pOwnInteraction = new ::svt::OFilePickerInteractionHandler(xGlobalInteractionHandler);
        m_pOwnInteraction->enableInterceptions(eInterceptions);
        m_xOwnInteraction = m_pOwnInteraction;

        m_xCmdEnv = new ::ucbhelper::CommandEnvironment( m_xOwnInteraction, Reference< XProgressHandler >() );
    }

    
    void SmartContent::enableDefaultInteractionHandler()
    {
        
        
        m_pOwnInteraction = NULL;
        m_xOwnInteraction.clear();

        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        Reference< XInteractionHandler > xGlobalInteractionHandler(
            InteractionHandler::createWithParent(xContext, 0), UNO_QUERY_THROW );
        m_xCmdEnv = new ucbhelper::CommandEnvironment( xGlobalInteractionHandler, Reference< XProgressHandler >() );
    }

    
    ::svt::OFilePickerInteractionHandler* SmartContent::getOwnInteractionHandler() const
    {
        if (!m_xOwnInteraction.is())
            return NULL;
        return m_pOwnInteraction;
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
        
        
        m_pOwnInteraction = NULL;
        m_xOwnInteraction.clear();

        m_xCmdEnv.clear();
    }

    
    void SmartContent::bindTo( const OUString& _rURL )
    {
        if ( getURL() == _rURL )
            
            return;

        DELETEZ( m_pContent );
        m_eState = INVALID; 
        m_sURL = _rURL;

        if ( !m_sURL.isEmpty() )
        {
            try
            {
                m_pContent = new ::ucbhelper::Content( _rURL, m_xCmdEnv, comphelper::getProcessComponentContext() );
                m_eState = UNKNOWN;
                    
                    
                    
            }
            catch( const ContentCreationException& )
            {
            }
            catch( const Exception& )
            {
                OSL_FAIL( "SmartContent::bindTo: unexpected exception caught!" );
            }
        }
        else
        {
            m_eState = NOT_BOUND;
        }


        
        
        ::svt::OFilePickerInteractionHandler* pHandler = getOwnInteractionHandler();
        if (pHandler)
        {
            pHandler->resetUseState();
            pHandler->forgetRequest();
        }
    }

    
    sal_Bool SmartContent::implIs( const OUString& _rURL, Type _eType )
    {
        
        bindTo( _rURL );

        
        if ( isInvalid() || !isBound() )
            return sal_False;

        DBG_ASSERT( m_pContent, "SmartContent::implIs: inconsistence!" );
            
            

        sal_Bool bIs = sal_False;
        try
        {
            if ( Folder == _eType )
                bIs = m_pContent->isFolder();
            else
                bIs = m_pContent->isDocument();

            
            m_eState = VALID;
        }
        catch( const Exception& )
        {
            
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
            m_pContent->getPropertyValue("Title") >>= sTitle;
            _rTitle =  sTitle;

            
            m_eState = VALID;
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
            
            m_eState = INVALID;
        }
    }

    
    sal_Bool SmartContent::hasParentFolder( )
    {
        if ( !isBound() || isInvalid() )
            return sal_False;

        sal_Bool bRet = sal_False;
        try
        {
            Reference< XChild > xChild( m_pContent->get(), UNO_QUERY );
            if ( xChild.is() )
            {
                Reference< XContent > xParent( xChild->getParent(), UNO_QUERY );
                if ( xParent.is() )
                {
                    const OUString aParentURL( xParent->getIdentifier()->getContentIdentifier() );
                    bRet = ( !aParentURL.isEmpty() && aParentURL != m_pContent->getURL() );

                    
                    m_eState = VALID;
                }
            }
        }
        catch( const Exception& )
        {
            
            m_eState = INVALID;
        }
        return bRet;
    }

    
    sal_Bool SmartContent::canCreateFolder( )
    {
        if ( !isBound() || isInvalid() )
            return sal_False;

        sal_Bool bRet = sal_False;
        try
        {
            Sequence< ContentInfo > aInfo = m_pContent->queryCreatableContentsInfo();
            const ContentInfo* pInfo = aInfo.getConstArray();
            sal_Int32 nCount = aInfo.getLength();
            for ( sal_Int32 i = 0; i < nCount; ++i, ++pInfo )
            {
                
                if ( pInfo->Attributes & ContentInfoAttribute::KIND_FOLDER )
                {
                    bRet = sal_True;
                    break;
                }
            }

            
            m_eState = VALID;
        }
        catch( const Exception& )
        {
            
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

            Sequence< ContentInfo > aInfo = m_pContent->queryCreatableContentsInfo();
            const ContentInfo* pInfo = aInfo.getConstArray();
            sal_Int32 nCount = aInfo.getLength();
            for ( sal_Int32 i = 0; i < nCount; ++i, ++pInfo )
            {
                
                if ( pInfo->Attributes & ContentInfoAttribute::KIND_FOLDER )
                {
                    sFolderType = pInfo->Type;
                    break;
                }
            }

            if ( !sFolderType.isEmpty() )
            {
                ucbhelper::Content aCreated;
                Sequence< OUString > aNames( 1 );
                OUString* pNames = aNames.getArray();
                pNames[0] = "Title";
                Sequence< Any > aValues( 1 );
                Any* pValues = aValues.getArray();
                pValues[0] = makeAny( _rTitle );
                m_pContent->insertNewContent( sFolderType, aNames, aValues, aCreated );

                aCreatedUrl = aCreated.getURL();
            }
        }
        catch( const Exception& )
        {
        }
        return aCreatedUrl;
    }


} 


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
