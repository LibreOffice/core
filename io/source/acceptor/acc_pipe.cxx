/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acc_pipe.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:27:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
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
        PipeConnection( const OUString & s , const OUString &sConnectionDescription);
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



    PipeConnection::PipeConnection( const OUString &s, const OUString &sConnectionDescription) :
        m_nStatus( 0 ),
        m_sDescription( sConnectionDescription )
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );

        // make it unique
        m_sDescription += OUString::createFromAscii( ",uniqueValue=" );
        m_sDescription += OUString::valueOf( (sal_Int64) &m_pipe , 10 );
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
            if( aReadBytes.getLength() != nBytesToRead )
            {
                aReadBytes.realloc( nBytesToRead );
            }
            return m_pipe.read( aReadBytes.getArray()  , aReadBytes.getLength() );
        }
        else {
            throw IOException();
        }

        return 0;
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
        m_bClosed( sal_False ),
        m_sPipeName( sPipeName ),
        m_sConnectionDescription( sConnectionDescription )
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
        PipeConnection *pConn = new PipeConnection( m_sPipeName , m_sConnectionDescription );

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
