/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef CHART_OPROPERTYSET_HXX
#define CHART_OPROPERTYSET_HXX

// helper classes
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weak.hxx>

// interfaces and types
// #ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
// #include <com/sun/star/lang/XServiceInfo.hpp>
// #endif
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/style/XStyleSupplier.hpp>
// #ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSTATE_HPP_
// #include <com/sun/star/beans/XFastPropertyState.hpp>
// #endif
#include <osl/mutex.hxx>
#include "charttoolsdllapi.hxx"

#include <memory>

namespace property
{

namespace impl
{ class ImplOPropertySet; }


class OOO_DLLPUBLIC_CHARTTOOLS OPropertySet :
    public ::cppu::OBroadcastHelper,
    // includes beans::XPropertySet, XMultiPropertySet and XFastPropertySet
    public ::cppu::OPropertySetHelper,
    // includes uno::XWeak (and XInterface, esp. ref-counting)
//     public virtual ::cppu::OWeakObject,

//     public virtual ::com::sun::star::lang::XServiceInfo,
    public ::com::sun::star::lang::XTypeProvider,
    public ::com::sun::star::beans::XPropertyState,
    public ::com::sun::star::beans::XMultiPropertyStates,
    public ::com::sun::star::style::XStyleSupplier
//     public ::com::sun::star::beans::XFastPropertyState
{
public:
    OPropertySet( ::osl::Mutex & rMutex );
    virtual ~OPropertySet();

protected:
    explicit OPropertySet( const OPropertySet & rOther, ::osl::Mutex & rMutex );

    void SetNewValuesExplicitlyEvenIfTheyEqualDefault();

    /** implement this method to provide default values for all properties
        supporting defaults.  If a property does not have a default value, you
        may throw an UnknownPropertyException.
     */
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException) = 0;

    /** The InfoHelper table contains all property names and types of
        this object.

        @return the object that provides information for the
                PropertySetInfo

        @see ::cppu::OPropertySetHelper
     */
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() = 0;


    /** Try to convert the value <code>rValue</code> to the type required by the
        property associated with <code>nHandle</code>.

        Overload this method to take influence in modification of properties.

        If the conversion changed , </sal_True> is returned and the converted value
        is in <code>rConvertedValue</code>.  The former value is contained in
        <code>rOldValue</code>.

        After this call returns successfully, the vetoable listeners are
        notified.

        @throws IllegalArgumentException, if the conversion was not successful,
                or if there is no corresponding property to the given handle.

        @param rConvertedValue the converted value. Only set if return is true.
        @param rOldValue the old value. Only set if return is true.
        @param nHandle the handle of the property.

        @return true, if the conversion was successful and converted value
                differs from the old value.

        @see ::cppu::OPropertySetHelper
     */
    virtual sal_Bool SAL_CALL convertFastPropertyValue
        ( ::com::sun::star::uno::Any & rConvertedValue,
          ::com::sun::star::uno::Any & rOldValue,
          sal_Int32 nHandle,
          const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException);

    /** The same as setFastProperyValue; nHandle is always valid.
        The changes must not be broadcasted in this method.

        @attention
        Although you are permitted to throw any UNO exception, only the following
        are valid for usage:
        -- ::com::sun::star::beans::UnknownPropertyException
        -- ::com::sun::star::beans::PropertyVetoException
        -- ::com::sun::star::lang::IllegalArgumentException
        -- ::com::sun::star::lang::WrappedTargetException
        -- ::com::sun::star::uno::RuntimeException

        @param nHandle handle
        @param rValue  value

        @see ::cppu::OPropertySetHelper
    */
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast
        ( sal_Int32 nHandle,
          const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);

    /**
       The same as getFastProperyValue, but return the value through rValue and
       nHandle is always valid.

        @see ::cppu::OPropertySetHelper
     */
    virtual void SAL_CALL getFastPropertyValue
        ( ::com::sun::star::uno::Any& rValue,
          sal_Int32 nHandle ) const;

    /// make original interface function visible again
    using ::com::sun::star::beans::XFastPropertySet::getFastPropertyValue;

    /** implement this method in derived classes to get called when properties
        change.
     */
    virtual void firePropertyChangeEvent();

    /// call this when a derived component is disposed
    virtual void disposePropertySet();

    // ========================================
    // Interfaces
    // ========================================

    // ____ XInterface ____
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);
//     virtual void SAL_CALL acquire() throw ();
//     virtual void SAL_CALL release() throw ();


    // ____ XServiceInfo ____
//     virtual ::rtl::OUString SAL_CALL
//         getImplementationName()
//         throw (::com::sun::star::uno::RuntimeException);
//     virtual sal_Bool SAL_CALL
//         supportsService( const ::rtl::OUString& ServiceName )
//         throw (::com::sun::star::uno::RuntimeException);
//     virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
//         getSupportedServiceNames()
//         throw (::com::sun::star::uno::RuntimeException);

    // ____ XTypeProvider ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
        getTypes()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
        getImplementationId()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XPropertyState ____
    virtual ::com::sun::star::beans::PropertyState SAL_CALL
        getPropertyState( const ::rtl::OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL
        getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setPropertyToDefault( const ::rtl::OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL
        getPropertyDefault( const ::rtl::OUString& aPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XMultiPropertyStates ____
    // Note: getPropertyStates() is already implemented in XPropertyState with the
    // same signature
    virtual void SAL_CALL
        setAllPropertiesToDefault()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setPropertiesToDefault( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL
        getPropertyDefaults( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XStyleSupplier ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle > SAL_CALL getStyle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setStyle( const ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle >& xStyle )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XFastPropertyState ____
//     virtual ::com::sun::star::beans::PropertyState SAL_CALL getFastPropertyState( sal_Int32 nHandle )
//         throw (::com::sun::star::beans::UnknownPropertyException,
//                ::com::sun::star::uno::RuntimeException);
//     virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getFastPropertyStates( const ::com::sun::star::uno::Sequence< sal_Int32 >& aHandles )
//         throw (::com::sun::star::beans::UnknownPropertyException,
//                ::com::sun::star::uno::RuntimeException);
//     virtual void SAL_CALL setFastPropertyToDefault( sal_Int32 nHandle )
//         throw (::com::sun::star::beans::UnknownPropertyException,
//                ::com::sun::star::uno::RuntimeException);
//     virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyDefault( sal_Int32 nHandle )
//         throw (::com::sun::star::beans::UnknownPropertyException,
//                ::com::sun::star::lang::WrappedTargetException,
//                ::com::sun::star::uno::RuntimeException);

    // ____ XMultiPropertySet ____
    virtual void SAL_CALL setPropertyValues(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values )
        throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // ____ XFastPropertySet ____
    virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // Note: it is assumed that the base class implements setPropertyValue by
    // using setFastPropertyValue

private:
    /// reference to mutex of class deriving from here
    ::osl::Mutex &   m_rMutex;

    /// pImpl idiom implementation
    ::std::auto_ptr< impl::ImplOPropertySet > m_pImplProperties;
    bool m_bSetNewValuesExplicitlyEvenIfTheyEqualDefault;
};

} //  namespace property

// CHART_OPROPERTYSET_HXX
#endif
