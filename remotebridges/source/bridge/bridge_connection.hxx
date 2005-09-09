/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bridge_connection.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:21:42 $
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
#include <osl/interlck.h>

#include <bridges/remote/connection.h>

#include <com/sun/star/connection/XConnection.hpp>

namespace remotebridges_bridge
{

    class OConnectionWrapper :
        public remote_Connection
    {
    public:
        OConnectionWrapper( const ::com::sun::star::uno::Reference <
                            ::com::sun::star::connection::XConnection > & );
        ~OConnectionWrapper();

        static void      SAL_CALL thisAcquire( remote_Connection *);
        static void      SAL_CALL thisRelease( remote_Connection *);
        static sal_Int32 SAL_CALL thisRead( remote_Connection * , sal_Int8 *pDest , sal_Int32 nSize );
        static sal_Int32 SAL_CALL thisWrite( remote_Connection * ,
                                             const sal_Int8 *pSource ,
                                             sal_Int32 nSize );
        static void      SAL_CALL thisFlush( remote_Connection * );
        static void      SAL_CALL thisClose( remote_Connection * );

        ::com::sun::star::uno::Reference < ::com::sun::star::connection::XConnection > m_r;
        oslInterlockedCount m_nRef;
    };
}

