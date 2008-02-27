#ifndef CPPUNIT_TEST_H
#define CPPUNIT_TEST_H

#include <cppunit/Portability.h>
#include <string>
#include <cppunit/autoregister/htestresult.h>

namespace CppUnit {

//# class TestResult;

/*! \brief Base class for all test objects.
 * \ingroup BrowsingCollectedTestResult
 *l-
 * All test objects should be a subclass of Test.  Some test objects,
 * TestCase for example, represent one individual test.  Other test
 * objects, such as TestSuite, are comprised of several tests.
 *
 * When a Test is run, the result is collected by a TestResult object.
 *
 * \see TestCase
 * \see TestSuite
 */
class CPPUNIT_API Test
{
public:
    virtual                       ~Test () {};

    /*! \brief Run the test, collecting results.
     */
    virtual void                  run (hTestResult pResult)    = 0;

    /*! \brief Return the number of test cases invoked by run().
     *
     * The base unit of testing is the class TestCase.  This
     * method returns the number of TestCase objects invoked by
     * the run() method.
     */
    virtual int                   countTestCases () const     = 0;

    /*! \brief Returns the test name.
     *
     * Each test has a name.  This name may be used to find the
     * test in a suite or registry of tests.
     */
    virtual std::string           getName () const            = 0;

    /*! \brief Description of the test, for diagnostic output.
     *
     * The test description will typically include the test name,
     * but may have additional description.  For example, a test
     * suite named <tt>complex_add</tt> may be described as
     * <tt>suite complex_add</tt>.
     */
    virtual std::string           toString () const           = 0;


};


} // namespace CppUnit

#endif // CPPUNIT_TEST_H

