/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_CPPUHELPER_PROPSHLP_HXX
#define INCLUDED_CPPUHELPER_PROPSHLP_HXX

#include <rtl/alloc.h>

#include <cppuhelper/interfacecontainer.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetOption.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>

#include <memory>
#include <cppuhelper/cppuhelperdllapi.h>


namespace cppu
{


/*************************************************************************
*************************************************************************/


/**
   This interface is used by the OPropertyHelper, to access the property description.
 */
class CPPUHELPER_DLLPUBLIC IPropertyArrayHelper
{
public:
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW(())
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW(())
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW(())
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW(())
        {}

    /**
       Following the rule, the first virtual method implies a virtual destructor.
     */
    virtual ~IPropertyArrayHelper();

    /**
       Return the property members Name and Attribute from the handle nHandle.
       @param nHandle   the handle of a property. If the values of the handles
                          are sorted in the same way as the names and the highest handle value
                          is getCount() -1, than it must be an indexed acces to the property array.
       @param pPropName is an out parameter filled with property name of the property with the
                              handle nHandle. May be NULL.
       @param pAttributes is an out parameter filled with attributes of the property with the
                              handle nHandle. May be NULL.
       @return True, if the handle exist, otherwise false.
     */
    virtual sal_Bool SAL_CALL fillPropertyMembersByHandle(
        ::rtl::OUString * pPropName, sal_Int16 * pAttributes, sal_Int32 nHandle ) = 0;
    /**
       Return the sequence of properties. The sequence is sorted by name.
     */
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties(void) = 0;
    /**
       Return the property with the name rPropertyName.
       @param rPropertyName the name of the property.
       @exception UnknownPropertyException  thrown if the property name is unknown.
     */
    virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName(
        const ::rtl::OUString& rPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException) = 0;
    /**
       Return true if the property with the name rPropertyName exist, otherwise false.
       @param rPropertyName the name of the property.
     */
    virtual sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& rPropertyName) = 0;
    /**
       Return the handle of the property with the name rPropertyName.
       If the property does not exist -1 is returned.
       @param rPropertyName the name of the property.
     */
    virtual sal_Int32 SAL_CALL getHandleByName( const ::rtl::OUString & rPropertyName ) = 0;
    /**
       Fill the array with the handles of the properties.
       @return the handles of the names from the pHandles array. -1
       indicates an unknown property name.
     */
    virtual sal_Int32 SAL_CALL fillHandles(
        /*out*/ sal_Int32 * pHandles, const ::com::sun::star::uno::Sequence< ::rtl::OUString > & rPropNames ) = 0;
};

/**
   You can use this helper class to map a XPropertySet-Interface to a XFast-
   or a XMultiPropertySet interface.
 */
class CPPUHELPER_DLLPUBLIC OPropertyArrayHelper : public IPropertyArrayHelper
{
public:
     /**
       Create an object which supports the common property interfaces.

       @param pProps    array of properties
                          The array pProps should be sorted.
       @param nElements is the number of properties in the pProps structure.
       @param bSorted   indicates that the elements are sorted.
      *********/
    OPropertyArrayHelper(
        ::com::sun::star::beans::Property *pProps,
        sal_Int32 nElements ,
        sal_Bool bSorted = sal_True )
        SAL_THROW(());

     /**
       Create an object which supports the common property interfaces.
       @param aProps     sequence of properties which are supported by this helper.
                           The sequence aProps should be sorted.
       @param bSorted    indicates that the elements are sorted.
     */
    OPropertyArrayHelper(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > & aProps,
        sal_Bool bSorted = sal_True )
        SAL_THROW(());

    /**
       Return the number of properties.
     */
    sal_Int32 SAL_CALL getCount() const SAL_THROW(());
    /**
       Return the property members Name and Attribute from the handle nHandle.
       @param nHandle   the handle of a property. If the values of the handles
                          are sorted in the same way as the names and the highest handle value
                          is getCount() -1, than it is only an indexed acces to the property array.
                          Otherwise it is a linear search through the array.
       @param pPropName is an out parameter filled with property name of the property with the
                              handle nHandle. May be NULL.
       @param pAttributes is an out parameter filled with attributes of the property with the
                              handle nHandle. May be NULL.
       @return True, if the handle exist, otherwise false.
     */
    virtual sal_Bool SAL_CALL fillPropertyMembersByHandle(
        ::rtl::OUString * pPropName, sal_Int16 * pAttributes, sal_Int32 nHandle );
    /**
       Return the sequence of properties. The sequence is sorted by name.
     */
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties(void);
    /**
       Return the property with the name rPropertyName.
       @param rPropertyName the name of the property.
       @exception UnknownPropertyException  thrown if the property name is unknown.
     */
    virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName(
        const ::rtl::OUString& rPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException);
    /**
       Return true if the property with the name rPropertyName exist, otherwise false.
       @param rPropertyName the name of the property.
     */
    virtual sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& rPropertyName);
    /**
       Return the handle of the property with the name rPropertyName.
       If the property does not exist -1 is returned.
       @param rPropertyName the name of the property.
     */
    virtual sal_Int32 SAL_CALL getHandleByName( const ::rtl::OUString & rPropertyName );
    /**
       Fill the array with the handles of the properties.
       @return the handles of the names from the pHandles array. -1
       indicates an unknown property name.
     */
    virtual sal_Int32 SAL_CALL fillHandles(
        /*out*/sal_Int32 * pHandles, const ::com::sun::star::uno::Sequence< ::rtl::OUString > & rPropNames );

