// To debug me use:
// $ export ENVCFGFLAGS='-me -ti -tw -tp -td'

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: performance.cxx,v $
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

#include "common.hxx"

void Test::setUp()
{
    char const * f = getForwardString();
    rtl::OUString rdb(
        f, rtl_str_getLength(f), osl_getThreadTextEncoding());
        //TODO: handle conversion failure

    css::uno::Reference< css::beans::XPropertySet > factory(
        cppu::createRegistryServiceFactory(rdb), css::uno::UNO_QUERY_THROW);
    mxContext = css::uno::Reference< css::uno::XComponentContext >(
        factory->getPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext"))),
        css::uno::UNO_QUERY_THROW);

    try {
        mxProvider = css::uno::Reference< css::lang::XMultiServiceFactory >(
            (css::uno::Reference< css::lang::XMultiComponentFactory >(
                mxContext->getServiceManager(), css::uno::UNO_QUERY_THROW)->
             createInstanceWithContext(
                 rtl::OUString(
                     RTL_CONSTASCII_USTRINGPARAM(
                         "com.sun.star.configuration.ConfigurationProvider")),
                 mxContext)),
            css::uno::UNO_QUERY_THROW);

    } catch (css::uno::Exception&e) {
        CPPUNIT_FAIL( "exception creating provider" );
        throw;
    }
}

