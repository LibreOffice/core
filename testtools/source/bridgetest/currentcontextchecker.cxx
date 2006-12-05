/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: currentcontextchecker.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-05 15:02:17 $
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
#include "precompiled_testtools.hxx"

#include "sal/config.h"

#include "currentcontextchecker.hxx"

#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XCurrentContext.hpp"
#include "cppu/unotype.hxx"
#include "cppuhelper/implbase1.hxx"
#include "osl/diagnose.h"
#include "osl/diagnose.hxx"
#include "rtl/string.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "test/testtools/bridgetest/XCurrentContextChecker.hpp"
#include "uno/current_context.hxx"

namespace {

namespace css = ::com::sun::star;

static char const key[] = "testtools.bridgetest.Key";
static char const value[] = "good";

class CurrentContext:
    public ::osl::DebugBase< CurrentContext >,
    public ::cppu::WeakImplHelper1< css::uno::XCurrentContext >
{
public:
    CurrentContext();

    virtual ~CurrentContext();

    virtual css::uno::Any SAL_CALL getValueByName(::rtl::OUString const & Name)
        throw (css::uno::RuntimeException);

private:
    CurrentContext(CurrentContext &); // not defined
    void operator =(CurrentContext &); // not defined
};

CurrentContext::CurrentContext() {}

CurrentContext::~CurrentContext() {}

css::uno::Any CurrentContext::getValueByName(::rtl::OUString const & Name)
    throw (css::uno::RuntimeException)
{
    return Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(key))
        ? css::uno::makeAny(::rtl::OUString::createFromAscii(value))
        : css::uno::Any();
}

}

testtools::bridgetest::CurrentContextChecker::CurrentContextChecker() {}

testtools::bridgetest::CurrentContextChecker::~CurrentContextChecker() {}

::sal_Bool testtools::bridgetest::CurrentContextChecker::perform(
    css::uno::Reference<
        ::test::testtools::bridgetest::XCurrentContextChecker > const & other,
    ::sal_Int32 setSteps, ::sal_Int32 checkSteps)
    throw (css::uno::RuntimeException)
{
    if (setSteps == 0) {
        css::uno::ContextLayer layer(new CurrentContext);
        return performCheck(other, setSteps, checkSteps);
    } else {
        return performCheck(other, setSteps, checkSteps);
    }
}

bool testtools::bridgetest::CurrentContextChecker::performCheck(
    css::uno::Reference<
        ::test::testtools::bridgetest::XCurrentContextChecker > const & other,
    ::sal_Int32 setSteps, ::sal_Int32 checkSteps)
{
    OSL_ASSERT(other.is() && checkSteps >= 0);
    if (checkSteps == 0) {
        css::uno::Reference< css::uno::XCurrentContext > context(
            css::uno::getCurrentContext());
        if (!context.is()) {
            return false;
        }
        css::uno::Any a(
            context->getValueByName(::rtl::OUString::createFromAscii(key)));
        if (a.getValueType() != ::cppu::UnoType< ::rtl::OUString >::get()) {
            return false;
        }
        ::rtl::OUString s;
        OSL_VERIFY(a >>= s);
        return s.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(value));
    } else {
        return other->perform(
            this, setSteps >= 0 ? setSteps - 1 : -1, checkSteps - 1);
    }
}
