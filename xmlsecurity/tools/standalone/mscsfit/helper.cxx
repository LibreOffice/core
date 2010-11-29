/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include "helper.hxx"
#include "osl/diagnose.h"
#include "rtl/ustring.h"

/*-
 * Helper : create a input stream from a file
 */
Reference< XInputStream > createStreamFromFile( const OUString sFile )
{
    const sal_Char* pcFile ;
    OString aString ;

    aString = OUStringToOString( sFile , RTL_TEXTENCODING_ASCII_US ) ;
    pcFile = aString.getStr() ;
    if( pcFile != NULL ) {
        FILE *f = fopen( pcFile , "rb" );
        Reference<  XInputStream >  r;

        if( f ) {
            fseek( f , 0 , SEEK_END );
            int nLength = ftell( f );
            fseek( f , 0 , SEEK_SET );

            Sequence<sal_Int8> seqIn(nLength);
            fread( seqIn.getArray() , nLength , 1 , f );

            r = Reference< XInputStream > ( new OInputStream( seqIn ) );
            fclose( f );
        }
        return r;
    } else {
        return NULL ;
    }

    return NULL ;
}

/*-
 * Helper : set a output stream to a file
 */
Reference< XOutputStream > createStreamToFile( const OUString sFile )
{
    const sal_Char* pcFile ;
    OString aString ;

    aString = OUStringToOString( sFile , RTL_TEXTENCODING_ASCII_US ) ;
    pcFile = aString.getStr() ;
    if( pcFile != NULL )
        return Reference< XOutputStream >( new OOutputStream( pcFile ) ) ;
    else
        return NULL ;
}

/*-
 * Helper : get service manager and context
 */
