#ifndef CPPUNIT_TESTCASE_H
#define CPPUNIT_TESTCASE_H

#include <cppunit/Portability.h>
#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <string>
#include <cppunit/autoregister/htestresult.h>

namespace CppUnit {

//# class TestResult;

/*! \brief A single test object.
 *
 * This class is used to implement a simple test case: define a subclass
 * that overrides the runTest method.
 *
 * You don't usually need to use that class, but TestFixture and TestCaller instead.
 *
 * You are expected to subclass TestCase is you need to write a class similiar
 * to TestCaller.
 */
class CPPUNIT_API TestCase : public Test,
                             public TestFixture
{
public:

    TestCase( std::string const& Name );
    //! \internal
    TestCase();
    ~TestCase();

    virtual void run(hTestResult pResult);
    virtual int countTestCases() const;
    std::string getName() const;
    std::string toString() const;

    //! FIXME: what is this for?
    //# virtual TestResult *run();

protected:
    //! FIXME: this should probably be pure virtual.
    virtual void runTest();

    //! Create TestResult for the run(void) method.
    //# TestResult *defaultResult();

private:
    TestCase( const TestCase &other );
    TestCase &operator=( const TestCase &other );

private:
    const std::string m_name;
};

} // namespace CppUnit

#endif // CPPUNIT_TESTCASE_H
