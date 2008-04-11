/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unohelper.hxx,v $
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
#ifndef _UNOHELPER_HXX
#define _UNOHELPER_HXX

#include <sal/types.h>

// forward declarations
namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace uno {
        class XInterface;
        template<class T> class Reference;
    }
    namespace beans { class XPropertySet; }
} } }


#define OUSTRING(msg) rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( msg ) )

namespace xforms
{

/** instantiate a UNO service using the process global service factory */
com::sun::star::uno::Reference<com::sun::star::uno::XInterface>
    createInstance( const rtl::OUString& sServiceName );

/** copy the properties from one PropertySet into the next */
void copy( const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& , com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& );

} // namespace

#endif
