/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LConfigAccess.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:43:16 $
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

#ifndef _CONNECTIVITY_EVOAB_LCONFIGACCESS_HXX_
#define _CONNECTIVITY_EVOAB_LCONFIGACCESS_HXX_

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
    namespace evoab
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                createDriverConfigNode( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB, ::rtl::OUString _sDriverImplementationName );
        ::rtl::OUString getFullPathExportingCommand( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB );
    }
}
#endif // _CONNECTIVITY_EVOAB_LCONFIGACCESS_HXX_
