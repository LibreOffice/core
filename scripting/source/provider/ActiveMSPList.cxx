/*************************************************************************
 *
 *  $RCSfile: ActiveMSPList.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: npower $ $Date: 2003-09-04 07:23:28 $
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

#include <drafts/com/sun/star/script/framework/provider/XScriptProviderAccess.hpp>
#include <drafts/com/sun/star/script/framework/browse/BrowseNodeTypes.hpp>

#include "ActiveMSPList.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace drafts::com::sun::star::script::framework;

namespace func_provider
{

class BrowseNodeImpl :  public ::cppu::WeakImplHelper1< browse::XBrowseNode >
{
public:
BrowseNodeImpl( const Reference< provider::XScriptProvider >& msp, const ::rtl::OUString& location ): m_sNodeName( location ), m_xSP( msp )
{
    // strip out the last leaf of location name
    // e.g. file://dir1/dir2/Blah.sxw - > Blah.sxw
    ::rtl::OUString temp = location;
    sal_Int32 lastSlashIndex = temp.lastIndexOf( ::rtl::OUString::createFromAscii( "/" ) );

    if ( ( lastSlashIndex + 1 ) <  temp.getLength()  )
    {
        temp = temp.copy( lastSlashIndex + 1 );
    }
    // maybe we should throw here!!!
    else
    {
        OSL_TRACE("Something wrong with name, perhaps we should throw an exception");
    }
    m_sNodeName =  temp ;
}
 virtual ::rtl::OUString
SAL_CALL getName()
        throw ( RuntimeException )
{
    return m_sNodeName;
}
virtual Sequence< Reference< browse::XBrowseNode > > SAL_CALL
getChildNodes()
        throw ( RuntimeException )
{
    Reference < provider::XScriptProviderAccess > providerGetter( m_xSP, UNO_QUERY );
    Sequence< Reference< provider::XScriptProvider > > providers = providerGetter->getAllProviders();
    Sequence<  Reference< browse::XBrowseNode > > children( providers.getLength() );
    for ( sal_Int32 index = 0; index < providers.getLength(); index++ )
    {
        children[ index ] = Reference< browse::XBrowseNode >( providers[ index ], UNO_QUERY );
    }
    return children;
}

virtual sal_Bool SAL_CALL
hasChildNodes()
        throw ( RuntimeException )
{
    return true; //will always be user and share
}

virtual sal_Int16 SAL_CALL getType()
        throw ( RuntimeException )
{
    return browse::BrowseNodeTypes::CONTAINER;
}
private:
    ::rtl::OUString m_sNodeName;
    Reference < provider::XScriptProvider > m_xSP;
};
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
               const Reference< dcsssf::provider::XScriptProvider >& msp )
{
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

    ::rtl::OUString doc = getDocNameOrURLFromModel( xModel );

    if ( doc.getLength() != 0 )
    {
        ::osl::MutexGuard guard( m_mutex );
        Model_map::const_iterator itr = m_mModels.find( xModel );
        if ( itr != m_mModels.end() )
        {
            OSL_TRACE("ActiveMSPList::addActiveMSP() have MSP for model already" );
        }
        else
        {
            MspInst theMsp;
            theMsp.docName = doc;
            theMsp.provider = msp;
            m_mModels[ xModel ] = theMsp;
        }

    }
    else
    {
        OSL_TRACE("ActiveMSPList::addActiveMSP() couldn't get document name from model" );
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

::rtl::OUString
ActiveMSPList::getDocNameOrURLFromModel( const Reference< frame::XModel >& xModel  )
{
    ::rtl::OUString docNameOrURL;
    if ( xModel.is() )
    {
        if ( xModel->getURL().getLength() != 0)
        {
            docNameOrURL =  xModel->getURL();
            OSL_TRACE("ActiveMSPList::getDocNameOrURLFromModel() url for document %s.",
                     ::rtl::OUStringToOString( docNameOrURL,
                                                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
        else
        // Untitled document
        {
            ::rtl::OUString tempName;
            try
            {
                Reference< beans::XPropertySet > propSet( xModel->getCurrentController()->getFrame(), UNO_QUERY );
                if ( propSet.is() )
                {
                    if ( sal_True == ( propSet->getPropertyValue(::rtl::OUString::createFromAscii( "Title" ) ) >>= tempName ) )
                    {
                        // process "UntitledX - YYYYYYYY"
                        // to get UntitledX
                        sal_Int32 pos = 0;
                        docNameOrURL = tempName.getToken(0,' ',pos);
                        OSL_TRACE("ActiveMSPList::getDocNameOrURLFromModel() Title for document is %s.",
                            ::rtl::OUStringToOString( docNameOrURL,
                                                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                    }
                }
                else
                {
                    OSL_TRACE("ActiveMSPList::getDocNameOrURLFromModel() doc model invalid" );
                }
            }
            catch ( Exception& e )
            {
                OSL_TRACE("ActiveMSPList::getDocNameOrURLFromModel() exception thrown: ",
                    ::rtl::OUStringToOString( e.Message,
                        RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            }
        }
    }
    else
    {
        OSL_TRACE("ActiveMSPList::getDocNameOrURLFromModel() doc model is null" );
    }
    return docNameOrURL;
}

ActiveMSPList&
ActiveMSPList::instance( const Reference< XComponentContext > & xContext )
{
    static ActiveMSPList* inst = 0;
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
        ::rtl::OUString serviceName = ::rtl::OUString::createFromAscii("drafts.com.sun.star.script.framework.provider.MasterScriptProvider");
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

::rtl::OUString SAL_CALL
ActiveMSPList::getName()
        throw ( css::uno::RuntimeException )
{
    return ::rtl::OUString::createFromAscii("Root");
}

Sequence< Reference< browse::XBrowseNode > > SAL_CALL
ActiveMSPList::getChildNodes()
        throw ( css::uno::RuntimeException )
{
    // 1. create a XBrowseNodeImpl (location )node for each MSP in the ActiveMSPList
    // 2. add each Provider as a childNode for (location ) node
    // create user & share MSP's if needed
    ::osl::MutexGuard guard( m_mutex );
    createNonDocMSPs();

    // number of child nodes is number of providers for application ( user & share )
    // + providers for active documents
    sal_Int32 numChildNodes = m_hMsps.size() + m_mModels.size();

    // get providers for application
    Msp_hash::iterator h_itEnd =  m_hMsps.end();
    Sequence< Reference< browse::XBrowseNode > > children( numChildNodes );
    sal_Int32 count = 0;
    for ( Msp_hash::iterator h_it = m_hMsps.begin(); h_it != h_itEnd; ++h_it )
    {
        OSL_TRACE("Adding application browsenode index [ %d ]", count );
        children[ count++ ] = new BrowseNodeImpl( h_it->second, h_it->first );
    }

    // get providers for active documents
    Model_map::iterator m_itEnd =  m_mModels.end();

    for ( Model_map::iterator m_it = m_mModels.begin(); m_it != m_itEnd; ++m_it )
    {
        OSL_TRACE("Adding document browsenode index [ %d ]", count  );
        ::rtl::OUString docName = m_it->second.docName;
        if ( m_it->first->getURL().getLength() > 0 )
        {
            if ( ! m_it->second.docName.equals( m_it->first->getURL() ) )
            {
                OSL_TRACE("Need to change doc name" );
                m_it->second.docName = m_it->first->getURL();
            }
        }
        children[ count++ ] = new BrowseNodeImpl( m_it->second.provider, m_it->second.docName );
    }

    return children;
}

sal_Bool SAL_CALL
ActiveMSPList::hasChildNodes()
        throw ( css::uno::RuntimeException )
{
    return sal_True;
}

sal_Int16 SAL_CALL
ActiveMSPList::getType()
        throw ( css::uno::RuntimeException )
{
    return browse::BrowseNodeTypes::ROOT;
}

} // namespace func_provider

