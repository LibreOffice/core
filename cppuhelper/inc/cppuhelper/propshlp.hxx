/*************************************************************************
 *
 *  $RCSfile: propshlp.hxx,v $
 *
 *  $Revision: 1.3 $
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

#ifndef _CPPUHELPER_PROPSHLP_HXX
#define _CPPUHELPER_PROPSHLP_HXX

#include <rtl/alloc.h>

#include <hash_map>

#include <cppuhelper/interfacecontainer.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>

namespace cppu
{


/*************************************************************************
*************************************************************************/


/**
 * This interface is used by the OPropertyHelper, to access the property description.
 */
class IPropertyArrayHelper
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
     * Folowing the rule, the first virtual method impies the virtual destructor.
     */
    virtual ~IPropertyArrayHelper();

    /**
     * Return the property members Name and Attribute from the handle nHandle.
     * @param nHandle   the handle of a property. If the values of the handles
     *                  are sorted in the same way as the names and the highest handle value
     *                  is getCount() -1, than it must be an indexed acces to the property array.
     * @param pPropName is an out parameter filled with property name of the property with the
     *                      handle nHandle. May be NULL.
     * @param rAttributes is an out parameter filled with attributes of the property with the
     *                      handle nHandle. May be NULL.
     * @return True, if the handle exist, otherwise false.
     */
    virtual sal_Bool SAL_CALL fillPropertyMembersByHandle(
        ::rtl::OUString * pPropName, sal_Int16 * pAttributes, sal_Int32 nHandle ) = 0;
    /**
     * Return the sequence of properties. The sequence is sorted by name.
     */
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties(void) = 0;
    /**
     * Return the property with the name rPropertyName.
     * @param rPropertyName the name of the property.
     * @exception UnknownPropertyException  thrown if the property name is unknown.
     */
    virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName(
        const ::rtl::OUString& rPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException) = 0;
    /**
     * Return true if the property with the name rPropertyName exist, otherwise false.
     * @param rPropertyName the name of the property.
     */
    virtual sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& rPropertyName) = 0;
    /**
     * Return the handle of the property with the name rPropertyName.
     * If the property does not exist -1 is returned.
     * @param rPropertyName the name of the property.
     */
    virtual sal_Int32 SAL_CALL getHandleByName( const ::rtl::OUString & rPropertyName ) = 0;
    /**
     * Fill the array with the handles of the properties.
     * @return the handles of the names from the pHandles array. -1
     * indicates an unknown property name.
     */
    virtual sal_Int32 SAL_CALL fillHandles(
        /*out*/ sal_Int32 * pHandles, const ::com::sun::star::uno::Sequence< ::rtl::OUString > & rPropNames ) = 0;
};

/**
 * You can use this helper class to map a XPropertySet-Interface to a XFast-
 * or a XMultiPropertySet interface.
 */
class OPropertyArrayHelper : public IPropertyArrayHelper
{
public:
     /*********
     * Create an object which supports the common property interfaces.
     *
     * @param pProps    array of properties
     *                  The array pProps should be sorted.
     * @param nElements is the number of properties in the pProps structure.
     * @param bSorted   indicates that the elements are sorted.
      *********/
    OPropertyArrayHelper(
        ::com::sun::star::beans::Property *pProps,
        sal_Int32 nElements ,
        sal_Bool bSorted = sal_True )
        SAL_THROW( () );

     /*********
     * Create an object which supports the common property interfaces.
     * @param aProps     sequence of properties which are supported by this helper.
     *                   The sequence aProps should be sorted.
     * @param bSorted    indicates that the elements are sorted.
     *********/
    OPropertyArrayHelper(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > & aProps,
        sal_Bool bSorted = sal_True )
        SAL_THROW( () );

    /**
     * Return the number of properties.
     */
    sal_Int32 SAL_CALL getCount() const SAL_THROW( () );
    /**
     * Return the property members Name and Attribute from the handle nHandle.
     * @param nHandle   the handle of a property. If the values of the handles
     *                  are sorted in the same way as the names and the highest handle value
     *                  is getCount() -1, than it is only an indexed acces to the property array.
     *                  Otherwise it is a linear search through the array.
     * @param pPropName is an out parameter filled with property name of the property with the
     *                      handle nHandle. May be NULL.
     * @param rAttributes is an out parameter filled with attributes of the property with the
     *                      handle nHandle. May be NULL.
     * @return True, if the handle exist, otherwise false.
     */
    virtual sal_Bool SAL_CALL fillPropertyMembersByHandle(
        ::rtl::OUString * pPropName, sal_Int16 * pAttributes, sal_Int32 nHandle );
    /**
     * Return the sequence of properties. The sequence is sorted by name.
     */
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties(void);
    /**
     * Return the property with the name rPropertyName.
     * @param rPropertyName the name of the property.
     * @exception UnknownPropertyException  thrown if the property name is unknown.
     */
    virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName(
        const ::rtl::OUString& rPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException);
    /**
     * Return true if the property with the name rPropertyName exist, otherwise false.
     * @param rPropertyName the name of the property.
     */
    virtual sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& rPropertyName);
    /**
     * Return the handle of the property with the name rPropertyName.
     * If the property does not exist -1 is returned.
     * @param rPropertyName the name of the property.
     */
    virtual sal_Int32 SAL_CALL getHandleByName( const ::rtl::OUString & rPropertyName );
    /**
     * Fill the array with the handles of the properties.
     * @return the handles of the names from the pHandles array. -1
     * indicates an unknown property name.
     */
    virtual sal_Int32 SAL_CALL fillHandles(
        /*out*/sal_Int32 * pHandles, const ::com::sun::star::uno::Sequence< ::rtl::OUString > & rPropNames );
