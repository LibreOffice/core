/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _ENCRYPTED_DATA_HEADER_HXX_
#define _ENCRYPTED_DATA_HEADER_HXX_

#include <sal/types.h>

/* The structure of this header is as follows:

   Header signature  4 bytes
   Version number    2 bytes
   Iteraction count  4 bytes
   Size              4 bytes
   EncAlgorithm      4 bytes
   DigestAlgorithm   4 bytes
   DerivedKeySize    4 bytes
   StartKeyAlgorithm 4 bytes
   Salt length       2 bytes
   IV length         2 bytes
   Digest length     2 bytes
   MediaType length  2 bytes
   Salt content      X bytes
   IV content        X bytes
   digest content    X bytes
   MediaType         X bytes

*/
const sal_uInt32 n_ConstHeader = 0x05024d4dL; // "MM\002\005"
const sal_Int32 n_ConstHeaderSize = 38; // + salt length + iv length + digest length + mediatype length
const sal_Int16 n_ConstCurrentVersion = 1;
#endif
