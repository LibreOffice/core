/*************************************************************************
 *
 *  $RCSfile: bridge_connection.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jbu $ $Date: 2000-09-28 08:47:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "bridge_connection.hxx"
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
        acquire = thisAcquire;
        release = thisRelease;
        read = thisRead;
        write = thisWrite;
        flush = thisFlush;
        close = thisClose;
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



