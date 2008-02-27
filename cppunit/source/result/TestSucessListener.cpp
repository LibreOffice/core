#include <cppunit/result/TestSucessListener.h>



namespace CppUnit
{


TestSucessListener::TestSucessListener( SynchronizationObject *syncObject )
    : SynchronizedObject( syncObject )
    , m_sucess( true )
{
}


TestSucessListener::~TestSucessListener()
{
}


void
TestSucessListener::reset()
{
  ExclusiveZone zone( m_syncObject );
  m_sucess = true;
}


void
TestSucessListener::addFailure( const TestFailure & )
{
  ExclusiveZone zone( m_syncObject );
  m_sucess = false;
}


bool
TestSucessListener::wasSuccessful() const
{
  ExclusiveZone zone( m_syncObject );
  return m_sucess;
}


} //  namespace CppUnit

