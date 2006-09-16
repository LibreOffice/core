/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ActiveMSPList.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 12:27:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_scripting.hxx"
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <util/scriptingconstants.hxx>
#include <util/util.hxx>
#include <util/MiscUtils.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>

#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>

#include "MasterScriptProvider.hxx"
#include "ActiveMSPList.hxx"

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
ActiveMSPList::createNewMSP( const ::rtl::OUString& context ) throw( RuntimeException )
{
    ::rtl::OUString serviceName = ::rtl::OUString::createFromAscii("com.sun.star.script.provider.MasterScriptProvider");
    Sequence< Any > args(1);
    args[ 0 ] <<= context;

    Reference< provider::XScriptProvider > msp( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( serviceName, args, m_xContext ), UNO_QUERY );
    return msp;
}

Sequence < Reference< provider::XScriptProvider > >
ActiveMSPList::getActiveProviders()
{
    ::osl::MutexGuard guard( m_mutex );

    sal_Int32 numChildNodes = m_hMsps.size() + m_mModels.size();
    // get providers for application
    Msp_hash::iterator h_itEnd =  m_hMsps.end();
    Sequence< Reference< provider::XScriptProvider > > children( numChildNodes );
    sal_Int32 count = 0;


    for ( Msp_hash::iterator h_it = m_hMsps.begin(); h_it != h_itEnd; ++h_it )
    {
        children[ count++ ] =  h_it->second;
    }

    // get providers for active documents
    Model_map::iterator m_itEnd =  m_mModels.end();

    for ( Model_map::iterator m_it = m_mModels.begin(); m_it != m_itEnd; ++m_it )
    {
        children[ count++ ] = m_it->second;
    }
    return children;
}


Reference< provider::XScriptProvider >
ActiveMSPList::createMSP( const Any& aContext )
            throw ( RuntimeException )
{
    Reference< provider::XScriptProvider > msp;
    if (  ! ( aContext.getValueType() == ::getCppuType((const ::rtl::OUString* ) NULL ) ) )
    {
        Reference< frame::XModel> xModel( aContext, UNO_QUERY );
        if ( xModel.is() )
        {
            ::rtl::OUString sContext = MiscUtils::xModelToTdocUrl( xModel, m_xContext );
            msp = createMSP( sContext );
        }
        else
        {
            createNonDocMSPs();
            return m_hMsps[ shareDirString ];
        }

    }
    else
    {
        ::rtl::OUString sContext;
        aContext >>= sContext;
        msp = createMSP( sContext );
    }
    return msp;
}

Reference< provider::XScriptProvider >
ActiveMSPList::createMSP( const ::rtl::OUString& context )
            throw ( RuntimeException )
{
    Reference< provider::XScriptProvider > msp;
    if ( context.indexOf( OUSTR( "vnd.sun.star.tdoc" ) ) == 0 )
    {
        Reference< frame::XModel > xModel( MiscUtils::tDocUrlToModel( context ), UNO_QUERY );
        if ( !xModel.is() )
        {
            ::rtl::OUStringBuffer buf( 80 );
            buf.append( OUSTR("Failed to create MasterScriptProvider for " ) );
            buf.append( context);
            ::rtl::OUString message = buf.makeStringAndClear();
            throw RuntimeException( message, Reference< XInterface >() );
        }
        ::osl::MutexGuard guard( m_mutex );
        Model_map::const_iterator itr = m_mModels.find( xModel );
        if ( itr == m_mModels.end() )
        {
            msp = createNewMSP( context );
            addActiveMSP( xModel, msp );
        }
        else
        {
            msp = itr->second;
        }
    }
    else
    {
        ::osl::MutexGuard guard( m_mutex );
        Msp_hash::iterator h_itEnd =  m_hMsps.end();
        Msp_hash::const_iterator itr = m_hMsps.find( context );
        if ( itr ==  h_itEnd )
        {
            try
            {
                msp = createNewMSP( context );
            }
            catch ( RuntimeException& )
            {
                ::rtl::OUStringBuffer buf( 80 );
                buf.append( OUSTR("Failed to create MasterScriptProvider for " ) );
                buf.append( context);
                ::rtl::OUString message = buf.makeStringAndClear();
                throw RuntimeException( message, Reference< XInterface >() );
            }
            m_hMsps[ context ] = msp;
        }
        else
        {
            msp = m_hMsps[ context ];
        }
    }
    return msp;
}

void
ActiveMSPList::addActiveMSP( const Reference< frame::XModel >& xModel,
               const Reference< provider::XScriptProvider >& msp )
{
    ::osl::MutexGuard guard( m_mutex );
    Model_map::const_iterator itr = m_mModels.find( xModel );
    if ( itr == m_mModels.end() )
    {
        m_mModels[ xModel ] = msp;

        // add self as listener for document dispose
        // should probably throw from this method!!, reexamine
        try
        {
            Reference< lang::XComponent > xComponent =
                Reference< lang::XComponent >( xModel, UNO_QUERY_THROW );
            validateXRef( xComponent, "ActiveMSPList::addActiveMSP: model not XComponent\n" );
            xComponent->addEventListener( this );

        }
        catch ( RuntimeException& )
        {
        }
    }
}

//*************************************************************************
void SAL_CALL
ActiveMSPList::disposing( const ::com::sun::star::lang::EventObject& Source )
throw ( ::com::sun::star::uno::RuntimeException )

{
    Reference< frame::XModel > xModel;
    try
    {
        Reference< XInterface > xInterface = Source.Source;
        xModel = Reference< frame::XModel > ( xInterface, UNO_QUERY );
        if ( xModel.is() )
        {
            ::osl::MutexGuard guard( m_mutex );
            Model_map::const_iterator itr = m_mModels.find( xModel );
            if ( itr != m_mModels.end() )
            {
                m_mModels.erase( xModel );
            }
        }
    }
    catch ( RuntimeException& e )
    {
        // if we get an exception here, there is not much we can do about
        // it can't throw as it will screw up the model that is calling dispose
        ::rtl::OUString message =
            OUSTR( "ActiveMSPList::disposing: document invalid model." );
        message = message.concat( e.Message );
        OSL_TRACE( ::rtl::OUStringToOString( message,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
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

