/*************************************************************************
 *
 *  $RCSfile: version.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 02:43:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

   @since #i21150#
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
