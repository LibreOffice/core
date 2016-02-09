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

#include "osl/security.hxx"
#include "acceptor.hxx"
#include <com/sun/star/connection/ConnectionSetupException.hpp>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase.hxx>

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::connection;
using namespace ::com::sun::star::io;


namespace io_acceptor
{

    typedef WeakImplHelper< XConnection > MyPipeConnection;

    class PipeConnection :
        public MyPipeConnection
    {
    public:
        explicit PipeConnection( const OUString &sConnectionDescription);
        virtual ~PipeConnection();

        virtual sal_Int32 SAL_CALL read( Sequence< sal_Int8 >& aReadBytes, sal_Int32 nBytesToRead )
            throw(css::io::IOException,
                  css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL write( const Sequence< sal_Int8 >& aData )
            throw(css::io::IOException,
                  css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL flush(  ) throw(
            css::io::IOException,
            css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL close(  )
            throw(css::io::IOException,
                  css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getDescription(  )
            throw(css::uno::RuntimeException, std::exception) override;
    public:
        ::osl::StreamPipe m_pipe;
        oslInterlockedCount m_nStatus;
        OUString m_sDescription;
    };


    PipeConnection::PipeConnection( const OUString &sConnectionDescription) :
        m_nStatus( 0 ),
        m_sDescription( sConnectionDescription )
    {
        // make it unique
        m_sDescription += ",uniqueValue=";
        m_sDescription += OUString::number(
            sal::static_int_cast<sal_Int64 >(
                reinterpret_cast< sal_IntPtr >(&m_pipe)) );
    }

    PipeConnection::~PipeConnection()
    {
    }

    sal_Int32 PipeConnection::read( Sequence < sal_Int8 > & aReadBytes , sal_Int32 nBytesToRead )
        throw(css::io::IOException,
              css::uno::RuntimeException, std::exception)
    {
        if( ! m_nStatus )
        {
            if( aReadBytes.getLength() < nBytesToRead )
            {
                aReadBytes.realloc( nBytesToRead );
            }
            sal_Int32 n = m_pipe.read( aReadBytes.getArray(), nBytesToRead );
            OSL_ASSERT( n >= 0 && n <= aReadBytes.getLength() );
            if( n < aReadBytes.getLength() )
            {
                aReadBytes.realloc( n );
            }
            return n;
        }
        else {
            throw IOException();
        }
    }

    void PipeConnection::write( const Sequence < sal_Int8 > &seq )
            throw(css::io::IOException,
                  css::uno::RuntimeException, std::exception)
    {
        if( ! m_nStatus )
        {
            if( m_pipe.write( seq.getConstArray() , seq.getLength() ) != seq.getLength() )
            {
                throw IOException();
            }
        }
        else {
            throw IOException();
        }
    }

    void PipeConnection::flush( )
        throw(  css::io::IOException,
                css::uno::RuntimeException, std::exception)
    {
    }

    void PipeConnection::close()
        throw( css::io::IOException,
               css::uno::RuntimeException, std::exception)
    {
        if(  1 == osl_atomic_increment( (&m_nStatus) ) )
        {
            m_pipe.close();
        }
    }

    OUString PipeConnection::getDescription()
            throw(css::uno::RuntimeException, std::exception)
    {
        return m_sDescription;
    }

    /***************
     * PipeAcceptor
     **************/
    PipeAcceptor::PipeAcceptor( const OUString &sPipeName , const OUString & sConnectionDescription) :
        m_sPipeName( sPipeName ),
        m_sConnectionDescription( sConnectionDescription ),
        m_bClosed( false )
    {
    }


    void PipeAcceptor::init()
    {
        m_pipe = Pipe( m_sPipeName.pData , osl_Pipe_CREATE , osl::Security() );
        if( ! m_pipe.is() )
        {
            OUString error = "io.acceptor: Couldn't setup pipe " + m_sPipeName;
            throw ConnectionSetupException( error );
        }
    }

    Reference< XConnection > PipeAcceptor::accept( )
    {
        Pipe pipe;
        {
            MutexGuard guard( m_mutex );
            pipe = m_pipe;
        }
        if( ! pipe.is() )
        {
            OUString error = "io.acceptor: pipe already closed" + m_sPipeName;
            throw ConnectionSetupException( error );
        }
        PipeConnection *pConn = new PipeConnection( m_sConnectionDescription );

        oslPipeError status = pipe.accept( pConn->m_pipe );

        if( m_bClosed )
        {
            // stopAccepting was called !
            delete pConn;
            return Reference < XConnection >();
        }
        else if( osl_Pipe_E_None == status )
        {
            return Reference < XConnection > ( static_cast<XConnection *>(pConn) );
        }
        else
        {
            OUString error = "io.acceptor: Couldn't setup pipe " + m_sPipeName;
            throw ConnectionSetupException( error );
        }
    }

    void PipeAcceptor::stopAccepting()
    {
        m_bClosed = true;
        Pipe pipe;
        {
            MutexGuard guard( m_mutex );
            pipe = m_pipe;
            m_pipe.clear();
        }
        if( pipe.is() )
        {
            pipe.close();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
