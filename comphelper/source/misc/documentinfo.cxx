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


#include <comphelper/documentinfo.hxx>
#include <comphelper/namedvaluecollection.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XTitle.hpp>

#include <cppuhelper/exc_hlp.hxx>

#include <osl/diagnose.h>
#include <sal/log.hxx>

namespace comphelper {


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::frame::XTitle;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::document::XDocumentPropertiesSupplier;
    using ::com::sun::star::document::XDocumentProperties;
    using ::com::sun::star::frame::XStorable;
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
            // 1. ask the model and the controller for their XTitle::getTitle
            sTitle = lcl_getTitle( _rxDocument );
            if ( !sTitle.isEmpty() )
                return sTitle;

            Reference< XController > xController( _rxDocument->getCurrentController() );
            sTitle = lcl_getTitle( xController );
            if ( !sTitle.isEmpty() )
                return sTitle;

            // work around a problem with embedded objects, which sometimes return
            // private:object as URL
            sDocURL = _rxDocument->getURL();
            if ( sDocURL.startsWithIgnoreAsciiCase( "private:" ) )
                sDocURL.clear();

            // 2. if the document is not saved, yet, check the frame title
            if ( sDocURL.isEmpty() )
            {
                Reference< XFrame > xFrame;
                if ( xController.is() )
                    xFrame.set( xController->getFrame() );
                sTitle = lcl_getTitle( xFrame );
                if ( !sTitle.isEmpty() )
                    return sTitle;
            }

            // 3. try the UNO XDocumentProperties
            Reference< XDocumentPropertiesSupplier > xDPS( _rxDocument, UNO_QUERY );
            if ( xDPS.is() )
            {
                Reference< XDocumentProperties > xDocProps (
                    xDPS->getDocumentProperties(), UNO_QUERY_THROW );
                sTitle = xDocProps->getTitle();
                if ( !sTitle.isEmpty() )
                    return sTitle;
            }

            // 4. try model arguments
            NamedValueCollection aModelArgs( _rxDocument->getArgs() );
            sTitle = aModelArgs.getOrDefault( "Title", sTitle );
            if ( !sTitle.isEmpty() )
                return sTitle;

            // 5. try the last segment of the document URL
            // this formerly was an INetURLObject::getName( LAST_SEGMENT, true, DecodeMechanism::WithCharset ),
            // but since we moved this code to comphelper, we do not have access to an INetURLObject anymore
            // This heuristics here should be sufficient - finally, we will get an UNO title API in a not
            // too distant future (hopefully), then  this complete class is superfluous)
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

            // 5.
            // <-- #i88104# (05-16-08) TKR: use the new XTitle Interface to get the Title -->

            Reference< XTitle > xTitle( _rxDocument, UNO_QUERY );
            if ( xTitle.is() )
            {
                if ( !xTitle->getTitle().isEmpty() )
                    return xTitle->getTitle();
            }
        }
        catch ( const Exception& )
        {
            // Cannot use tools::exceptionToString here, because the tools module depends on the comphelper module
            css::uno::Any caught( ::cppu::getCaughtException() );
            css::uno::Exception exception;
            caught >>= exception;
            SAL_WARN( "comphelper", "caught an exception!\ntype   : " << caught.getValueTypeName()
                                    << "\nmessage: " << exception
                                    << "\nin function:\n" << OSL_THIS_FUNC);
        }

        return sTitle;
    }


} // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
