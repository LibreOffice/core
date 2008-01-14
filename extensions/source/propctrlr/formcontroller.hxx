/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formcontroller.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:58:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_FORMCONTROLLER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_FORMCONTROLLER_HXX

#ifndef _EXTENSIONS_PROPCTRLR_PROPCONTROLLER_HXX_
#include "propcontroller.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

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

