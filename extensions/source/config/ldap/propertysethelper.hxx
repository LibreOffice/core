/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertysethelper.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef EXTENSIONS_MISC_PROPERTYSETHELPER_HXX
#define EXTENSIONS_MISC_PROPERTYSETHELPER_HXX

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/weak.hxx>

//..........................................................................
namespace extensions {
    namespace apihelper {
//..........................................................................
        namespace uno   = com::sun::star::uno;
        namespace lang  = com::sun::star::lang;
        namespace beans = com::sun::star::beans;
//..........................................................................
class BroadcasterBase
{
    osl::Mutex              m_aMutex;
    cppu::OBroadcastHelper  m_aBroadcastHelper;

protected:
    BroadcasterBase() : m_aMutex(), m_aBroadcastHelper(m_aMutex) {}
    ~BroadcasterBase() {}

    osl::Mutex &             getBroadcastMutex()  { return m_aMutex; }
    cppu::OBroadcastHelper & getBroadcastHelper() { return m_aBroadcastHelper; }
};

//..........................................................................

class PropertySetHelper : protected BroadcasterBase // must be first
                        , public cppu::OWeakObject
                        , public cppu::OPropertySetHelper // not copyable
{
public:
    PropertySetHelper();
    ~PropertySetHelper();

    // XInterface
    virtual uno::Any SAL_CALL queryInterface( uno::Type const & rType ) throw (uno::RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();
    // XTypeProvider
    virtual uno::Sequence< uno::Type > SAL_CALL getTypes() throw (uno::RuntimeException);

    // to be provided by derived classes
    // virtual uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (uno::RuntimeException) = 0;

    // XPropertySet
     virtual uno::Reference< beans::XPropertySetInfo > SAL_CALL
         getPropertySetInfo(  ) throw (uno::RuntimeException);


protected:
// new methods still to be overridden
    virtual cppu::IPropertyArrayHelper * SAL_CALL newInfoHelper() = 0;

// cppu::OPropertySetHelper interface
#if 0 // these methods still must be overridden
    virtual cppu::IPropertyArrayHelper * SAL_CALL newInfoHelper() = 0;

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const uno::Any& rValue )
            throw (uno::Exception) = 0;

    virtual void SAL_CALL getFastPropertyValue( uno::Any& rValue, sal_Int32 nHandle ) const = 0;
#endif
    virtual cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // default implementation: does not do any conversion
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        uno::Any & rConvertedValue, uno::Any & rOldValue,
        sal_Int32 nHandle, const uno::Any& rValue )
            throw (lang::IllegalArgumentException);
private:
    cppu::IPropertyArrayHelper * m_pHelper;
};
//..........................................................................

class ReadOnlyPropertySetHelper : public PropertySetHelper
{
public:
    ReadOnlyPropertySetHelper() {}
    ~ReadOnlyPropertySetHelper() {}

protected:
#if 0 // these methods still must be overridden
    virtual cppu::IPropertyArrayHelper * SAL_CALL newInfoHelper() = 0;

    virtual void SAL_CALL getFastPropertyValue( uno::Any& rValue, sal_Int32 nHandle ) const = 0;
#endif
    // default implementation: rejects any attempt to set a value
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const uno::Any& rValue )
            throw (uno::Exception);

    // default implementation: rejects any attempt to do a conversion
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        uno::Any & rConvertedValue, uno::Any & rOldValue,
        sal_Int32 nHandle, const uno::Any& rValue )
            throw (lang::IllegalArgumentException);
};
//..........................................................................
    }   // namespace apihelper
}   // namespace extensions
//..........................................................................

#endif


