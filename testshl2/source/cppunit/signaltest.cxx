/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
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

// MARKER(update_precomp.py): autogen include statement, do not remove
// #include "precompiled_cppunit.hxx"

#include <typeinfo>

#include "testshl/signaltest.h"

namespace CppUnit
{

/** Constructs a test case for a suite.
 *  This TestCase is intended for use by the TestCaller and should not
 *  be used by a test case for which run() is called.
 **/
SignalTest::SignalTest(std::string const& _sName)
    : m_sName( _sName )
{
}


/// Destructs a test case
SignalTest::~SignalTest()
{
}


/// Returns a count of all the tests executed
int
SignalTest::countTestCases() const
{
  return 1;
}

/// We never need to start this
void SignalTest::run(hTestResult) {}


/// Returns the name of the test case
std::string
SignalTest::getName() const
{
  return m_sName;
}


/// Returns the name of the test case instance
std::string
SignalTest::toString() const
{
  std::string className;

#if CPPUNIT_USE_TYPEINFO_NAME
  const std::type_info& thisClass = typeid( *this );
  className = thisClass.name();
#else
  className = "SignalTest";
#endif

  return className + "." + getName();
}

} // namespace CppUnit
