#include <cppunit/result/SynchronizedObject.h>


namespace CppUnit
{




SynchronizedObject::SynchronizedObject( SynchronizationObject *syncObject )
    : m_syncObject( syncObject == 0 ? new SynchronizationObject() :
                                      syncObject )
{
}


SynchronizedObject::~SynchronizedObject()
{
  delete m_syncObject;
}


/** Accept a new synchronization object for protection of this instance
 * TestResult assumes ownership of the object
 */
void
SynchronizedObject::setSynchronizationObject( SynchronizationObject *syncObject )
{
  delete m_syncObject;
  m_syncObject = syncObject;
}


} //  namespace CppUnit

