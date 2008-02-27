/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: signaltest.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:24:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppunit.hxx"

#include <typeinfo>

#include "cppunit/signaltest.h"

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
