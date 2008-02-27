/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testshlTestResult.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:26:38 $
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

#include <stdlib.h>

#include <map>
#include <cppunit/Exception.h>
#include <cppunit/NotEqualException.h>
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/result/testshlTestResult.h>
// #include <TextTestResult.h>
//!io #include <iostream>
#include <string>

#include <rtl/string.hxx>
#include "cppunit/result/outputter.hxx"

namespace CppUnit {

    testshlTestResult::testshlTestResult(GetOpt & _aOptions)
            :TestResult(_aOptions),
             m_aOptions(_aOptions),
             // m_aOptionHelper(_aOptions),
             m_aResulter(this)
    {
        addListener( &m_aResulter );
    }

    testshlTestResult::~testshlTestResult()
    {
    }

//# void
//# testshlTestResult::addFailure( const TestFailure &failure )
//# {
//#     // TestResultCollector::addFailure( failure );
//#     // std::cerr << ( failure.isError() ? "E" : "F" );
//#     m_aResulter.addFailure(failure);
//# }
//#
//#
//# void
//# testshlTestResult::startTest( Test *test )
//# {
//#     // TestResultCollector::startTest (test);
//#     m_aResulter.startTest(test);
//#     // std::cerr << ".";
//# }
//#
//# void testshlTestResult::endTest( Test *test )
//# {
//# }

// -----------------------------------------------------------------------------

//# void
//# testshlTestResult::printFailures( Outputter &stream )
//# {
//#   TestFailures::const_iterator itFailure = failures().begin();
//#   int failureNumber = 1;
//#   while ( itFailure != failures().end() )
//#   {
//#       stream  <<  std::endl;
//#       printFailure( *itFailure++, failureNumber++, stream );
//#   }
//# }
//#
//#
//# void
//# testshlTestResult::printFailure( TestFailure *failure,
//#                               int failureNumber,
//#                               std::ostream &stream )
//# {
//#   printFailureListMark( failureNumber, stream );
//#   stream << ' ';
//#   printFailureTestName( failure, stream );
//#   stream << ' ';
//#   printFailureType( failure, stream );
//#   stream << ' ';
//#   printFailureLocation( failure->sourceLine(), stream );
//#   stream << std::endl;
//#   printFailureDetail( failure->thrownException(), stream );
//#   stream << std::endl;
//# }
//#
//#
//# void
//# testshlTestResult::printFailureListMark( int failureNumber,
//#                                       std::ostream &stream )
//# {
//#     stream << failureNumber << ")";
//# }
//#
//#
//# void
//# testshlTestResult::printFailureTestName( TestFailure *failure,
//#                                       std::ostream &stream )
//# {
//#     Test* pTest = failure->failedTest();
//#     stream << "test: " << pTest->getName();
//# }
//#
//#
//# void
//# testshlTestResult::printFailureType( TestFailure *failure,
//#                                   std::ostream &stream )
//# {
//#     stream << "("
//#          << (failure->isError() ? "E" : "F")
//#          << ")";
//# }
//#
//#
//# void
//# testshlTestResult::printFailureLocation( SourceLine sourceLine,
//#                                       std::ostream &stream )
//# {
//#     if ( !sourceLine.isValid() )
//#         return;
//#
//#     stream << "line: " << sourceLine.lineNumber()
//#            << ' ' << sourceLine.fileName();
//# }
//#
//#
//# void
//# testshlTestResult::printFailureDetail( Exception *thrownException,
//#                                     std::ostream &stream )
//# {
//#     if ( thrownException->isInstanceOf( NotEqualException::type() ) )
//#     {
//#         NotEqualException *e = (NotEqualException*)thrownException;
//#         stream << "expected: " << e->expectedValue() << std::endl
//#                << "but was:  " << e->actualValue();
//#         if ( !e->additionalMessage().empty() )
//#         {
//#             stream  << std::endl;
//#             stream  <<  "additional message:"  <<  std::endl
//#                     <<  e->additionalMessage();
//#         }
//#     }
//#     else
//#     {
//#         stream << " \"" << thrownException->what() << "\"";
//#     }
//# }


// LLA: output format:
// Header
// # -- BEGIN
// Error lines
// No error lines
// unknown lines
// # -- END
// Text 'Test #PASSED#' or 'Errors only'

void
testshlTestResult::print( Outputter& stream )
{
    printHeader( stream );
    // stream << std::endl;
    // printFailures( stream );

    bool bPassed = false;
    if (isOnlyShowJobs())
    {
        JobList jl(m_aOptionHelper.getJobOnlyList());
        HashMap aJobList = jl.getHashMap();
        printLines(stream, aJobList);
    }
    else
    {
        stream << "# -- BEGIN:" << Outputter::endl();

        TestPtrList aFailedTests;

        // List of failures
        printFailedTests(stream, aFailedTests);

        if (! m_aOptionHelper.getOptions().hasOpt("-onlyerrors"))
        {
        // List of well done tests
        printTestLines(stream, aFailedTests);

        // List of unknown Tests
        JobList jl(m_aOptionHelper.getJobOnlyList());
        HashMap aJobList = jl.getHashMap();
        printUnknownLines(stream, aJobList);

            bPassed = true;
        }
        stream << "# -- END:" << Outputter::endl();

        if (bPassed)
        {
        stream << "Test #PASSED#" << Outputter::endl();
        }

        if (m_aOptionHelper.getOptions().hasOpt("-onlyerrors"))
        {
            stream << "Errors only." << Outputter::endl();
        }
    }
}

// -----------------------------------------------------------------------------
void
testshlTestResult::printLines(Outputter &stream, HashMap & _aJobList)
{
    int nCount = _aJobList.size();
    if (nCount != 0)
    {
        for (HashMap::const_iterator it = _aJobList.begin();
             it != _aJobList.end();
             ++it)
        {
            std::string aKey = (*it).first;
            if (_aJobList[aKey] == JOB_ACCESS)
            {
                stream << aKey;
                stream << Outputter::endl();
            }
        }
    }
}

void
testshlTestResult::printFailedTests(Outputter &stream, TestPtrList &aFailedTests)
{
    for (TestResultCollector::TestFailures::const_iterator it2 = m_aResulter.failures().begin();
         it2 != m_aResulter.failures().end();
         ++it2)
    {
        TestFailureEnvelope *pEnvelop = *it2;
        TestFailure *pFailure = pEnvelop->getTestFailure();
        std::string sNodeName = pEnvelop->getString();

        aFailedTests[ pFailure->failedTest() ] = true;

        printFailureLine(stream, pFailure, sNodeName);
    }
}

void
testshlTestResult::printTestLines(Outputter &stream, TestPtrList &aFailedTests)
{
    for (TestResultCollector::Tests::const_iterator it = m_aResulter.tests().begin();
         it != m_aResulter.tests().end();
         ++it)
    {
        TestEnvelope *pEnvelop = *it;
        Test* pTest = pEnvelop->getTest();
        std::string sNodeName = pEnvelop->getString();

        if (aFailedTests.find(pTest) == aFailedTests.end())
        {
            std::string sInfo = m_aResulter.getInfo(pTest);
            printTestLine(stream, pTest, sNodeName, sInfo);
        }
    }
}

void
testshlTestResult::printUnknownLines(Outputter &stream, HashMap & _aJobList)
{
    int nCount = _aJobList.size();
    if (nCount != 0)
    {
        for (HashMap::const_iterator it = _aJobList.begin();
             it != _aJobList.end();
             ++it)
        {
            std::string aKey = (*it).first;
            if (_aJobList[aKey] == JOB_UNKNOWN)
            {
                printUnknownLine(stream, aKey);
            }
        }
    }
}

void
testshlTestResult::printHeader( Outputter &stream )
{
    std::string sDate(m_aOptionHelper.createDateTag());
    if (isOnlyShowJobs())
    {
            stream << "# This is an automatically generated job file." << Outputter::endl();
            stream << "# ";
    }
    else
    {
        stream << sDate << Outputter::endl();
    }
}

void
testshlTestResult::printFailureLine( Outputter &stream, TestFailure *_pFailure, std::string const& _sNodeName)
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

