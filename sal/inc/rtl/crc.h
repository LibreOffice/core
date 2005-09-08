/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: crc.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:37:03 $
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

#ifndef _RTL_CRC_H_
#define _RTL_CRC_H_ "$Revision: 1.3 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 *
 * rtl_crc32 interface.
 *
 *======================================================================*/
/** Evaluate CRC32 over given data.
    @descr This function evaluates the CRC polynomial 0xEDB88320.

    @param  Crc    [in] CRC32 over previous data or zero.
    @param  Data   [in] data buffer.
    @param  DatLen [in] data buffer length.
    @return new CRC32 value.
 */
sal_uInt32 SAL_CALL rtl_crc32 (
    sal_uInt32  Crc,
    const void *Data, sal_uInt32 DatLen
) SAL_THROW_EXTERN_C();

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* _RTL_CRC_H_ */

