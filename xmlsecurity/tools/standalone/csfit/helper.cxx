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

    OSL_ENSURE( sUnoUrl.getLength() ,
        "serviceManager - "
        "No uno URI specified" ) ;

    OSL_ENSURE( sRdbUrl.getLength() ,
        "serviceManager - "
        "No rdb URI specified" ) ;

    if ( sUnoUrl == "local" ) {
        Reference< XSimpleRegistry > xSimpleRegistry = createSimpleRegistry();
        OSL_ENSURE( xSimpleRegistry.is() ,
            "serviceManager - "
            "Cannot create simple registry" ) ;

        //xSimpleRegistry->open(OUString("xmlsecurity.rdb"), sal_False, sal_False);
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
            xLocalServiceManager->createInstanceWithContext( OUString("com.sun.star.bridge.UnoUrlResolver") , xLocalComponentContext ) ;
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
            xNamingService->getRegisteredObject( OUString("StarOffice.ServiceManager") ) ;
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

        xPropSet->getPropertyValue( OUString("DefaultContext") ) >>= xUsedComponentContext ;
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

        xPropSet->getPropertyValue( OUString("DefaultContext") ) >>= xUsedComponentContext ;
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

        xPropSet->getPropertyValue( OUString("DefaultContext") ) >>= xUsedComponentContext ;
        OSL_ENSURE( xUsedComponentContext.is() ,
            "serviceManager - "
            "Cannot create remote component context" ) ;
        /********************************************************************/
    }

    xContext = xUsedComponentContext ;
    return xUsedServiceManager ;
}

char* PriPK11PasswordFunc(
    PK11SlotInfo *slot ,
    PRBool retry ,
    void* arg
) {
    char* passwd = NULL ;

    if( retry != PR_TRUE ) {
        passwd = ( char* )PORT_Alloc( 20 ) ;
        printf( "Input Password:\n" ) ;
        scanf( "%s" , passwd ) ;
        printf( "The passwod is [%s]\n" , passwd ) ;
    }

    return passwd ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