Reference< XMultiComponentFactory > serviceManager( Reference< XComponentContext >& xContext , OUString sUnoUrl , OUString sRdbUrl ) throw( RuntimeException , Exception )
{
    Reference< XMultiComponentFactory > xLocalServiceManager = NULL ;
    Reference< XComponentContext > xLocalComponentContext = NULL ;
    Reference< XMultiComponentFactory > xUsedServiceManager = NULL ;
    Reference< XComponentContext > xUsedComponentContext = NULL ;

    OSL_ENSURE( !sUnoUrl.equalsAscii( "" ) ,
        "serviceManager - "
        "No uno URI specified" ) ;

    OSL_ENSURE( !sRdbUrl.equalsAscii( "" ) ,
        "serviceManager - "
        "No rdb URI specified" ) ;

    if( sUnoUrl.equalsAscii( "local" ) ) {
        Reference< XSimpleRegistry > xSimpleRegistry = createSimpleRegistry();
        OSL_ENSURE( xSimpleRegistry.is() ,
            "serviceManager - "
            "Cannot create simple registry" ) ;

        //xSimpleRegistry->open(OUString(RTL_CONSTASCII_USTRINGPARAM("xmlsecurity.rdb")), sal_False, sal_False);
        xSimpleRegistry->open(sRdbUrl, sal_True, sal_False);
        OSL_ENSURE( xSimpleRegistry->isValid() ,
            "serviceManager - "
            "Cannot open xml security registry rdb" ) ;

        xLocalComponentContext = bootstrap_InitialComponentContext( xSimpleRegistry ) ;
        OSL_ENSURE( xLocalComponentContext.is() ,
            "serviceManager - "
            "Cannot create intial component context" ) ;

        xLocalServiceManager = xLocalComponentContext->getServiceManager() ;
        OSL_ENSURE( xLocalServiceManager.is() ,
            "serviceManager - "
            "Cannot create intial service manager" ) ;

        /*-
         * Because of the exception rasied from
         * ucbhelper/source/provider/provconf.cxx, lin 323
         * I do not use the content broker at present
         ********************************************************************
        //init ucb
        if( ::ucb::ContentBroker::get() == NULL ) {
            Reference< lang::XMultiServiceFactory > xSvmg( xLocalServiceManager , UNO_QUERY ) ;
            OSL_ENSURE( xLocalServiceManager.is() ,
                "serviceManager - "
                "Cannot get multi-service factory" ) ;

            Sequence< Any > args( 2 ) ;
            args[ 0 ] <<= OUString(RTL_CONSTASCII_USTRINGPARAM( UCB_CONFIGURATION_KEY1_LOCAL )) ;
            args[ 1 ] <<= OUString(RTL_CONSTASCII_USTRINGPARAM( UCB_CONFIGURATION_KEY2_OFFICE )) ;
            if( ! ::ucb::ContentBroker::initialize( xSvmg , args ) ) {
                throw RuntimeException( OUString(RTL_CONSTASCII_USTRINGPARAM("Cannot inlitialize ContentBroker")) , Reference< XInterface >() , Any() ) ;
            }
        }
        ********************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

        xUsedComponentContext = xLocalComponentContext ;
        xUsedServiceManager = xLocalServiceManager ;
    } else {
        Reference< XComponentContext > xLocalComponentContext = defaultBootstrap_InitialComponentContext() ;
        OSL_ENSURE( xLocalComponentContext.is() ,
            "serviceManager - "
            "Cannot create intial component context" ) ;

        Reference< XMultiComponentFactory > xLocalServiceManager = xLocalComponentContext->getServiceManager();
        OSL_ENSURE( xLocalServiceManager.is() ,
            "serviceManager - "
            "Cannot create intial service manager" ) ;

        Reference< XInterface > urlResolver =
            xLocalServiceManager->createInstanceWithContext( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.UnoUrlResolver")) , xLocalComponentContext ) ;
        OSL_ENSURE( urlResolver.is() ,
            "serviceManager - "
            "Cannot get service instance of \"bridge.UnoUrlResolver\"" ) ;

        Reference< XUnoUrlResolver > xUnoUrlResolver( urlResolver , UNO_QUERY ) ;
        OSL_ENSURE( xUnoUrlResolver.is() ,
            "serviceManager - "
            "Cannot get interface of \"XUnoUrlResolver\" from service \"bridge.UnoUrlResolver\"" ) ;

        Reference< XInterface > initialObject = xUnoUrlResolver->resolve( sUnoUrl ) ;
        OSL_ENSURE( initialObject.is() ,
            "serviceManager - "
            "Cannot resolve uno url" ) ;

        /*-
         * Method 1: with Naming Service
         ********************************************************************
        Reference< XNamingService > xNamingService( initialObject , UNO_QUERY ) ;
        OSL_ENSURE( xNamingService.is() ,
            "serviceManager - "
            "Cannot get interface of \"XNamingService\" from URL resolver" ) ;

        Reference< XInterface > serviceManager =
            xNamingService->getRegisteredObject( OUString(RTL_CONSTASCII_USTRINGPARAM("StarOffice.ServiceManager")) ) ;
        OSL_ENSURE( serviceManager.is() ,
            "serviceManager - "
            "Cannot get service instance of \"StarOffice.ServiceManager\"" ) ;

        xUsedServiceManager = Reference< XMultiComponentFactory >( serviceManager , UNO_QUERY );
        OSL_ENSURE( xUsedServiceManager.is() ,
            "serviceManager - "
            "Cannot get interface of \"XMultiComponentFactory\" from service \"StarOffice.ServiceManager\"" ) ;

        Reference< XPropertySet > xPropSet( xUsedServiceManager , UNO_QUERY ) ;
        OSL_ENSURE( xPropSet.is() ,
            "serviceManager - "
            "Cannot get interface of \"XPropertySet\" from service \"StarOffice.ServiceManager\"" ) ;

        xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xUsedComponentContext ;
        OSL_ENSURE( xUsedComponentContext.is() ,
            "serviceManager - "
            "Cannot create remote component context" ) ;

        ********************************************************************/

        /*-
         * Method 2: with Componnent context
         ********************************************************************
        Reference< XPropertySet > xPropSet( initialObject , UNO_QUERY ) ;
        OSL_ENSURE( xPropSet.is() ,
            "serviceManager - "
            "Cannot get interface of \"XPropertySet\" from URL resolver" ) ;

        xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xUsedComponentContext ;
        OSL_ENSURE( xUsedComponentContext.is() ,
            "serviceManager - "
            "Cannot create remote component context" ) ;

        xUsedServiceManager = xUsedComponentContext->getServiceManager();
        OSL_ENSURE( xUsedServiceManager.is() ,
            "serviceManager - "
            "Cannot create remote service manager" ) ;
        ********************************************************************/

        /*-
         * Method 3: with Service Manager
         ********************************************************************/
        xUsedServiceManager = Reference< XMultiComponentFactory >( initialObject , UNO_QUERY );
        OSL_ENSURE( xUsedServiceManager.is() ,
            "serviceManager - "
            "Cannot create remote service manager" ) ;

        Reference< XPropertySet > xPropSet( xUsedServiceManager , UNO_QUERY ) ;
        OSL_ENSURE( xPropSet.is() ,
            "serviceManager - "
            "Cannot get interface of \"XPropertySet\" from service \"StarOffice.ServiceManager\"" ) ;

        xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xUsedComponentContext ;
        OSL_ENSURE( xUsedComponentContext.is() ,
            "serviceManager - "
            "Cannot create remote component context" ) ;
        /********************************************************************/
    }

    xContext = xUsedComponentContext ;
    return xUsedServiceManager ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
