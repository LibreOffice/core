/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: documentinfo.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_comphelper.hxx"

#include "comphelper/documentinfo.hxx"
#include "comphelper/namedvaluecollection.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/frame/XStorable.hpp>
/** === end UNO includes === **/

#include <cppuhelper/exc_hlp.hxx>

#include <osl/diagnose.h>
#include <osl/thread.h>

#include <boost/current_function.hpp>

//........................................................................
namespace comphelper {
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::document::XDocumentPropertiesSupplier;
    using ::com::sun::star::document::XDocumentProperties;
    using ::com::sun::star::frame::XStorable;
    using ::com::sun::star::beans::XPropertySetInfo;
    /** === end UNO using === **/

    //====================================================================
    //= DocumentInfo
    //====================================================================
    //--------------------------------------------------------------------
    ::rtl::OUString DocumentInfo::getDocumentTitle( const Reference< XModel >& _rxDocument )
    {
        ::rtl::OUString sTitle;

        if ( !_rxDocument.is() )
            return sTitle;

        ::rtl::OUString sDocURL;
        try
        {
            // note that the following was roughly ripped from the former SvxScriptSelectorDialog::GetDocTitle
            // implementation. Additionally, heuristics from ucb/source/ucp/tdoc/tdoc_docmgr.cxx/getDocumentTitle
            // have been merged in.
            // I'm not really sure the steps make sense in this order ...
            const ::rtl::OUString sTitlePropName( RTL_CONSTASCII_USTRINGPARAM( "Title" ) );

            sDocURL = _rxDocument->getURL();
            if ( sDocURL.matchAsciiL( "private:", 8 ) )
                sDocURL = ::rtl::OUString();

            // 1. if the document is not saved, yet, check the frame title
            if ( sDocURL.getLength() == 0 )
            {
                Reference< XController > xCurrentController( _rxDocument->getCurrentController() );
                Reference< XPropertySet > xFrameProps;
                if ( xCurrentController.is() )
                    xFrameProps.set( xCurrentController->getFrame(), UNO_QUERY );
                Reference< XPropertySetInfo > xFramePSI;
                if ( xFrameProps.is() )
                    xFramePSI = xFrameProps->getPropertySetInfo();

                if ( xFramePSI.is() && xFramePSI->hasPropertyByName( sTitlePropName ) )
                {
                    OSL_VERIFY( xFrameProps->getPropertyValue( sTitlePropName ) >>= sTitle );
                    // process "UntitledX - YYYYYYYY" // to get UntitledX
                    sal_Int32 pos = 0;
                    sTitle = sTitle.getToken( 0, ' ', pos);
                }
            }

            if ( sTitle.getLength() != 0 )
                return sTitle;

            // 2. try the UNO DocumentInfo
            Reference< XDocumentPropertiesSupplier > xDPS( _rxDocument, UNO_QUERY );
            if ( xDPS.is() )
            {
                Reference< XDocumentProperties > xDocProps (
                    xDPS->getDocumentProperties(), UNO_QUERY_THROW );
                OSL_ENSURE(xDocProps.is(), "no DocumentProperties");
                sTitle = xDocProps->getTitle();
            }

            if ( sTitle.getLength() != 0 )
                return sTitle;

            // 3. try model arguments
            NamedValueCollection aModelArgs( _rxDocument->getArgs() );
            sTitle = aModelArgs.getOrDefault( "Title", sTitle );

            if ( sTitle.getLength() != 0 )
                return sTitle;

            // 4. try the last segment of the document URL
            // this formerly was an INetURLObject::getName( LAST_SEGMENT, true, DECODE_WITH_CHARSET ),
            // but since we moved this code to comphelper, we do not have access to an INetURLObject anymore
            // This heuristics here should be sufficient - finally, we will get an UNO title API in a not
            // too distant future (hopefully), then  this complete class is superfluous)
            if ( sDocURL.getLength() == 0 )
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
        }
        catch ( const Exception& )
        {
            ::com::sun::star::uno::Any caught( ::cppu::getCaughtException() );
            ::rtl::OString sMessage( "caught an exception!" );
            sMessage += "\ntype   : ";
            sMessage += ::rtl::OString( caught.getValueTypeName().getStr(), caught.getValueTypeName().getLength(), osl_getThreadTextEncoding() );
            sMessage += "\nmessage: ";
            ::com::sun::star::uno::Exception exception;
            caught >>= exception;
            sMessage += ::rtl::OString( exception.Message.getStr(), exception.Message.getLength(), osl_getThreadTextEncoding() );
            sMessage += "\nin function:\n";
            sMessage += BOOST_CURRENT_FUNCTION;
            sMessage += "\n";
            OSL_ENSURE( false, sMessage );
        }

        return sTitle;
    }

//........................................................................
} // namespace comphelper
//........................................................................
