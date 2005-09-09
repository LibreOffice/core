/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: version.h,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:13:59 $
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
