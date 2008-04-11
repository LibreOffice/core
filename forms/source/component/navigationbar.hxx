/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: navigationbar.hxx,v $
 * $Revision: 1.6 $
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

#ifndef FORMS_COMPONENT_NAVIGATION_BAR_HXX
#define FORMS_COMPONENT_NAVIGATION_BAR_HXX

#include "FormComponent.hxx"
#include <com/sun/star/io/XPersistObject.hpp>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainerhelper.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include "formcontrolfont.hxx"

#include <set>

//.........................................................................
namespace frm
{
//.........................................................................

    //==================================================================
    // ONavigationBarModel
    //==================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::awt::XControlModel
                                > ONavigationBarModel_BASE;

    class ONavigationBarModel
                        :public OControlModel
                        ,public FontControlModel
                        ,public OPropertyContainerHelper
                        ,public ONavigationBarModel_BASE
    {
        // <properties>
        ::com::sun::star::uno::Any          m_aTabStop;
        ::com::sun::star::uno::Any          m_aBackgroundColor;
        ::rtl::OUString                     m_sDefaultControl;
        ::rtl::OUString                     m_sHelpText;
        ::rtl::OUString                     m_sHelpURL;
        sal_Int16                           m_nIconSize;
        sal_Int16                           m_nBorder;
        sal_Int32                           m_nDelay;
        sal_Bool                            m_bEnabled;
        sal_Bool                            m_bShowPosition;
        sal_Bool                            m_bShowNavigation;
        sal_Bool                            m_bShowActions;
        sal_Bool                            m_bShowFilterSort;
        // </properties>

    public:
        DECLARE_DEFAULT_LEAF_XTOR( ONavigationBarModel );

        // XServiceInfo - static version
        static  ::rtl::OUString SAL_CALL getImplementationName_Static();
        static  ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static();
        static  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory );

    protected:
        // UNO
        DECLARE_UNO3_AGG_DEFAULTS( ONavigationBarModel, OControlModel );
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName()  throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()  throw(::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // OComponentHelper
        virtual void SAL_CALL disposing();

        // XPersistObject
        virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

        // XPropertySet
        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                              sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                            throw(::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception);

        // XPropertyState
        virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const;

        // OControlModel's property handling
        virtual void describeFixedProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
        ) const;

        // prevent method hiding
        using OControlModel::disposing;
        using OControlModel::getFastPropertyValue;

    protected:
        DECLARE_XCLONEABLE();

    private:
        void implInitPropertyContainer();
    };

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // FORMS_COMPONENT_NAVIGATION_BAR_HXX