    ErrorType::num eErr = _pFailure->getErrorType();

    stream << aName;
    stream << ";";

    if (eErr == ErrorType::ET_FAILURE)
        stream << "FAILED";

    else if (eErr == ErrorType::ET_ERROR)
        stream << "ERROR";

    else if (eErr == ErrorType::ET_SIGNAL)
        stream << "SIGNAL";

    else
        stream << "UNKNOWN";

    stream << "#FAILED#";

    stream << sWhat;
    stream << Outputter::endl();
}

void
testshlTestResult::printTestLine( Outputter &stream, Test* _pTest, std::string const& _sNodeName, std::string const& _sInfo)
{
    std::string aName;
    aName += _sNodeName;
    aName += ".";
    aName += _pTest->getName();

    stream << aName;
    stream << ";";
    stream << _sInfo << "#";
    if (_sInfo == "PASSED")
    {
        stream << "OK#";
    }
    else
    {
        stream << "FAILED#";
    }
    stream << Outputter::endl();
}

void
testshlTestResult::printUnknownLine( Outputter &stream, std::string const& _sTestName)
{
    stream << _sTestName;
    stream << ";";
    stream << "UNKNOWN#";
    stream << "FAILED#";
    stream << Outputter::endl();
}

//# void
//# testshlTestResult::printStatistics( Outputter &stream )
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
             testshlTestResult &result )
{
    result.print (stream); return stream;
}


} // namespace CppUnit
