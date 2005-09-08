/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cellbindinghandler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:05:57 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_CELLBINDINGHANDLER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_CELLBINDINGHANDLER_HXX

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYHANDLER_HXX
#include "propertyhandler.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
/** === end UNO includes === **/

#include <memory>

//........................................................................
namespace pcr
{
//........................................................................

    class CellBindingHelper;
    //====================================================================
    //= CellBindingPropertyHandler
    //====================================================================
    class CellBindingPropertyHandler : public PropertyHandler
    {
    private:
        ::std::auto_ptr< CellBindingHelper >    m_pHelper;

    public:
        // HandlerFactory
        inline static IPropertyHandler* Create(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxIntrospectee,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument,
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >& _rxTypeConverter
        )
        {
            return new CellBindingPropertyHandler( _rxORB, _rxIntrospectee, _rxContextDocument, _rxTypeConverter );
        }

    protected:
        CellBindingPropertyHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxIntrospectee,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument,
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >& _rxTypeConverter
        );

        ~CellBindingPropertyHandler();

    protected:
        // IPropertyHandler overriables
        virtual ::com::sun::star::uno::Any      SAL_CALL getPropertyValue( PropertyId _nPropId, bool _bLazy = true ) const;
        virtual void                            SAL_CALL setPropertyValue( PropertyId _nPropId, const ::com::sun::star::uno::Any& _rValue );
        virtual ::com::sun::star::uno::Any      SAL_CALL getPropertyValueFromStringRep( PropertyId _nPropId, const ::rtl::OUString& _rStringRep ) const;
        virtual ::rtl::OUString                 SAL_CALL getStringRepFromPropertyValue( PropertyId _nPropId, const ::com::sun::star::uno::Any& _rValue ) const;

        // PropertyHandler overridables
        virtual ::std::vector< ::com::sun::star::beans::Property >
                                                SAL_CALL implDescribeSupportedProperties() const;
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_CELLBINDINGHANDLER_HXX

