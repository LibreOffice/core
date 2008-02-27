/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: emacsTestResult.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:15:24 $
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

#ifndef cppunit_emacstestresult_h
#define cppunit_emacstestresult_h

#include <cppunit/result/TestResult.h>
#include <cppunit/result/TestResultCollector.h>
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


