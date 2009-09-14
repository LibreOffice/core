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

#include <stdlib.h>

#include <map>
#include <cppunit/Exception.h>
#include <cppunit/NotEqualException.h>
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <testshl/result/emacsTestResult.hxx>
// #include <TextTestResult.h>
//!io #include <iostream>
#include <string>

#include <rtl/string.hxx>
#include "testshl/result/outputter.hxx"

namespace CppUnit {

    emacsTestResult::emacsTestResult(GetOpt & _aOptions)
            :TestResult(_aOptions),
             m_aOptions(_aOptions),
             m_aResulter(this)
    {
        addListener( &m_aResulter );
    }

    //# struct ltstr
    //# {
    //#     bool operator()(const CppUnit::Test* p1, const CppUnit::Test* p2) const
    //#         {
    //#             return p1 < p2;
    //#         }
    //# };
    //# typedef std::map<CppUnit::Test*, bool, ltstr> TestPtrList;

    void
    emacsTestResult::print( Outputter& stream )
    {
        printHeader( stream );
        // stream << std::endl;
        // printFailures( stream );

        // TestPtrList aFailedTests;

        for (TestResultCollector::TestFailures::const_iterator it2 = m_aResulter.failures().begin();
             it2 != m_aResulter.failures().end();
             ++it2)
        {
            TestFailureEnvelope *pEnvelop = *it2;
            TestFailure *pFailure = pEnvelop->getTestFailure();
            std::string sNodeName = pEnvelop->getString();

            // aFailedTests[ pFailure->failedTest() ] = true;

            printFailureLine(stream, pFailure, sNodeName);
        }

        // only errors are from interest here
        //# for (TestResultCollector::Tests::const_iterator it = m_aResulter.tests().begin();
        //#      it != m_aResulter.tests().end();
        //#      ++it)
        //# {
        //#     TestEnvelope *pEnvelop = *it;
        //#     Test* pTest = pEnvelop->getTest();
        //#     std::string sNodeName = pEnvelop->getString();
        //#
        //#     if (aFailedTests.find(pTest) == aFailedTests.end())
        //#     {
        //#         std::string sInfo = m_aResulter.getInfo(pTest);
        //#         printTestLine(stream, pTest, sNodeName, sInfo);
        //#     }
        //# }
        stream << "Test #PASSED#" << Outputter::endl();
    }


    void
    emacsTestResult::printHeader( Outputter &stream )
    {
    std::string sDate(m_aOptionHelper.createDateTag());
    stream << sDate << Outputter::endl();
    }

    void
    emacsTestResult::printFailureLine( Outputter &stream, TestFailure *_pFailure, std::string const& _sNodeName)
    {
    std::string aName;
    aName += _sNodeName;
    aName += ".";
    aName += _pFailure->failedTestName();

    SourceLine aLine = _pFailure->sourceLine();
    sal_Int32 nLine = -1;
    std::string sFilename;
    if (aLine.isValid())
    {
            nLine = aLine.lineNumber();
            sFilename = aLine.fileName();
    }

    Exception  *pExp  = _pFailure->thrownException();
    std::string sWhat;
    if (pExp)
    {
            sWhat = pExp->what();
    }

    stream << sFilename;
    stream << ":";
    stream << nLine;
    stream << ":";

    stream << aName;
    stream << ";#FAILED#";
    // ErrorType::num eErr = _pFailure->getErrorType();
    //# if (isErr)
        //#     stream << "FAILED#;";
    //# else
        //#     stream << "ERROR#;";

    stream << sWhat;
    stream << Outputter::endl();
    }

    //# void
    //# emacsTestResult::printTestLine( Outputter &stream, Test* _pTest, std::string const& _sNodeName, std::string const& _sInfo)
    //# {
    //#     std::string aName;
    //#     aName += _sNodeName;
    //#     aName += ".";
    //#     aName += _pTest->getName();
    //#
    //#     stream << aName;
    //#     stream << ";";
    //#     stream << _sInfo << "#";
    //#     stream << "OK#";
    //#     stream << std::endl;
    //# }


//# void
//# emacsTestResult::printStatistics( Outputter &stream )
//# {
//#     stream  << "Test Results:" << std::endl;
//#
//#     stream  <<  "Run:  "  <<  runTests()
//#             <<  "   Failures: "  <<  testFailures()
//#             <<  "   Errors: "  <<  testErrors()
//#             <<  std::endl;
//# }


    Outputter &
    operator <<( Outputter &stream,
                 emacsTestResult &result )
    {
        result.print (stream); return stream;
    }


} // namespace CppUnit
