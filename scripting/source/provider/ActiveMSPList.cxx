/*************************************************************************
 *
 *  $RCSfile: ActiveMSPList.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 14:09:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>

#include <util/scriptingconstants.hxx>
#include <util/util.hxx>
#include <util/MiscUtils.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>

#include <drafts/com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>

#include "MasterScriptProvider.hxx"
#include "ActiveMSPList.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace drafts::com::sun::star::script;
using namespace sf_misc;

namespace func_provider
{

ActiveMSPList::ActiveMSPList(  const Reference< XComponentContext > & xContext ) : m_xContext( xContext )
{
    OSL_TRACE("ActiveMSPList::ActiveMSPList) - ctor");
    userDirString = ::rtl::OUString::createFromAscii("user");
    shareDirString =  ::rtl::OUString::createFromAscii("share");
}

ActiveMSPList::~ActiveMSPList()
{
    OSL_TRACE("ActiveMSPList::ActiveMSPList) - dtor");
}

Reference< provider::XScriptProvider >
ActiveMSPList::createNewMSP( const ::rtl::OUString& context ) throw( RuntimeException )
{
    OSL_TRACE("ActiveMSPList::createNewMSP() context [%s]",
        ::rtl::OUStringToOString( context , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    ::rtl::OUString serviceName = ::rtl::OUString::createFromAscii("drafts.com.sun.star.script.provider.MasterScriptProvider");
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
        OSL_TRACE("Adding application browsenode index [ %d ]", count );
        children[ count++ ] =  h_it->second;
    }

    // get providers for active documents
    Model_map::iterator m_itEnd =  m_mModels.end();

    for ( Model_map::iterator m_it = m_mModels.begin(); m_it != m_itEnd; ++m_it )
    {
        OSL_TRACE("Adding document browsenode index [ %d ]", count  );
        children[ count++ ] = m_it->second;
    }
    return children;
}


Reference< provider::XScriptProvider >
ActiveMSPList::createMSP( const Any& aContext )
            throw ( RuntimeException )
{
    Reference< provider::XScriptProvider > msp;
    if (  aContext.getValueType() == ::getCppuType((const Reference< frame::XModel >* ) NULL ) )

    {
        OSL_TRACE("ActiveMSPList::createMSP() for model");
        Reference< frame::XModel> xModel( aContext, UNO_QUERY );
        if ( xModel.is() )
        {
            ::rtl::OUString sContext = MiscUtils::xModelToTdocUrl( xModel );
            msp = createMSP( sContext );
        }
        else
        {
            ::rtl::OUString message =
                OUSTR( "Failed to extract XModel from context, could not create MasterScriptProvider" );
            throw RuntimeException( message, Reference< XInterface >() );
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
            catch ( RuntimeException& e )
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
    OSL_TRACE("ActiveMSPList::addActiveMSP() for %s",
                ::rtl::OUStringToOString( xModel->getURL(),
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
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
        catch ( RuntimeException& e )
        {
            OSL_TRACE("ActiveMSPList::addActiveMSP() failed to add self as listener: %s",
                ::rtl::OUStringToOString( e.Message,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
    }

    else
    {
        OSL_TRACE("ActiveMSPList::addActiveMSP() model for document exists already in map" );
    }
}

//*************************************************************************
void SAL_CALL
ActiveMSPList::disposing( const ::com::sun::star::lang::EventObject& Source )
throw ( ::com::sun::star::uno::RuntimeException )

{
    OSL_TRACE("ActiveMSPList::disposing() ");
    Reference< frame::XModel > xModel;
    try
    {
        Reference< XInterface > xInterface = Source.Source;
        xModel = Reference< frame::XModel > ( xInterface, UNO_QUERY );
        if ( xModel.is() )
        {
            OSL_TRACE("ActiveMSPList::disposing() model is valid");

            ::osl::MutexGuard guard( m_mutex );
            Model_map::const_iterator itr = m_mModels.find( xModel );
            if ( itr != m_mModels.end() )
            {
                OSL_TRACE("ActiveMSPList::disposing()  model for doc %s exists in map",
                     ::rtl::OUStringToOString( xModel->getURL(),
                         RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                m_mModels.erase( xModel );
            }
        }
        else
        {
            OSL_TRACE("ActiveMSPList::disposing() doc model invalid or unknown" );
        }
    }
    catch ( RuntimeException& e )
    {
        // if we get an exception here, there is not much we can do
        // about it can't throw as it will screw up the model that is calling dispose
        ::rtl::OUString message =
            OUSTR(
                "ActiveMSPList::disposing: document invalid model." );
        message = message.concat( e.Message );
        OSL_TRACE( ::rtl::OUStringToOString( message,
                                            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    }
}


ActiveMSPList&
ActiveMSPList::instance( const Reference< XComponentContext > & xContext )
{
    static ActiveMSPList* inst = 0;
    // need to not only hold a static pointer to this object but also
    // keep it aqcuired
    static Reference< lang::XEventListener > holder;
    if ( !inst )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if ( !inst )
        {
           inst = new ActiveMSPList( xContext );
           holder = inst;
        }
    }
    return *inst;
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
        ::rtl::OUString serviceName = ::rtl::OUString::createFromAscii("drafts.com.sun.star.script.provider.MasterScriptProvider");
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

