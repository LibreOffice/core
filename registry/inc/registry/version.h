/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: version.h,v $
 * $Revision: 1.6 $
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

#ifndef INCLUDED_registry_version_h
#define INCLUDED_registry_version_h

#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @HTML

/**
   The version of a binary blob that represents a UNOIDL type.

   <p>All values between <code>TYPEREG_VERSION_0</code> and
   <code>TYPEREG_MAX_VERSION</code> are valid, where currently unallocated
   values represent future versions.  Negative values are not valid.</p>

   @see typereg::Reader
   @see typereg::Writer

   @since UDK 3.2.0
 */
enum typereg_Version {
    /**
       Denotes the original version of UNOIDL type blobs.
     */
    TYPEREG_VERSION_0,

    /**
       Denotes the updated version of UNOIDL type blobs.

       <p>This version added support for multiple-inheritance interface types,
       extended interface type attributes, single-interface&ndash;based
       services, interface-based singletons, polymorphic struct types, and
       published entities.</p>
     */
    TYPEREG_VERSION_1,

    /**
       Denotes the maximum future version of UNOIDL type blobs.
     */
    TYPEREG_MAX_VERSION = SAL_MAX_INT32
};

#ifdef __cplusplus
}
#endif

#endif
