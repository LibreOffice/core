/*************************************************************************
 *
 *  $RCSfile: eformspropertyhandler.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-07-01 11:50:00 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_EFORMSPROPERTYHANDLER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_EFORMSPROPERTYHANDLER_HXX

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

    class EFormsHelper;
    //====================================================================
    //= EFormsPropertyHandler
    //====================================================================
    class EFormsPropertyHandler : public PropertyHandler
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                        m_xORB;
        ::std::auto_ptr< EFormsHelper > m_pHelper;
        /** current value of the Model property, if there is no binding, yet
        */
        ::rtl::OUString                 m_sBindingLessModelName;

    public:
        // HandlerFactory
        inline static IPropertyHandler* Create(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxIntrospectee,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument,
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >& _rxTypeConverter
        )
        {
            return new EFormsPropertyHandler( _rxORB, _rxIntrospectee, _rxContextDocument, _rxTypeConverter );
        }

    protected:
        EFormsPropertyHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxIntrospectee,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument,
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >& _rxTypeConverter
        );

        ~EFormsPropertyHandler();

    protected:
        // IPropertyHandler overriables
        virtual bool                        SAL_CALL supportsUIDescriptor(  PropertyId _nPropId ) const;
        virtual ::com::sun::star::uno::Any  SAL_CALL getPropertyValue( PropertyId _nPropId, bool _bLazy = true ) const;
        virtual void                        SAL_CALL setPropertyValue( PropertyId _nPropId, const ::com::sun::star::uno::Any& _rValue );
        virtual ::std::vector< ::rtl::OUString >
                                            SAL_CALL getActuatingProperties( ) const;
        virtual void                        SAL_CALL describePropertyUI( PropertyId _nPropId, PropertyUIDescriptor& /* [out] */ _rDescriptor ) const;
        virtual void                        SAL_CALL initializePropertyUI( PropertyId _nPropId, IPropertyBrowserUI* _pUpdater );
        virtual bool                        SAL_CALL requestUserInputOnButtonClick( PropertyId _nPropId, bool _bPrimary, ::com::sun::star::uno::Any& _rData );
        virtual void                        SAL_CALL executeButtonClick( PropertyId _nPropId, bool _bPrimary, const ::com::sun::star::uno::Any& _rData, IPropertyBrowserUI* _pUpdater );
        virtual void                        SAL_CALL actuatingPropertyChanged( PropertyId _nActuatingPropId, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, IPropertyBrowserUI* _pUpdater, bool );

        virtual ::com::sun::star::uno::Any  SAL_CALL getPropertyValueFromStringRep( PropertyId _nPropId, const ::rtl::OUString& _rStringRep ) const;
        virtual ::rtl::OUString             SAL_CALL getStringRepFromPropertyValue( PropertyId _nPropId, const ::com::sun::star::uno::Any& _rValue ) const;

        virtual void                        SAL_CALL startAllPropertyChangeListening( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener );
        virtual void                        SAL_CALL stopAllPropertyChangeListening( );

        // PropertyHandler overridables
        virtual ::std::vector< ::com::sun::star::beans::Property >
                                            SAL_CALL implDescribeSupportedProperties() const;

    protected:
        /** returns the value of the PROPERTY_XML_DATA_MODEL property.

            An extra method is necessary here, which respects both the value set at our helper,
            and <member>m_sBindingLessModelName</member>
        */
        ::rtl::OUString getModelNamePropertyValue() const;
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_EFORMSPROPERTYHANDLER_HXX

