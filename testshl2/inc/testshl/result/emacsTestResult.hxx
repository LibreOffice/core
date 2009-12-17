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

#ifndef cppunit_emacstestresult_h
#define cppunit_emacstestresult_h

#include <testshl/result/TestResult.h>
#include <testshl/result/TestResultCollector.h>
#include <ostream>
#include "testshl/getopt.hxx"

class Outputter;
namespace CppUnit {

class SourceLine;
class Exception;
class Test;

/*! \brief Holds printable test result (DEPRECATED).
 * \ingroup TrackingTestExecution
 *
 * deprecated Use class TextTestProgressListener and TextOutputter instead.
 */
    class CPPUNIT_API emacsTestResult : public TestResult

/* ,public TestResultCollector*/
    {
        GetOpt & m_aOptions;
        // OptionHelper m_aOptionHelper;
        TestResultCollector m_aResulter;

    public:
        emacsTestResult(GetOpt & _aOptions);

        // virtual void addFailure( const TestFailure &failure );
        // virtual void startTest( Test *test );
        // virtual void endTest( Test *test );

        virtual void print( Outputter &stream );

    protected:
        virtual void printHeader( Outputter &stream );
        // virtual void printTestLine( Outputter &stream, Test* pTest, std::string const& _sNodeName, std::string const& _sInfo);
        virtual void printFailureLine( Outputter &stream, TestFailure* pFailure, std::string const& _sNodeName );
    };

/** insertion operator for easy output */
//    std::ostream &operator <<( std::ostream &stream,
//                               emacsTestResult &result );

} // namespace CppUnit

#endif // CPPUNIT_testshlTESTRESULT_H


