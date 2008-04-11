/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ActiveMSPList.cxx,v $
 * $Revision: 1.15 $
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
#include "precompiled_scripting.hxx"
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
using namespace ::scripting_util;
using namespace ::sf_misc;

namespace func_provider
{

ActiveMSPList::ActiveMSPList(  const Reference< XComponentContext > & xContext ) : m_xContext( xContext )
{
    userDirString = ::rtl::OUString::createFromAscii("user");
    shareDirString =  ::rtl::OUString::createFromAscii("share");
}

ActiveMSPList::~ActiveMSPList()
{
}

Reference< provider::XScriptProvider >
ActiveMSPList::createNewMSP( const uno::Any& context )
{
    ::rtl::OUString serviceName = ::rtl::OUString::createFromAscii("com.sun.star.script.provider.MasterScriptProvider");
    Sequence< Any > args( &context, 1 );

    Reference< provider::XScriptProvider > msp(
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            serviceName, args, m_xContext ), UNO_QUERY );
    return msp;
}

Sequence < Reference< provider::XScriptProvider > >
ActiveMSPList::getActiveProviders()
{
    ::osl::MutexGuard guard( m_mutex );

    sal_Int32 numChildNodes = m_hMsps.size() + m_mScriptComponents.size();
    // get providers for application
    Msp_hash::iterator h_itEnd =  m_hMsps.end();
    Sequence< Reference< provider::XScriptProvider > > children( numChildNodes );
    sal_Int32 count = 0;


    for ( Msp_hash::iterator h_it = m_hMsps.begin(); h_it != h_itEnd; ++h_it )
    {
        children[ count++ ] =  h_it->second;
    }

    // get providers for active documents
    ScriptComponent_map::iterator m_itEnd =  m_mScriptComponents.end();

    for ( ScriptComponent_map::iterator m_it = m_mScriptComponents.begin(); m_it != m_itEnd; ++m_it )
    {
        children[ count++ ] = m_it->second;
    }
    return children;
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
        // the component supports executing scripts embedded in a - possibly foreign document.
        // Check whether this other document its the component itself.
        if ( !xModel.is() || ( xModel != xScriptContext->getScriptContainer() ) )
        {
            msp = getMSPFromInvocationContext( xScriptContext );
            return msp;
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
        if ( context.indexOf( OUSTR( "vnd.sun.star.tdoc" ) ) == 0 )
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
        ::rtl::OUString serviceName = ::rtl::OUString::createFromAscii("com.sun.star.script.provider.MasterScriptProvider");
        Sequence< Any > args(1);

        args[ 0 ] <<= userDirString;
        Reference< provider::XScriptProvider > userMsp( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( serviceName, args, m_xContext ), UNO_QUERY );
        // should check if provider reference is valid
        m_hMsps[ userDirString ] = userMsp;

        args[ 0 ] <<= shareDirString;
        Reference< provider::XScriptProvider > shareMsp( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( serviceName, args, m_xContext ), UNO_QUERY );
        // should check if provider reference is valid
        m_hMsps[ shareDirString ] = shareMsp;
        created = true;
    }

}


} // namespace func_provider

