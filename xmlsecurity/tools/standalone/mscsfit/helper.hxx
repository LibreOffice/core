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

#include <stdio.h>
#include <rtl/ustring.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>

#include <com/sun/star/xml/crypto/XUriBinding.hpp>
#include <com/sun/star/xml/wrapper/XXMLDocumentWrapper.hpp>
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>

#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/uno/XNamingService.hpp>

using namespace ::rtl ;
using namespace ::cppu ;
using namespace ::com::sun::star ;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::io ;
using namespace ::com::sun::star::ucb ;
using namespace ::com::sun::star::beans ;
using namespace ::com::sun::star::document ;
using namespace ::com::sun::star::lang ;
using namespace ::com::sun::star::bridge ;
using namespace ::com::sun::star::registry ;
using namespace ::com::sun::star::task ;
using namespace ::com::sun::star::xml ;
using namespace ::com::sun::star::xml::wrapper ;
using namespace ::com::sun::star::xml::sax ;


/**
 * Helper: Implementation of XInputStream
 */
class OInputStream : public WeakImplHelper1 < XInputStream >
{
    public:
        OInputStream( const Sequence< sal_Int8 >&seq ) : m_seq( seq ), nPos( 0 ) {}

        virtual sal_Int32 SAL_CALL readBytes(
            Sequence< sal_Int8 >& aData ,
            sal_Int32 nBytesToRead
        ) throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
        {
            nBytesToRead = ( nBytesToRead > m_seq.getLength() - nPos ) ?
                m_seq.getLength() - nPos :
                nBytesToRead ;
            aData = Sequence< sal_Int8 > ( &( m_seq.getConstArray()[nPos] ), nBytesToRead ) ;
            nPos += nBytesToRead ;
            return nBytesToRead ;
        }

        virtual sal_Int32 SAL_CALL readSomeBytes(
            ::com::sun::star::uno::Sequence< sal_Int8 >& aData ,
            sal_Int32 nMaxBytesToRead
        ) throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
        {
            return readBytes( aData, nMaxBytesToRead ) ;
        }

        virtual void SAL_CALL skipBytes(
            sal_Int32 nBytesToSkip
        ) throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
        {
            // not implemented
        }

        virtual sal_Int32 SAL_CALL available(
            void
        ) throw( NotConnectedException, IOException, RuntimeException )
        {
            return m_seq.getLength() - nPos ;
        }

        virtual void SAL_CALL closeInput(
            void
        ) throw( NotConnectedException, IOException, RuntimeException )
        {
            // not needed
        }

    private:
        sal_Int32 nPos;
        Sequence< sal_Int8> m_seq;
} ;

/**
 * Helper : create a input stream from a file
 */
Reference< XInputStream > createStreamFromFile( const OUString sFile ) ;

/**
 * Helper: Implementation of XOutputStream
 */
class OOutputStream : public WeakImplHelper1 < XOutputStream >
{
    public:
        OOutputStream( const char *pcFile ) {
            strcpy( m_pcFile , pcFile ) ;
            m_f = 0 ;
        }

        virtual void SAL_CALL writeBytes(
            const Sequence< sal_Int8 >& aData
        ) throw( NotConnectedException , BufferSizeExceededException , RuntimeException ) {
            if( !m_f ) {
                m_f = fopen( m_pcFile , "w" ) ;
            }

            fwrite( aData.getConstArray() , 1 , aData.getLength() , m_f ) ;
        }

        virtual void SAL_CALL flush(
            void
        ) throw( NotConnectedException , BufferSizeExceededException , RuntimeException ) {
            fflush( m_f ) ;
        }

        virtual void SAL_CALL closeOutput(
            void
        ) throw( NotConnectedException , BufferSizeExceededException , RuntimeException ) {
            fclose( m_f ) ;
            m_f = 0 ;
        }

    private:
        char m_pcFile[256];
        FILE *m_f;
} ;

/**
 * Helper: Implementation of XUriBinding
 */
class OUriBinding : public WeakImplHelper1 < ::com::sun::star::xml::crypto::XUriBinding >
{
    public:
        OUriBinding() {
            //Do nothing
        }

        OUriBinding(
            ::rtl::OUString& aUri,
            ::com::sun::star::uno::Reference< com::sun::star::io::XInputStream >& aInputStream ) {
            m_vUris.push_back( aUri ) ;
            m_vStreams.push_back( aInputStream ) ;
        }

        virtual void SAL_CALL setUriBinding(
            const ::rtl::OUString& aUri ,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& aInputStream
        ) throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException ) {
            m_vUris.push_back( aUri ) ;
            m_vStreams.push_back( aInputStream ) ;
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getUriBinding( const ::rtl::OUString& uri ) throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException ) {
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInputStream ;

            int size = m_vUris.size() ;
            for( int i = 0 ; i<size ; ++i ) {
                if( uri == m_vUris[i] ) {
                    xInputStream = m_vStreams[i];
                    break;
                }
            }

            return xInputStream;
        }

    private:
        std::vector< ::rtl::OUString > m_vUris ;
        std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > > m_vStreams ;
} ;

/**
 * Helper : set a output stream to a file
 */
Reference< XOutputStream > createStreamToFile( const OUString sFile ) ;

/**
 * Helper : get service manager and context
 */
Reference< XMultiComponentFactory > serviceManager( Reference< XComponentContext >& xContext , OUString sUnoUrl , OUString sRdbUrl  ) throw( RuntimeException , Exception ) ;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
