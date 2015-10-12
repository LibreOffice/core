/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_UNOTEST_BOOTSTRAPFIXTUREBASE_HXX
#define INCLUDED_UNOTEST_BOOTSTRAPFIXTUREBASE_HXX

#include <sal/config.h>

#include <rtl/string.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <unotest/detail/unotestdllapi.hxx>

namespace test {

// Class to do lots of heavy-lifting UNO & environment
// bootstrapping for unit tests, such that we can use
// almost an entire LibreOffice during compile - so
// that we can get pieces of code alone to beat them up.

// NB. this class is instantiated multiple times during a
// run of unit tests ...
class OOO_DLLPUBLIC_UNOTEST BootstrapFixtureBase : public CppUnit::TestFixture
{
protected:
  OUString m_aSrcRootURL;
  OUString m_aSrcRootPath;
  OUString m_aWorkdirRootURL;
  OUString m_aWorkdirRootPath;

  com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext> m_xContext;
  com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory> m_xSFactory;
  com::sun::star::uno::Reference<com::sun::star::lang::XMultiComponentFactory> m_xFactory;

public:
  BootstrapFixtureBase();
  virtual ~BootstrapFixtureBase();

  com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>
	          getComponentContext() { return m_xContext; }
  com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>
	          getMultiServiceFactory() { return m_xSFactory; }

  OUString getSrcRootURL()       { return m_aSrcRootURL; }
  OUString getSrcRootPath()      { return m_aSrcRootPath; }

  // return a URL to a given c-str path from the source directory
  OUString getURLFromSrc( const char *pPath );
  OUString getURLFromSrc( const OUString& rPath );

  // return a Path to a given c-str path from the source directory
  OUString getPathFromSrc( const char *pPath );


  // return a URL to a given c-str path from the workdir directory
  OUString getURLFromWorkdir( const char *pPath );

#ifdef _WIN32
  // return a Path to a given c-str path from the workdir directory
  OUString getPathFromWorkdir( const char *pPath );
#endif
  virtual void setUp() override;
  virtual void tearDown() override;

};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
