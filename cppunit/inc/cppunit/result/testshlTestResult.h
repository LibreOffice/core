#ifndef cppunit_testshltestresult_h
#define cppunit_testshltestresult_h

#include <map>
#include <cppunit/result/TestResult.h>
#include <cppunit/result/TestResultCollector.h>
#include <ostream>
#include "testshl/getopt.hxx"

class Outputter;

namespace CppUnit {

class SourceLine;
class Exception;
class Test;

struct ltstr
{
    bool operator()(const CppUnit::Test* p1, const CppUnit::Test* p2) const
        {
            return p1 < p2;
        }
};
typedef std::map<CppUnit::Test*, bool, ltstr> TestPtrList;


/*! \brief Holds printable test result (DEPRECATED).
 * \ingroup TrackingTestExecution
 *
 * deprecated Use class TextTestProgressListener and TextOutputter instead.
 */
    class CPPUNIT_API testshlTestResult : public TestResult

/* ,public TestResultCollector*/
    {
        GetOpt & m_aOptions;
        // OptionHelper m_aOptionHelper;
        TestResultCollector m_aResulter;

    public:
        testshlTestResult(GetOpt & _aOptions);
        virtual ~testshlTestResult();

        // virtual void addFailure( const TestFailure &failure );
        // virtual void startTest( Test *test );
        // virtual void endTest( Test *test );

        virtual void print( Outputter &stream );

    protected:
        virtual void printHeader( Outputter &stream );

        void printLines(Outputter &stream, HashMap & _aJobList);
        void printFailedTests(Outputter &stream, TestPtrList &aFailedTests);
        void printTestLines(Outputter &stream, TestPtrList &aFailedTests);
        void printUnknownLines(Outputter &stream, HashMap & _aJobList);

        virtual void printTestLine( Outputter &stream, Test* pTest, std::string const& _sNodeName, std::string const& _sInfo);
        virtual void printFailureLine( Outputter &stream, TestFailure* pFailure, std::string const& _sNodeName );
        virtual void printUnknownLine( Outputter &stream, std::string const& _sTestName);
    };

/** insertion operator for easy output */
    Outputter &operator <<( Outputter &stream,
                            testshlTestResult &result );

} // namespace CppUnit

#endif // CPPUNIT_testshlTESTRESULT_H


