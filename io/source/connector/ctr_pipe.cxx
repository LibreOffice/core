/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_io.hxx"

#include "connector.hxx"

using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::connection;


namespace stoc_connector {

    PipeConnection::PipeConnection( const OUString & sConnectionDescription ) :
        m_nStatus( 0 ),
        m_sDescription( sConnectionDescription )
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
        // make it unique
        m_sDescription += OUString::createFromAscii( ",uniqueValue=" );
        m_sDescription += OUString::valueOf(
            sal::static_int_cast< sal_Int64 >(
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
            if( aReadBytes.getLength() != nBytesToRead )
            {
                aReadBytes.realloc( nBytesToRead );
            }
            return m_pipe.read( aReadBytes.getArray()  , aReadBytes.getLength() );
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
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException)
    {

    }

    void PipeConnection::close()
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException)
    {
        // ensure that close is called only once
        if(1 == osl_incrementInterlockedCount( (&m_nStatus) ) )
        {
            m_pipe.close();
        }
    }

    OUString PipeConnection::getDescription()
            throw( ::com::sun::star::uno::RuntimeException)
    {
        return m_sDescription;
    }

}

