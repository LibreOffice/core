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

// MARKER(update_precomp.py): autogen include statement, do not remove


#ifndef SVTOOLS_SOURCE_MISC_IMAGERESOURCEACCESS_HXX
#include "imageresourceaccess.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_IO_NOTCONNECTEDEXCEPTION_HPP_
#include <com/sun/star/io/NotConnectedException.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHICPROVIDER_HPP_
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
/** === end UNO includes === **/

#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

//........................................................................
namespace binfilter
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
    //= ImageResourceAccess
    //====================================================================
    //--------------------------------------------------------------------
    bool ImageResourceAccess::isImageResourceURL( const ::rtl::OUString& _rURL )
    {
        ::rtl::OUString sIndicator( RTL_CONSTASCII_USTRINGPARAM( "private:resource/" ) );
        return _rURL.indexOf( sIndicator ) == 0;
    }

    //--------------------------------------------------------------------
    SvStream* ImageResourceAccess::getImageStream( const Reference< XMultiServiceFactory >& _rxORB, const ::rtl::OUString& _rImageResourceURL )
    {
        SvStream* pReturn = NULL;

        try
        {
            // get a GraphicProvider
            Reference< XGraphicProvider > xProvider;
            if ( _rxORB.is() )
                xProvider = xProvider.query( _rxORB->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.graphic.GraphicProvider" ) ) ) );
            OSL_ENSURE( xProvider.is(), "ImageResourceAccess::getImageStream: could not create a graphic provider!" );

            if ( !xProvider.is() )
                return pReturn;

            // let it create a graphic from the given URL
            Sequence< PropertyValue > aMediaProperties( 1 );
            aMediaProperties[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
            aMediaProperties[0].Value <<= _rImageResourceURL;
            Reference< XGraphic > xGraphic( xProvider->queryGraphic( aMediaProperties ) );
            OSL_ENSURE( xGraphic.is(), "ImageResourceAccess::getImageStream: the provider did not give us a graphic object!" );
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
            OSL_ENSURE( sal_False, "ImageResourceAccess::getImageStream: caught an exception!" );
        }

        return pReturn;
    }

}
//........................................................................

