/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MExtConfigAccess.hxx,v $
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

#ifndef CONNECTIVITY_MOZAB_MEXTCONFIGACCESS_HXX

// This is the extended version (for use on the SO side of the driver) of MConfigAccess
// (which is for use on the mozilla side only)

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace connectivity
{
    namespace mozab
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                createDriverConfigNode( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB );
    }
}

// MConfigAccess was invented to allow non-UNO parts access to the configuration.
// Unfortunately, configuration access requires a XMultiServiceFactory - which the
// mozilla side does not have.
// So we create a "library-local" service factory here: Every need for a service
// factory can be fullfilled by this factory (similar to the get/setProcessServiceFactory
// in comphelper).
// This is halfway valid, as usually, the mozabdrv library is invoked from the mozab library
// only. The latter contains the driver class (and only this class and nothing more), and
// the driver class is a singleton. The driver itself is created with a service factory,
// which (by definition) can and should be used for all subsequent service requests.
// And this is exactly what we're allowing with the following functions ....

/** _pFactory must point to an XMultiServiceFactory, which must be aquired once
    for purpose of safely transfering it. The callee will release this interface
    when it has stored the pointer somewhere else.
*/
extern "C" void SAL_CALL setMozabServiceFactory( void* _pFactory );

#endif // CONNECTIVITY_MOZAB_MEXTCONFIGACCESS_HXX

