/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_DEPENDENCIES_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_DEPENDENCIES_HXX

#include "unotools/configmgr.hxx"
#include "sal/config.h"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "dp_misc_api.hxx"

/// @HTML

namespace com { namespace sun { namespace star { namespace xml { namespace dom {
    class XElement;
} } } } }
namespace dp_misc { class DescriptionInfoset; }
namespace rtl { class OUString; }

namespace dp_misc {

struct BrandName : public ::rtl::StaticWithInit<const ::rtl::OUString, BrandName> {
    const ::rtl::OUString operator () () {
        return ::utl::ConfigManager::GetDirectConfigProperty(
            ::utl::ConfigManager::PRODUCTNAME ).get< ::rtl::OUString >();
    }
};


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
       Obtain the (human-readable) error message of a failed dependency.

       @param dependency
       a dependency represented as a non-null XML element

       @return
       the name of the dependency; will never be empty, as a localized
       &ldquo;unknown&rdquo; is substituted for an empty/missing name
     */
    DESKTOP_DEPLOYMENTMISC_DLLPUBLIC ::rtl::OUString getErrorText(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::dom::XElement > const & dependency);
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
