/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_dependencies.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 13:46:50 $
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
            "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE("version")
            ":Version:OOOBaseVersion}"));
    ::rtl::Bootstrap::expandMacros(v);
    return ::dp_misc::compareVersions(version, v) != ::dp_misc::GREATER;
};

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
        // Currently, the only satisfied dependency is OpenOffice.org-minimal-
        // version with a value of the current OOo release or less (the actual
        // version string has to be updated here for every OOo release):
        if (e->getNamespaceURI().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM(xmlNamespace))
            && e->getTagName().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM(minimalVersion)))
        {
            sat = satisfiesMinimalVersion(
                e->getAttribute(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"))));
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

::rtl::OUString
name(css::uno::Reference< css::xml::dom::XElement > const & dependency) {
    ::rtl::OUString n(
        dependency->getAttributeNS(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(xmlNamespace)),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("name"))));
    return n.getLength() == 0
        ? ::rtl::OUString(
            ::String(::dp_misc::getResId(RID_DEPLYOMENT_DEPENDENCIES_UNKNOWN)))
        : n;
}

}

}