private:
    void init( sal_Bool bSorted ) SAL_THROW( () );

    /** The sequence generstet from the pProperties array. */
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > aInfos;
    /**
     * True, If the values of the handles are sorted in the same way as the names
     * and the highest handle value is getCount() -1, otherwise false.
     */
    sal_Bool                    bRightOrdered;
};


/**
 * This abstract class map the methods of the interfaces XMultiPropertySet, XFastPropertySet
 * and XPropertySet to the methods getInfoHelper, convertFastPropertyValue,
 * setFastPropertyValue_NoBroadcast and getFastPropertyValue. You must subclass
 * this one and overload the methods.<BR>
 * It provide a standard implementation of the XPropertySetInfo.<BR>
 * The XPropertiesChangeListener are inserted in the rBHelper.aLC structure.<BR>
 * The XPropertyChangeListener and XVetoableChangeListener with no names are inserted
 * in the rBHelper.aLC structure. So it is possible to advise property listeners with
 * the connection point interfaces. But only listeners that listen to all property changes.
 * The subclass must explicite allow the access through the XConnectionPoint interface.<BR>
 * <B>Not tested under MT conditions</B>
 *
 * @see     OConnectionPointContainerHelper
 * @see     createPropertySetInfo
 * @author  Markus Meyer
 * @since   12/04/98
 */



//-----------------------------------------------------------------------------
// helper defines needed for an interface container with a 32 bit key values

struct equalInt32_Impl
{
    bool operator()(const sal_Int32 & i1 , const sal_Int32 & i2) const SAL_THROW( () )
        { return i1 == i2; }
};


struct hashInt32_Impl
{
    size_t operator()(const sal_Int32 & i) const SAL_THROW( () )
        { return i; }
};
typedef OMultiTypeInterfaceContainerHelperVar< sal_Int32, hashInt32_Impl, equalInt32_Impl >
    OMultiTypeInterfaceContainerHelperInt32;


class OPropertySetHelper : public ::com::sun::star::beans::XMultiPropertySet,
                           public ::com::sun::star::beans::XFastPropertySet,
                           public ::com::sun::star::beans::XPropertySet
{
public:
    /**
     * Create empty property listener container and hold the helper structure.
     *
     * @param rBHelper  this structure containes the basic members of
     *                  a broadcaster.
     *                  The lifetime must be longer as the lifetime
     *                  of this object. Stored in the variable rBHelper.
     */
    OPropertySetHelper( OBroadcastHelper & rBHelper ) SAL_THROW( () );
    /**
     * You must call disposing before destruction.
     */
    ~OPropertySetHelper() SAL_THROW( () );

    /**
     * Only return a reference to XMultiPropertySet, XFastPropertySet, XPropertySet and
     * XEventListener.
     * <B>Do not return a reference to XInterface.</B>
     */
    ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType )
        throw (::com::sun::star::uno::RuntimeException);

    /**
     * Send a disposing notification to the listeners in the conatiners aBoundLC
     * and aVetoableLC.
     *
     * @see OComponentHelper
     */
    void SAL_CALL disposing() SAL_THROW( () );

    // XPropertySet
    //XPropertySetInfoRef getPropertySetInfo() const;
    /**
     * Throw UnknownPropertyException or PropertyVetoException if the property with the name
     * rPropertyName does not exist or is readonly. Otherwise rPropertyName is changed to its handle
     * value and setFastPropertyValue is called.
     */
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    /**
     * Throw UnknownPropertyException if the property with the name
     * rPropertyName does not exist.
     */
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(const ::rtl::OUString& aPropertyName)
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    /** Ignored if the property is not bound. */
    virtual void SAL_CALL addPropertyChangeListener(
        const ::rtl::OUString& aPropertyName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener)
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    /** Ignored if the property is not bound. */
    virtual void SAL_CALL removePropertyChangeListener(
        const ::rtl::OUString& aPropertyName,
        const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertyChangeListener >& aListener)
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    /** Ignored if the property is not constrained. */
    virtual void SAL_CALL addVetoableChangeListener(
        const ::rtl::OUString& aPropertyName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener)
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    /** Ignored if the property is not constrained. */
    virtual void SAL_CALL removeVetoableChangeListener(
        const ::rtl::OUString& aPropertyName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XFastPropertySet
    /**
     * Throw UnknownPropertyException or PropertyVetoException if the property with the name
     * rPropertyName does not exist or is readonly. Otherwise the method convertFastPropertyValue
     * is called, than the vetoable listeners are notified. After this the value of the property
     * is changed with the setFastPropertyValue_NoBroadcast method and the bound listeners are
     * notified.
     */
    virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    /**
     * Throw UnknownPropertyException if the property with the handle
     * nHandle does not exist.
     */
    virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue( sal_Int32 nHandle )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values )
        throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames )
        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addPropertiesChangeListener(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& Listener )
        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removePropertiesChangeListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& Listener )
        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL firePropertiesChangeEvent(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener > & Listener )
        throw(::com::sun::star::uno::RuntimeException);
    /**
     * The property sequence is create in the call. The interface isn't used after the call.
     */
    static ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        createPropertySetInfo( IPropertyArrayHelper & rProperties ) SAL_THROW( () );
