/*************************************************************************
 *
 *  $RCSfile: interfacecontainer.h,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:15:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#define _CPPUHELPER_INTERFACECONTAINER_H_

#include <hash_map>
#include <functional>

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_EVENTOBJECT_HXX_
#include <com/sun/star/lang/EventObject.hpp>
#endif


/** */ //for docpp
namespace cppu
{

//===================================================================
class OInterfaceContainerHelper;
/**
  This is the iterator of a InterfaceContainerHelper. The iterator
  ist is not thread save. It is not allowed to assign or copy an
  instance of this class.

  @version      0.1
  @author       Markus Meyer
  @since        03/12/98
  @see OInterfaceContainerHelper
 */
class OInterfaceIteratorHelper
{
public:
    OInterfaceIteratorHelper() SAL_THROW( () );
    /**
       Create an iterator over the elements of the container. The iterator
       copies the elements of the conatainer. A change to the container does not
       affect the iterator.<BR>
       Remark: The copy is on demand. The iterator copy the elements only if the container
       change the contents. It is not allowed to destroy the container if a iterator exist.

       @param rCont the container of the elements.
     */
    OInterfaceIteratorHelper( OInterfaceContainerHelper & rCont ) SAL_THROW( () );

    /**
      Release the connection to the container.
     */
    ~OInterfaceIteratorHelper() SAL_THROW( () );

    /** Return true, if there are more elements in the iterator. */
    sal_Bool SAL_CALL hasMoreElements() const SAL_THROW( () )
        { return nRemain != 0; }
    /** Return the next element of the iterator. Calling this method if
        hasMoreElements return false, is an error.
     */
    ::com::sun::star::uno::XInterface * SAL_CALL next() SAL_THROW( () );

    /** Removes the current element (the last one returned by next())
        from the underlying container. Calling this method before
        next() has been called or calling it twice with no next()
        inbetween is an error.
        @since udk211 (has been added in udk210)
    */
    void SAL_CALL remove() SAL_THROW( () );

private:
    OInterfaceContainerHelper & rCont;
    sal_Bool                    bIsList;
    void *                      pData;
    sal_Int32                   nRemain;

    OInterfaceIteratorHelper( const OInterfaceIteratorHelper & ) SAL_THROW( () );
    OInterfaceIteratorHelper &  operator = ( const OInterfaceIteratorHelper & ) SAL_THROW( () );
};

//===================================================================
/**
  A container of interfaces. To access the elements use an iterator.
  This implementation is thread save.<BR>
  <B>Inserting null pointers is allowed, but is not tested and does not work.</B>

  @version      0.1
  @author       Markus Meyer
  @since        03/12/98
  @see OInterfaceIteratorHelper
 */
class OInterfaceContainerHelper
{
public:
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW( () )
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW( () )
        {}

    /**
       Create an interface container. The internal representation
       is an array, so it is not effective to store a large number
       of elements.

       @param rMutex    the mutex to protect multi thread access.
                         The lifetime must be longer than the lifetime
                         of this object.
     */
    OInterfaceContainerHelper( ::osl::Mutex & rMutex ) SAL_THROW( () );
    /**
      Release all interfaces. All iterators must be destroyed before
      the container.
     */
    ~OInterfaceContainerHelper() SAL_THROW( () );
    /**
      Return the number of Elements in the container. Only usefull if you are acquire
      the mutex.
     */
    sal_Int32 SAL_CALL getLength() const SAL_THROW( () );

    /**
      Return all interfaces added to this container.
     **/
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > > SAL_CALL getElements() const SAL_THROW( () );

    /**
      Insert an element in the container. The position is not specified.
      @param rxIFace    the added interface. It is allowed to insert null or
                         the same pointer more than once.
      @return the new count of elements in the container.
     */
    sal_Int32 SAL_CALL addInterface( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rxIFace ) SAL_THROW( () );
    /**
      Remove an element from the container. It uses the equal definition of
      uno objects to remove the interfaces.
      @param rxIFace    the removed interface.
      @return the new count of elements in the container.
     */
    sal_Int32 SAL_CALL removeInterface( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rxIFace ) SAL_THROW( () );
    /**
      Call disposing on all object in the container that
      support XEventListener. Than clear the container.
     */
    void SAL_CALL disposeAndClear( const ::com::sun::star::lang::EventObject & rEvt ) SAL_THROW( () );
    /**
      Remove all elements.
     */
    void SAL_CALL clear() SAL_THROW( () );

private:
friend class OInterfaceIteratorHelper;
    /**
      bIsList == TRUE -> pData of type Sequence< XInterfaceSequence >,
      otherwise pData == of type (XEventListener *)
     */
    void *                  pData;
    ::osl::Mutex &          rMutex;
    /** TRUE -> used by an iterator. */
    sal_Bool                bInUse;
    /** TRUE -> pData is of type Sequence< XInterfaceSequence >. */
    sal_Bool                bIsList;

    OInterfaceContainerHelper( const OInterfaceContainerHelper & ) SAL_THROW( () );
    OInterfaceContainerHelper & operator = ( const OInterfaceContainerHelper & ) SAL_THROW( () );

