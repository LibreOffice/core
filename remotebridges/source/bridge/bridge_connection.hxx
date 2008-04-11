/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bridge_connection.hxx,v $
 * $Revision: 1.4 $
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

