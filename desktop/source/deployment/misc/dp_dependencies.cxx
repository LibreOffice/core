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

#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/xml/dom/XElement.hpp"
#include "com/sun/star/xml/dom/XNodeList.hpp"
#include "rtl/bootstrap.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "tools/resid.hxx"
#include "unotools/configmgr.hxx"

#include "deployment.hrc"
#include "dp_resource.h"

#include "dp_dependencies.hxx"
#include "dp_descriptioninfoset.hxx"
#include "dp_version.hxx"

namespace {

namespace css = com::sun::star;

static char const namespaceLibreOffice[] =
    "http://libreoffice.org/extensions/description/2011";

static char const namespaceOpenOfficeOrg[] =
    "http://openoffice.org/extensions/description/2006";

static char const minimalVersionLibreOffice[] = "LibreOffice-minimal-version";

static char const minimalVersionOpenOfficeOrg[] =
    "OpenOffice.org-minimal-version";

static char const maximalVersionOpenOfficeOrg[] =
    "OpenOffice.org-maximal-version";

rtl::OUString getLibreOfficeMajorMinorMicro() {
    return utl::ConfigManager::getAboutBoxProductVersion();
}

rtl::OUString getReferenceOpenOfficeOrgMajorMinor() {
    rtl::OUString v(
        RTL_CONSTASCII_USTRINGPARAM(
            "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE("version")
            ":Version:ReferenceOOoMajorMinor}"));
    rtl::Bootstrap::expandMacros(v); //TODO: check for failure
    return v;
}

bool satisfiesMinimalVersion(
    rtl::OUString const & actual, rtl::OUString const & specified)
{
    return dp_misc::compareVersions(actual, specified) != dp_misc::LESS;
}

bool satisfiesMaximalVersion(
    rtl::OUString const & actual, rtl::OUString const & specified)
{
    return dp_misc::compareVersions(actual, specified) != dp_misc::GREATER;
}

rtl::OUString produceErrorText(
    rtl::OUString const & reason, rtl::OUString const & version)
{
    return reason.replaceFirst("%VERSION",
        (version.isEmpty()
         ?  dp_misc::getResId(RID_DEPLOYMENT_DEPENDENCIES_UNKNOWN).toString()
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
        if ( e->getNamespaceURI() == namespaceOpenOfficeOrg && e->getTagName() == minimalVersionOpenOfficeOrg )
        {
            sat = satisfiesMinimalVersion(
                getReferenceOpenOfficeOrgMajorMinor(),
                e->getAttribute(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"))));
        } else if ( e->getNamespaceURI() == namespaceOpenOfficeOrg && e->getTagName() == maximalVersionOpenOfficeOrg )
        {
            sat = satisfiesMaximalVersion(
                getReferenceOpenOfficeOrgMajorMinor(),
                e->getAttribute(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"))));
        } else if (e->getNamespaceURI() == namespaceLibreOffice && e->getTagName() == minimalVersionLibreOffice )
        {
            sat = satisfiesMinimalVersion(
                getLibreOfficeMajorMinorMicro(),
                e->getAttribute(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"))));
        } else if (e->hasAttributeNS(
                       rtl::OUString(
                           RTL_CONSTASCII_USTRINGPARAM(namespaceOpenOfficeOrg)),
                       rtl::OUString(
                           RTL_CONSTASCII_USTRINGPARAM(
                               minimalVersionOpenOfficeOrg))))
        {
            sat = satisfiesMinimalVersion(
                getReferenceOpenOfficeOrgMajorMinor(),
                e->getAttributeNS(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(namespaceOpenOfficeOrg)),
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            minimalVersionOpenOfficeOrg))));
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
    if ( dependency->getNamespaceURI() == namespaceOpenOfficeOrg && dependency->getTagName() == minimalVersionOpenOfficeOrg )
    {
        return produceErrorText(
                dp_misc::getResId(RID_DEPLOYMENT_DEPENDENCIES_OOO_MIN).toString(),
            dependency->getAttribute(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"))));
    } else if (dependency->getNamespaceURI() == namespaceOpenOfficeOrg && dependency->getTagName() == maximalVersionOpenOfficeOrg )
    {
        return produceErrorText(
                dp_misc::getResId(RID_DEPLOYMENT_DEPENDENCIES_OOO_MAX).toString(),
            dependency->getAttribute(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"))));
    } else if (dependency->getNamespaceURI() == namespaceLibreOffice && dependency->getTagName() == minimalVersionLibreOffice )
    {
        return produceErrorText(
                dp_misc::getResId(RID_DEPLOYMENT_DEPENDENCIES_LO_MIN).toString(),
            dependency->getAttribute(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"))));
    } else if (dependency->hasAttributeNS(
                   rtl::OUString(
                       RTL_CONSTASCII_USTRINGPARAM(namespaceOpenOfficeOrg)),
                   rtl::OUString(
                       RTL_CONSTASCII_USTRINGPARAM(
                           minimalVersionOpenOfficeOrg))))
    {
        return produceErrorText(
                dp_misc::getResId(RID_DEPLOYMENT_DEPENDENCIES_OOO_MIN).toString(),
            dependency->getAttributeNS(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(namespaceOpenOfficeOrg)),
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(minimalVersionOpenOfficeOrg))));
    } else {
        return dp_misc::getResId(RID_DEPLOYMENT_DEPENDENCIES_UNKNOWN).toString();
    }
}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
