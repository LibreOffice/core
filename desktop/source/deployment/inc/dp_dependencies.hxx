/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_DEPENDENCIES_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_DEPENDENCIES_HXX

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include "dp_misc_api.hxx"

/// @HTML

namespace com { namespace sun { namespace star { namespace xml { namespace dom {
    class XElement;
} } } } }
namespace dp_misc { class DescriptionInfoset; }

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
    DESKTOP_DEPLOYMENTMISC_DLLPUBLIC css::uno::Sequence<
        css::uno::Reference< css::xml::dom::XElement > >
    check(dp_misc::DescriptionInfoset const & infoset);

    /**
       Obtain the (human-readable) error message of a failed dependency.

       @param dependency
       a dependency represented as a non-null XML element

       @return
       the name of the dependency; will never be empty, as a localized
       &ldquo;unknown&rdquo; is substituted for an empty/missing name
    */
    DESKTOP_DEPLOYMENTMISC_DLLPUBLIC OUString getErrorText(
        css::uno::Reference< css::xml::dom::XElement >
            const & dependency);
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