protected:
    /** reserved for future use. do not use.
     */
    void * m_pReserved;

private:
    void init( sal_Bool bSorted ) SAL_THROW(());

    /** The sequence generated from the pProperties array. */
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > aInfos;

    /**
       True, If the values of the handles are sorted in the same way as the names
       and the highest handle value is getCount() -1, otherwise false.
     */
    sal_Bool                    bRightOrdered;
};


//-----------------------------------------------------------------------------
// helper defines needed for an interface container with a 32 bit key values

struct equalInt32_Impl
{
    bool operator()(const sal_Int32 & i1 , const sal_Int32 & i2) const SAL_THROW(())
        { return i1 == i2; }
};

struct hashInt32_Impl
{
    size_t operator()(const sal_Int32 & i) const SAL_THROW(())
        { return i; }
};
/** Specialized class for key type sal_Int32,
    without explicit usage of STL symbols.
*/
class CPPUHELPER_DLLPUBLIC OMultiTypeInterfaceContainerHelperInt32
{
public:
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW(())
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW(())
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW(())
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW(())
        {}

    /**
      Create a container of interface containers.

      @param rMutex the mutex to protect multi thread access.
                         The lifetime must be longer than the lifetime
                         of this object.
     */
    OMultiTypeInterfaceContainerHelperInt32( ::osl::Mutex & rMutex ) SAL_THROW(());
    /**
      Delete all containers.
     */
    ~OMultiTypeInterfaceContainerHelperInt32() SAL_THROW(());

    /**
      Return all id's under which at least one interface is added.
     */
    ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL getContainedTypes() const SAL_THROW(());

    /**
      Return the container created under this key.
      @return the container created under this key. If the container
                 was not created, null was returned.
     */
    OInterfaceContainerHelper * SAL_CALL getContainer( const sal_Int32 & rKey ) const SAL_THROW(());

    /**
      Insert an element in the container specified with the key. The position is not specified.
      @param rKey       the id of the container.
      @param r          the added interface. It is allowed to insert null or
                         the same pointer more than once.
      @return the new count of elements in the container.
     */
    sal_Int32 SAL_CALL addInterface(
        const sal_Int32 & rKey,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & r )
        SAL_THROW(());

    /**
      Remove an element from the container specified with the key.
      It uses the equal definition of uno objects to remove the interfaces.
      @param rKey       the id of the container.
      @param rxIFace    the removed interface.
      @return the new count of elements in the container.
     */
    sal_Int32 SAL_CALL removeInterface(
        const sal_Int32 & rKey,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rxIFace )
        SAL_THROW(());

    /**
      Call disposing on all objects in the container that
      support XEventListener. Then clear the container.
     */
    void    SAL_CALL disposeAndClear( const ::com::sun::star::lang::EventObject & rEvt ) SAL_THROW(());
    /**
      Remove all elements of all containers. Does not delete the container.
     */
    void SAL_CALL clear() SAL_THROW(());

    typedef sal_Int32 keyType;
private:
    void *m_pMap;
    ::osl::Mutex &  rMutex;

    inline OMultiTypeInterfaceContainerHelperInt32( const OMultiTypeInterfaceContainerHelperInt32 & ) SAL_THROW(());
    inline OMultiTypeInterfaceContainerHelperInt32 & operator = ( const OMultiTypeInterfaceContainerHelperInt32 & ) SAL_THROW(());
};


/** An interface to extend event notification actions.
  */
