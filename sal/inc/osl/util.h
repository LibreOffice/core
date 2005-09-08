/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: util.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:34:54 $
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


#ifndef _OSL_UTIL_H_
#define _OSL_UTIL_H_

#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
    @param pEthernetAddr 6 bytes of memory

    @return sal_True  if the ethernetaddress could be retrieved. <br>
            sal_False if no address could be found. This may be either because
           there is no ethernet card or there is no appropriate algorithm
           implemented on the platform. In this case, pEthernetAddr is
           unchanged.
*/
sal_Bool SAL_CALL osl_getEthernetAddress( sal_uInt8 *pEthernetAddr );

#ifdef __cplusplus
}
#endif

#endif
