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

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <util/scriptingconstants.hxx>
#include <util/util.hxx>
#include <util/MiscUtils.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>

#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>

#include "MasterScriptProvider.hxx"
#include "ActiveMSPList.hxx"

#include <tools/diagnose_ex.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::script;
using namespace ::sf_misc;

namespace func_provider
{

ActiveMSPList::ActiveMSPList(  const Reference< XComponentContext > & xContext ) : m_xContext( xContext )
{
    userDirString = ::rtl::OUString("user");
    shareDirString =  ::rtl::OUString("share");
    bundledDirString = ::rtl::OUString("bundled");
}

ActiveMSPList::~ActiveMSPList()
{
}

Reference< provider::XScriptProvider >
ActiveMSPList::createNewMSP( const uno::Any& context )
{
    ::rtl::OUString serviceName("com.sun.star.script.provider.MasterScriptProvider");
    Sequence< Any > args( &context, 1 );

    Reference< provider::XScriptProvider > msp(
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            serviceName, args, m_xContext ), UNO_QUERY );
    return msp;
}

Reference< provider::XScriptProvider >
ActiveMSPList::getMSPFromAnyContext( const Any& aContext )
            SAL_THROW(( lang::IllegalArgumentException, RuntimeException ))
{
    Reference< provider::XScriptProvider > msp;
    ::rtl::OUString sContext;
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
            // Check whether this other document its the component itself.
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

    createNonDocMSPs();
    return m_hMsps[ shareDirString ];
}

Reference< provider::XScriptProvider >
    ActiveMSPList::getMSPFromInvocationContext( const Reference< document::XScriptInvocationContext >& xContext )
        SAL_THROW(( lang::IllegalArgumentException, RuntimeException ))
{
    Reference< provider::XScriptProvider > msp;

    Reference< document::XEmbeddedScripts > xScripts;
    if ( xContext.is() )
        xScripts.set( xContext->getScriptContainer() );
    if ( !xScripts.is() )
    {
        ::rtl::OUStringBuffer buf;
        buf.appendAscii( "Failed to create MasterScriptProvider for ScriptInvocationContext: " );
        buf.appendAscii( "Component supporting XEmbeddScripts interface not found." );
        throw lang::IllegalArgumentException( buf.makeStringAndClear(), NULL, 1 );
    }

    ::osl::MutexGuard guard( m_mutex );

    Reference< XInterface > xNormalized( xContext, UNO_QUERY );
    ScriptComponent_map::const_iterator pos = m_mScriptComponents.find( xNormalized );
    if ( pos == m_mScriptComponents.end() )
    {
        // TODO
        msp = createNewMSP( uno::makeAny( xContext ) );
        addActiveMSP( xNormalized, msp );
    }
    else
    {
        msp = pos->second;
    }

    return msp;
}

Reference< provider::XScriptProvider >
    ActiveMSPList::getMSPFromStringContext( const ::rtl::OUString& context )
        SAL_THROW(( lang::IllegalArgumentException, RuntimeException ))
{
    Reference< provider::XScriptProvider > msp;
    try
    {
        if ( context.indexOf( "vnd.sun.star.tdoc" ) == 0 )
        {
            Reference< frame::XModel > xModel( MiscUtils::tDocUrlToModel( context ) );

            Reference< document::XEmbeddedScripts > xScripts( xModel, UNO_QUERY );
            Reference< document::XScriptInvocationContext > xScriptsContext( xModel, UNO_QUERY );
            if ( !xScripts.is() && !xScriptsContext.is() )
            {
                ::rtl::OUStringBuffer buf;
                buf.appendAscii( "Failed to create MasterScriptProvider for '" );
                buf.append     ( context );
                buf.appendAscii( "': Either XEmbeddScripts or XScriptInvocationContext need to be supported by the document." );
                throw lang::IllegalArgumentException( buf.makeStringAndClear(), NULL, 1 );
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
        ::rtl::OUStringBuffer aMessage;
        aMessage.appendAscii( "Failed to create MasterScriptProvider for context '" );
        aMessage.append     ( context );
        aMessage.appendAscii( "'." );
        throw lang::WrappedTargetRuntimeException(
            aMessage.makeStringAndClear(), *this, ::cppu::getCaughtException() );
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
    if ( pos == m_mScriptComponents.end() )
    {
        m_mScriptComponents[ xNormalized ] = msp;

        // add self as listener for component disposal
        // should probably throw from this method!!, reexamine
        try
        {
            Reference< lang::XComponent > xBroadcaster =
                Reference< lang::XComponent >( xComponent, UNO_QUERY_THROW );
            xBroadcaster->addEventListener( this );
        }
        catch ( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

//*************************************************************************
void SAL_CALL
ActiveMSPList::disposing( const ::com::sun::star::lang::EventObject& Source )
throw ( ::com::sun::star::uno::RuntimeException )

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
        DBG_UNHANDLED_EXCEPTION();
    }
}


void
ActiveMSPList::createNonDocMSPs()
{
    static bool created = false;
    if ( created )
    {
        return;
    }
    else
    {
        ::osl::MutexGuard guard( m_mutex );
        if ( created )
        {
            return;
        }
        // do creation of user and share MSPs here
        ::rtl::OUString serviceName("com.sun.star.script.provider.MasterScriptProvider");
        Sequence< Any > args(1);

        args[ 0 ] <<= userDirString;
        Reference< provider::XScriptProvider > userMsp( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( serviceName, args, m_xContext ), UNO_QUERY );
        // should check if provider reference is valid
        m_hMsps[ userDirString ] = userMsp;

        args[ 0 ] <<= shareDirString;
        Reference< provider::XScriptProvider > shareMsp( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( serviceName, args, m_xContext ), UNO_QUERY );
        // should check if provider reference is valid
        m_hMsps[ shareDirString ] = shareMsp;

        args[ 0 ] <<= bundledDirString;
        Reference< provider::XScriptProvider > bundledMsp( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( serviceName, args, m_xContext ), UNO_QUERY );
        // should check if provider reference is valid
        m_hMsps[ bundledDirString ] = bundledMsp;

        created = true;
    }

}


} // namespace func_provider

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
