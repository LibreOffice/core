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


#include <svtools/imageresourceaccess.hxx>

#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <osl/diagnose.h>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/processfactory.hxx>


namespace svt
{


    using namespace ::utl;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::graphic;


    //= StreamSupplier

    typedef ::cppu::WeakImplHelper <   XStream
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
        virtual Reference< XInputStream > SAL_CALL getInputStream(  ) throw (RuntimeException, std::exception) override;
        virtual Reference< XOutputStream > SAL_CALL getOutputStream(  ) throw (RuntimeException, std::exception) override;

        // XSeekable
        virtual void SAL_CALL seek( ::sal_Int64 location ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int64 SAL_CALL getPosition(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int64 SAL_CALL getLength(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    };


    StreamSupplier::StreamSupplier( const Reference< XInputStream >& _rxInput, const Reference< XOutputStream >& _rxOutput )
        :m_xInput( _rxInput )
        ,m_xOutput( _rxOutput )
    {
        m_xSeekable.set(m_xInput, css::uno::UNO_QUERY);
        if ( !m_xSeekable.is() )
            m_xSeekable.set(m_xOutput, css::uno::UNO_QUERY);
        OSL_ENSURE( m_xSeekable.is(), "StreamSupplier::StreamSupplier: at least one of both must be seekable!" );
    }


    Reference< XInputStream > SAL_CALL StreamSupplier::getInputStream(  ) throw (RuntimeException, std::exception)
    {
        return m_xInput;
    }


    Reference< XOutputStream > SAL_CALL StreamSupplier::getOutputStream(  ) throw (RuntimeException, std::exception)
    {
        return m_xOutput;
    }


    void SAL_CALL StreamSupplier::seek( ::sal_Int64 location ) throw (IllegalArgumentException, IOException, RuntimeException, std::exception)
    {
        if ( !m_xSeekable.is() )
            throw NotConnectedException();

        m_xSeekable->seek( location );
    }


    ::sal_Int64 SAL_CALL StreamSupplier::getPosition(  ) throw (IOException, RuntimeException, std::exception)
    {
        if ( !m_xSeekable.is() )
            throw NotConnectedException();

        return m_xSeekable->getPosition();
    }


    ::sal_Int64 SAL_CALL StreamSupplier::getLength(  ) throw (IOException, RuntimeException, std::exception)
    {
        if ( !m_xSeekable.is() )
            throw NotConnectedException();

        return m_xSeekable->getLength();
    }


    //= GraphicAccess


    bool GraphicAccess::isSupportedURL( const OUString& _rURL )
    {
        if  (  _rURL.startsWith( "private:resource/" )
            || _rURL.startsWith( "private:graphicrepository/" )
            || _rURL.startsWith( "private:standardimage/" )
            || _rURL.startsWith( "vnd.sun.star.GraphicObject:" )
            || _rURL.startsWith( "vnd.sun.star.extension://" )
            )
            return true;
        return false;
    }


    SvStream* GraphicAccess::getImageStream( const Reference< XComponentContext >& _rxContext, const OUString& _rImageResourceURL )
    {
        SvStream* pReturn = NULL;

        try
        {
            // get a GraphicProvider
            Reference< XGraphicProvider > xProvider = ::com::sun::star::graphic::GraphicProvider::create(_rxContext);

            // let it create a graphic from the given URL
            Sequence< PropertyValue > aMediaProperties( 1 );
            aMediaProperties[0].Name = "URL";
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
            aMediaProperties[0].Name = "OutputStream";
            aMediaProperties[0].Value <<= xBufferAccess;
            aMediaProperties[1].Name = "MimeType";
            aMediaProperties[1].Value <<= OUString( "image/png" );
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


    Reference< XInputStream > GraphicAccess::getImageXStream( const Reference< XComponentContext >& _rxContext, const OUString& _rImageResourceURL )
    {
        return new OSeekableInputStreamWrapper( getImageStream( _rxContext, _rImageResourceURL ), true );   // take ownership
    }


} // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
