/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <svtools/imageresourceaccess.hxx>

#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <cppuhelper/implbase2.hxx>
#include <comphelper/processfactory.hxx>

//........................................................................
namespace svt
{
//........................................................................

    using namespace ::utl;
    using namespace ::comphelper;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::graphic;

    //====================================================================
    //= StreamSupplier
    //====================================================================
    typedef ::cppu::WeakImplHelper2 <   XStream
                                    ,   XSeekable
                                    >   StreamSupplier_Base;
    class StreamSupplier : public StreamSupplier_Base
    {
    private:
        Reference< XInputStream >   m_xInput;
        Reference< XOutputStream >  m_xOutput;
        Reference< XSeekable >      m_xSeekable;

    public:
        StreamSupplier( const Reference< XInputStream >& _rxInput, const Reference< XOutputStream >& _rxOutput );

    protected:
        // XStream
        virtual Reference< XInputStream > SAL_CALL getInputStream(  ) throw (RuntimeException);
        virtual Reference< XOutputStream > SAL_CALL getOutputStream(  ) throw (RuntimeException);

        // XSeekable
        virtual void SAL_CALL seek( ::sal_Int64 location ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int64 SAL_CALL getPosition(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int64 SAL_CALL getLength(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    };

    //--------------------------------------------------------------------
    StreamSupplier::StreamSupplier( const Reference< XInputStream >& _rxInput, const Reference< XOutputStream >& _rxOutput )
        :m_xInput( _rxInput )
        ,m_xOutput( _rxOutput )
    {
        m_xSeekable = m_xSeekable.query( m_xInput );
        if ( !m_xSeekable.is() )
            m_xSeekable = m_xSeekable.query( m_xOutput );
        OSL_ENSURE( m_xSeekable.is(), "StreamSupplier::StreamSupplier: at least one of both must be seekable!" );
    }

    //--------------------------------------------------------------------
    Reference< XInputStream > SAL_CALL StreamSupplier::getInputStream(  ) throw (RuntimeException)
    {
        return m_xInput;
    }

    //--------------------------------------------------------------------
    Reference< XOutputStream > SAL_CALL StreamSupplier::getOutputStream(  ) throw (RuntimeException)
    {
        return m_xOutput;
    }

    //--------------------------------------------------------------------
    void SAL_CALL StreamSupplier::seek( ::sal_Int64 location ) throw (IllegalArgumentException, IOException, RuntimeException)
    {
        if ( !m_xSeekable.is() )
            throw NotConnectedException();

        m_xSeekable->seek( location );
    }

    //--------------------------------------------------------------------
    ::sal_Int64 SAL_CALL StreamSupplier::getPosition(  ) throw (IOException, RuntimeException)
    {
        if ( !m_xSeekable.is() )
            throw NotConnectedException();

        return m_xSeekable->getPosition();
    }

    //--------------------------------------------------------------------
    ::sal_Int64 SAL_CALL StreamSupplier::getLength(  ) throw (IOException, RuntimeException)
    {
        if ( !m_xSeekable.is() )
            throw NotConnectedException();

        return m_xSeekable->getLength();
    }

    //====================================================================
    //= GraphicAccess
    //====================================================================
    //--------------------------------------------------------------------
    bool GraphicAccess::isSupportedURL( const ::rtl::OUString& _rURL )
    {
        if  (   ( _rURL.indexOfAsciiL( RTL_CONSTASCII_STRINGPARAM( "private:resource/" ) ) == 0 )
            ||  ( _rURL.indexOfAsciiL( RTL_CONSTASCII_STRINGPARAM( "private:graphicrepository/" ) ) == 0 )
            ||  ( _rURL.indexOfAsciiL( RTL_CONSTASCII_STRINGPARAM( "private:standardimage/" ) ) == 0 )
            ||  ( _rURL.indexOfAsciiL( RTL_CONSTASCII_STRINGPARAM( "vnd.sun.star.GraphicObject:" ) ) == 0 )
            ||  ( _rURL.indexOfAsciiL( RTL_CONSTASCII_STRINGPARAM( "vnd.sun.star.extension://" ) ) == 0 )
            )
            return true;
        return false;
    }

    //--------------------------------------------------------------------
    SvStream* GraphicAccess::getImageStream( const Reference< XMultiServiceFactory >& _rxORB, const ::rtl::OUString& _rImageResourceURL )
    {
        SvStream* pReturn = NULL;

        try
        {
            // get a GraphicProvider
            Reference< XGraphicProvider > xProvider = ::com::sun::star::graphic::GraphicProvider::create(comphelper::getComponentContext(_rxORB));

            // let it create a graphic from the given URL
            Sequence< PropertyValue > aMediaProperties( 1 );
            aMediaProperties[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
            aMediaProperties[0].Value <<= _rImageResourceURL;
            Reference< XGraphic > xGraphic( xProvider->queryGraphic( aMediaProperties ) );
            OSL_ENSURE( xGraphic.is(), "GraphicAccess::getImageStream: the provider did not give us a graphic object!" );
            if ( !xGraphic.is() )
                return pReturn;

            // copy the graphic to a in-memory buffer
            SvMemoryStream* pMemBuffer = new SvMemoryStream;
            Reference< XStream > xBufferAccess = new StreamSupplier(
                new OSeekableInputStreamWrapper( *pMemBuffer ),
                new OSeekableOutputStreamWrapper( *pMemBuffer )
            );

            aMediaProperties.realloc( 2 );
            aMediaProperties[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OutputStream" ) );
            aMediaProperties[0].Value <<= xBufferAccess;
            aMediaProperties[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MimeType" ) );
            aMediaProperties[1].Value <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "image/png" ) );
            xProvider->storeGraphic( xGraphic, aMediaProperties );

            pMemBuffer->Seek( 0 );
            pReturn = pMemBuffer;
        }
        catch( const Exception& )
        {
            OSL_FAIL( "GraphicAccess::getImageStream: caught an exception!" );
        }

        return pReturn;
    }

    //--------------------------------------------------------------------
    Reference< XInputStream > GraphicAccess::getImageXStream( const Reference< XMultiServiceFactory >& _rxORB, const ::rtl::OUString& _rImageResourceURL )
    {
        return new OSeekableInputStreamWrapper( getImageStream( _rxORB, _rImageResourceURL ), sal_True );   // take ownership
    }

//........................................................................
} // namespace svt
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