css::uno::Reference< css::uno::XInterface >
Test::createView(const sal_Char *pNodepath, bool bUpdate)
{
    rtl::OUString aNodePath = rtl::OUString::createFromAscii(pNodepath);
    static const rtl::OUString kInfoViewService(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationAccess")) ;
    static const rtl::OUString kUpdateViewService(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationUpdateAccess")) ;
    static const rtl::OUString kNodepath(RTL_CONSTASCII_USTRINGPARAM("nodepath")) ;
    static const rtl::OUString kAsync(RTL_CONSTASCII_USTRINGPARAM("EnableAsync")) ;

    const rtl::OUString & kViewService = bUpdate ? kUpdateViewService : kInfoViewService;
    css::uno::Sequence< css::uno::Any > aViewArgs(2);
    aViewArgs[0] <<= css::beans::NamedValue( kNodepath, css::uno::makeAny(aNodePath) );
    aViewArgs[1] <<= css::beans::NamedValue( kAsync, css::uno::makeAny(sal_False) );

    css::uno::Reference< css::uno::XInterface > xView(
        mxProvider->createInstanceWithArguments( kViewService, aViewArgs ) );
    return xView;
}

void disposeComponent (const css::uno::Reference<css::uno::XInterface> &xComp)
{
    css::uno::Reference< css::lang::XComponent >(
        xComp, css::uno::UNO_QUERY_THROW)->dispose();
}

void Test::tearDown()
{
    disposeComponent (mxProvider);
    disposeComponent (mxContext);
}

void Test::normalizePathKey (rtl::OString &rPath, rtl::OString &rKey)
{
    sal_Int32 idx = rKey.lastIndexOf("/");
    if (idx > 0) {
        rPath += "/";
        rPath += rKey.copy (0, idx);
        rKey = rKey.copy (idx + 1);
//        t_print ("write to '%s' '%s'\n", (const sal_Char *)rPath,
//                 (const sal_Char *)rKey);
    }
}

css::uno::Any Test::getKey (const sal_Char *pPath, rtl::OUString aName)
{
    css::uno::Reference< css::container::XHierarchicalNameAccess > xNameAccess(
        createView(pPath, false), css::uno::UNO_QUERY_THROW);
    css::uno::Any aVal;
    aVal = xNameAccess->getByHierarchicalName (aName);
    disposeComponent (xNameAccess);
    return aVal;
}

void Test::setKey (const sal_Char *pPath, rtl::OUString aName, css::uno::Any a)
{
    css::uno::Reference< css::util::XChangesBatch > xAppView;

    xAppView = css::uno::Reference< css::util::XChangesBatch > (
        createView(pPath, true), css::uno::UNO_QUERY_THROW );
    css::uno::Reference< css::container::XNameReplace > xSettings(xAppView, css::uno::UNO_QUERY_THROW);
    rtl::OUString aStr;

    // set key
    xSettings->replaceByName(aName, a);
    xAppView->commitChanges();

    disposeComponent(xAppView);
}

bool Test::resetKey (const sal_Char *pPath, rtl::OUString aName)
{
    css::uno::Reference< css::util::XChangesBatch > xAppView;

    // reset to default
    xAppView = css::uno::Reference< css::util::XChangesBatch > ( createView(pPath, true), css::uno::UNO_QUERY_THROW );
    css::uno::Reference< css::container::XNameReplace > xSettings(xAppView, css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::beans::XPropertyState > xSettingsState(xSettings, css::uno::UNO_QUERY);
    if (!xSettingsState.is()) {
        return false;
    }
    xSettingsState->setPropertyToDefault(aName);
    xAppView->commitChanges();

    disposeComponent(xAppView);
    return true;
}

void Test::keyFetch()
{
    try {
        rtl::OUString aStr;
        if (!(getKey ("/org.openoffice.Setup", "L10N/ooLocale") >>= aStr))
            CPPUNIT_FAIL("to fetch key");
        if (!(getKey ("/org.openoffice.Setup", "Test/AString") >>= aStr))
            CPPUNIT_FAIL("to fetch key");
    } CATCH_FAIL ("fetching key")
}

void Test::keySet()
{
    try {
        setKey ("/org.openoffice.Setup/Test",
                rtl::OUString::createFromAscii("AString"),
                css::uno::makeAny (rtl::OUString::createFromAscii("baa")));

        // check value
        rtl::OUString aStr;
        if (!(getKey ("/org.openoffice.Setup/Test", "AString") >>= aStr))
            CPPUNIT_FAIL("to fetch key");

        CPPUNIT_ASSERT_MESSAGE ("check set value valid", aStr.equalsAscii("baa"));
    } CATCH_FAIL ("exception setting keys" )
}

void Test::keyReset()
{
    try {
        bool supported= resetKey ("/org.openoffice.Setup/Test",
                  rtl::OUString::createFromAscii("AString"));

        // check value
        rtl::OUString aStr;
        if (!(getKey ("/org.openoffice.Setup/Test", "AString") >>= aStr))
            CPPUNIT_FAIL("to fetch key");

        CPPUNIT_ASSERT_MESSAGE ("check default value valid",
                                !supported ||
                                aStr == rtl::OUString::createFromAscii("Foo"));
    } CATCH_FAIL ("exception setting keys" )
}

// This simulates the framework UI description code paths
void Test::readCommands()
{
    rtl::OUString aPropUILabel( RTL_CONSTASCII_USTRINGPARAM( "Label" ));
    rtl::OUString aPropUIContextLabel( RTL_CONSTASCII_USTRINGPARAM( "ContextLabel" ));
    rtl::OUString aPropProperties( RTL_CONSTASCII_USTRINGPARAM( "Properties" ));

    try {
        css::uno::Reference< css::container::XNameAccess > xNameAccess (
            createView("/org.openoffice.UI.GenericCommands/UserInterface/Commands", false),
            css::uno::UNO_QUERY_THROW);

        CPPUNIT_ASSERT_MESSAGE ("fetched UI generic commands", xNameAccess.is());

        css::uno::Any a;
        css::uno::Sequence< rtl::OUString > aNameSeq = xNameAccess->getElementNames();

        CPPUNIT_ASSERT_MESSAGE ("right element / sequence", aNameSeq.getLength() == 696);
        sal_uInt32 end, start = osl_getGlobalTimer();
        for ( sal_Int32 j = 0; j < 8; j++ )
        {
            for ( sal_Int32 i = 0; i < aNameSeq.getLength(); i++ )
            {
                try
                {
                    {
                        css::uno::Reference< css::container::XNameAccess > xChildNameAccess;
                        // This is the slow bit ! ...
                        // Creating the @#$@#$ing XNameAccess object [ 650 times ]
                        // which we then use to 'getByName' etc.
                        a = xNameAccess->getByName( aNameSeq[i] );
                        if ( a >>= xChildNameAccess )
                        {
                            a = xChildNameAccess->getByName( aPropUILabel );
                            a = xChildNameAccess->getByName( aPropUIContextLabel );
                            a = xChildNameAccess->getByName( aPropProperties );
                        }
                    }
                } CATCH_FAIL( "fetching keys" );
            }
        }
        end = osl_getGlobalTimer();
        t_print ("Reading elements took %d ms\n", (int)(end-start));
        disposeComponent (xNameAccess);
    } CATCH_FAIL( "accessing commands" );
}

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test, "alltests");

NOADDITIONAL;
