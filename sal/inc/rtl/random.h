/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: random.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:40:34 $
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

#ifndef _RTL_RANDOM_H_
#define _RTL_RANDOM_H_ "$Revision: 1.6 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 *
 * rtlRandom interface.
 *
 *======================================================================*/
/** Random Pool opaque type.
 */
typedef void* rtlRandomPool;


/** Error Code enumeration.
 */
enum __rtl_RandomError
{
    rtl_Random_E_None,
    rtl_Random_E_Argument,
    rtl_Random_E_Memory,
    rtl_Random_E_Unknown,
    rtl_Random_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

/** Error Code type.
 */
typedef enum __rtl_RandomError rtlRandomError;


/** Create a Random Pool.
    @return initialized Random Pool, or NULL upon failure.
 */
rtlRandomPool SAL_CALL rtl_random_createPool (void) SAL_THROW_EXTERN_C();


/** Destroy a Random Pool.
    @param  Pool [in] a Random Pool.
    @return none. Pool is invalid.
 */
void SAL_CALL rtl_random_destroyPool (
    rtlRandomPool Pool
) SAL_THROW_EXTERN_C();


/** Add bytes to a Random Pool.
    @param Pool    [in] a Random Pool.
    @param pBuffer [in] a buffer containing the bytes to add.
    @param nBufLen [in] the number of bytes to read from the buffer.
    @return rtl_Random_E_None upon success.
 */
rtlRandomError SAL_CALL rtl_random_addBytes (
    rtlRandomPool  Pool,
    const void    *Buffer,
    sal_Size       Bytes
) SAL_THROW_EXTERN_C();


/** Retrieve bytes from a Random Pool.
    @param Pool    [in] a Random Pool.
    @param pBuffer [inout] a buffer to receive the random bytes.
    @param nBufLen [in] the number of bytes to write to the buffer.
    @return rtl_Random_E_None upon success.
 */
rtlRandomError SAL_CALL rtl_random_getBytes (
    rtlRandomPool  Pool,
    void          *Buffer,
    sal_Size       Bytes
) SAL_THROW_EXTERN_C();

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* _RTL_RANDOM_H_ */

