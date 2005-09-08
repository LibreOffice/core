/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unohelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:22:25 $
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
