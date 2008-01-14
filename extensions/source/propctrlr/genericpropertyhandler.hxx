/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: genericpropertyhandler.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:59:13 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_GENERICPROPERTYHANDLER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_GENERICPROPERTYHANDLER_HXX

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYHANDLER_HXX
#include "propertyhandler.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PCRCOMMONTYPES_HXX
#include "pcrcommontypes.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_
#include "pcrcommon.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PCROMPONENTCONTEXT_HXX
#include "pcrcomponentcontext.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XINTROSPECTIONACCESS_HPP_
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#include <map>

//........................................................................
namespace pcr
{
//........................................................................

    struct TypeLess : ::std::binary_function< ::com::sun::star::uno::Type, ::com::sun::star::uno::Type, bool >
    {
        bool operator()( const ::com::sun::star::uno::Type& _rLHS, const ::com::sun::star::uno::Type& _rRHS ) const
        {
            return _rLHS.getTypeName() < _rRHS.getTypeName();
        }
    };

    class IPropertyInfoService;
    class IPropertyEnumRepresentation;
    //====================================================================
    //= GenericPropertyHandler
    //====================================================================
    typedef ::cppu::WeakComponentImplHelper2    <   ::com::sun::star::inspection::XPropertyHandler
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                >   GenericPropertyHandler_Base;
    class GenericPropertyHandler : public GenericPropertyHandler_Base
    {
    private:
        mutable ::osl::Mutex    m_aMutex;

    private:
        /// the service factory for creating services
        ComponentContext                                                                    m_aContext;
        /// need this to keep alive as long as m_xComponent lives
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess >   m_xComponentIntrospectionAccess;
        /// the properties of the object we're handling
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >           m_xComponent;
        /// cached interface of ->m_xComponent
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >         m_xPropertyState;
        /// type converter, needed on various occasions
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >        m_xTypeConverter;
        /// cache of our supported properties
        PropertyMap                                                                         m_aProperties;
        /// property change listeners
        ::cppu::OInterfaceContainerHelper                                                   m_aPropertyListeners;
        ::std::map< ::com::sun::star::uno::Type, ::rtl::Reference< IPropertyEnumRepresentation >, TypeLess >
                                                                                            m_aEnumConverters;

        /// has ->m_aProperties been initialized?
        bool    m_bPropertyMapInitialized : 1;

    public:
        // XServiceInfo - static versions
        static ::rtl::OUString SAL_CALL getImplementationName_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext );

    protected:
        GenericPropertyHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
        );

        ~GenericPropertyHandler();

    protected:
        // XPropertyHandler overridables
        virtual void                                                SAL_CALL inspect( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxIntrospectee ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any                          SAL_CALL getPropertyValue( const ::rtl::OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void                                                SAL_CALL setPropertyValue( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any                          SAL_CALL convertToPropertyValue( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rControlValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any                          SAL_CALL convertToControlValue( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rPropertyValue, const ::com::sun::star::uno::Type& _rControlValueType ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::beans::PropertyState              SAL_CALL getPropertyState( const ::rtl::OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void                                                SAL_CALL addPropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void                                                SAL_CALL removePropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >
                                                                    SAL_CALL getSupportedProperties() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                                                    SAL_CALL getSupersededProperties() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >  SAL_CALL getActuatingProperties() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::inspection::LineDescriptor        SAL_CALL describePropertyLine( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool                                          SAL_CALL isComposable( const ::rtl::OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::inspection::InteractiveSelectionResult
                                                                    SAL_CALL onInteractivePropertySelection( const ::rtl::OUString& _rPropertyName, sal_Bool _bPrimary, ::com::sun::star::uno::Any& _rData, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual void                                                SAL_CALL actuatingPropertyChanged( const ::rtl::OUString& _rActuatingPropertyName, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool                                            SAL_CALL suspend( sal_Bool _bSuspend ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        DECLARE_XCOMPONENT()
        virtual void                                                SAL_CALL disposing();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    private:
        /** ensures that ->m_aProperties is initialized
            @precond
                our mutex is locked
        */
        void    impl_ensurePropertyMap();

        /** retrieves the enum converter for the given ENUM type
        */
        ::rtl::Reference< IPropertyEnumRepresentation >
                impl_getEnumConverter( const ::com::sun::star::uno::Type& _rEnumType );

    private:
        GenericPropertyHandler();                                           // never implemented
        GenericPropertyHandler( const GenericPropertyHandler& );            // never implemented
        GenericPropertyHandler& operator=( const GenericPropertyHandler& ); // never implemented
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_GENERICPROPERTYHANDLER_HXX

