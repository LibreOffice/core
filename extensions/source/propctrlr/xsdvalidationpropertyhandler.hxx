/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xsdvalidationpropertyhandler.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:33:57 $
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
    class XSDValidationPropertyHandler : public PropertyHandler
    {
    private:
        ::std::auto_ptr< XSDValidationHelper >  m_pHelper;

    public:
        // HandlerFactory
        inline static IPropertyHandler* Create(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxIntrospectee,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument,
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >& _rxTypeConverter
        )
        {
            return new XSDValidationPropertyHandler( _rxIntrospectee, _rxContextDocument, _rxTypeConverter );
        }

    protected:
        XSDValidationPropertyHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxIntrospectee,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument,
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >& _rxTypeConverter
        );
        ~XSDValidationPropertyHandler();

    protected:
        // IPropertyHandler overriables
        virtual bool                        SAL_CALL supportsUIDescriptor(  PropertyId _nPropId ) const;
        virtual ::com::sun::star::uno::Any  SAL_CALL getPropertyValue( PropertyId _nPropId, bool _bLazy = true ) const;
        virtual void                        SAL_CALL setPropertyValue( PropertyId _nPropId, const ::com::sun::star::uno::Any& _rValue );
        virtual ::std::vector< ::rtl::OUString >
                                            SAL_CALL getSupersededProperties( ) const;
        virtual ::std::vector< ::rtl::OUString >
                                            SAL_CALL getActuatingProperties( ) const;
        virtual void                        SAL_CALL describePropertyUI( PropertyId _nPropId, PropertyUIDescriptor& /* [out] */ _rDescriptor ) const;
        virtual void                        SAL_CALL initializePropertyUI( PropertyId _nPropId, IPropertyBrowserUI* _pUpdater );
        virtual bool                        SAL_CALL requestUserInputOnButtonClick( PropertyId _nPropId, bool _bPrimary, ::com::sun::star::uno::Any& _rData );
        virtual void                        SAL_CALL executeButtonClick( PropertyId _nPropId, bool _bPrimary, const ::com::sun::star::uno::Any& _rData, IPropertyBrowserUI* _pUpdater );
        virtual void                        SAL_CALL actuatingPropertyChanged( PropertyId _nActuatingPropId, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, IPropertyBrowserUI* _pUpdater, bool );
        virtual void                        SAL_CALL startAllPropertyChangeListening( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener );
        virtual void                        SAL_CALL stopAllPropertyChangeListening( );

        // PropertyHandler overridables
        virtual ::std::vector< ::com::sun::star::beans::Property >
                                            SAL_CALL implDescribeSupportedProperties() const;

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

