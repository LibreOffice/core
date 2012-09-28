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


#include "opropertybag.hxx"
#include "comphelper_module.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/Property.hpp>

#include <comphelper/namedvaluecollection.hxx>

#include <cppuhelper/exc_hlp.hxx>
#include <osl/thread.h>

#include <algorithm>
#include <functional>
#include <iterator>


//--------------------------------------------------------------------------
using namespace ::com::sun::star;

void createRegistryInfo_OPropertyBag()
{
    static ::comphelper::module::OAutoRegistration< ::comphelper::OPropertyBag > aAutoRegistration;
}

//........................................................................
namespace comphelper
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::container;

    //====================================================================
    //= OPropertyBag
    //====================================================================
    //--------------------------------------------------------------------
    OPropertyBag::OPropertyBag()
        :OPropertyBag_PBase( GetBroadcastHelper(), this )
        ,::cppu::IEventNotificationHook()
        ,m_bAutoAddProperties( false )
        ,m_NotifyListeners(m_aMutex)
        ,m_isModified(false)

    {
    }

    //--------------------------------------------------------------------
    OPropertyBag::~OPropertyBag()
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( OPropertyBag, OPropertyBag_Base, OPropertyBag_PBase )
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OPropertyBag, OPropertyBag_Base, OPropertyBag_PBase )

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > OPropertyBag::getSupportedServiceNames_static() throw( RuntimeException )
    {
        Sequence< ::rtl::OUString > aServices(1);
        aServices[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.beans.PropertyBag" ) );
        return aServices;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OPropertyBag::initialize( const Sequence< Any >& _rArguments ) throw (Exception, RuntimeException)
    {
        ::comphelper::NamedValueCollection aArguments( _rArguments );

        Sequence< Type > aTypes;
        if ( aArguments.get_ensureType( "AllowedTypes", aTypes ) )
            ::std::copy(
                aTypes.getConstArray(),
                aTypes.getConstArray() + aTypes.getLength(),
                ::std::insert_iterator< TypeBag >( m_aAllowedTypes, m_aAllowedTypes.begin() )
            );

        aArguments.get_ensureType( "AutomaticAddition", m_bAutoAddProperties );
        bool AllowEmptyPropertyName(false);
        aArguments.get_ensureType( "AllowEmptyPropertyName",
            AllowEmptyPropertyName );
        if (AllowEmptyPropertyName) {
            m_aDynamicProperties.setAllowEmptyPropertyName(
                AllowEmptyPropertyName);
        }
    }

    //--------------------------------------------------------------------
    ::rtl::OUString OPropertyBag::getImplementationName_static() throw( RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.comphelper.OPropertyBag" ) );
    }

    //--------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OPropertyBag::Create( SAL_UNUSED_PARAMETER const Reference< XComponentContext >& )
    {
        return *new OPropertyBag;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OPropertyBag::getImplementationName() throw (RuntimeException)
    {
        return getImplementationName_static();
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL OPropertyBag::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( getSupportedServiceNames_static() );
        const ::rtl::OUString* pStart = aServices.getConstArray();
        const ::rtl::OUString* pEnd = aServices.getConstArray() + aServices.getLength();
        return ::std::find( pStart, pEnd, rServiceName ) != pEnd;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OPropertyBag::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //--------------------------------------------------------------------
    void OPropertyBag::fireEvents(
            sal_Int32 * /*pnHandles*/,
            sal_Int32 nCount,
            sal_Bool bVetoable,
            bool bIgnoreRuntimeExceptionsWhileFiring)
    {
        if (nCount && !bVetoable) {
            setModifiedImpl(sal_True, bIgnoreRuntimeExceptionsWhileFiring);
        }
    }

    void OPropertyBag::setModifiedImpl(::sal_Bool bModified,
            bool bIgnoreRuntimeExceptionsWhileFiring)
    {
        { // do not lock mutex while notifying (#i93514#) to prevent deadlock
            ::osl::MutexGuard aGuard( m_aMutex );
            m_isModified = bModified;
        }
        if (bModified) {
            try {
                Reference<XInterface> xThis(*this);
                EventObject event(xThis);
                m_NotifyListeners.notifyEach(
                    &XModifyListener::modified, event);
            } catch (RuntimeException &) {
                if (!bIgnoreRuntimeExceptionsWhileFiring) {
                    throw;
                }
            } catch (Exception &) {
                // ignore
            }
        }
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL OPropertyBag::isModified()
        throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_isModified;
    }

    void SAL_CALL OPropertyBag::setModified( ::sal_Bool bModified )
        throw (PropertyVetoException, RuntimeException)
    {
        setModifiedImpl(bModified, false);
    }

    void SAL_CALL OPropertyBag::addModifyListener(
        const Reference< XModifyListener > & xListener)
        throw (RuntimeException)
    {
        m_NotifyListeners.addInterface(xListener);
    }

    void SAL_CALL OPropertyBag::removeModifyListener(
        const Reference< XModifyListener > & xListener)
        throw (RuntimeException)
    {
        m_NotifyListeners.removeInterface(xListener);
    }

    //--------------------------------------------------------------------
    Reference< XPropertySetInfo > SAL_CALL OPropertyBag::getPropertySetInfo(  ) throw(RuntimeException)
    {
        return createPropertySetInfo( getInfoHelper() );
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL OPropertyBag::has( const Any& /*aElement*/ ) throw (RuntimeException)
    {
        // XSet is only a workaround for addProperty not being able to add default-void properties.
        // So, everything of XSet except insert is implemented empty
        return sal_False;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OPropertyBag::insert( const Any& _element ) throw (IllegalArgumentException, ElementExistException, RuntimeException)
    {
        // This is a workaround for addProperty not being able to add default-void properties.
        // If we ever have a smarter XPropertyContainer::addProperty interface, we can remove this, ehm, well, hack.
        Property aProperty;
        if ( !( _element >>= aProperty ) )
            throw IllegalArgumentException( ::rtl::OUString(), *this, 1 );

        ::osl::ClearableMutexGuard g( m_aMutex );

        // check whether the type is allowed, everything else will be checked
        // by m_aDynamicProperties
        if  (   !m_aAllowedTypes.empty()
            &&  m_aAllowedTypes.find( aProperty.Type ) == m_aAllowedTypes.end()
            )
            throw IllegalTypeException( ::rtl::OUString(), *this );

        m_aDynamicProperties.addVoidProperty( aProperty.Name, aProperty.Type, findFreeHandle(), aProperty.Attributes );

        // our property info is dirty
        m_pArrayHelper.reset();

        g.clear();
        setModified(sal_True);
    }

    //--------------------------------------------------------------------
    void SAL_CALL OPropertyBag::remove( const Any& /*aElement*/ ) throw (IllegalArgumentException, NoSuchElementException, RuntimeException)
    {
        // XSet is only a workaround for addProperty not being able to add default-void properties.
        // So, everything of XSet except insert is implemented empty
        throw NoSuchElementException( ::rtl::OUString(), *this );
    }


    //--------------------------------------------------------------------
    Reference< XEnumeration > SAL_CALL OPropertyBag::createEnumeration(  ) throw (RuntimeException)
    {
        // XSet is only a workaround for addProperty not being able to add default-void properties.
        // So, everything of XSet except insert is implemented empty
        return NULL;
    }

    //--------------------------------------------------------------------
    Type SAL_CALL OPropertyBag::getElementType(  ) throw (RuntimeException)
    {
        // XSet is only a workaround for addProperty not being able to add default-void properties.
        // So, everything of XSet except insert is implemented empty
        return Type();
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL OPropertyBag::hasElements(  ) throw (RuntimeException)
    {
        // XSet is only a workaround for addProperty not being able to add default-void properties.
        // So, everything of XSet except insert is implemented empty
        return sal_False;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OPropertyBag::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        m_aDynamicProperties.getFastPropertyValue( _nHandle, _rValue );
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OPropertyBag::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue ) throw (IllegalArgumentException)
    {
        return m_aDynamicProperties.convertFastPropertyValue( _nHandle, _rValue, _rConvertedValue, _rOldValue );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OPropertyBag::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (Exception)
    {
        m_aDynamicProperties.setFastPropertyValue( nHandle, rValue );
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& SAL_CALL OPropertyBag::getInfoHelper()
    {
        if ( !m_pArrayHelper.get() )
        {
            Sequence< Property > aProperties;
            m_aDynamicProperties.describeProperties( aProperties );
            m_pArrayHelper.reset( new ::cppu::OPropertyArrayHelper( aProperties ) );
        }
        return *m_pArrayHelper;

    }

    //--------------------------------------------------------------------
    sal_Int32 OPropertyBag::findFreeHandle() const
    {
        const sal_Int32 nPrime = 1009;
        const sal_Int32 nSeed = 11;

        sal_Int32 nCheck = nSeed;
        while ( m_aDynamicProperties.hasPropertyByHandle( nCheck ) && ( nCheck != 1 ) )
        {
            nCheck = ( nCheck * nSeed ) % nPrime;
        }

        if ( nCheck == 1 )
        {   // uh ... we already have 1008 handles used up
            // -> simply count upwards
            while ( m_aDynamicProperties.hasPropertyByHandle( nCheck ) )
                ++nCheck;
        }

        return nCheck;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OPropertyBag::addProperty( const ::rtl::OUString& _rName, ::sal_Int16 _nAttributes, const Any& _rInitialValue ) throw (PropertyExistException, IllegalTypeException, IllegalArgumentException, RuntimeException)
    {
        ::osl::ClearableMutexGuard g( m_aMutex );

        // check whether the type is allowed, everything else will be checked
        // by m_aDynamicProperties
        Type aPropertyType = _rInitialValue.getValueType();
        if  (   _rInitialValue.hasValue()
            &&  !m_aAllowedTypes.empty()
            &&  m_aAllowedTypes.find( aPropertyType ) == m_aAllowedTypes.end()
            )
            throw IllegalTypeException( ::rtl::OUString(), *this );

        m_aDynamicProperties.addProperty( _rName, findFreeHandle(), _nAttributes, _rInitialValue );

        // our property info is dirty
        m_pArrayHelper.reset();

        g.clear();
        setModified(sal_True);
    }

    //--------------------------------------------------------------------
    void SAL_CALL OPropertyBag::removeProperty( const ::rtl::OUString& _rName ) throw (UnknownPropertyException, NotRemoveableException, RuntimeException)
    {
        ::osl::ClearableMutexGuard g( m_aMutex );

        m_aDynamicProperties.removeProperty( _rName );

        // our property info is dirty
        m_pArrayHelper.reset();

        g.clear();
        setModified(sal_True);
    }

    //--------------------------------------------------------------------
    namespace
    {
        struct ComparePropertyValueByName : public ::std::binary_function< PropertyValue, PropertyValue, bool >
        {
            bool operator()( const PropertyValue& _rLHS, const PropertyValue& _rRHS )
            {
                return _rLHS.Name < _rRHS.Name;
            }
        };

        template< typename CLASS >
        struct TransformPropertyToName : public ::std::unary_function< CLASS, ::rtl::OUString >
        {
            const ::rtl::OUString& operator()( const CLASS& _rProp )
            {
                return _rProp.Name;
            }
        };

        struct ExtractPropertyValue : public ::std::unary_function< PropertyValue, Any >
        {
            const Any& operator()( const PropertyValue& _rProp )
            {
                return _rProp.Value;
            }
        };
    }

    //--------------------------------------------------------------------
    Sequence< PropertyValue > SAL_CALL OPropertyBag::getPropertyValues(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // all registered properties
        Sequence< Property > aProperties;
        m_aDynamicProperties.describeProperties( aProperties );

        // their names
        Sequence< ::rtl::OUString > aNames( aProperties.getLength() );
        ::std::transform(
            aProperties.getConstArray(),
            aProperties.getConstArray() + aProperties.getLength(),
            aNames.getArray(),
            TransformPropertyToName< Property >()
        );

        // their values
        Sequence< Any > aValues;
        try
        {
            aValues = OPropertyBag_PBase::getPropertyValues( aNames );
            if ( aValues.getLength() != aNames.getLength() )
                throw RuntimeException();
        }
        catch( const RuntimeException& )
        {
            throw;
        }
        catch( const Exception& )
        {
            // ignore
        }

        // merge names and values, and retrieve the state/handle
        ::cppu::IPropertyArrayHelper& rPropInfo = getInfoHelper();

        Sequence< PropertyValue > aPropertyValues( aNames.getLength() );
        const ::rtl::OUString* pName = aNames.getConstArray();
        const ::rtl::OUString* pNamesEnd = aNames.getConstArray() + aNames.getLength();
        const Any* pValue = aValues.getArray();
        PropertyValue* pPropertyValue = aPropertyValues.getArray();

        for ( ; pName != pNamesEnd; ++pName, ++pValue, ++pPropertyValue )
        {
            pPropertyValue->Name = *pName;
            pPropertyValue->Handle = rPropInfo.getHandleByName( *pName );
            pPropertyValue->Value = *pValue;
            pPropertyValue->State = getPropertyStateByHandle( pPropertyValue->Handle );
        }

        return aPropertyValues;
    }

    //--------------------------------------------------------------------
    void OPropertyBag::impl_setPropertyValues_throw( const Sequence< PropertyValue >& _rProps )
    {
        // sort (the XMultiPropertySet interface requires this)
        Sequence< PropertyValue > aProperties( _rProps );
        ::std::sort(
            aProperties.getArray(),
            aProperties.getArray() + aProperties.getLength(),
            ComparePropertyValueByName()
        );

        // a sequence of names
        Sequence< ::rtl::OUString > aNames( aProperties.getLength() );
        ::std::transform(
            aProperties.getConstArray(),
            aProperties.getConstArray() + aProperties.getLength(),
            aNames.getArray(),
            TransformPropertyToName< PropertyValue >()
        );

        try
        {
            // check for unknown properties
            // we cannot simply rely on the XMultiPropertySet::setPropertyValues
            // implementation of our base class, since it does not throw
            // an UnknownPropertyException. More precise, XMultiPropertySet::setPropertyValues
            // does not allow to throw this exception, while XPropertyAccess::setPropertyValues
            // requires it
            sal_Int32 nCount = aNames.getLength();

            Sequence< sal_Int32 > aHandles( nCount );
            sal_Int32* pHandle = aHandles.getArray();
            const PropertyValue* pProperty = aProperties.getConstArray();
            for (   const ::rtl::OUString* pName = aNames.getConstArray();
                    pName != aNames.getConstArray() + aNames.getLength();
                    ++pName, ++pHandle, ++pProperty
                )
            {
                ::cppu::IPropertyArrayHelper& rPropInfo = getInfoHelper();
                *pHandle = rPropInfo.getHandleByName( *pName );
                if ( *pHandle != -1 )
                    continue;

                // there's a property requested which we do not know
                if ( m_bAutoAddProperties )
                {
                    // add the property
                    sal_Int16 nAttributes = PropertyAttribute::BOUND | PropertyAttribute::REMOVEABLE | PropertyAttribute::MAYBEDEFAULT;
                    addProperty( *pName, nAttributes, pProperty->Value );
                    continue;
                }

                // no way out
                throw UnknownPropertyException( *pName, *this );
            }

            // a sequence of values
            Sequence< Any > aValues( aProperties.getLength() );
            ::std::transform(
                aProperties.getConstArray(),
                aProperties.getConstArray() + aProperties.getLength(),
                aValues.getArray(),
                ExtractPropertyValue()
            );

            setFastPropertyValues( nCount, aHandles.getArray(), aValues.getConstArray(), nCount );
        }
        catch( const PropertyVetoException& )       { throw; }
        catch( const IllegalArgumentException& )    { throw; }
        catch( const WrappedTargetException& )      { throw; }
        catch( const RuntimeException& )            { throw; }
        catch( const UnknownPropertyException& )    { throw; }
        catch( const Exception& )
        {
            throw WrappedTargetException( ::rtl::OUString(), *this, ::cppu::getCaughtException() );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL OPropertyBag::setPropertyValues( const Sequence< PropertyValue >& _rProps ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_setPropertyValues_throw( _rProps );
    }

    //--------------------------------------------------------------------
    PropertyState OPropertyBag::getPropertyStateByHandle( sal_Int32 _nHandle )
    {
        // for properties which do not support the MAYBEDEFAULT attribute, don't rely on the base class, but
        // assume they're always in DIRECT state.
        // (Note that this probably would belong into the base class. However, this would mean we would need
        // to check all existent usages of the base class, where MAYBEDEFAULT is *not* set, but
        // a default is nonetheless supplied/used. This is hard to accomplish reliably, in the
        // current phase.
        // #i78593# / 2007-07-07 / frank.schoenheit@sun.com

        ::cppu::IPropertyArrayHelper& rPropInfo = getInfoHelper();
        sal_Int16 nAttributes(0);
        OSL_VERIFY( rPropInfo.fillPropertyMembersByHandle( NULL, &nAttributes, _nHandle ) );
        if ( ( nAttributes & PropertyAttribute::MAYBEDEFAULT ) == 0 )
            return PropertyState_DIRECT_VALUE;

        return OPropertyBag_PBase::getPropertyStateByHandle( _nHandle );
    }

    //--------------------------------------------------------------------
    Any OPropertyBag::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
    {
        Any aDefault;
        m_aDynamicProperties.getPropertyDefaultByHandle( _nHandle, aDefault );
        return aDefault;
    }

//........................................................................
}   // namespace comphelper
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
