#ifndef _PYUNO_PYUNO_HXX_
#define _PYUNO_PYUNO_HXX_

#ifndef Py_PYTHON_H

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif // #ifdef _DEBUG
#endif // #ifdef Py_PYTHON_H

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _COM_SUN_STAR_SCRIPT_CANNOTCONVERTEXCEPTION_HPP_
#include <com/sun/star/script/CannotConvertException.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

/**
   External interface of the Python UNO bridge.

   This is a C++ interface, because the core UNO components
   invocation and proxyfactory are used to implement the bridge.

   This interface is somewhat private and my change in future.

   A scripting framework implementation may use this interface
   to do the necessary conversions.
*/

#ifdef WIN32
#define PY_DLLEXPORT __declspec(dllexport)
#else
#define PY_DLLEXPORT
#endif

/** function called by the python runtime to initialize the
    pyuno module.

    preconditions: python has been initialized before and
                   the global interpreter lock is held
*/
extern "C" PY_DLLEXPORT void SAL_CALL initpyuno();


namespace pyuno
{

/** Helper class for keeping references to python objects.
    BEWARE: Look up every python function you use to check
    wether you get an acquired or not acquired object pointer
    (python terminus for a not acquired object pointer
    is 'borrowed reference'). Use in the acquired pointer cases the
    PyRef( pointer, SAL_NO_ACQUIRE) ctor.

    precondition: python has been initialized before and
    the global interpreter lock is held

*/
class PyRef
{
    PyObject *m;
public:
    PyRef () : m(0) {}
    PyRef( PyObject * p ) : m( p ) { Py_XINCREF( m ); }

    PyRef( PyObject * p, __sal_NoAcquire ) : m( p ) {}

    PyRef( const PyRef &r ) : m( r.get() ) { Py_XINCREF( m ); }

    ~PyRef() { Py_XDECREF( m ); }

    PyObject *get() const { return m; }

    PyObject * getAcquired() const
    {
        Py_XINCREF( const_cast< PyObject*> (m) );
        return m;
    }

    PyRef & operator = (  const PyRef & r )
    {
        PyObject *tmp = m;
        m = r.getAcquired();
        Py_XDECREF( tmp );
        return *this;
    }

    bool operator == (  const PyRef & r ) const
    {
        return r.get() == m;
    }

    /** clears the reference without decreasing the reference count
        only seldomly needed ! */
    void scratch()
    {
        m = 0;
    }

    /** clears the reference decreasing the refcount of the holded object.
     */
    void clear()
    {
        Py_XDECREF( m );
        m = 0;
    }

    /** returns 1 when the reference points to a python object python object,
        otherwise 0.
    */
    sal_Bool is() const
    {
        return m != 0;
    }

    struct Hash
    {
        int operator () ( const PyRef &r) const { return (int) r.get(); }
    };
};

struct stRuntimeImpl;
typedef struct stRuntimeImpl RuntimeImpl;


/** The pyuno::Runtime class keeps the internal state of the python UNO bridge
    for the currently in use python interpreter.

    You may keep a Runtime instance, use it from a different thread, etc. But you must
    make sure to fulfill all preconditions mentioned for the specific methods.
*/

class PY_DLLEXPORT Runtime
{
    RuntimeImpl *impl;
public:
    ~Runtime( );

    /**
        preconditions: python has been initialized before,
        the global interpreter lock is held and pyuno
        has been initialized for the currently used interpreter.

        Note: This method exists for efficiency reasons to save
        lookup costs for any2PyObject and pyObject2Any

        @throw RuntimeException in case the runtime has not been
               initialized before
     */
    Runtime() throw( com::sun::star::uno::RuntimeException );

    Runtime( const Runtime & );
    Runtime & operator = ( const Runtime & );

    /** Initializes the python-UNO bridge. May be called only once per python interpreter.

        @param ctx the component context is used to instantiate bridge services needed
        for bridging such as invocation, typeconverter, invocationadapterfactory, etc.

        preconditions: python has been initialized before and
        the global interpreter lock is held and pyuno is not
        initialized (see isInitialized() ).

        @throw RuntimeException in case the thread is not attached or the runtime
                                has not been initialized.
    */
    static void SAL_CALL initialize(
        const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > & ctx )
        throw ( com::sun::star::uno::RuntimeException );


    /** Checks, whether the uno runtime is already initialized in the current python interpreter.
     */
    static bool SAL_CALL isInitialized() throw (com::sun::star::uno::RuntimeException);


    /** disposes the UNO bridge in this interpreter. All existing stubs/proxies
        become non-functional, using these proxies/stubs leads to runtime errors.

        preconditions: python has been initialized before and
        the global interpreter lock is held and pyuno was
        initialized before for the currently in use interpreter.
    */
    static void SAL_CALL finalize() throw(com::sun::star::uno::RuntimeException );

    /** converts something contained in an UNO Any to a Python object

        preconditions: python has been initialized before,
        the global interpreter lock is held and pyuno::Runtime
        has been initialized.
    */
    PyRef any2PyObject (const com::sun::star::uno::Any &source ) const
        throw ( com::sun::star::script::CannotConvertException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::uno::RuntimeException );

    /** converts a Python object to a UNO any

        preconditions: python has been initialized before,
        the global interpreter lock is held and pyuno
        has been initialized
    */
    com::sun::star::uno::Any pyObject2Any ( const PyRef & source ) const
        throw ( com::sun::star::uno::RuntimeException);


    /** Returns the internal handle. Should only be used by the module implementation
     */
    RuntimeImpl *getImpl() const { return impl; }
};


/** helper class for attaching the current thread to the python runtime.

    Attaching is done creating a new threadstate for the given interpreter
    and acquiring the global interpreter lock.

    Usage:

    ... don't use python here
    {
        PyThreadAttach guard( PyInterpreterState_Head() );
        {
            ... do whatever python code you want
            {
               PyThreadDetach antiguard;
               ... don't use python here
            }
            ... do whatever python code you want
        }
    }
    ... don't use python here

    Note: The additional scope brackets after the PyThreadAttach are needed,
          e.g. when you would leave them away, dtors of potential pyrefs
          may be called after the thread has detached again.
 */
class PY_DLLEXPORT PyThreadAttach
{
    PyThreadState *tstate;
    PyThreadAttach ( const PyThreadAttach & ); // not implemented
    PyThreadAttach & operator = ( const PyThreadAttach & );
public:

    /** Creates a new python threadstate and acquires the global interpreter lock.
        precondition: The current thread MUST NOT hold the global interpreter lock.
        postcondition: The global interpreter lock is acquired

        @raises com::sun::star::uno::RuntimeException
             in case no pythread state could be created
     */
    PyThreadAttach( PyInterpreterState *interp) throw ( com::sun::star::uno::RuntimeException );


    /** Releases the global interpreter lock and destroys the thread state.
     */
    ~PyThreadAttach();
};

/** helper class for detaching the current thread from the python runtime
    to do some blocking, non-python related operation.

    @see PyThreadAttach
*/
class PY_DLLEXPORT PyThreadDetach
{
    PyThreadState *tstate;
    PyThreadDetach ( const PyThreadDetach & ); // not implemented
    PyThreadDetach & operator = ( const PyThreadDetach & ); // not implemented

public:
    /** Releases the global interpreter lock.

       precondition: The current thread MUST hold the global interpreter lock.
       postcondition: The current thread does not hold the global interpreter lock anymore.
    */
    PyThreadDetach() throw ( com::sun::star::uno::RuntimeException );
    /** Acquires the global interpreter lock again
    */
    ~PyThreadDetach();
};

}
#endif