protected:
    /**
     * This method fire events to all registered property listeners.
     * @param pnHandles     the id's of the properties that changed.
     * @param pNewValues    the new values of the properties.
     * @param pOldValues    the old values of the properties.
     * @param nCount        the number of elements in the arrays pnHandles, pNewValues and pOldValues.
     * @param bVetoable true means fire to VetoableChangeListener, false means fire to
     *          XPropertyChangedListener and XMultiPropertyChangedListener.
     */
    void SAL_CALL fire(
        sal_Int32 * pnHandles,
        const ::com::sun::star::uno::Any * pNewValues,
        const ::com::sun::star::uno::Any * pOldValues,
        sal_Int32 nCount,
        sal_Bool bVetoable );

    /**
     * Set multible properties with the handles.
     * @param nSeqLen   the length of the arrays pHandles and Values.
     * @param pHandles the handles of the properties. The number of elements
     *      in the Values sequence is the length of the handle array. A value of -1
     *      of a handle means invalid property. These are ignored.
     * @param pValues the values of the properties.
     * @param nHitCount the number of valid entries in the handle array.
     */
    void SAL_CALL setFastPropertyValues(
        sal_Int32 nSeqLen,
        sal_Int32 * pHandles,
        const ::com::sun::star::uno::Any * pValues,
        sal_Int32 nHitCount )
        SAL_THROW( (::com::sun::star::uno::Exception) );

    /**
     * This abstract method must return the name to index table. This table contains all property
     * names and types of this object. The method is not implemented in this class.
     */
    virtual IPropertyArrayHelper & SAL_CALL getInfoHelper() = 0;

    /**
     * Converted the value rValue and return the result in rConvertedValue and the
     * old value in rOldValue. A IllegalArgumentException is thrown.
     * The method is not implemented in this class. After this call the vetoable
     * listeners are notified.
     *
     * @param rConvertedValue the converted value. Only set if return is true.
     * @param rOldValue the old value. Only set if return is true.
     * @param nHandle the handle of the proberty.
     * @return true if the value converted.
     */
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        ::com::sun::star::uno::Any & rConvertedValue,
        ::com::sun::star::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException) = 0;
    /**
     * The same as setFastProperyValue, but no exception is thrown and nHandle
     * is always valid. You must not broadcast the changes in this method.<BR>
     * <B>You type is correct you need't test it.</B>
     * The method is not implemented in this class.
     */
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception) = 0;
    /**
     * The same as getFastProperyValue, but return the value through rValue and nHandle
     * is always valid.
     * The method is not implemented in this class.
     */
    virtual void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue,
        sal_Int32 nHandle ) const = 0;

    /** The common data of a broadcaster. Use the mutex, disposing state and the listener container. */
    OBroadcastHelper    &rBHelper;
    /**
     * Container for the XProperyChangedListener. The listeners are inserted by handle.<BR>
     * Listeners added without name are inserted in the rBHelper.aLC container under
     * the Uik XPropertyChangeListener::getSmartUik().
     */
    OMultiTypeInterfaceContainerHelperInt32  aBoundLC;
    /**
     * Container for the XPropertyVetoableListener. The listeners are inserted by handle.
     * Listeners added without name are inserted in the rBHelper.aLC container under
     * the Uik XVetoableChangeListener::getSmartUik().
     */
    OMultiTypeInterfaceContainerHelperInt32 aVetoableLC;
private:
    OPropertySetHelper( const OPropertySetHelper & ) SAL_THROW( () );
    OPropertySetHelper &    operator = ( const OPropertySetHelper & ) SAL_THROW( () );
};

} // end namespace cppuhelper
#endif  //



