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

#include <cppuhelper/exc_hlp.hxx>
#include <util/MiscUtils.hxx>

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>

#include "ActiveMSPList.hxx"

#include <comphelper/diagnose_ex.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::script;
using namespace ::sf_misc;

namespace func_provider
{

ActiveMSPList::ActiveMSPList(  const Reference< XComponentContext > & xContext ) : m_xContext( xContext )
{
    userDirString = "user";
    shareDirString = "share";
    bundledDirString = "bundled";
}

ActiveMSPList::~ActiveMSPList()
{
}

Reference< provider::XScriptProvider >
ActiveMSPList::createNewMSP( const uno::Any& context )
{
    Sequence< Any > args( &context, 1 );

    Reference< provider::XScriptProvider > msp(
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            u"com.sun.star.script.provider.MasterScriptProvider"_ustr, args, m_xContext ), UNO_QUERY );
    return msp;
}

class NonDocMSPCreator
{
public:
    explicit NonDocMSPCreator(ActiveMSPList *pList)
    {
        pList->createNonDocMSPs();
    }
};

namespace
{
    //thread-safe method to ensure createNonDocMSPs is called once
    void ensureNonDocMSPs(ActiveMSPList *pList)
    {
        static NonDocMSPCreator theCreator(pList);
    }
}

Reference< provider::XScriptProvider >
ActiveMSPList::getMSPFromAnyContext( const Any& aContext )
{
    Reference< provider::XScriptProvider > msp;
    OUString sContext;
    if ( aContext >>= sContext )
    {
        msp = getMSPFromStringContext( sContext );
        return msp;
    }

    Reference< frame::XModel > xModel( aContext, UNO_QUERY );

    Reference< document::XScriptInvocationContext > xScriptContext( aContext, UNO_QUERY );
    if ( xScriptContext.is() )
    {
        try
        {
            // the component supports executing scripts embedded in a - possibly foreign document.
            // Check whether this other document it's the component itself.
            if ( !xModel.is() || ( xModel != xScriptContext->getScriptContainer() ) )
            {
                msp = getMSPFromInvocationContext( xScriptContext );
                return msp;
            }
        }
        catch( const lang::IllegalArgumentException& )
        {
            xModel.set( Reference< frame::XModel >() );
        }
    }

    if ( xModel.is() )
    {
        sContext = MiscUtils::xModelToTdocUrl( xModel, m_xContext );
        msp = getMSPFromStringContext( sContext );
        return msp;
    }

    ensureNonDocMSPs(this);
    return m_hMsps[ shareDirString ];
}

Reference< provider::XScriptProvider >
    ActiveMSPList::getMSPFromInvocationContext( const Reference< document::XScriptInvocationContext >& xContext )
{
    Reference< provider::XScriptProvider > msp;

    Reference< document::XEmbeddedScripts > xScripts;
    if ( xContext.is() )
        xScripts.set( xContext->getScriptContainer() );
    if ( !xScripts.is() )
    {
        throw lang::IllegalArgumentException(
            u"Failed to create MasterScriptProvider for ScriptInvocationContext: "
            "Component supporting XEmbeddScripts interface not found."_ustr,
            nullptr, 1 );
    }

    ::osl::MutexGuard guard( m_mutex );

    Reference< XInterface > xNormalized( xContext, UNO_QUERY );
    ScriptComponent_map::const_iterator pos = m_mScriptComponents.find( xNormalized );
    if ( pos == m_mScriptComponents.end() )
    {
        // TODO
        msp = createNewMSP( uno::Any( xContext ) );
        addActiveMSP( xNormalized, msp );
    }
    else
    {
        msp = pos->second;
    }

    return msp;
}

