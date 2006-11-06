/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_version.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-06 14:54:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_VERSION_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_VERSION_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif

namespace com { namespace sun { namespace star { namespace deployment {
    class XPackage;
} } } }
namespace rtl { class OUString; }

namespace dp_misc {

enum Order { LESS, EQUAL, GREATER };

Order compareVersions(
    ::rtl::OUString const & version1, ::rtl::OUString const & version2);

Order comparePackageVersions(
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage >
        const & package1,
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage >
        const & package2);

}

#endif
