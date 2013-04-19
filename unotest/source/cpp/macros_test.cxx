/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "unotest/macros_test.hxx"

#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>

#include "cppunit/TestAssert.h"
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star;

namespace unotest {

uno::Reference< com::sun::star::lang::XComponent > MacrosTest::loadFromDesktop(const OUString& rURL, const char* pDocService)
{
    uno::Reference< com::sun::star::frame::XComponentLoader> xLoader = uno::Reference< com::sun::star::frame::XComponentLoader >( mxDesktop, uno::UNO_QUERY );
    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > args(1);
    args[0].Name = "MacroExecutionMode";
    args[0].Handle = -1;
    args[0].Value <<=
        com::sun::star::document::MacroExecMode::ALWAYS_EXECUTE_NO_WARN;
    args[0].State = com::sun::star::beans::PropertyState_DIRECT_VALUE;

    if (pDocService)
    {
        args.realloc(2);
        args[1].Name = "DocumentService";
        args[1].Handle = -1;
        args[1].Value <<= OUString::createFromAscii(pDocService);
        args[1].State = com::sun::star::beans::PropertyState_DIRECT_VALUE;
    }

    uno::Reference< com::sun::star::lang::XComponent> xComponent= xLoader->loadComponentFromURL(rURL, OUString("_default"), 0, args);
    OUString sMessage = OUString( "loading failed: " ) + rURL;
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString( sMessage, RTL_TEXTENCODING_UTF8 ).getStr( ), xComponent.is());
    return xComponent;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
