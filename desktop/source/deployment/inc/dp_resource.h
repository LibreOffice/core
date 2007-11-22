/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_resource.h,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:04:10 $
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

#if ! defined INCLUDED_DP_RESOURCE_H
#define INCLUDED_DP_RESOURCE_H

#include "tools/resmgr.hxx"
#include "tools/string.hxx"
#include "tools/resid.hxx"
#include "com/sun/star/lang/Locale.hpp"
#include "dp_misc.h"
#include <memory>

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_MISC_API_HXX
#include "dp_misc_api.hxx"
#endif

namespace dp_misc {

//==============================================================================
ResId getResId( USHORT id );

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC String getResourceString( USHORT id );

template <typename Unique, USHORT id>
struct StaticResourceString :
        public ::rtl::StaticWithInit<const ::rtl::OUString, Unique> {
    const ::rtl::OUString operator () () { return getResourceString(id); }
};

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
::com::sun::star::lang::Locale toLocale( ::rtl::OUString const & slang );

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
::com::sun::star::lang::Locale getOfficeLocale();

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
::rtl::OUString getOfficeLocaleString();

}

#endif