Reference< provider::XScriptProvider >
    ActiveMSPList::getMSPFromStringContext( const OUString& context )
{
    Reference< provider::XScriptProvider > msp;
    try
    {
        if ( context.startsWith( "vnd.sun.star.tdoc" ) )
        {
            Reference< frame::XModel > xModel( MiscUtils::tDocUrlToModel( context ) );

            Reference< document::XEmbeddedScripts > xScripts( xModel, UNO_QUERY );
            Reference< document::XScriptInvocationContext > xScriptsContext( xModel, UNO_QUERY );
            if ( !xScripts.is() && !xScriptsContext.is() )
            {
                throw lang::IllegalArgumentException(
                    "Failed to create MasterScriptProvider for '"
                    + context +
                    "': Either XEmbeddScripts or XScriptInvocationContext need to be supported by the document.",
                    nullptr, 1 );
            }

            ::osl::MutexGuard guard( m_mutex );
            Reference< XInterface > xNormalized( xModel, UNO_QUERY );
            ScriptComponent_map::const_iterator pos = m_mScriptComponents.find( xNormalized );
            if ( pos == m_mScriptComponents.end() )
            {
                msp = createNewMSP( context );
                addActiveMSP( xNormalized, msp );
            }
            else
            {
                msp = pos->second;
            }
        }
        else
        {
            ::osl::MutexGuard guard( m_mutex );
            Msp_hash::iterator h_itEnd =  m_hMsps.end();
            Msp_hash::const_iterator itr = m_hMsps.find( context );
            if ( itr ==  h_itEnd )
            {
                msp = createNewMSP( context );
                m_hMsps[ context ] = msp;
            }
            else
            {
                msp = m_hMsps[ context ];
            }
        }
    }
    catch( const lang::IllegalArgumentException& )
    {
        // allowed to leave
    }
    catch( const RuntimeException& )
    {
        // allowed to leave
    }
    catch( const Exception& )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException(
            "Failed to create MasterScriptProvider for context '"
            + context + "'.",
            *this, anyEx );
    }
    return msp;
}

void
ActiveMSPList::addActiveMSP( const Reference< uno::XInterface >& xComponent,
               const Reference< provider::XScriptProvider >& msp )
{
    ::osl::MutexGuard guard( m_mutex );
    Reference< XInterface > xNormalized( xComponent, UNO_QUERY );
    ScriptComponent_map::const_iterator pos = m_mScriptComponents.find( xNormalized );
    if ( pos != m_mScriptComponents.end() )
        return;

    m_mScriptComponents[ xNormalized ] = msp;

    // add self as listener for component disposal
    // should probably throw from this method!!, reexamine
    try
    {
        Reference< lang::XComponent > xBroadcaster( xComponent, UNO_QUERY_THROW );
        xBroadcaster->addEventListener( this );
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("scripting");
    }
}


void SAL_CALL ActiveMSPList::disposing( const css::lang::EventObject& Source )

{
    try
    {
        Reference< XInterface > xNormalized( Source.Source, UNO_QUERY );
        if ( xNormalized.is() )
        {
            ::osl::MutexGuard guard( m_mutex );
            ScriptComponent_map::iterator pos = m_mScriptComponents.find( xNormalized );
            if ( pos != m_mScriptComponents.end() )
                m_mScriptComponents.erase( pos );
        }
    }
    catch ( const Exception& )
    {
        // if we get an exception here, there is not much we can do about
        // it can't throw as it will screw up the model that is calling dispose
        DBG_UNHANDLED_EXCEPTION("scripting");
    }
}

void
ActiveMSPList::createNonDocMSPs()
{
    // do creation of user and share MSPs here
    OUString serviceName(u"com.sun.star.script.provider.MasterScriptProvider"_ustr);

    Sequence< Any > args{ Any(userDirString) };
    // should check if provider reference is valid
    m_hMsps[ userDirString ].set( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( serviceName, args, m_xContext ), UNO_QUERY );

    args = { Any(shareDirString) };
    // should check if provider reference is valid
    m_hMsps[ shareDirString ].set( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( serviceName, args, m_xContext ), UNO_QUERY );

    args = { Any(bundledDirString) };
    // should check if provider reference is valid
    m_hMsps[ bundledDirString ].set( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( serviceName, args, m_xContext ), UNO_QUERY );
}

} // namespace func_provider

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
