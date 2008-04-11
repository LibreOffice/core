/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: spinbutton.hxx,v $
 * $Revision: 1.9 $
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
#ifndef FORMS_SOURCE_COMPONENT_SPINBUTTON_HXX
#define FORMS_SOURCE_COMPONENT_SPINBUTTON_HXX

#include "FormComponent.hxx"
#include "frm_module.hxx"

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= OSpinButtonModel
    //====================================================================
    class OSpinButtonModel   :public OBoundControlModel
    {
    private:
        // <properties>
        sal_Int32   m_nDefaultSpinValue;
        // </properties>

    protected:
        DECLARE_DEFAULT_LEAF_XTOR( OSpinButtonModel );

        // XServiceInfo
        DECLARE_SERVICE_REGISTRATION( OSpinButtonModel )

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

#endif // FORMS_SOURCE_COMPONENT_SPINBUTTON_HXX
