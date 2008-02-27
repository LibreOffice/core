#ifndef CPPUNIT_TESTSUCESSLISTENER_H
#define CPPUNIT_TESTSUCESSLISTENER_H

#include <cppunit/result/SynchronizedObject.h>
#include <cppunit/result/TestListener.h>


namespace CppUnit
{

/*! \brief TestListener that checks if any test case failed.
 * \ingroup TrackingTestExecution
 */
class CPPUNIT_API TestSucessListener : public TestListener,
                                       public SynchronizedObject
{
public:
  /*! Constructs a TestSucessListener object.
   */
  TestSucessListener( SynchronizationObject *syncObject = 0 );

  /// Destructor.
  virtual ~TestSucessListener();

  virtual void reset();

  void addFailure( const TestFailure &failure );

  /// Returns whether the entire test was successful or not.
  virtual bool wasSuccessful() const;

private:
  bool m_sucess;
};


} //  namespace CppUnit


#endif  // CPPUNIT_TESTSUCESSLISTENER_H
