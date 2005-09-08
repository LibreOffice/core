/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MExtConfigAccess.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:18:55 $
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

#ifndef CONNECTIVITY_MOZAB_MEXTCONFIGACCESS_HXX

// This is the extended version (for use on the SO side of the driver) of MConfigAccess
// (which is for use on the mozilla side only)

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

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

