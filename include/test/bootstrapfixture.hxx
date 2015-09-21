/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_TEST_BOOTSTRAPFIXTURE_HXX
#define INCLUDED_TEST_BOOTSTRAPFIXTURE_HXX

#include <sal/config.h>

#include <rtl/string.hxx>
#include <tools/link.hxx>
#include <vcl/salctype.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <unotest/bootstrapfixturebase.hxx>
#include <test/testdllapi.hxx>

namespace test {

enum ValidationFormat
{
    OOXML,
    ODF
};

// Class to do lots of heavy-lifting UNO & environment
// bootstrapping for unit tests, such that we can use
// almost an entire LibreOffice during compile - so
// that we can get pieces of code alone to beat them up.

// NB. this class is instantiated multiple times during a
// run of unit tests ...
class OOO_DLLPUBLIC_TEST BootstrapFixture : public BootstrapFixtureBase
{
  bool m_bNeedUCB;
  bool m_bAssertOnDialog;

public:
  DECL_STATIC_LINK_TYPED( BootstrapFixture, ImplInitFilterHdl, ConvertData&, bool );

  BootstrapFixture( bool bAssertOnDialog = true, bool bNeedUCB = true );
  virtual ~BootstrapFixture();

  virtual void setUp() SAL_OVERRIDE;
  virtual void tearDown() SAL_OVERRIDE;

  static void validate(const OUString& rURL, ValidationFormat);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
