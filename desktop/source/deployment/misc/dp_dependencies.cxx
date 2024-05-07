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

#include <config_folders.h>

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/xml/dom/XElement.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <osl/diagnose.h>
#include <rtl/bootstrap.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <unotools/configmgr.hxx>

#include <strings.hrc>
#include <dp_shared.hxx>

#include <dp_dependencies.hxx>
#include <dp_descriptioninfoset.hxx>
#include <dp_version.hxx>

namespace {

char const namespaceLibreOffice[] =
    "http://libreoffice.org/extensions/description/2011";

constexpr OUString namespaceOpenOfficeOrg =
    u"http://openoffice.org/extensions/description/2006"_ustr;

char const minimalVersionLibreOffice[] = "LibreOffice-minimal-version";
char const maximalVersionLibreOffice[] = "LibreOffice-maximal-version";

constexpr OUString minimalVersionOpenOfficeOrg =
    u"OpenOffice.org-minimal-version"_ustr;

char const maximalVersionOpenOfficeOrg[] =
    "OpenOffice.org-maximal-version";

OUString getLibreOfficeMajorMinorMicro() {
    return utl::ConfigManager::getAboutBoxProductVersion();
}

OUString getReferenceOpenOfficeOrgMajorMinor() {
#ifdef ANDROID
    // just hardcode the version
    OUString v("4.1");
#else
    OUString v(
            u"${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("version")
            ":Version:ReferenceOOoMajorMinor}"_ustr);
    rtl::Bootstrap::expandMacros(v); //TODO: check for failure
#endif
    return v;
}

bool satisfiesMinimalVersion(
    std::u16string_view actual, std::u16string_view specified)
{
    return dp_misc::compareVersions(actual, specified) != dp_misc::LESS;
}

bool satisfiesMaximalVersion(
    std::u16string_view actual, std::u16string_view specified)
{
    return dp_misc::compareVersions(actual, specified) != dp_misc::GREATER;
}

OUString produceErrorText(
    OUString const & reason, OUString const & version)
{
    return reason.replaceFirst("%VERSION",
        (version.isEmpty()
         ?  DpResId(RID_DEPLOYMENT_DEPENDENCIES_UNKNOWN)
         : version));
}

}

namespace dp_misc::Dependencies {

css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > >
check(dp_misc::DescriptionInfoset const & infoset) {
    css::uno::Reference< css::xml::dom::XNodeList > deps(
        infoset.getDependencies());
    sal_Int32 n = deps->getLength();
    css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > >
        unsatisfied(n);
    auto unsatisfiedRange = asNonConstRange(unsatisfied);
    sal_Int32 unsat = 0;
    // check first if minimalVersionLibreOffice is specified -- in that case ignore the legacy OOo dependencies
    bool bIgnoreOoo = false;
    for (sal_Int32 i = 0; i < n; ++i) {
        css::uno::Reference< css::xml::dom::XElement > e(
            deps->item(i), css::uno::UNO_QUERY_THROW);
        if ( e->getNamespaceURI() == namespaceLibreOffice && e->getTagName() == minimalVersionLibreOffice)
        {
            bIgnoreOoo = true;
            break;
        }
    }
    for (sal_Int32 i = 0; i < n; ++i) {
        css::uno::Reference< css::xml::dom::XElement > e(
            deps->item(i), css::uno::UNO_QUERY_THROW);
        bool sat = false;
        if ( e->getNamespaceURI() == namespaceOpenOfficeOrg && e->getTagName() == minimalVersionOpenOfficeOrg )
        {
            sat = bIgnoreOoo || satisfiesMinimalVersion(
                getReferenceOpenOfficeOrgMajorMinor(),
                e->getAttribute(u"value"_ustr));
        } else if ( e->getNamespaceURI() == namespaceOpenOfficeOrg && e->getTagName() == maximalVersionOpenOfficeOrg )
        {
            sat = bIgnoreOoo || satisfiesMaximalVersion(
                getReferenceOpenOfficeOrgMajorMinor(),
                e->getAttribute(u"value"_ustr));
        } else if (e->getNamespaceURI() == namespaceLibreOffice && e->getTagName() == minimalVersionLibreOffice )
        {
            sat = satisfiesMinimalVersion(
                getLibreOfficeMajorMinorMicro(),
                e->getAttribute(u"value"_ustr));
        } else if (e->getNamespaceURI() == namespaceLibreOffice && e->getTagName() == maximalVersionLibreOffice )
        {
            sat = satisfiesMaximalVersion(getLibreOfficeMajorMinorMicro(), e->getAttribute(u"value"_ustr));
        } else if (e->hasAttributeNS(namespaceOpenOfficeOrg,
                       minimalVersionOpenOfficeOrg))
        {
            sat = satisfiesMinimalVersion(
                getReferenceOpenOfficeOrgMajorMinor(),
                e->getAttributeNS(namespaceOpenOfficeOrg,
                    minimalVersionOpenOfficeOrg));
        }
        if (!sat) {
            unsatisfiedRange[unsat++] = e;
        }
    }
    unsatisfied.realloc(unsat);
    return unsatisfied;
}

OUString getErrorText(
    css::uno::Reference< css::xml::dom::XElement > const & dependency)
{
    OSL_ASSERT(dependency.is());
    if ( dependency->getNamespaceURI() == namespaceOpenOfficeOrg && dependency->getTagName() == minimalVersionOpenOfficeOrg )
    {
        return produceErrorText(
                DpResId(RID_DEPLOYMENT_DEPENDENCIES_OOO_MIN),
            dependency->getAttribute(u"value"_ustr));
    } else if (dependency->getNamespaceURI() == namespaceOpenOfficeOrg && dependency->getTagName() == maximalVersionOpenOfficeOrg )
    {
        return produceErrorText(
                DpResId(RID_DEPLOYMENT_DEPENDENCIES_OOO_MAX),
            dependency->getAttribute(u"value"_ustr));
    } else if (dependency->getNamespaceURI() == namespaceLibreOffice && dependency->getTagName() == minimalVersionLibreOffice )
    {
        return produceErrorText(
                DpResId(RID_DEPLOYMENT_DEPENDENCIES_LO_MIN),
            dependency->getAttribute(u"value"_ustr));
    } else if (dependency->getNamespaceURI() == namespaceLibreOffice && dependency->getTagName() == maximalVersionLibreOffice )
    {
        return produceErrorText(
                DpResId(RID_DEPLOYMENT_DEPENDENCIES_LO_MAX),
            dependency->getAttribute(u"value"_ustr));
    } else if (dependency->hasAttributeNS(namespaceOpenOfficeOrg,
                   minimalVersionOpenOfficeOrg))
    {
        return produceErrorText(
                DpResId(RID_DEPLOYMENT_DEPENDENCIES_OOO_MIN),
            dependency->getAttributeNS(namespaceOpenOfficeOrg,
                minimalVersionOpenOfficeOrg));
    } else {
        return DpResId(RID_DEPLOYMENT_DEPENDENCIES_UNKNOWN);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
