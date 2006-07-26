/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xsdvalidationpropertyhandler.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 08:02:35 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONPROPERTYHANDLER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONPROPERTYHANDLER_HXX

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYHANDLER_HXX
#include "propertyhandler.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <memory>

//........................................................................
namespace pcr
{
//........................................................................

    class XSDValidationHelper;
    //====================================================================
    //= XSDValidationPropertyHandler
    //====================================================================
    class XSDValidationPropertyHandler;
    typedef HandlerComponentBase< XSDValidationPropertyHandler > XSDValidationPropertyHandler_Base;
    class XSDValidationPropertyHandler : public XSDValidationPropertyHandler_Base
    {
    private:
        ::std::auto_ptr< XSDValidationHelper >  m_pHelper;

    public:
        XSDValidationPropertyHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
        );

        static ::rtl::OUString SAL_CALL getImplementationName_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        ~XSDValidationPropertyHandler();

    protected:
        // XPropertyHandler overriables
        virtual ::com::sun::star::uno::Any  SAL_CALL getPropertyValue( const ::rtl::OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void                        SAL_CALL setPropertyValue( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                            SAL_CALL getSupersededProperties( ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                            SAL_CALL getActuatingProperties( ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::inspection::LineDescriptor
                                            SAL_CALL describePropertyLine( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::inspection::InteractiveSelectionResult
                                            SAL_CALL onInteractivePropertySelection( const ::rtl::OUString& _rPropertyName, sal_Bool _bPrimary, ::com::sun::star::uno::Any& _rData, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual void                        SAL_CALL actuatingPropertyChanged( const ::rtl::OUString& _rActuatingPropertyName, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual void                        SAL_CALL addPropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void                        SAL_CALL removePropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException);

        // PropertyHandler overridables
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >
                                            SAL_CALL doDescribeSupportedProperties() const;
        virtual void onNewComponent();

    private:
        bool    implPrepareRemoveCurrentDataType() SAL_THROW(());
        bool    implDoRemoveCurrentDataType() SAL_THROW(());

        bool    implPrepareCloneDataCurrentType( ::rtl::OUString& _rNewName ) SAL_THROW(());
        bool    implDoCloneCurrentDataType( const ::rtl::OUString& _rNewName ) SAL_THROW(());

        /** retrieves the names of the data types which our introspectee can be validated against
        */
        void    implGetAvailableDataTypeNames( ::std::vector< ::rtl::OUString >& /* [out] */ _rNames ) const SAL_THROW(());
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONPROPERTYHANDLER_HXX

