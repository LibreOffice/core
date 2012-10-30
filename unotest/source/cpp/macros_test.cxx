/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *  Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "unotest/macros_test.hxx"

#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>

#include "cppunit/TestAssert.h"
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star;

namespace unotest {

uno::Reference< com::sun::star::lang::XComponent > MacrosTest::loadFromDesktop(const rtl::OUString& rURL)
{
    uno::Reference< com::sun::star::frame::XComponentLoader> xLoader = uno::Reference< com::sun::star::frame::XComponentLoader >( mxDesktop, uno::UNO_QUERY );
    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > args(1);
    args[0].Name = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("MacroExecutionMode"));
    args[0].Handle = -1;
    args[0].Value <<=
        com::sun::star::document::MacroExecMode::ALWAYS_EXECUTE_NO_WARN;
    args[0].State = com::sun::star::beans::PropertyState_DIRECT_VALUE;
    uno::Reference< com::sun::star::lang::XComponent> xComponent= xLoader->loadComponentFromURL(rURL, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_default")), 0, args);
    rtl::OUString sMessage = rtl::OUString( "loading failed: " ) + rURL;
    CPPUNIT_ASSERT_MESSAGE(rtl::OUStringToOString( sMessage, RTL_TEXTENCODING_UTF8 ).getStr( ), xComponent.is());
    return xComponent;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
