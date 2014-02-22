/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "comphelper/documentinfo.hxx"
#include "comphelper/namedvaluecollection.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XTitle.hpp>

#include <cppuhelper/exc_hlp.hxx>

#include <osl/diagnose.h>
#include <osl/thread.h>

#include <boost/current_function.hpp>


namespace comphelper {


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::frame::XTitle;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::document::XDocumentPropertiesSupplier;
    using ::com::sun::star::document::XDocumentProperties;
    using ::com::sun::star::frame::XStorable;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::frame::XFrame;

    
    
    
    namespace
    {
        OUString lcl_getTitle( const Reference< XInterface >& _rxComponent )
        {
            Reference< XTitle > xTitle( _rxComponent, UNO_QUERY );
            if ( xTitle.is() )
                return xTitle->getTitle();
            return OUString();
        }
    }

    
    
    
    
    OUString DocumentInfo::getDocumentTitle( const Reference< XModel >& _rxDocument )
    {
        OUString sTitle;

        if ( !_rxDocument.is() )
            return sTitle;

        OUString sDocURL;
        try
        {
            
            sTitle = lcl_getTitle( _rxDocument );
            if ( !sTitle.isEmpty() )
                return sTitle;

            Reference< XController > xController( _rxDocument->getCurrentController() );
            sTitle = lcl_getTitle( xController );
            if ( !sTitle.isEmpty() )
                return sTitle;

            
            
            sDocURL = _rxDocument->getURL();
            if ( sDocURL.matchAsciiL( "private:", 8 ) )
                sDocURL = OUString();

            
            if ( sDocURL.isEmpty() )
            {
                Reference< XFrame > xFrame;
                if ( xController.is() )
                    xFrame.set( xController->getFrame() );
                sTitle = lcl_getTitle( xFrame );
                if ( !sTitle.isEmpty() )
                    return sTitle;
            }

            
            Reference< XDocumentPropertiesSupplier > xDPS( _rxDocument, UNO_QUERY );
            if ( xDPS.is() )
            {
                Reference< XDocumentProperties > xDocProps (
                    xDPS->getDocumentProperties(), UNO_QUERY_THROW );
                OSL_ENSURE(xDocProps.is(), "no DocumentProperties");
                sTitle = xDocProps->getTitle();
                if ( !sTitle.isEmpty() )
                    return sTitle;
            }

            
            NamedValueCollection aModelArgs( _rxDocument->getArgs() );
            sTitle = aModelArgs.getOrDefault( "Title", sTitle );
            if ( !sTitle.isEmpty() )
                return sTitle;

            
            
            
            
            
            if ( sDocURL.isEmpty() )
            {
                Reference< XStorable > xDocStorable( _rxDocument, UNO_QUERY_THROW );
                sDocURL = xDocStorable->getLocation();
            }
            sal_Int32 nLastSepPos = sDocURL.lastIndexOf( '/' );
            if ( ( nLastSepPos != -1 ) && ( nLastSepPos == sDocURL.getLength() - 1 ) )
            {
                sDocURL = sDocURL.copy( 0, nLastSepPos );
                nLastSepPos = sDocURL.lastIndexOf( '/' );
            }
            sTitle = sDocURL.copy( nLastSepPos + 1 );

            if ( !sTitle.isEmpty() )
                return sTitle;

            
            

            Reference< XTitle > xTitle( _rxDocument, UNO_QUERY );
            if ( xTitle.is() )
            {
                if ( !xTitle->getTitle().isEmpty() )
                    return xTitle->getTitle();
            }
        }
        catch ( const Exception& )
        {
            ::com::sun::star::uno::Any caught( ::cppu::getCaughtException() );
            OString sMessage( "caught an exception!" );
            sMessage += "\ntype   : ";
            sMessage += OString( caught.getValueTypeName().getStr(), caught.getValueTypeName().getLength(), osl_getThreadTextEncoding() );
            sMessage += "\nmessage: ";
            ::com::sun::star::uno::Exception exception;
            caught >>= exception;
            sMessage += OString( exception.Message.getStr(), exception.Message.getLength(), osl_getThreadTextEncoding() );
            sMessage += "\nin function:\n";
            sMessage += BOOST_CURRENT_FUNCTION;
            sMessage += "\n";
            OSL_FAIL( sMessage.getStr() );
        }

        return sTitle;
    }


} 


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
