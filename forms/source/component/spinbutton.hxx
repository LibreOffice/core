/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_SPINBUTTON_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_SPINBUTTON_HXX

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
            throw ( ::com::sun::star::uno::Exception, std::exception );
        virtual sal_Bool SAL_CALL convertFastPropertyValue( ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
            throw ( ::com::sun::star::lang::IllegalArgumentException );

        // OBoundControlModel
        virtual ::com::sun::star::uno::Any
                                translateDbColumnToControlValue( );
        virtual sal_Bool        commitControlValueToDbColumn( bool _bPostReset );
        virtual ::com::sun::star::uno::Any
                                getDefaultForReset() const;

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >
                                getSupportedBindingTypes();
        virtual ::com::sun::star::uno::Any
                                translateExternalValueToControlValue( const ::com::sun::star::uno::Any& _rExternalValue ) const;
        virtual ::com::sun::star::uno::Any
                                translateControlValueToExternalValue( ) const;

        // XCoponent and related helpers
        virtual void SAL_CALL disposing();

        // prevent method hiding
        using OBoundControlModel::disposing;
        using OBoundControlModel::getFastPropertyValue;

    };
//........................................................................
} // namespacefrm
//........................................................................

#endif // INCLUDED_FORMS_SOURCE_COMPONENT_SPINBUTTON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
