/*************************************************************************
 *
 *  $RCSfile: MasterScriptProvider.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 14:10:18 $
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
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>

#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>

#include <com/sun/star/deployment/XPackage.hpp>
#include <drafts/com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <drafts/com/sun/star/script/provider/XScriptProviderFactory.hpp>
#include <drafts/com/sun/star/script/provider/ScriptFrameworkErrorType.hpp>

#include <util/scriptingconstants.hxx>
#include <util/util.hxx>
#include <util/MiscUtils.hxx>

#include "ActiveMSPList.hxx"
#include "MasterScriptProvider.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script;
using namespace ::sf_misc;

namespace func_provider
{

::rtl::OUString s_implName = ::rtl::OUString::createFromAscii(
    "drafts.com.sun.star.script.provider.MasterScriptProvider" );
const ::rtl::OUString s_serviceNameList[] = {
    ::rtl::OUString::createFromAscii(
        "drafts.com.sun.star.script.provider.MasterScriptProvider" ),
    ::rtl::OUString::createFromAscii(
        "drafts.com.sun.star.script.provider.ScriptProvider" ) };

Sequence< ::rtl::OUString > s_serviceNames = Sequence <
        ::rtl::OUString > ( s_serviceNameList, 2 );

//*************************************************************************
//  Definitions for MasterScriptProviderFactory global methods.
//*************************************************************************

::rtl::OUString SAL_CALL mspf_getImplementationName() ;
Reference< XInterface > SAL_CALL mspf_create( Reference< XComponentContext > const & xComponentContext );
Sequence< ::rtl::OUString > SAL_CALL mspf_getSupportedServiceNames();


bool endsWith( const ::rtl::OUString& target,
    const ::rtl::OUString& item )
{
    sal_Int32 index = 0;
    if (  ( index = target.indexOf( item ) ) != -1  &&
       ( index == ( target.getLength() - item.getLength() ) ) )
    {
        return true;
    }
    return false;
}
//::rtl_StandardModuleCount s_moduleCount = MODULE_COUNT_INIT;

/* should be available in some central location. */
//*************************************************************************
// XScriptProvider implementation
//
//*************************************************************************
MasterScriptProvider::MasterScriptProvider( const Reference< XComponentContext > & xContext ) throw ( RuntimeException ):
        m_xContext( xContext ), m_bIsValid( false ), m_bInitialised( false ),
        m_bIsPkgMSP( false ), m_pPCache( 0 )
{
    OSL_TRACE( "< MasterScriptProvider ctor called >\n" );

    validateXRef( m_xContext, "MasterScriptProvider::MasterScriptProvider: No context available\n" );
    m_xMgr = m_xContext->getServiceManager();
    validateXRef( m_xMgr,
                  "MasterScriptProvider::MasterScriptProvider: No service manager available\n" );
    m_bIsValid = true;
}

//*************************************************************************
MasterScriptProvider::~MasterScriptProvider()
{
    OSL_TRACE( "< MasterScriptProvider dtor called >\n" );

    //s_moduleCount.modCnt.release( &s_moduleCount.modCnt );
    if ( m_pPCache )
    {
        delete m_pPCache;
    }
    m_pPCache = 0;
}

