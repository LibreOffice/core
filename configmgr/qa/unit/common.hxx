/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: common.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 13:57:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

class Magic
{
    rtl::OUString maTempDir;
  public:
    Magic();
    ~Magic();
};

class Test: public CppUnit::TestFixture {
    Magic *mpMagic;
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
    void resetKey (const sal_Char *pPath, rtl::OUString aName);

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
