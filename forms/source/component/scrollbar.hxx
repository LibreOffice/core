/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef FORMS_SOURCE_COMPONENT_SCROLLBAR_HXX
#define FORMS_SOURCE_COMPONENT_SCROLLBAR_HXX

#include "FormComponent.hxx"
#include "frm_module.hxx"

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

#endif // FORMS_SOURCE_COMPONENT_SCROLLBAR_HXX
