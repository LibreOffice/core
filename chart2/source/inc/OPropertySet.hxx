/*************************************************************************
 *
 *  $RCSfile: OPropertySet.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:29 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef CHART_OPROPERTYSET_HXX
#define CHART_OPROPERTYSET_HXX

// helper classes
#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

// interfaces and types
// #ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
// #include <com/sun/star/lang/XServiceInfo.hpp>
// #endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSTATES_HPP_
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_STYLE_XSTYLESUPPLIER_HPP_
#include <drafts/com/sun/star/style/XStyleSupplier.hpp>
#endif
// #ifndef _DRAFTS_COM_SUN_STAR_BEANS_XFASTPROPERTYSTATE_HPP_
// #include <drafts/com/sun/star/beans/XFastPropertyState.hpp>
// #endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <memory>

namespace property
{

namespace impl
{ class ImplOPropertySet; }


class OPropertySet :
    public ::cppu::OBroadcastHelper,
    // includes beans::XPropertySet, XMultiPropertySet and XFastPropertySet
    public ::cppu::OPropertySetHelper,
    // includes uno::XWeak (and XInterface, esp. ref-counting)
//     public virtual ::cppu::OWeakObject,

//     public virtual ::com::sun::star::lang::XServiceInfo,
    public ::com::sun::star::lang::XTypeProvider,
    public ::com::sun::star::beans::XPropertyState,
    public ::com::sun::star::beans::XMultiPropertyStates,
    public ::drafts::com::sun::star::style::XStyleSupplier
//     public ::drafts::com::sun::star::beans::XFastPropertyState
{
public:
    OPropertySet( ::osl::Mutex & rMutex );
    virtual ~OPropertySet();

protected:
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

    // ____ XPropertySet ____
    /** sample implementation using the InfoHelper:

        <pre>
        uno::Reference&lt; beans::XPropertySetInfo &gt; SAL_CALL
            OPropertySet::getPropertySetInfo()
                throw (uno::RuntimeException)
        {
            static uno::Reference&lt; beans::XPropertySetInfo &gt; xInfo;

            // /--
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if( !xInfo.is())
            {
                xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo(
                    getInfoHelper());
            }

            return xInfo;
            // \--
        }
        </pre>

        <p>(The reason why this isn't implemented here is, that the static
        object is only valid per concrete PropertySet.  Otherwise all
        PropertySets derived from this base calss would have the same
        properties.)</p>

        @see ::cppu::OPropertySetHelper
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException) = 0;

    /** Try to convert the value <code>rValue</code> to the type required by the
        property associated with <code>nHandle</code>.

        Overload this method to take influence in modification of properties.

        If the conversion changed , </TRUE> is returned and the converted value
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

private:
    /// reference to mutex of class deriving from here
    ::osl::Mutex &   m_rMutex;

    /// pImpl idiom implementation
    ::std::auto_ptr< impl::ImplOPropertySet > m_pImplProperties;
};

} //  namespace property

// CHART_OPROPERTYSET_HXX
#endif
