/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_dependencies.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 14:26:12 $
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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_DEPENDENCIES_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_DEPENDENCIES_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include "com/sun/star/uno/Sequence.hxx"
#endif

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_MISC_API_HXX
#include "dp_misc_api.hxx"
#endif

/// @HTML

namespace com { namespace sun { namespace star { namespace xml { namespace dom {
    class XElement;
} } } } }
namespace dp_misc { class DescriptionInfoset; }
namespace rtl { class OUString; }

namespace dp_misc {

/**
   Dependency handling.
*/
namespace Dependencies {
    /**
       Check for unsatisfied dependencies.

       @param infoset
       the infoset containing the dependencies to check

       @return
       a list of the unsatisfied dependencies from <code>infoset</code> (in no
       specific order)
    */
    DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::dom::XElement > >
    check(::dp_misc::DescriptionInfoset const & infoset);

    /**
       Obtain the (human-readable) name of a dependency.

       @param dependency
       a dependency represented as a non-null XML element

       @return
       the name of the dependency; will never be empty, as a localized
       &ldquo;unknown&rdquo; is substituted for an empty/missing name
     */
    DESKTOP_DEPLOYMENTMISC_DLLPUBLIC ::rtl::OUString name(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::dom::XElement > const & dependency);
}

}

#endif
