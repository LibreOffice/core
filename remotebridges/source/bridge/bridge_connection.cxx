/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bridge_connection.cxx,v $
 * $Revision: 1.6 $
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
#include "bridge_connection.hxx"
#include "remote_bridge.hxx"
#include <rtl/byteseq.hxx>
#include <string.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::connection;

namespace remotebridges_bridge
{
    OConnectionWrapper::OConnectionWrapper( const Reference < XConnection > &r ) :
        m_r( r ),
        m_nRef( 0 )
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
        acquire = thisAcquire;
        release = thisRelease;
        read = thisRead;
        write = thisWrite;
        flush = thisFlush;
        close = thisClose;
    }

    OConnectionWrapper::~OConnectionWrapper()
    {
        g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

    void OConnectionWrapper::thisAcquire( remote_Connection *p)
    {
        OConnectionWrapper * m = ( OConnectionWrapper * ) p;
        osl_incrementInterlockedCount( &(m->m_nRef ) );
    }

    void OConnectionWrapper::thisRelease( remote_Connection * p)
    {
        OConnectionWrapper * m = ( OConnectionWrapper * ) p;
        if( ! osl_decrementInterlockedCount( &( m->m_nRef ) ) )
        {
            delete m;
        }
    }

    sal_Int32 OConnectionWrapper::thisRead( remote_Connection *p , sal_Int8 *pDest , sal_Int32 nSize )
    {
        // guard the C-Code
        OConnectionWrapper * m = ( OConnectionWrapper * ) p;
        try
        {
            // TODO possible optimization : give
            ::rtl::ByteSequence seq( nSize , ::rtl::BYTESEQ_NODEFAULT );
            sal_Int32 nRead = m->m_r->read( *(Sequence<sal_Int8>*)&seq , nSize );
            memcpy( pDest , seq.getConstArray() , nRead );
            return nRead;
        }
        catch ( Exception & )
        {
            return 0;
        }
        catch (::std::bad_alloc &)
        {
            return 0;
        }
    }

    sal_Int32 OConnectionWrapper::thisWrite( remote_Connection *p ,
                                             const sal_Int8 *pSource ,
                                             sal_Int32 nSize )
    {
        // guard the C-Code
        OConnectionWrapper * m = ( OConnectionWrapper * ) p;
        try
        {
            Sequence< sal_Int8 > seq( pSource , nSize  );
            m->m_r->write( seq );
            return nSize;
        }
        catch ( Exception &  )
        {
            return 0;
        }
    }

    void OConnectionWrapper::thisFlush( remote_Connection *p )
    {
        // guard the C-Code
        try
        {
            OConnectionWrapper * m = ( OConnectionWrapper * ) p;
            m->m_r->flush();
        }
        catch ( Exception & )
        {
        }
    }

    void OConnectionWrapper::thisClose( remote_Connection * p)
    {
        // guard the C-Code
        try
        {
            OConnectionWrapper * m = ( OConnectionWrapper * ) p;
            m->m_r->close();
        }
        catch( Exception & )
        {

        }
    }
}



