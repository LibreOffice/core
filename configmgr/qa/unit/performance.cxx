// To debug me use:
// $ export ENVCFGFLAGS='-me -ti -tw -tp -td'

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: performance.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 13:58:34 $
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

#include "common.hxx"

using namespace css;

void Test::setUp()
{
    mpMagic = new Magic();

    char const * f = getForwardString();
    rtl::OUString args(
        f, rtl_str_getLength(f), osl_getThreadTextEncoding());
        //TODO: handle conversion failure
    sal_Int32 i = args.indexOf('#');
    if (i < 0)
        std::abort();

    rtl::OUString rdb(args.copy(0, i));
    rtl::OUString regpath(args.copy(i + 1));
    rtl::OUString regurl;
    if (osl::FileBase::getFileURLFromSystemPath(regpath, regurl) !=
        osl::FileBase::E_None)
        std::abort();

    css::uno::Reference< css::beans::XPropertySet > factory(
        cppu::createRegistryServiceFactory(rdb), css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::uno::XComponentContext > context(
        factory->getPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext"))),
        css::uno::UNO_QUERY_THROW);
    cppu::ContextEntry_Init entry(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "/modules/com.sun.star.configuration/bootstrap/Strata")),
        css::uno::makeAny(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.comp.configuration.backend.LocalStratum:"))
            + regurl));
    mxContext = cppu::createComponentContext(&entry, 1, context);

    CPPUNIT_ASSERT_MESSAGE ("component context is valid", mxContext.is());

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

    delete mpMagic;
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

uno::Any Test::getKey (const sal_Char *pPath, rtl::OUString aName)
{
    uno::Reference< container::XHierarchicalNameAccess > xNameAccess(
        createView(pPath, false), css::uno::UNO_QUERY_THROW);
    uno::Any aVal;
    aVal = xNameAccess->getByHierarchicalName (aName);
    disposeComponent (xNameAccess);
    return aVal;
}

void Test::setKey (const sal_Char *pPath, rtl::OUString aName, css::uno::Any a)
{
    uno::Reference< util::XChangesBatch > xAppView;

    xAppView = uno::Reference< util::XChangesBatch > (
        createView(pPath, true), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameReplace > xSettings(xAppView, uno::UNO_QUERY_THROW);
    rtl::OUString aStr;

    // set key
    xSettings->replaceByName(aName, a);
    xAppView->commitChanges();

    disposeComponent(xAppView);
}

void Test::resetKey (const sal_Char *pPath, rtl::OUString aName)
{
    uno::Reference< util::XChangesBatch > xAppView;

    // reset to default
    xAppView = uno::Reference< util::XChangesBatch > ( createView(pPath, true), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameReplace > xSettings(xAppView, uno::UNO_QUERY_THROW);

    uno::Reference< beans::XPropertyState > xSettingsState(xSettings, uno::UNO_QUERY);
    xSettingsState->setPropertyToDefault(aName);
    xAppView->commitChanges();

    disposeComponent(xAppView);
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
                uno::makeAny (rtl::OUString::createFromAscii("baa")));

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
        resetKey ("/org.openoffice.Setup/Test",
                  rtl::OUString::createFromAscii("AString"));

        // check value
        rtl::OUString aStr;
        if (!(getKey ("/org.openoffice.Setup/Test", "AString") >>= aStr))
            CPPUNIT_FAIL("to fetch key");

        CPPUNIT_ASSERT_MESSAGE ("check default value valid",
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
        uno::Reference< container::XNameAccess > xNameAccess (
            createView("/org.openoffice.UI.GenericCommands/UserInterface/Commands", false),
            css::uno::UNO_QUERY_THROW);

        CPPUNIT_ASSERT_MESSAGE ("fetched UI generic commands", xNameAccess.is());

        uno::Any a;
        uno::Sequence< rtl::OUString > aNameSeq = xNameAccess->getElementNames();

        CPPUNIT_ASSERT_MESSAGE ("right element / sequence", aNameSeq.getLength() == 696);
        sal_uInt32 end, start = osl_getGlobalTimer();
        for ( sal_Int32 j = 0; j < 8; j++ )
        {
            for ( sal_Int32 i = 0; i < aNameSeq.getLength(); i++ )
            {
                try
                {
                    {
                        uno::Reference< container::XNameAccess > xChildNameAccess;
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