//*************************************************************************
void SAL_CALL MasterScriptProvider::initialize( const Sequence < Any >& args )
throw ( Exception, RuntimeException )
{
    OSL_TRACE( "< MasterScriptProvider::initialize() method called >\n" );
    if ( m_bInitialised )
    {
        OSL_TRACE( "MasterScriptProvider Already initialised" );
        return ;
    }

    m_bIsValid = false;


    sal_Int32 len = args.getLength();
    if ( len > 1  )
    {
        throw RuntimeException(
            OUSTR( "MasterScriptProvider::initialize: invalid number of arguments" ),
            Reference< XInterface >() );
    }

    Sequence< Any > invokeArgs( len );

    if ( len != 0 )
    {
        Any stringAny = makeAny( ::rtl::OUString() );

        // check if first parameter is a string
        // if it is, this implies that this is a MSP created
        // with a user or share ctx ( used for browse functionality )

        //
        if ( args[ 0 ].getValueType() ==  ::getCppuType((const ::rtl::OUString* ) NULL ) )
        {
             args[ 0 ] >>= m_sCtxString;
            OSL_TRACE("Creating MSP for context is %s",
            ::rtl::OUStringToOString( m_sCtxString , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            invokeArgs[ 0  ] = args[ 0 ];
            if ( m_sCtxString.indexOf( OUSTR("vnd.sun.star.tdoc") ) == 0 )
            {
                OSL_TRACE("Creating MSP for tdoc url %s",
                    ::rtl::OUStringToOString( m_sCtxString , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                m_xModel =  MiscUtils::tDocUrlToModel( m_sCtxString );
            }

        }
        else if (  args[ 0 ].getValueType() == ::getCppuType((const Reference< frame::XModel >* ) NULL ) )

        {
            try
            {
                m_xModel.set( args[ 0 ], UNO_QUERY_THROW );
                m_sCtxString =  MiscUtils::xModelToTdocUrl( m_xModel );
                Any propValURL = makeAny( m_sCtxString );
                OSL_TRACE( "!!** XModel URL inserted into any is %s \n",
                    ::rtl::OUStringToOString(
                        m_sCtxString , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                invokeArgs[ 0 ] <<= propValURL;
            }

            catch ( beans::UnknownPropertyException & e )
            {
                ::rtl::OUString temp = OUSTR(
                                       "MasterScriptProvider::initialize: caught UnknownPropertyException: " );
                throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
            }
            catch ( RuntimeException & e )
            {
                ::rtl::OUString temp = OUSTR( "MasterScriptProvider::initialize: " );
                throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
            }
        }
        ::rtl::OUString pkgSpec( OUSTR("uno_packages") );
        sal_Int32 indexOfPkgSpec = m_sCtxString.lastIndexOf( pkgSpec );

        // if contex string ends with "uno_packages"
        if ( indexOfPkgSpec > -1 && ( m_sCtxString.match( pkgSpec, indexOfPkgSpec ) == sal_True ) )
        {
            OSL_TRACE("** ** Setting this to be a package MSP");
            m_bIsPkgMSP = sal_True;
        }
        else
        {
            m_bIsPkgMSP = sal_False;
        }
    }
    else // no args
    {
        // use either scriping context or maybe zero args?
        invokeArgs = Sequence< Any >( 0 ); // no arguments
    }
    m_sAargs = invokeArgs;
    // don't create pkg mgr MSP for documents, not supported
    if ( m_bIsPkgMSP == sal_False && !m_xModel.is() )
    {
        createPkgProvider();
    }

    m_bInitialised = true;
    m_bIsValid = true;
    OSL_TRACE( "Initialised XMasterScriptProvider for %s",
            ::rtl::OUStringToOString( m_sCtxString , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
}


//*************************************************************************
void MasterScriptProvider::createPkgProvider()
{
    OSL_TRACE("In MasterScriptProvider::createPkgProvider()");
    try
    {
        ::rtl::OUString loc = m_sCtxString;
        Any location;
        ::rtl::OUString sPkgCtx =  m_sCtxString.concat( OUSTR(":uno_packages") );
        location <<= sPkgCtx;

        OSL_TRACE("About to create pkg MSP");

        Reference< provider::XScriptProviderFactory > xFac(
            m_xContext->getValueByName(
                OUSTR( "/singletons/drafts.com.sun.star.script.provider.theMasterScriptProviderFactory") ), UNO_QUERY_THROW );

        m_xMSPPkg.set(
            xFac->createScriptProvider( location ), UNO_QUERY_THROW );

    }
    catch ( Exception& e )
    {
        OSL_TRACE("Failed to create msp for uno_packages in context %s, error is %s",
                ::rtl::OUStringToOString( m_sCtxString,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer,
                ::rtl::OUStringToOString( e.Message,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    }
}

//*************************************************************************
Reference< provider::XScript >
MasterScriptProvider::getScript( const ::rtl::OUString& scriptURI )
throw ( provider::ScriptFrameworkErrorException,
        RuntimeException )
{
    if ( !isValid() )
    {
        throw provider::ScriptFrameworkErrorException(
            OUSTR( "MasterScriptProvider not initialised" ), Reference< XInterface >(),
            scriptURI, OUSTR(""),
            provider::ScriptFrameworkErrorType::UNKNOWN );
        throw RuntimeException(
            OUSTR( "MasterScriptProvider::getScript(), service object not initialised properly." ),
            Reference< XInterface >() );
    }

    OSL_TRACE( "**  ==> MasterScriptProvider in getScript\n" );
    OSL_TRACE( "Script URI is %s\n Context is %s",
        ::rtl::OUStringToOString( scriptURI,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer,
        ::rtl::OUStringToOString( m_sCtxString,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer  );

    // need to get the language from the string

    Reference< uri::XUriReferenceFactory > xFac (
         m_xMgr->createInstanceWithContext( rtl::OUString::createFromAscii(
            "com.sun.star.uri.UriReferenceFactory"), m_xContext ) , UNO_QUERY );
    if ( !xFac.is() )
    {
        ::rtl::OUString message = ::rtl::OUString::createFromAscii("Failed to instantiate  UriReferenceFactory");
        throw provider::ScriptFrameworkErrorException(
            message, Reference< XInterface >(),
            scriptURI, ::rtl::OUString(),
            provider::ScriptFrameworkErrorType::UNKNOWN );
    }

    Reference<  uri::XUriReference > uriRef(
        xFac->parse( scriptURI ), UNO_QUERY );

    Reference < uri::XVndSunStarScriptUrl > sfUri( uriRef, UNO_QUERY );

    if ( !uriRef.is() || !sfUri.is() )
    {
        ::rtl::OUString errorMsg = OUSTR( "Incorrect format for Script URI: " );
        errorMsg = errorMsg.concat( scriptURI );
        OSL_TRACE( "%s",
            ::rtl::OUStringToOString( errorMsg,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer  );
        throw provider::ScriptFrameworkErrorException(
            errorMsg, Reference< XInterface >(),
            scriptURI, OUSTR(""),
            provider::ScriptFrameworkErrorType::UNKNOWN );
    }

    ::rtl::OUString langKey = ::rtl::OUString::createFromAscii( "language" );
    ::rtl::OUString locKey = ::rtl::OUString::createFromAscii( "location" );

    if ( sfUri->hasParameter( langKey ) == sal_False ||
         sfUri->hasParameter( locKey ) == sal_False ||
         ( sfUri->getName().getLength() == 0  ) )
    {
        ::rtl::OUString errorMsg = OUSTR( "Incorrect format for Script URI: " );
        OSL_TRACE( "%s",
            ::rtl::OUStringToOString( errorMsg,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer  );
        errorMsg = errorMsg.concat( scriptURI );
        throw provider::ScriptFrameworkErrorException(
            errorMsg, Reference< XInterface >(),
            scriptURI, OUSTR(""),
            provider::ScriptFrameworkErrorType::UNKNOWN );
    }

    ::rtl::OUString language = sfUri->getParameter( langKey );
    ::rtl::OUString location = sfUri->getParameter( locKey );
    // Temporary debug lines
    OSL_TRACE( "Script location %s\n Context is %s\n endwith %d",
        ::rtl::OUStringToOString( location,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer,
        ::rtl::OUStringToOString( m_sCtxString,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer,
        endsWith( m_sCtxString, location )  );

    // if script us located in uno pkg
    sal_Int32 index = -1;
    ::rtl::OUString pkgTag =
        ::rtl::OUString::createFromAscii( ":uno_packages" );
    // for languages other than basic,  scripts located in uno packages
    // are merged into the user/share location context.
    // For other languages the location attribute in script url has the form
    // location = [user|share]:uno_packages or location :uno_pacakges/xxxx.uno.pkg
    // we need to extract the value of location part from the
    // location attribute of the script, if the script is located in an
    // uno package then that is the location part up to and including
    // ":uno_packages", if the script is not in an uno package then the
    // normal value is used e.g. user or share.
    // The value extracted will be used to determine if the script is
    // located in the same location context as this MSP.
    // For Basic, the language script provider can handle the execution of a
    // script in any location context
    if ( ( index = location.indexOf( pkgTag ) ) > -1 )
    {
        location = location.copy( 0, index + pkgTag.getLength() );
    }

    Reference< provider::XScript > xScript;

    // If the script location is in the same location context as this
    // MSP then delate to the lanaguage provider controlled by this MSP
    // ** Special case is BASIC, all calls to getScript will be handled
    // by the language script provider in the current location context
    // even if its different
    if ( ( location.equals( OUSTR( "document" ) ) && m_xModel.is() )  ||
         ( endsWith( m_sCtxString, location ) ) ||
         ( language.equals( OUSTR( "Basic" ) ) )
         )
    {
        OSL_TRACE("MasterScriptProvider::getScript() location is in this context or is Basic script");
        Reference< provider::XScriptProvider > xScriptProvider;
        ::rtl::OUStringBuffer buf( 80 );
        buf.appendAscii( "drafts.com.sun.star.script.provider.ScriptProviderFor");
        buf.append( language );
        ::rtl::OUString serviceName = buf.makeStringAndClear();
        if ( providerCache() )
        {
            try
            {
                xScriptProvider.set(
                    providerCache()->getProvider( serviceName ),
                    UNO_QUERY_THROW );
            }
            catch( Exception& e )
            {
                OSL_TRACE("Attempt to throw ScriptFrameworkException");
                throw provider::ScriptFrameworkErrorException(
                    e.Message, Reference< XInterface >(),
                    sfUri->getName(), language,
                    provider::ScriptFrameworkErrorType::NOTSUPPORTED );
            }
        }
        else
        {
            throw provider::ScriptFrameworkErrorException(
                OUSTR( "No LanguageProviders detected" ),
                Reference< XInterface >(),
                sfUri->getName(), language,
                provider::ScriptFrameworkErrorType::NOTSUPPORTED );
        }
        xScript=xScriptProvider->getScript( scriptURI );
    }
    else
    {
        Reference< provider::XScriptProviderFactory > xFac(
            m_xContext->getValueByName(
                OUSTR( "/singletons/drafts.com.sun.star.script.provider.theMasterScriptProviderFactory") ), UNO_QUERY_THROW );

        Reference< provider::XScriptProvider > xSP(
            xFac->createScriptProvider( makeAny( location ) ), UNO_QUERY_THROW );
        xScript = xSP->getScript( scriptURI );
    }

    return xScript;
}
//*************************************************************************
bool
MasterScriptProvider::isValid()
{
    return m_bIsValid;
}

//*************************************************************************
ProviderCache*
MasterScriptProvider::providerCache()
{
    if ( !m_pPCache )
    {
        ::osl::MutexGuard aGuard( m_mutex );
        if ( !m_pPCache )
        {
            ::rtl::OUString serviceName1 = OUSTR("drafts.com.sun.star.script.provider.ScriptProviderForBasic");
            ::rtl::OUString serviceName2 = OUSTR("drafts.com.sun.star.script.provider.ScriptProviderForPython");
            Sequence< ::rtl::OUString > blacklist(2);
            blacklist[ 0 ] = serviceName1;
            blacklist[ 1 ] = serviceName2;

            if ( !m_bIsPkgMSP )
            {
                m_pPCache = new ProviderCache( m_xContext, m_sAargs );
            }
            else
            {
                m_pPCache = new ProviderCache( m_xContext, m_sAargs, blacklist );
            }
        }
    }
    return m_pPCache;
}


//*************************************************************************
::rtl::OUString SAL_CALL
MasterScriptProvider::getName()
        throw ( css::uno::RuntimeException )
{
    OSL_TRACE("MasterScriptProvider::getName() for %s",
        ::rtl::OUStringToOString( getContextString(),
            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    if ( !isPkgProvider() )
    {
        ::rtl::OUString sCtx = getContextString();
        if ( sCtx.indexOf( OUSTR( "vnd.sun.star.tdoc" ) ) == 0 )
        {
            // seems to be a bug with tdoc, Title is
            // incorrect after a save-as, also
            // Title property is not returned if it is set
            //m_sNodeName = MiscUtils::tDocUrlToTitle( sCtx );

            Reference< frame::XModel > xModel = m_xModel;
            if ( !xModel.is() )
            {
                xModel = MiscUtils::tDocUrlToModel( sCtx );
            }

            m_sNodeName = MiscUtils::xModelToDocTitle( xModel );
        }
        else
        {
            m_sNodeName = parseLocationName( getContextString() );
        }
    }
    else
    {
        m_sNodeName = OUSTR("uno_packages");
    }
    OSL_TRACE("MasterScriptProvider::getName() returning name %s ",
        ::rtl::OUStringToOString( m_sNodeName,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    return m_sNodeName;
}

//*************************************************************************
Sequence< Reference< browse::XBrowseNode > > SAL_CALL
MasterScriptProvider::getChildNodes()
        throw ( css::uno::RuntimeException )
{
    OSL_TRACE("MasterScriptProvider:getChildNodes() ctx = %s",
        ::rtl::OUStringToOString( getContextString(),
            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    Sequence< Reference< provider::XScriptProvider > > providers = getAllProviders();

    Reference< provider::XScriptProvider > pkgProv = getPkgProvider();
    sal_Int32 size = providers.getLength();
    bool hasPkgs = pkgProv.is();
    if ( hasPkgs  )
    {
        size++;
    }
    Sequence<  Reference< browse::XBrowseNode > > children( size );
    sal_Int32 provIndex = 0;
    for ( ; provIndex < providers.getLength(); provIndex++ )
    {
        children[ provIndex ] = Reference< browse::XBrowseNode >( providers[ provIndex ], UNO_QUERY );
    }

    if ( hasPkgs  )
    {
        children[ provIndex ] = Reference< browse::XBrowseNode >( pkgProv, UNO_QUERY );

    }

    return children;
}

//*************************************************************************
sal_Bool SAL_CALL
MasterScriptProvider::hasChildNodes()
        throw ( css::uno::RuntimeException )
{
    return sal_True;
}

//*************************************************************************
sal_Int16 SAL_CALL
MasterScriptProvider::getType()
        throw ( css::uno::RuntimeException )
{
    return browse::BrowseNodeTypes::CONTAINER;
}

//*************************************************************************

::rtl::OUString
MasterScriptProvider::parseLocationName( const ::rtl::OUString& location )
{
    // strip out the last leaf of location name
    // e.g. file://dir1/dir2/Blah.sxw - > Blah.sxw
    ::rtl::OUString temp = location;
    sal_Int32 lastSlashIndex = temp.lastIndexOf( ::rtl::OUString::createFromAscii( "/" ) );

    if ( lastSlashIndex > -1 )
    {
        if ( ( lastSlashIndex + 1 ) <  temp.getLength()  )
        {
            temp = temp.copy( lastSlashIndex + 1 );
        }
    }
    return temp;
}

//*************************************************************************
// Register Package
void SAL_CALL
MasterScriptProvider::insertByName( const ::rtl::OUString& aName, const Any& aElement ) throw ( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, css::uno::RuntimeException)
{
    if ( !m_bIsPkgMSP )
    {
        if ( m_xMSPPkg.is() )
        {
            Reference< container::XNameContainer > xCont( m_xMSPPkg, UNO_QUERY );
            if ( !xCont.is() )
            {
                throw RuntimeException(
                    OUSTR("PackageMasterScriptProvider doesn't implement XNameContainer"),
                    Reference< XInterface >() );
            }
            xCont->insertByName( aName, aElement );
        }
        else
        {
            throw RuntimeException( OUSTR("PackageMasterScriptProvider is unitialised"),
                                        Reference< XInterface >() );
        }

    }
    else
    {
        Reference< deployment::XPackage > xPkg( aElement, UNO_QUERY );
        if ( !xPkg.is() )
        {
            throw lang::IllegalArgumentException( OUSTR("Couldn't convert to XPackage"),
                                                      Reference < XInterface > (), 2 );
        }
        if ( !aName.getLength() )
        {
            throw lang::IllegalArgumentException( OUSTR("Name not set!!"),
                                                      Reference < XInterface > (), 1 );
        }
        // TODO for library pacakge parse the language, for the moment will try
        // to get each provider to process the new Package, the first one the succeeds
        // will terminate processing
        if ( !providerCache() )
        {
            throw RuntimeException(
                OUSTR("insertByName cannot instantiate "
                    "child script providers."),
                Reference< XInterface >() );
        }
        Sequence < Reference< provider::XScriptProvider > > xSProviders =
            providerCache()->getAllProviders();
        sal_Int32 index = 0;
        for ( ; index < xSProviders.getLength(); index++ )
        {
            Reference< container::XNameContainer > xCont( xSProviders[ index ], UNO_QUERY );
            if ( !xCont.is() )
            {
                OSL_TRACE("Ignoring script provider [%d]", index );
                continue;
            }
            try
            {
                xCont->insertByName( aName, aElement );
                OSL_TRACE("ScriptProvider [%d] insertByName succeeded for package named %s ", index,
                    ::rtl::OUStringToOString( aName,
                        RTL_TEXTENCODING_ASCII_US ).pData->buffer );

                break;
            }
            catch ( Exception& ignore )
            {
            }

        }
        if ( index == xSProviders.getLength() )
        {
            // No script providers could process the package
            ::rtl::OUString message = OUSTR("Failed to register package for ");
            message = message.concat( aName );
            OSL_TRACE("Failed to register package, throwing illegalArgument: %s",
                ::rtl::OUStringToOString( message,
                        RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            throw lang::IllegalArgumentException( message,
                                                      Reference < XInterface > (), 2 );
        }
   }
}

//*************************************************************************
// Revoke Package
void SAL_CALL
MasterScriptProvider::removeByName( const ::rtl::OUString& Name ) throw ( container::NoSuchElementException, lang::WrappedTargetException, RuntimeException)
{
    if ( !m_bIsPkgMSP )
    {
        if ( m_xMSPPkg.is() )
        {
            Reference< container::XNameContainer > xCont( m_xMSPPkg, UNO_QUERY );
            if ( !xCont.is() )
            {
                throw RuntimeException(
                    OUSTR("PackageMasterScriptProvider doesn't implement XNameContainer"),
                    Reference< XInterface >() );
            }
            xCont->removeByName( Name );
        }
        else
        {
            throw RuntimeException( OUSTR("PackageMasterScriptProvider is unitialised"),
                                        Reference< XInterface >() );
        }

   }
   else
   {
        if ( !Name.getLength() )
        {
            throw lang::IllegalArgumentException( OUSTR("Name not set!!"),
                                                      Reference < XInterface > (), 1 );
        }
        // TODO for Script library pacakge url parse the language,
        // for the moment will just try to get each provider to process remove/revoke
        // request, the first one the succeeds will terminate processing

        if ( !providerCache() )
        {
            throw RuntimeException(
                OUSTR("removeByName() cannot instantiate "
                    "child script providers."),
                Reference< XInterface >() );
        }
        Sequence < Reference< provider::XScriptProvider > > xSProviders =
            providerCache()->getAllProviders();
        sal_Int32 index = 0;
        for ( ; index < xSProviders.getLength(); index++ )
        {
            Reference< container::XNameContainer > xCont( xSProviders[ index ], UNO_QUERY );
            if ( !xCont.is() )
            {
                OSL_TRACE("Ignoring script provider [%d]", index );
                continue;
            }
            try
            {
                xCont->removeByName( Name );
                OSL_TRACE("ScriptProvider [%d] removeByName succeeded for package named %s ", index,
                    ::rtl::OUStringToOString( Name,
                        RTL_TEXTENCODING_ASCII_US ).pData->buffer );

                break;
            }
            catch ( Exception& ignore )
            {
            }

        }
        if ( index == xSProviders.getLength() )
        {
            // No script providers could process the package
            ::rtl::OUString message = OUSTR("Failed to revoke package for ");
            message = message.concat( Name );
            throw lang::IllegalArgumentException( message,
                                                      Reference < XInterface > (), 1 );
        }

    }
}

//*************************************************************************
void SAL_CALL
MasterScriptProvider::replaceByName( const ::rtl::OUString& aName, const Any& aElement ) throw ( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, RuntimeException)
{
    // TODO needs implementing
    if ( true )
    {
        throw RuntimeException(  OUSTR("replaceByName not implemented!!!!") ,
                Reference< XInterface >() );
    }
}
//*************************************************************************
Any SAL_CALL
MasterScriptProvider::getByName( const ::rtl::OUString& aName ) throw ( container::NoSuchElementException, lang::WrappedTargetException, RuntimeException)
{
    // TODO needs to be implemented
    Any result;
    if ( true )
    {
        throw RuntimeException(  OUSTR("getByName not implemented!!!!") ,
                Reference< XInterface >() );
    }
    return result;
}
//*************************************************************************
sal_Bool SAL_CALL
MasterScriptProvider::hasByName( const ::rtl::OUString& aName ) throw (RuntimeException)
{
    sal_Bool result = sal_False;
    if ( !m_bIsPkgMSP )
    {
        if ( m_xMSPPkg.is() )
        {
            Reference< container::XNameContainer > xCont( m_xMSPPkg, UNO_QUERY );
            if ( !xCont.is() )
            {
                throw RuntimeException(
                    OUSTR("PackageMasterScriptProvider doesn't implement XNameContainer"),
                    Reference< XInterface >() );
            }

            result = xCont->hasByName( aName );
        }
        else
        {
            throw RuntimeException( OUSTR("PackageMasterScriptProvider is unitialised"),
                                        Reference< XInterface >() );
        }

   }
   else
   {
        if ( !aName.getLength() )
        {
            throw lang::IllegalArgumentException( OUSTR("Name not set!!"),
                                                      Reference < XInterface > (), 1 );
        }
        // TODO for Script library pacakge url parse the language,
        // for the moment will just try to get each provider to see if the
        // package exists in any provider, first one that succeed will
        // terminate the loop

        if ( !providerCache() )
        {
            throw RuntimeException(
                OUSTR("removeByName() cannot instantiate "
                    "child script providers."),
                Reference< XInterface >() );
        }
        Sequence < Reference< provider::XScriptProvider > > xSProviders =
            providerCache()->getAllProviders();
        for ( sal_Int32 index = 0; index < xSProviders.getLength(); index++ )
        {
            Reference< container::XNameContainer > xCont( xSProviders[ index ], UNO_QUERY );
            if ( !xCont.is() )
            {
                OSL_TRACE("Ignoring script provider [%d]", index );
                continue;
            }
            try
            {
                result = xCont->hasByName( aName );
                OSL_TRACE("ScriptProvider [%d] hasByName for %s returned %s",
                    index,
                    ::rtl::OUStringToOString( aName,
                        RTL_TEXTENCODING_ASCII_US ).pData->buffer,
                    ( result == sal_True ) ? "TRUE" : "FALSE" );
                if ( result == sal_True )
                {
                    break;
                }
            }
            catch ( Exception& ignore )
            {
            }

        }
    }
    return result;
}

//*************************************************************************
Sequence< ::rtl::OUString > SAL_CALL
MasterScriptProvider::getElementNames(  ) throw ( RuntimeException)
{
    // TODO needs implementing
    Sequence< ::rtl::OUString >  names;
    if ( true )
    {
        throw RuntimeException(  OUSTR("getElementNames not implemented!!!!") ,
                Reference< XInterface >() );
    }
    return names;
}
//*************************************************************************
Type SAL_CALL
MasterScriptProvider::getElementType(  ) throw ( RuntimeException)
{
    // TODO needs implementing
    Type t;
    return t;
}
//*************************************************************************
sal_Bool SAL_CALL MasterScriptProvider::hasElements(  ) throw ( RuntimeException)
{
    // TODO needs implementing
    if ( true )
    {
        throw RuntimeException(  OUSTR("hasElements not implemented!!!!") ,
                Reference< XInterface >() );
    }
}

//*************************************************************************
Sequence< Reference< provider::XScriptProvider > > SAL_CALL
MasterScriptProvider::getAllProviders() throw ( css::uno::RuntimeException )
{
    OSL_TRACE("Entering MasterScriptProvider Get ALL Providers() " );
    if ( providerCache() )
    {
        OSL_TRACE("Leaving MasterScriptProvider Get ALL Providers() " );
        return providerCache()->getAllProviders();
    }
    else
    {
        OSL_TRACE("MasterScriptProvider::getAllProviders() something wrong, no language providers");
        ::rtl::OUString errorMsg;
        errorMsg = ::rtl::OUString::createFromAscii("MasterScriptProvider::getAllProviders, unknown error, cache not initialised");
        OSL_TRACE("Leaving MasterScriptProvider Get ALL Providers() THROWING " );
        return providerCache()->getAllProviders();
        throw RuntimeException( errorMsg.concat( errorMsg ),
                        Reference< XInterface >() );

    }
}


//*************************************************************************
::rtl::OUString SAL_CALL MasterScriptProvider::getImplementationName( )
throw( RuntimeException )
{
    return s_implName;
}

//*************************************************************************
sal_Bool SAL_CALL MasterScriptProvider::supportsService( const ::rtl::OUString& serviceName )
throw( RuntimeException )
{
    ::rtl::OUString const * pNames = s_serviceNames.getConstArray();
    for ( sal_Int32 nPos = s_serviceNames.getLength(); nPos--; )
    {
        if ( serviceName.equals( pNames[ nPos ] ) )
        {
            return sal_True;
        }
    }
    return sal_False;
}

//*************************************************************************
Sequence< ::rtl::OUString > SAL_CALL MasterScriptProvider::getSupportedServiceNames( )
throw( RuntimeException )
{
        return s_serviceNames;
}

} // namespace func_provider


namespace browsenodefactory
{
::rtl::OUString SAL_CALL bnf_getImplementationName() ;
Reference< XInterface > SAL_CALL bnf_create( Reference< XComponentContext > const & xComponentContext );
Sequence< ::rtl::OUString > SAL_CALL bnf_getSupportedServiceNames();
}

namespace scripting_runtimemgr
{
//*************************************************************************
Reference< XInterface > SAL_CALL sp_create(
    const Reference< XComponentContext > & xCompC )
{
    return ( cppu::OWeakObject * ) new ::func_provider::MasterScriptProvider( xCompC );
}

//*************************************************************************
Sequence< ::rtl::OUString > sp_getSupportedServiceNames( )
    SAL_THROW( () )
{
    return ::func_provider::s_serviceNames;
}

//*************************************************************************
::rtl::OUString sp_getImplementationName( )
SAL_THROW( () )
{
    return ::func_provider::s_implName;
}

static struct cppu::ImplementationEntry s_entries [] =
    {
        {
            sp_create, sp_getImplementationName,
            sp_getSupportedServiceNames, cppu::createSingleComponentFactory,
            0, 0
        },
        {
            func_provider::mspf_create, func_provider::mspf_getImplementationName,
            func_provider::mspf_getSupportedServiceNames, cppu::createSingleComponentFactory,
            0, 0
        },
        {
            browsenodefactory::bnf_create, browsenodefactory::bnf_getImplementationName,
            browsenodefactory::bnf_getSupportedServiceNames, cppu::createSingleComponentFactory,
            0, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };
}

//############################################################################
//#### EXPORTED ##############################################################
//############################################################################

/**
 * Gives the environment this component belongs to.
 */
extern "C"
{
    void SAL_CALL component_getImplementationEnvironment(
            const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    /**
     * This function creates an implementation section in the registry and another subkey
     *
     * for each supported service.
     * @param pServiceManager   the service manager
     * @param pRegistryKey      the registry key
     */
    sal_Bool SAL_CALL component_writeInfo(
            lang::XMultiServiceFactory * pServiceManager,
            registry::XRegistryKey * pRegistryKey )
    {
        if (::cppu::component_writeInfoHelper( pServiceManager, pRegistryKey,
            ::scripting_runtimemgr::s_entries ))
        {
            try
            {
                // MasterScriptProviderFactory Mangager singleton
                registry::XRegistryKey * pKey =
                    reinterpret_cast< registry::XRegistryKey * >(pRegistryKey);

                Reference< registry::XRegistryKey >xKey = pKey->createKey(
                    OUSTR("drafts.com.sun.star.script.provider.MasterScriptProviderFactory/UNO/SINGLETONS/drafts.com.sun.star.script.provider.theMasterScriptProviderFactory"));
                xKey->setStringValue( OUSTR("drafts.com.sun.star.script.provider.MasterScriptProviderFactory") );
                // BrowseNodeFactory Mangager singleton
                xKey = pKey->createKey(
                    OUSTR("drafts.com.sun.star.script.browse.BrowseNodeFactory/UNO/SINGLETONS/drafts.com.sun.star.script.browse.theBrowseNodeFactory"));
                xKey->setStringValue( OUSTR("drafts.com.sun.star.script.browse.BrowseNodeFactory") );
                return sal_True;
            }
            catch (Exception & exc)
            {
            }
        }
        return sal_False;
    }

    /**
     * This function is called to get service factories for an implementation.
     *
     * @param pImplName       name of implementation
     * @param pServiceManager a service manager, need for component creation
     * @param pRegistryKey    the registry key for this component, need for persistent
     *                        data
     * @return a component factory
     */
    void * SAL_CALL component_getFactory( const sal_Char * pImplName,
        lang::XMultiServiceFactory * pServiceManager,
        registry::XRegistryKey * pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper( pImplName, pServiceManager,
            pRegistryKey, ::scripting_runtimemgr::s_entries );
    }
}