class IEventNotificationHook
{
public:
    /**
        Method to be called by OPropertySetHelper::fire.

        @param bIgnoreRuntimeExceptionsWhileFiring
                        indicates whether occuring RuntimeExceptions shall be
                        ignored when firing notifications
       @param pnHandles     the id's of the properties that changed.
       @param nCount        the number of elements in the arrays pnHandles, pNewValues and pOldValues.
       @param bVetoable true means fire to VetoableChangeListener, false means fire to
                  XPropertyChangedListener and XMultiPropertyChangedListener.
       @param bIgnoreRuntimeExceptionsWhileFiring
                        indicates whether occurring RuntimeExceptions will be
                        ignored when firing notifications
                        (vetoableChange(), propertyChange())
                        to listeners.
                        PropertyVetoExceptions may still be thrown.
                        This flag is useful in an inter-process scenario when
                        remote bridges may break down
                        (firing DisposedExceptions).

        @see OPropertySetHelper::fire
     */
    virtual void fireEvents(
        sal_Int32 * pnHandles,
        sal_Int32 nCount,
        sal_Bool bVetoable,
        bool bIgnoreRuntimeExceptionsWhileFiring) = 0;

#if !defined _MSC_VER // public -> protected changes mangled names there
protected:
#endif
    ~IEventNotificationHook() {}
        // avoid warnings about virtual members and non-virtual dtor
};



/**
   This abstract class maps the methods of the interfaces XMultiPropertySet, XFastPropertySet
   and XPropertySet to the methods getInfoHelper, convertFastPropertyValue,
   setFastPropertyValue_NoBroadcast and getFastPropertyValue. You must derive from
   this class and overload the methods.
   It provides a standard implementation of the XPropertySetInfo.
   The XPropertiesChangeListener are inserted in the rBHelper.aLC structure.
   The XPropertyChangeListener and XVetoableChangeListener with no names are inserted
   in the rBHelper.aLC structure. So it is possible to advise property listeners with
   the connection point interfaces. But only listeners that listen to all property changes.

 */
class CPPUHELPER_DLLPUBLIC OPropertySetHelper :
                           public ::com::sun::star::beans::XMultiPropertySet,
                           public ::com::sun::star::beans::XFastPropertySet,
                           public ::com::sun::star::beans::XPropertySet
{
public:
    /**
       @param rBHelper  this structure contains the basic members of
                          a broadcaster.
                          The lifetime must be longer than the lifetime
                          of this object. Stored in the variable rBHelper.
     */
    OPropertySetHelper( OBroadcastHelper & rBHelper ) SAL_THROW(());

    /** Constructor.

        @param rBHelper
                        this structure contains the basic members of
                        a broadcaster.
                          The lifetime must be longer than the lifetime
                          of this object. Stored in the variable rBHelper.

        @param bIgnoreRuntimeExceptionsWhileFiring
                        indicates whether occurring RuntimeExceptions will be
                        ignored when firing notifications
                        (vetoableChange(), propertyChange())
                        to listeners.
                        PropertyVetoExceptions may still be thrown.
                        This flag is useful in an inter-process scenario when
                        remote bridges may break down
                        (firing DisposedExceptions).
    */
    OPropertySetHelper(
        OBroadcastHelper & rBHelper, bool bIgnoreRuntimeExceptionsWhileFiring );

    /** Constructor.

        @param rBHelper
                        this structure contains the basic members of
                        a broadcaster.
                        The lifetime must be longer than the lifetime
                        of this object. Stored in the variable rBHelper.

        @param i_pFireEvents
                        additional event notifier

        @param bIgnoreRuntimeExceptionsWhileFiring
                        indicates whether occurring RuntimeExceptions will be
                        ignored when firing notifications
                        (vetoableChange(), propertyChange())
                        to listeners.
                        PropertyVetoExceptions may still be thrown.
                        This flag is useful in an inter-process scenario when
                        remote bridges may break down
                        (firing DisposedExceptions).
    */
    OPropertySetHelper(
        OBroadcastHelper & rBHelper,
        IEventNotificationHook *i_pFireEvents,
        bool bIgnoreRuntimeExceptionsWhileFiring = false);

    /**
       Only returns a reference to XMultiPropertySet, XFastPropertySet, XPropertySet and
       XEventListener.
     */
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType )
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    /** eases implementing XTypeProvider::getTypes, returns the types of XMultiPropertySet, XFastPropertySet, XPropertySet
     */
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > getTypes()
        throw(::com::sun::star::uno::RuntimeException);

    /**
       Send a disposing notification to the listeners in the conatiners aBoundLC
       and aVetoableLC.

       @see OComponentHelper
     */
    void SAL_CALL disposing() SAL_THROW(());

    /**
       Throw UnknownPropertyException or PropertyVetoException if the property with the name
       rPropertyName does not exist or is readonly. Otherwise rPropertyName is changed to its handle
       value and setFastPropertyValue is called.
     */
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    /**
       Throw UnknownPropertyException if the property with the name
       rPropertyName does not exist.
     */
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(const ::rtl::OUString& aPropertyName)
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    /** Ignored if the property is not bound. */
    virtual void SAL_CALL addPropertyChangeListener(
        const ::rtl::OUString& aPropertyName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener)
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    /** Ignored if the property is not bound. */
    virtual void SAL_CALL removePropertyChangeListener(
        const ::rtl::OUString& aPropertyName,
        const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertyChangeListener >& aListener)
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    /** Ignored if the property is not constrained. */
    virtual void SAL_CALL addVetoableChangeListener(
        const ::rtl::OUString& aPropertyName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener)
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    /** Ignored if the property is not constrained. */
    virtual void SAL_CALL removeVetoableChangeListener(
        const ::rtl::OUString& aPropertyName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    /**
       Throw UnknownPropertyException or PropertyVetoException if the property with the name
       rPropertyName does not exist or is readonly. Otherwise the method convertFastPropertyValue
       is called, then the vetoable listeners are notified. After this the value of the property
       is changed with the setFastPropertyValue_NoBroadcast method and the bound listeners are
       notified.
      */
    virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    /**
       @exception com::sun::star::beans::UnknownPropertyException
         if the property with the handle nHandle does not exist.
     */
    virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue( sal_Int32 nHandle )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values )
        throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames )
        throw(::com::sun::star::uno::RuntimeException, std::exception);

    virtual void SAL_CALL addPropertiesChangeListener(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& Listener )
        throw(::com::sun::star::uno::RuntimeException, std::exception);

    virtual void SAL_CALL removePropertiesChangeListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& Listener )
        throw(::com::sun::star::uno::RuntimeException, std::exception);

    virtual void SAL_CALL firePropertiesChangeEvent(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener > & Listener )
        throw(::com::sun::star::uno::RuntimeException, std::exception);

    /**
       The property sequence is created in the call. The interface isn't used after the call.
     */
    static ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        createPropertySetInfo( IPropertyArrayHelper & rProperties ) SAL_THROW(());
