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

#include "precompiled_desktop.hxx"
#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/xml/dom/XElement.hpp"
#include "com/sun/star/xml/dom/XNodeList.hpp"
#include "comphelper/string.hxx"
#include "rtl/bootstrap.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "tools/resid.hxx"

#include "deployment.hrc"
#include "dp_resource.h"

#include "dp_dependencies.hxx"
#include "dp_descriptioninfoset.hxx"
#include "dp_version.hxx"

namespace {

namespace css = com::sun::star;

static char const xmlNamespace[] =
    "http://openoffice.org/extensions/description/2006";

static char const minimalVersion[] = "OpenOffice.org-minimal-version";

static char const maximalVersion[] = "OpenOffice.org-maximal-version";

bool versionIsNot(dp_misc::Order order, rtl::OUString const & version) {
    rtl::OUString oooVersion(
        RTL_CONSTASCII_USTRINGPARAM(
            "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE("version")
            ":Version:OOOPackageVersion}"));
    rtl::Bootstrap::expandMacros(oooVersion); //TODO: check for failure
    return dp_misc::compareVersions(oooVersion, version) != order;
}

bool satisfiesMinimalVersion(rtl::OUString const & version) {
    return versionIsNot(dp_misc::LESS, version);
}

bool satisfiesMaximalVersion(rtl::OUString const & version) {
    return versionIsNot(dp_misc::GREATER, version);
}

rtl::OUString produceErrorText(
    rtl::OUString const & reason, rtl::OUString const & version)
{
    return comphelper::string::searchAndReplaceAsciiL(
        reason, RTL_CONSTASCII_STRINGPARAM("%VERSION"),
        (version.isEmpty()
         ? ResId::toString(
             dp_misc::getResId(RID_DEPLOYMENT_DEPENDENCIES_UNKNOWN))
         : version));
}

}

namespace dp_misc {

namespace Dependencies {

css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > >
check(dp_misc::DescriptionInfoset const & infoset) {
    css::uno::Reference< css::xml::dom::XNodeList > deps(
        infoset.getDependencies());
    sal_Int32 n = deps->getLength();
    css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > >
        unsatisfied(n);
    sal_Int32 unsat = 0;
    for (sal_Int32 i = 0; i < n; ++i) {
        css::uno::Reference< css::xml::dom::XElement > e(
            deps->item(i), css::uno::UNO_QUERY_THROW);
        bool sat = false;
        if (e->getNamespaceURI().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM(xmlNamespace))
            && e->getTagName().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM(minimalVersion)))
        {
            sat = satisfiesMinimalVersion(
                e->getAttribute(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"))));
        } else if (e->getNamespaceURI().equalsAsciiL(
                       RTL_CONSTASCII_STRINGPARAM(xmlNamespace))
                   && e->getTagName().equalsAsciiL(
                       RTL_CONSTASCII_STRINGPARAM(maximalVersion)))
        {
            sat = satisfiesMaximalVersion(
                    e->getAttribute(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"))));
        } else if (e->hasAttributeNS(
                       rtl::OUString(
                           RTL_CONSTASCII_USTRINGPARAM(xmlNamespace)),
                       rtl::OUString(
                           RTL_CONSTASCII_USTRINGPARAM(minimalVersion))))
        {
            sat = satisfiesMinimalVersion(
                e->getAttributeNS(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(xmlNamespace)),
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(minimalVersion))));
        }
        if (!sat) {
            unsatisfied[unsat++] = e;
        }
    }
    unsatisfied.realloc(unsat);
    return unsatisfied;
}

rtl::OUString getErrorText(
    css::uno::Reference< css::xml::dom::XElement > const & dependency)
{
    OSL_ASSERT(dependency.is());
    if (dependency->getNamespaceURI().equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM(xmlNamespace))
        && dependency->getTagName().equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM(minimalVersion)))
    {
        return produceErrorText(
            ResId::toString(dp_misc::getResId(RID_DEPLOYMENT_DEPENDENCIES_MIN)),
            dependency->getAttribute(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"))));
    } else if (dependency->getNamespaceURI().equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM(xmlNamespace))
               && dependency->getTagName().equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM(maximalVersion)))
    {
        return produceErrorText(
            ResId::toString(dp_misc::getResId(RID_DEPLOYMENT_DEPENDENCIES_MAX)),
            dependency->getAttribute(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"))));
    } else if (dependency->hasAttributeNS(
                   rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(xmlNamespace)),
                   rtl::OUString(
                       RTL_CONSTASCII_USTRINGPARAM(minimalVersion))))
    {
        return produceErrorText(
            ResId::toString(dp_misc::getResId(RID_DEPLOYMENT_DEPENDENCIES_MIN)),
            dependency->getAttributeNS(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(xmlNamespace)),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(minimalVersion))));
    } else
        return ResId::toString(
            dp_misc::getResId(RID_DEPLOYMENT_DEPENDENCIES_UNKNOWN));
}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
