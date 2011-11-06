/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

bool satisfiesMinimalVersion(::rtl::OUString const & version) {
    ::rtl::OUString v(
        RTL_CONSTASCII_USTRINGPARAM(
            "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("version")
            ":Version:OOOPackageVersion}"));
    ::rtl::Bootstrap::expandMacros(v);
    return ::dp_misc::compareVersions(v, version) != ::dp_misc::LESS;
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
        static char const minimalVersion[] = "OpenOffice.org-minimal-version";
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
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"))));
        } else if (e->getNamespaceURI().equalsAsciiL(
                       RTL_CONSTASCII_STRINGPARAM(xmlNamespace))
                   && e->getTagName().equalsAsciiL(
                       RTL_CONSTASCII_STRINGPARAM(
                           "OpenOffice.org-maximal-version")))
        {
            ::rtl::OUString v(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("version")
                    ":Version:OOOBaseVersion}"));
            ::rtl::Bootstrap::expandMacros(v);
            sat =
                ::dp_misc::compareVersions(
                    v,
                    e->getAttribute(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"))))
                != ::dp_misc::GREATER;
        } else if (e->hasAttributeNS(
                       ::rtl::OUString(
                           RTL_CONSTASCII_USTRINGPARAM(xmlNamespace)),
                       ::rtl::OUString(
                           RTL_CONSTASCII_USTRINGPARAM(minimalVersion))))
        {
            sat = satisfiesMinimalVersion(
                e->getAttributeNS(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(xmlNamespace)),
                    ::rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(minimalVersion))));
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
    return sReason;
}

}

}
