/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: formcontroller.hxx,v $
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
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_FORMCONTROLLER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_FORMCONTROLLER_HXX

#include "propcontroller.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/
#include <cppuhelper/propshlp.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/uno3.hxx>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= ServiceDescriptor
    //====================================================================
    struct ServiceDescriptor
    {
        ::rtl::OUString
            ( *GetImplementationName )( void );
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
            ( *GetSupportedServiceNames )( void );
    };

    //====================================================================
    //= FormController
    //====================================================================
    class FormController;
    typedef ::cppu::OPropertySetHelper                                  FormController_PropertyBase1;
    typedef ::comphelper::OPropertyArrayUsageHelper< FormController >   FormController_PropertyBase2;

    /** Legacy implementation of com.sun.star.form.PropertyBrowserController

        Nowadays only a wrapper around an ObjectInspector using a
        DefaultFormComponentInspectorModel.
    */
    class FormController    :public OPropertyBrowserController
                            ,public FormController_PropertyBase1
                            ,public FormController_PropertyBase2
    {
    private:
        ServiceDescriptor           m_aServiceDescriptor;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                    m_xCurrentInspectee;
        bool                        m_bUseFormComponentHandlers;
    public:
        FormController(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            ServiceDescriptor _aServiceDescriptor,
            bool _bUseFormFormComponentHandlers
        );

        // XServiceInfo - static versions
        static ::rtl::OUString getImplementationName_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    protected:
        ~FormController();

        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        // XPropertySet and friends
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                ::com::sun::star::uno::Any & rConvertedValue, ::com::sun::star::uno::Any & rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue
            )   throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue
            ) throw (::com::sun::star::uno::Exception);
        virtual void SAL_CALL getFastPropertyValue(
                ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle
            ) const;
    private:
        using FormController_PropertyBase1::getFastPropertyValue;
    };

    //====================================================================
    //= DialogController
    //====================================================================
    /** Legacy implementation of com.sun.star.awt.PropertyBrowserController
    */
    class DialogController
    {
    public:
        // XServiceInfo - static versions
        static ::rtl::OUString getImplementationName_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    private:
        DialogController();                                     // never implemented
        DialogController( const DialogController& );            // never implemented
        DialogController& operator=( const DialogController& ); // never implemented
    };
//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_FORMCONTROLLER_HXX

