/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: common.hxx,v $
 * $Revision: 1.3 $
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
#include <cstdlib>
#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/XPropertyState.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/container/XNameReplace.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/util/XChangesBatch.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/component_context.hxx"
#include "cppuhelper/servicefactory.hxx"
#include "cppunit/simpleheader.hxx"
#include "osl/file.hxx"
#include "osl/thread.h"
#include "osl/process.h"
#include "rtl/string.h"
#include "rtl/bootstrap.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace css = com::sun::star;

#define CATCH_FAIL(msg) \
    catch (const css::uno::Exception &e) { \
        t_print ("msg '%s'\n", rtl::OUStringToOString (e.Message, RTL_TEXTENCODING_UTF8).getStr()); \
        CPPUNIT_FAIL( msg ); \
        throw; \
    }

class Test: public CppUnit::TestFixture {
public:
    // init
    virtual void setUp();
    virtual void tearDown();

    // helpers
    void normalizePathKey (rtl::OString &rPath, rtl::OString &rKey);
    css::uno::Reference< css::uno::XInterface > createView(const sal_Char *pNodepath, bool bUpdate);
    css::uno::Any getKey (const sal_Char *pPath, rtl::OUString aName);
    css::uno::Any getKey (const sal_Char *pPath, const sal_Char *pName)
        { return getKey (pPath, rtl::OUString::createFromAscii (pName)); }
    void setKey (const sal_Char *pPath, rtl::OUString aName, css::uno::Any a);
    bool resetKey (const sal_Char *pPath, rtl::OUString aName);

    // tests
    void keyFetch();
    void keySet();
    void keyReset();
    void readCommands();
    void threadTests();
    void recursiveTests();
    void eventTests();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(keyFetch);
    CPPUNIT_TEST(keySet);
    CPPUNIT_TEST(keyReset);
    CPPUNIT_TEST(readCommands);
    CPPUNIT_TEST(threadTests);
    CPPUNIT_TEST(recursiveTests);
    CPPUNIT_TEST(eventTests);
    CPPUNIT_TEST_SUITE_END();

private:
    css::uno::Reference< css::uno::XComponentContext > mxContext;
    css::uno::Reference< css::lang::XMultiServiceFactory > mxProvider;
};

void disposeComponent (const css::uno::Reference<css::uno::XInterface> &xComp);
