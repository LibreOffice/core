/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: connection.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:11:26 $
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
