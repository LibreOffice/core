/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_identifier.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-01-19 09:14:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_IDENTIFIER_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_IDENTIFIER_HXX

#include "sal/config.h"

#include "boost/optional.hpp"
#include "com/sun/star/uno/Reference.hxx"

#include "dp_misc_api.hxx"

namespace com { namespace sun { namespace star { namespace deployment {
    class XPackage;
} } } }
namespace rtl { class OUString; }

namespace dp_misc {

/**
   Generates an identifier from an optional identifier.

   @param optional
   an optional identifier

   @param fileName
   a file name

   @return
   the given optional identifier if present, otherwise a legacy identifier based
   on the given file name
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC ::rtl::OUString generateIdentifier(
    ::boost::optional< ::rtl::OUString > const & optional,
    ::rtl::OUString const & fileName);

/**
   Gets the identifier of a package.

   @param package
   a non-null package

   @return
   the explicit identifier of the given package if present, otherwise the
   implicit legacy identifier of the given package

   @throws com::sun::star::uno::RuntimeException
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC ::rtl::OUString getIdentifier(
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage >
        const & package);

/**
   Generates a legacy identifier based on a file name.

   @param fileName
   a file name

   @return
   a legacy identifier based on the given file name
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC ::rtl::OUString generateLegacyIdentifier(
    ::rtl::OUString const & fileName);

}

#endif
