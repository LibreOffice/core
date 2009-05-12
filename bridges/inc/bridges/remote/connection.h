/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: connection.h,v $
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
#ifndef _BRIDGES_REMOTE_CONNECTION_H_
#define _BRIDGES_REMOTE_CONNECTION_H_
#include <sal/types.h>

/** Specfies a C-interface for a bidirectional bytestream,
    which is used by a UNO remote environment.
 */
struct remote_Connection
{
    void ( SAL_CALL * acquire ) ( remote_Connection *);

    void ( SAL_CALL * release ) ( remote_Connection *);

    /**
      reads nSize bytes from the connection. This method blocks, until
      all bytes are available or an error occurs.
      @return Number of bytes read.
              If the return value is less than nSize, an unrecoverable
              i/o error has occured or the connection was closed.

     */
    sal_Int32 (SAL_CALL * read)(remote_Connection *, sal_Int8 *pDest, sal_Int32 nSize );

    /**
      @return Number of bytes written.
              if the return value is less than nSize an unrecoverable
              i/o error has occured or the connection was closed.
     */
    sal_Int32 (SAL_CALL * write)(remote_Connection *, const sal_Int8 *pSource, sal_Int32 nSize );

    void ( SAL_CALL * flush ) ( remote_Connection * );

    /** closes the connection.
        Any read or write operation after this call shall not be served
        anymore. Any ongoing read or write operation must return immeadiatly after this call.
        The implementation should cope with multiple calls to this method.
     */
    void (SAL_CALL * close) ( remote_Connection * );
};

#endif
