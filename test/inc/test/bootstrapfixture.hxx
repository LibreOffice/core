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
#include <tools/link.hxx>
#include <vcl/salctype.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include "unotest/bootstrapfixturebase.hxx"
#include "test/testdllapi.hxx"

namespace test {

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
  DECL_LINK( ImplInitFilterHdl, ConvertData* );

public:
  BootstrapFixture( bool bAssertOnDialog = true, bool bNeedUCB = true );
  virtual ~BootstrapFixture();

  virtual void setUp();
  virtual void tearDown();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