protected:
    /**
       This method fire events to all registered property listeners.
       @param pnHandles     the id's of the properties that changed.
       @param pNewValues    the new values of the properties.
       @param pOldValues    the old values of the properties.
       @param nCount        the number of elements in the arrays pnHandles, pNewValues and pOldValues.
       @param bVetoable true means fire to VetoableChangeListener, false means fire to
                  XPropertyChangedListener and XMultiPropertyChangedListener.
     */
    void SAL_CALL fire(
        sal_Int32 * pnHandles,
        const ::com::sun::star::uno::Any * pNewValues,
        const ::com::sun::star::uno::Any * pOldValues,
        sal_Int32 nCount,
        sal_Bool bVetoable );

    /**
       Set multiple properties with the handles.
       @param nSeqLen   the length of the arrays pHandles and Values.
       @param pHandles the handles of the properties. The number of elements
              in the Values sequence is the length of the handle array. A value of -1
              of a handle means invalid property. These are ignored.
       @param pValues the values of the properties.
       @param nHitCount the number of valid entries in the handle array.
     */
    void SAL_CALL setFastPropertyValues(
        sal_Int32 nSeqLen,
        sal_Int32 * pHandles,
        const ::com::sun::star::uno::Any * pValues,
        sal_Int32 nHitCount )
        SAL_THROW( (::com::sun::star::uno::Exception) );

    /**
       This abstract method must return the name to index table. This table contains all property
       names and types of this object. The method is not implemented in this class.
     */
    virtual IPropertyArrayHelper & SAL_CALL getInfoHelper() = 0;

    /**
       Converted the value rValue and return the result in rConvertedValue and the
       old value in rOldValue. A IllegalArgumentException is thrown.
       The method is not implemented in this class. After this call the vetoable
       listeners are notified.

       @param rConvertedValue the converted value. Only set if return is true.
       @param rOldValue the old value. Only set if return is true.
       @param nHandle the handle of the proberty.
       @param rValue the value to be converted
       @return true if the value converted.
     */
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        ::com::sun::star::uno::Any & rConvertedValue,
        ::com::sun::star::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException) = 0;

    /** The same as setFastProperyValue; nHandle is always valid.
        The changes must not be broadcasted in this method.
        The method is implemented in a derived class.

        @attention
        Although you are permitted to throw any UNO exception, only the following
        are valid for usage:
        -- com::sun::star::beans::UnknownPropertyException
        -- com::sun::star::beans::PropertyVetoException
        -- com::sun::star::lang::IllegalArgumentException
        -- com::sun::star::lang::WrappedTargetException
        -- com::sun::star::uno::RuntimeException

        @param nHandle
               handle
        @param rValue
               value
    */
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
            throw (::com::sun::star::uno::Exception,
                   std::exception) = 0;
    /**
       The same as getFastProperyValue, but return the value through rValue and nHandle
       is always valid.
       The method is not implemented in this class.
     */
    virtual void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue,
        sal_Int32 nHandle ) const = 0;

    /** sets an dependent property's value

        <p>Sometimes setting a given property needs to implicitly modify another property's value. Calling |setPropertyValue|
        from within |setFastPropertyValue_NoBroadcast| is not an option here, as it would notify the property listeners
        while our mutex is still locked. Setting the dependent property's value directly (e.g. by calling |setFastPropertyValue_NoBroadcast|
        recursively) is not an option, too, since it would miss firing the property change event.</p>

        <p>So, in such cases, you use |setDependentFastPropertyValue| from within |setFastPropertyValue_NoBroadcast|.
        It will convert and actually set the property value (invoking |convertFastPropertyValue| and |setFastPropertyValue_NoBroadcast|
        for the given handle and value), and add the property change event to the list of events to be notified
        when the bottom-most |setFastPropertyValue_NoBroadcast| on the stack returns.</p>

        <p><strong>Note</strong>: The method will <em>not</em> invoke veto listeners for the property.</p>

        <p><strong>Note</strong>: It's the caller's responsibility to ensure that our mutex is locked. This is
        canonically given when the method is invoked from within |setFastPropertyValue_NoBroadcast|, in other
        contexts, you might need to take own measures.</p>
    */
    void    setDependentFastPropertyValue(
                sal_Int32 i_handle,
                const ::com::sun::star::uno::Any& i_value
            );

    /** The common data of a broadcaster. Use the mutex, disposing state and the listener container. */
    OBroadcastHelper    &rBHelper;
    /**
       Container for the XProperyChangedListener. The listeners are inserted by handle.
     */
    OMultiTypeInterfaceContainerHelperInt32  aBoundLC;
    /**
       Container for the XPropertyVetoableListener. The listeners are inserted by handle.
     */
    OMultiTypeInterfaceContainerHelperInt32 aVetoableLC;

    class Impl;

    /** reserved for future use. finally, the future has arrived...
     */
    const std::auto_ptr<Impl> m_pReserved;

