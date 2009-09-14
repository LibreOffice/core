/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
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

#ifndef SIGNALTEST_H
#define SIGNALTEST_H

#include <string>
#include "cppunit/Test.h"
#include <testshl/autoregister/htestresult.h>

namespace CppUnit
{

class CPPUNIT_API SignalTest : public Test
{
public:

    SignalTest( std::string const& Name );

    ~SignalTest();

    virtual void run(hTestResult pResult);
    virtual int countTestCases() const;
    std::string getName() const;
    std::string toString() const;

private:
    SignalTest( const SignalTest &other );
    SignalTest &operator=( const SignalTest &other );

private:
    const std::string m_sName;
};

} // namespace CppUnit


#endif
