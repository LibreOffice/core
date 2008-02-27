#ifndef CPPUNIT_SYNCHRONIZEDOBJECT_H
#define CPPUNIT_SYNCHRONIZEDOBJECT_H

#include <cppunit/Portability.h>
#include <cppunit/nocopy.hxx>

namespace CppUnit
{

/*! \brief Base class for synchronized object.
 *
 * Synchronized object are object which members are used concurrently by mutiple
 * threads.
 *
 * This class define the class SynchronizationObject which must be subclassed
 * to implement an actual lock.
 *
 * Each instance of this class holds a pointer on a lock object.
 *
 * See src/msvc6/MfcSynchronizedObject.h for an example.
 */
class CPPUNIT_API SynchronizedObject
{
public:
  /*! \brief Abstract synchronization object (mutex)
   */
  class SynchronizationObject
  {
    public:
      SynchronizationObject() {}
      virtual ~SynchronizationObject() {}

      virtual void lock() {}
      virtual void unlock() {}
  };

  /*! Constructs a SynchronizedObject object.
   */
  SynchronizedObject( SynchronizationObject *syncObject =0 );

  /// Destructor.
  virtual ~SynchronizedObject();

protected:
  /*! \brief Locks a synchronization object in the current scope.
   */
    class ExclusiveZone : NOCOPY
  {
    SynchronizationObject *m_syncObject;

  public:
    ExclusiveZone( SynchronizationObject *syncObject )
        : m_syncObject( syncObject )
    {
      m_syncObject->lock();
    }

    ~ExclusiveZone()
    {
      m_syncObject->unlock ();
    }
  };

  virtual void setSynchronizationObject( SynchronizationObject *syncObject );

protected:
  SynchronizationObject *m_syncObject;

private:
  /// Prevents the use of the copy constructor.
  SynchronizedObject( const SynchronizedObject &copy );

  /// Prevents the use of the copy operator.
  void operator =( const SynchronizedObject &copy );
};



} //  namespace CppUnit


#endif  // CPPUNIT_SYNCHRONIZEDOBJECT_H