private:
    OPropertySetHelper( const OPropertySetHelper & ) SAL_THROW(());
    OPropertySetHelper &    operator = ( const OPropertySetHelper & ) SAL_THROW(());

    /** notifies the given changes in property's values, <em>plus</em> all property changes collected during recent
        |setDependentFastPropertyValue| calls.
    */
    void    impl_fireAll(
                sal_Int32* i_handles,
                const ::com::sun::star::uno::Any * i_newValues,
                const ::com::sun::star::uno::Any * i_oldValues,
                sal_Int32 i_count
            );

#if defined _MSC_VER // public -> protected changes mangled names there
public:
#else
protected:
#endif
// Suppress warning about virtual functions but non-virtual destructor:
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4265)
#endif
    /**
       You must call disposing before destruction.
     */
    ~OPropertySetHelper() SAL_THROW(());
};
#if defined _MSC_VER
#pragma warning(pop)
#endif

/**
   OPropertySetHelper plus XPropertySetOption
 */
class CPPUHELPER_DLLPUBLIC OPropertySetHelper2 : public OPropertySetHelper,
                            public ::com::sun::star::beans::XPropertySetOption
{
public:
    /** Constructor.

        See OPropertySetHelper constructors documentation
    */
    explicit OPropertySetHelper2(
        OBroadcastHelper & rBHelper,
        IEventNotificationHook *i_pFireEvents = NULL,
        bool bIgnoreRuntimeExceptionsWhileFiring = false);

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType )
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XPropertySetOption
    virtual void SAL_CALL enableChangeListenerNotification( sal_Bool bEnable )
        throw(::com::sun::star::uno::RuntimeException, std::exception);



private:
    OPropertySetHelper2( const OPropertySetHelper2 & ) SAL_THROW(());
    OPropertySetHelper2 &    operator = ( const OPropertySetHelper2 & ) SAL_THROW(());

#if defined _MSC_VER // public -> protected changes mangled names there
public:
#else
protected:
#endif
// Suppress warning about virtual functions but non-virtual destructor:
    /**
       You must call disposing before destruction.
     */
    virtual ~OPropertySetHelper2() SAL_THROW(());
};

} // end namespace cppuhelper
#endif  //



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
