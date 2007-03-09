/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scrollbar.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-09 13:34:42 $
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
#ifndef FORMS_SOURCE_COMPONENT_SCROLLBAR_HXX
#define FORMS_SOURCE_COMPONENT_SCROLLBAR_HXX

#ifndef _FORMS_FORMCOMPONENT_HXX_
#include "FormComponent.hxx"
#endif

#ifndef FRM_MODULE_HXX
#include "frm_module.hxx"
#endif

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= OScrollBarModel
    //====================================================================
    class OScrollBarModel   :public OBoundControlModel
    {
    private:
        // <properties>
        sal_Int32   m_nDefaultScrollValue;
        // </properties>

    protected:
        DECLARE_DEFAULT_LEAF_XTOR( OScrollBarModel );

        // XServiceInfo
        DECLARE_SERVICE_REGISTRATION( OScrollBarModel )

        // XPersistObject
        DECLARE_XPERSISTOBJECT()

        // XCloneable
        DECLARE_XCLONEABLE();

        // XPropertyState
        virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 _nHandle ) const;

        // OControlModel's property handling
        virtual void describeFixedProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
        ) const;

        // OPropertySetHelper
        virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& _rValue, sal_Int32 _nHandle ) const;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
            throw ( ::com::sun::star::uno::Exception );
        virtual sal_Bool SAL_CALL convertFastPropertyValue( ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
            throw ( ::com::sun::star::lang::IllegalArgumentException );

        // OBoundControlModel
        virtual ::com::sun::star::uno::Any
                                translateDbColumnToControlValue( );
        virtual sal_Bool        commitControlValueToDbColumn( bool _bPostReset );
        virtual ::com::sun::star::uno::Any
                                getDefaultForReset() const;

        virtual ::com::sun::star::uno::Any
                                translateExternalValueToControlValue( ) const;
        virtual ::com::sun::star::uno::Any
                                translateControlValueToExternalValue( ) const;
        virtual sal_Bool        approveValueBinding( const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XValueBinding >& _rxBinding );

        // XCoponent and related helpers
        virtual void SAL_CALL disposing();

        // prevent method hiding
        using OBoundControlModel::disposing;
        using OBoundControlModel::getFastPropertyValue;

    };
//........................................................................
} // namespacefrm
//........................................................................

#endif // FORMS_SOURCE_COMPONENT_SCROLLBAR_HXX
