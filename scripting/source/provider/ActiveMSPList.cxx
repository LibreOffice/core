/*************************************************************************
 *
 *  $RCSfile: ActiveMSPList.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-19 08:27:20 $
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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>

#include <drafts/com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>

#include "MasterScriptProvider.hxx"
#include "ActiveMSPList.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace drafts::com::sun::star::script;

namespace func_provider
{

ActiveMSPList::ActiveMSPList(  const Reference< XComponentContext > & xContext ) : m_xContext( xContext )
{
    OSL_TRACE("ActiveMSPList::ActiveMSPList) - ctor");
    // needs some exception handling
    Any aAny = m_xContext->getValueByName( ::rtl::OUString::createFromAscii( "/singletons/com.sun.star.util.theMacroExpander" ) );
    Reference< util::XMacroExpander > xME;
    if ( sal_False == ( aAny >>= xME ) )
    {
        OSL_TRACE("Couln't locate user or share directories");
    }
    else
    {
        ::rtl::OUString base = ::rtl::OUString::createFromAscii(
             SAL_CONFIGFILE( "${$SYSBINDIR/bootstrap" ) );
        ::rtl::OUString user = ::rtl::OUString::createFromAscii( "::UserInstallation}/user"  );
        ::rtl::OUString share = ::rtl::OUString::createFromAscii( "::BaseInstallation}/share" );
        userDirString = xME->expandMacros( base.concat( user ) );
        shareDirString = xME->expandMacros( base.concat( share ) );
    }

}

ActiveMSPList::~ActiveMSPList()
{
    OSL_TRACE("ActiveMSPList::ActiveMSPList) - dtor");
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
        MspInst theMsp;
        theMsp.provider = msp;
        m_mModels[ xModel ] = theMsp;

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
            msp = createMSP( xModel );
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
        OSL_TRACE("ActiveMSPList::createMSP() for user/share %s",
            ::rtl::OUStringToOString( sContext,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        msp = createMSP( sContext );
    }
    return msp;
}


Reference< provider::XScriptProvider >
ActiveMSPList::createMSP( const ::rtl::OUString& context )
            throw ( RuntimeException )
{
    Reference< provider::XScriptProvider > msp;
    ::osl::MutexGuard guard( m_mutex );
    Msp_hash::const_iterator itr = m_hMsps.find( context );
    if ( itr == m_hMsps.end() )
    {
        OSL_TRACE("ActiveMSPList::createMSP( user/share ) no msp in cache for %s",
            ::rtl::OUStringToOString( context,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        Any aCtx;
        if ( context.equals( OUSTR("user") ) )
        {
            aCtx = makeAny( userDirString );
        }
        else if ( context.equals( OUSTR("share") ) )
        {
            aCtx = makeAny( shareDirString );
        }
        else
        {
            ::rtl::OUString message = OUSTR("ActiveMSPList::createMSP create for MSP failed, invalid context ");
            message.concat( context );
            // We will allow a MSP to be created with a default context
            // such an msp should be capable of handling getScript() for
            // non document scripts
            //throw RuntimeException( message, Reference< XInterface > () );
        }
        msp = createNewMSP( aCtx );
        MspInst mspEntry;
        mspEntry.provider = msp;
        m_hMsps[ context ] = mspEntry;
    }
    else
    {
        msp = itr->second.provider;
    }
    return msp;
}

Reference< provider::XScriptProvider >
ActiveMSPList::createMSP( const Reference< frame::XModel >& xModel )
            throw ( RuntimeException )
{
    Reference< provider::XScriptProvider > msp;
    ::osl::MutexGuard guard( m_mutex );
    Model_map::const_iterator itr = m_mModels.find( xModel );
    if ( itr == m_mModels.end() )
    {

        Any aCtx = makeAny( xModel );
        msp = createNewMSP( aCtx );
        addActiveMSP( xModel, msp ); // will update map
    }
    else
    {
        msp = itr->second.provider;
    }
    return msp;
}

Reference< provider::XScriptProvider >
ActiveMSPList::createNewMSP( const Any& context )
            throw ( RuntimeException )
{
    OSL_TRACE("ActiveMSPList::createNewMSP( ANY )");

    ::rtl::OUString serviceName = ::rtl::OUString::createFromAscii("drafts.com.sun.star.script.provider.MasterScriptProvider");
    Sequence< Any > args(1);
    args[ 0 ] = context;

    Reference< provider::XScriptProvider > msp( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( serviceName, args, m_xContext ), UNO_QUERY );
    return msp;
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
        MspInst userInstance;
        userInstance.provider = userMsp;
        m_hMsps[ OUSTR("user") ] = userInstance;

        args[ 0 ] <<= shareDirString;
        Reference< provider::XScriptProvider > shareMsp( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( serviceName, args, m_xContext ), UNO_QUERY );
        MspInst shareInstance;
        shareInstance.provider = shareMsp;
        // should check if provider reference is valid
        m_hMsps[ OUSTR("share") ] = shareInstance;
        created = true;
    }

}

} // namespace func_provider

