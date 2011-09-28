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
 *   Copyright (C) 2011 Michael Meeks <michael.meeks@suse.com>
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
#ifndef INCLUDED_TEST_BOOTSTRAPFIXTURE_HXX
#define INCLUDED_TEST_BOOTSTRAPFIXTURE_HXX

#include <sal/config.h>

#include <rtl/string.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include "sal/precppunit.hxx"
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include "test/testdllapi.hxx"

namespace test {

// Class to do lots of heavy-lifting UNO & environment
// bootstrapping for unit tests, such that we can use
// almost an entire LibreOffice during compile - so
// that we can get pieces of code alone to beat them up.
class OOO_DLLPUBLIC_TEST BootstrapFixture : public CppUnit::TestFixture
{
  ::rtl::OUString m_aSrcRootURL;
  ::rtl::OUString m_aSrcRootPath;

  com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext> m_xContext;
  com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory> m_xSFactory;
  com::sun::star::uno::Reference<com::sun::star::lang::XMultiComponentFactory> m_xFactory;

public:
  BootstrapFixture( bool bAssertOnDialog = true );
  virtual ~BootstrapFixture();

  com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>
	          getComponentContext() { return m_xContext; }
  com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>
	          getMultiServiceFactory() { return m_xSFactory; }

  ::rtl::OUString getSrcRootURL()       { return m_aSrcRootURL; }
  ::rtl::OUString getSrcRootPath()      { return m_aSrcRootPath; }

  // return a URL to a given c-str path from the source directory
  ::rtl::OUString getURLFromSrc( const char *pPath );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
