/*************************************************************************
 *
 *  $RCSfile: propertysethelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-03-18 10:40:33 $
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

#ifndef EXTENSIONS_MISC_PROPERTYSETHELPER_HXX
#define EXTENSIONS_MISC_PROPERTYSETHELPER_HXX

#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

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


