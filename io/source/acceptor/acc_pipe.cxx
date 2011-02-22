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
#include "precompiled_io.hxx"
#include "osl/security.hxx"
#include "acceptor.hxx"
#include <com/sun/star/connection/ConnectionSetupException.hpp>

#include <cppuhelper/implbase1.hxx>

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::connection;
using namespace ::com::sun::star::io;


namespace io_acceptor
{

    typedef WeakImplHelper1< XConnection > MyPipeConnection;

    class PipeConnection :
        public MyPipeConnection
    {
    public:
        PipeConnection( const OUString &sConnectionDescription);
        ~PipeConnection();

        virtual sal_Int32 SAL_CALL read( Sequence< sal_Int8 >& aReadBytes, sal_Int32 nBytesToRead )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL write( const Sequence< sal_Int8 >& aData )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL flush(  ) throw(
            ::com::sun::star::io::IOException,
            ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL close(  )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getDescription(  )
            throw(::com::sun::star::uno::RuntimeException);
    public:
        ::osl::StreamPipe m_pipe;
        oslInterlockedCount m_nStatus;
        OUString m_sDescription;
    };



    PipeConnection::PipeConnection( const OUString &sConnectionDescription) :
        m_nStatus( 0 ),
        m_sDescription( sConnectionDescription )
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );

        // make it unique
        m_sDescription += OUString::createFromAscii( ",uniqueValue=" );
        m_sDescription += OUString::valueOf(
            sal::static_int_cast<sal_Int64 >(
                reinterpret_cast< sal_IntPtr >(&m_pipe)),
            10 );
    }

    PipeConnection::~PipeConnection()
    {
        g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
    }

    sal_Int32 PipeConnection::read( Sequence < sal_Int8 > & aReadBytes , sal_Int32 nBytesToRead )
        throw(::com::sun::star::io::IOException,
              ::com::sun::star::uno::RuntimeException)
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
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException)
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
        throw(  ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException)
    {
    }

    void PipeConnection::close()
        throw( ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException)
    {
        if(  1 == osl_incrementInterlockedCount( (&m_nStatus) ) )
        {
            m_pipe.close();
        }
    }

    OUString PipeConnection::getDescription()
            throw(::com::sun::star::uno::RuntimeException)
    {
        return m_sDescription;
    }

    /***************
     * PipeAcceptor
     **************/
    PipeAcceptor::PipeAcceptor( const OUString &sPipeName , const OUString & sConnectionDescription) :
        m_sPipeName( sPipeName ),
        m_sConnectionDescription( sConnectionDescription ),
        m_bClosed( sal_False )
    {
    }


    void PipeAcceptor::init()
    {
        m_pipe = Pipe( m_sPipeName.pData , osl_Pipe_CREATE , osl::Security() );
        if( ! m_pipe.is() )
        {
            OUString error = OUString::createFromAscii( "io.acceptor: Couldn't setup pipe " );
            error += m_sPipeName;
            throw ConnectionSetupException( error, Reference< XInterface > () );
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
            OUString error = OUString::createFromAscii( "io.acceptor: pipe already closed" );
            error += m_sPipeName;
            throw ConnectionSetupException( error, Reference< XInterface > () );
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
            return Reference < XConnection > ( (XConnection * ) pConn );
        }
        else
        {
            OUString error = OUString::createFromAscii( "io.acceptor: Couldn't setup pipe " );
            error += m_sPipeName;
            throw ConnectionSetupException( error, Reference< XInterface > ());
        }
    }

    void PipeAcceptor::stopAccepting()
    {
        m_bClosed = sal_True;
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
