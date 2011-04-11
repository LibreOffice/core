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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/xml/dom/XElement.hpp"
#include "com/sun/star/xml/dom/XNode.hpp"
#include "com/sun/star/xml/dom/XNodeList.hpp"
#include "rtl/bootstrap.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "tools/string.hxx"

#include "deployment.hrc"
#include "dp_resource.h"

#include "dp_dependencies.hxx"
#include "dp_descriptioninfoset.hxx"
#include "dp_version.hxx"

namespace {

namespace css = ::com::sun::star;

static char const xmlNamespace[] =
    "http://openoffice.org/extensions/description/2006";

bool
lcl_versionIsNot(dp_misc::Order i_eOrder, ::rtl::OUString const& i_rVersion)
{
    ::rtl::OUString aVersion(
        RTL_CONSTASCII_USTRINGPARAM(
            "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("version")
            ":Version:OOOPackageVersion}"));
    ::rtl::Bootstrap::expandMacros(aVersion);
    return ::dp_misc::compareVersions(aVersion, i_rVersion) != i_eOrder;
}

bool satisfiesMinimalVersion(::rtl::OUString const& i_rVersion)
{
    return lcl_versionIsNot(dp_misc::LESS, i_rVersion);
}

bool satisfiesMaximalVersion(::rtl::OUString const& i_rVersion)
{
    return lcl_versionIsNot(dp_misc::GREATER, i_rVersion);
}

}

namespace dp_misc {

namespace Dependencies {

css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > >
check(::dp_misc::DescriptionInfoset const & infoset) {
    css::uno::Reference< css::xml::dom::XNodeList > deps(
        infoset.getDependencies());
    ::sal_Int32 n = deps->getLength();
    css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > >
        unsatisfied(n);
    ::sal_Int32 unsat = 0;
    for (::sal_Int32 i = 0; i < n; ++i) {
        static rtl::OUString const minimalVersion(
                RTL_CONSTASCII_USTRINGPARAM("OpenOffice.org-minimal-version"));
        css::uno::Reference< css::xml::dom::XElement > e(
            deps->item(i), css::uno::UNO_QUERY_THROW);
        bool sat = false;
        if (e->getNamespaceURI().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM(xmlNamespace))
            && (e->getTagName() == minimalVersion))
        {
            sat = satisfiesMinimalVersion(
                e->getAttribute(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"))));
        } else if (e->getNamespaceURI().equalsAsciiL(
                       RTL_CONSTASCII_STRINGPARAM(xmlNamespace))
                   && e->getTagName().equalsAsciiL(
                       RTL_CONSTASCII_STRINGPARAM(
                           "OpenOffice.org-maximal-version")))
        {
            sat = satisfiesMaximalVersion(
                    e->getAttribute(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"))));
        } else if (e->hasAttributeNS(
                       ::rtl::OUString(
                           RTL_CONSTASCII_USTRINGPARAM(xmlNamespace)),
                        minimalVersion))
        {
            sat = satisfiesMinimalVersion(
                e->getAttributeNS(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(xmlNamespace)),
                    minimalVersion));
        }
        if (!sat) {
            unsatisfied[unsat++] = e;
        }
    }
    unsatisfied.realloc(unsat);
    return unsatisfied;
}

::rtl::OUString getErrorText( css::uno::Reference< css::xml::dom::XElement > const & dependency )
{
    ::rtl::OUString sReason;
    ::rtl::OUString sValue;
    ::rtl::OUString sVersion(RTL_CONSTASCII_USTRINGPARAM("%VERSION"));
    ::rtl::OUString sProductName(RTL_CONSTASCII_USTRINGPARAM("%PRODUCTNAME"));

    if ( dependency->getNamespaceURI().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( xmlNamespace ) )
         && dependency->getTagName().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "OpenOffice.org-minimal-version" ) ) )
    {
        sValue = dependency->getAttribute( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "value" ) ) );
        sReason = ::rtl::OUString( ::String(::dp_misc::getResId(RID_DEPLYOMENT_DEPENDENCIES_MIN)) );
    }
    else if ( dependency->getNamespaceURI().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( xmlNamespace ) )
              && dependency->getTagName().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "OpenOffice.org-maximal-version" ) ) )
    {
        sValue = dependency->getAttribute( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value") ) );
        sReason = ::rtl::OUString( ::String(::dp_misc::getResId(RID_DEPLYOMENT_DEPENDENCIES_MAX)) );
    }
    else if ( dependency->hasAttributeNS( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( xmlNamespace ) ),
                                          ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OpenOffice.org-minimal-version" ))))
    {
        sValue = dependency->getAttributeNS( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( xmlNamespace ) ),
                                             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OpenOffice.org-minimal-version" ) ) );
        sReason = ::rtl::OUString( ::String(::dp_misc::getResId(RID_DEPLYOMENT_DEPENDENCIES_MIN)) );
    }
    else
        return ::rtl::OUString( ::String(::dp_misc::getResId(RID_DEPLYOMENT_DEPENDENCIES_UNKNOWN)) );

    if ( sValue.getLength() == 0 )
        sValue = ::rtl::OUString( ::String(::dp_misc::getResId(RID_DEPLYOMENT_DEPENDENCIES_UNKNOWN)) );

    sal_Int32 nPos = sReason.indexOf( sVersion );
    if ( nPos >= 0 )
        sReason = sReason.replaceAt( nPos, sVersion.getLength(), sValue );
    nPos = sReason.indexOf( sProductName );
    if ( nPos >= 0 )
        sReason = sReason.replaceAt( nPos, sProductName.getLength(), BrandName::get() );
    return sReason;
}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
