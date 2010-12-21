/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef __FRAMEWORK_CLASSES_ACTIONTRIGGERPROPERTYSET_HXX_
#define __FRAMEWORK_CLASSES_ACTIONTRIGGERPROPERTYSET_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>

#ifndef __COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <framework/fwedllapi.h>

#define SERVICENAME_ACTIONTRIGGER "com.sun.star.ui.ActionTrigger"
#define IMPLEMENTATIONNAME_ACTIONTRIGGER "com.sun.star.comp.ui.ActionTrigger"

namespace framework
{

class ActionTriggerPropertySet :  public ThreadHelpBase                       ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                                    public ::com::sun::star::lang::XServiceInfo ,
                                    public ::com::sun::star::lang::XTypeProvider,
                                    public ::cppu::OBroadcastHelper             ,
                                    public ::cppu::OPropertySetHelper           ,   // -> XPropertySet, XFastPropertySet, XMultiPropertySet
                                    public ::cppu::OWeakObject
{
    public:
        FWE_DLLPUBLIC ActionTriggerPropertySet( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
        FWE_DLLPUBLIC virtual ~ActionTriggerPropertySet();

        // XInterface
        virtual FWE_DLLPUBLIC ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
            throw (::com::sun::star::uno::RuntimeException);
        virtual FWE_DLLPUBLIC void SAL_CALL acquire() throw ();
        virtual FWE_DLLPUBLIC void SAL_CALL release() throw ();

        // XServiceInfo
        virtual FWE_DLLPUBLIC ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual FWE_DLLPUBLIC sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual FWE_DLLPUBLIC ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        virtual FWE_DLLPUBLIC ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual FWE_DLLPUBLIC ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);

    private:
        //---------------------------------------------------------------------------------------------------------
        //  OPropertySetHelper
        //---------------------------------------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL convertFastPropertyValue( com::sun::star::uno::Any&       aConvertedValue,
                                                            com::sun::star::uno::Any&       aOldValue,
                                                            sal_Int32                       nHandle,
                                                            const com::sun::star::uno::Any& aValue          )
            throw( com::sun::star::lang::IllegalArgumentException );


        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const com::sun::star::uno::Any& aValue )
            throw( com::sun::star::uno::Exception );

        using cppu::OPropertySetHelper::getFastPropertyValue;
        virtual void SAL_CALL getFastPropertyValue( com::sun::star::uno::Any& aValue, sal_Int32 nHandle ) const;

        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        virtual com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
            throw (::com::sun::star::uno::RuntimeException);

        static const com::sun::star::uno::Sequence< com::sun::star::beans::Property > impl_getStaticPropertyDescriptor();

        //---------------------------------------------------------------------------------------------------------
        //  helper
        //---------------------------------------------------------------------------------------------------------

        sal_Bool impl_tryToChangeProperty(  const   rtl::OUString&              aCurrentValue   ,
                                            const   com::sun::star::uno::Any&   aNewValue       ,
                                            com::sun::star::uno::Any&           aOldValue       ,
                                            com::sun::star::uno::Any&           aConvertedValue ) throw( com::sun::star::lang::IllegalArgumentException );

        sal_Bool impl_tryToChangeProperty(  const   com::sun::star::uno::Reference< com::sun::star::awt::XBitmap >  xBitmap,
                                            const   com::sun::star::uno::Any&   aNewValue       ,
                                            com::sun::star::uno::Any&           aOldValue       ,
                                            com::sun::star::uno::Any&           aConvertedValue ) throw( com::sun::star::lang::IllegalArgumentException );

        sal_Bool impl_tryToChangeProperty(  const   com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xInterface,
                                            const   com::sun::star::uno::Any&   aNewValue       ,
                                            com::sun::star::uno::Any&           aOldValue       ,
                                            com::sun::star::uno::Any&           aConvertedValue ) throw( com::sun::star::lang::IllegalArgumentException );

        //---------------------------------------------------------------------------------------------------------
        //  members
        //---------------------------------------------------------------------------------------------------------

        rtl::OUString                                                         m_aCommandURL;
        rtl::OUString                                                         m_aHelpURL;
        rtl::OUString                                                         m_aText;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >    m_xBitmap;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xActionTriggerContainer;
};

}

#endif // __FRAMEWORK_CLASSES_ACTIONTRIGGERPROPERTYSET_HXX_
