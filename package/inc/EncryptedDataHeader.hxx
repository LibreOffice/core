/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EncryptedDataHeader.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:55:26 $
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
#ifndef _ENCRYPTED_DATA_HEADER_HXX_
#define _ENCRYPTED_DATA_HEADER_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

/* The structure of this header is as follows:

   Header signature 4 bytes
   Version number   2 bytes
   Iteraction count 4 bytes
   Size             4 bytes
   Salt length      2 bytes
   IV length        2 bytes
   Digest length    2 bytes
   MediaType length 2 bytes
   Salt content     X bytes
   IV content       X bytes
   digest content   X bytes
   MediaType        X bytes

*/
const sal_uInt32 n_ConstHeader = 0x0502474dL; // "MG\002\005"
const sal_Int32 n_ConstHeaderSize = 22; // + salt length + iv length + digest length + mediatype length
const sal_Int16 n_ConstCurrentVersion = 1;
#endif