    /**
      Dulicate content of the conaitner and release the old one without destroing.
      The mutex must be locked and the memberbInUse must be true.
     */
    void copyAndResetInUse() SAL_THROW( () );
public:
};



//===================================================================
/**
  A generic class to support the implementation of the XConnectionPointContainer interface.
  This class holds a STL hash_map to acces the InterfaceContainerHelper through a generic
  key value.
  The InterfaceContainerHelper you get with the method getContainer( ... ) exist
  until the whole PropertyListenerContainer is destroyed.

  @author       Markus Meyer
  @since        03/12/98
  @see OInterfaceIteratorHelper
  @see OInterfaceContainerHelper
 */
template< class key , class hashImpl , class equalImpl >
class OMultiTypeInterfaceContainerHelperVar
{
public:
    /**
      Create a container of interface containers.

      @param rMutex the mutex to protect multi thread access.
                         The lifetime must be longer than the lifetime
                         of this object.
     */
    inline OMultiTypeInterfaceContainerHelperVar( ::osl::Mutex & ) SAL_THROW( () );
    /**
      Delete all containers.
     */
    inline ~OMultiTypeInterfaceContainerHelperVar() SAL_THROW( () );

    /**
      Return all id's under which at least one interface is added.
     */
    inline ::com::sun::star::uno::Sequence< key > SAL_CALL getContainedTypes() const SAL_THROW( () );

    /**
      Return the container created under this key.
      @return the container created under this key. If the container
                 was not created, null was returned.
     */
    inline OInterfaceContainerHelper * SAL_CALL getContainer( const key & ) const SAL_THROW( () );

    /**
      Insert an element in the container specified with the key. The position is not specified.
      @param rKey       the id of the container.
      @param rxIFace    the added interface. It is allowed to insert null or
                         the same pointer more than once.
      @return the new count of elements in the container.
     */
    inline sal_Int32 SAL_CALL addInterface(
        const key & rKey,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & r )
        SAL_THROW( () );

    /**
      Remove an element from the container specified with the key.
      It uses the equal definition of uno objects to remove the interfaces.
      @param rKey       the id of the container.
      @param rxIFace    the removed interface.
      @return the new count of elements in the container.
     */
    inline sal_Int32 SAL_CALL removeInterface(
        const key & rKey,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rxIFace )
        SAL_THROW( () );

    /**
      Call disposing on all object in the container that
      support XEventListener. Than clear the container.
     */
    inline void SAL_CALL disposeAndClear( const ::com::sun::star::lang::EventObject & rEvt ) SAL_THROW( () );
    /**
      Remove all elements of all containers. Does not delete the container.
     */
    inline void SAL_CALL clear() SAL_THROW( () );

    typedef key keyType;
private:
    ::std::hash_map< key , void* , hashImpl , equalImpl > *m_pMap;
    ::osl::Mutex &  rMutex;

    inline OMultiTypeInterfaceContainerHelperVar( const OMultiTypeInterfaceContainerHelperVar & ) SAL_THROW( () );
    inline OMultiTypeInterfaceContainerHelperVar & operator = ( const OMultiTypeInterfaceContainerHelperVar & ) SAL_THROW( () );
};




/**
  This struct contains the standard variables of a broadcaster. Helper
  classes only know a reference to this struct instead of references
  to the four members. The access to the members must be guarded with
  rMutex.

  The additional template parameter keyType has been added, because gcc
  can't compile addListener( const container::keyType &key ).
 */
template < class container , class keyType >
struct OBroadcastHelperVar
{
    /** The shared mutex. */
    ::osl::Mutex &                      rMutex;
    /** ListenerContainer class is thread save. */
    container   aLC;
    /** Dispose call ready. */
    sal_Bool                            bDisposed;
    /** In dispose call. */
    sal_Bool                            bInDispose;

    /**
      Initialize the structur. bDispose and bInDispose are set to false.
      @param rMutex the mutex reference.
     */
    OBroadcastHelperVar( ::osl::Mutex & rMutex_ ) SAL_THROW( () )
        : rMutex( rMutex_ )
        , aLC( rMutex_ )
        , bDisposed( sal_False )
        , bInDispose( sal_False )
    {}

    /**
      adds a listener threadsafe.
     **/
    inline void addListener(
        const keyType &key,
        const ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > &r )
        SAL_THROW( () )
    {
        ::osl::MutexGuard guard( rMutex );
        OSL_ENSHURE( !bInDispose, "do not add listeners in the dispose call" );
        OSL_ENSHURE( !bDisposed, "object is disposed" );
        if( ! bInDispose && ! bDisposed  )
            aLC.addInterface( key , r );
    }

    /**
      removes a listener threadsafe
     **/
    inline void removeListener(
        const keyType &key,
        const ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > & r )
        SAL_THROW( () )
    {
        ::osl::MutexGuard guard( rMutex );
        OSL_ENSHURE( !bDisposed, "object is disposed" );
        if( ! bInDispose && ! bDisposed  )
            aLC.removeInterface( key , r );
    }

    /**
      Return the container created under this key.
      @return the container created under this key. If the container
                was not created, null was returned. This can be used to optimize
              performance ( construction of an event object can be avoided ).
     ***/
    inline OInterfaceContainerHelper * SAL_CALL getContainer( const keyType &key ) const SAL_THROW( () )
        { return aLC.getContainer( key ); }
};

/*------------------------------------------
*
* In general, the above templates are used with a Type as key.
* Therefore a default declaration is given ( OMultiTypeInterfaceContainerHelper and OBroadcastHelper )
*
*------------------------------------------*/

// helper function call class
struct hashType_Impl
{
    size_t operator()(const ::com::sun::star::uno::Type & s) const SAL_THROW( () )
        { return s.getTypeName().hashCode(); }
};


typedef OMultiTypeInterfaceContainerHelperVar<
    ::com::sun::star::uno::Type,
    hashType_Impl,
    std::equal_to< ::com::sun::star::uno::Type > > OMultiTypeInterfaceContainerHelper;

typedef OBroadcastHelperVar< OMultiTypeInterfaceContainerHelper , OMultiTypeInterfaceContainerHelper::keyType > OBroadcastHelper;

}

#endif

