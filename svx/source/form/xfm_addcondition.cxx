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


#include <xfm_addcondition.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <datanavi.hxx>
#include <fmservs.hxx>

namespace svxform
{

#define PROPERTY_ID_BINDING             5724
#define PROPERTY_ID_FORM_MODEL          5725
#define PROPERTY_ID_FACET_NAME          5726
#define PROPERTY_ID_CONDITION_VALUE     5727

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xforms;


    //= OAddConditionDialog


    Reference< XInterface > OAddConditionDialog_Create( const Reference< XMultiServiceFactory > & _rxORB )
    {
        return OAddConditionDialog::Create( _rxORB );
    }


    Sequence< OUString > OAddConditionDialog_GetSupportedServiceNames()
    {
        return { "com.sun.star.xforms.ui.dialogs.AddCondition" };
    }


    OUString OAddConditionDialog_GetImplementationName()
    {
        return OUString("org.openoffice.comp.svx.OAddConditionDialog");
    }

    OAddConditionDialog::OAddConditionDialog( const Reference< XComponentContext >& _rxORB )
        :OAddConditionDialogBase( _rxORB )
    {
        registerProperty(
            "Binding",
            PROPERTY_ID_BINDING,
            PropertyAttribute::TRANSIENT,
            &m_xBinding,
            cppu::UnoType<decltype(m_xBinding)>::get()
        );

        registerProperty(
            "FacetName",
            PROPERTY_ID_FACET_NAME,
            PropertyAttribute::TRANSIENT,
            &m_sFacetName,
            cppu::UnoType<decltype(m_sFacetName)>::get()
        );

        registerProperty(
            "ConditionValue",
            PROPERTY_ID_CONDITION_VALUE,
            PropertyAttribute::TRANSIENT,
            &m_sConditionValue,
            cppu::UnoType<decltype(m_sConditionValue)>::get()
        );

        registerProperty(
            "FormModel",
            PROPERTY_ID_FORM_MODEL,
            PropertyAttribute::TRANSIENT,
            &m_xWorkModel,
            cppu::UnoType<decltype(m_xWorkModel)>::get()
        );
    }


    Sequence<sal_Int8> SAL_CALL OAddConditionDialog::getImplementationId(  )
    {
        return css::uno::Sequence<sal_Int8>();
    }


    Reference< XInterface > OAddConditionDialog::Create( const Reference< XMultiServiceFactory >& _rxFactory )
    {
        return *( new OAddConditionDialog( comphelper::getComponentContext(_rxFactory) ) );
    }


    OUString SAL_CALL OAddConditionDialog::getImplementationName()
    {
        return OAddConditionDialog_GetImplementationName();
    }


    Sequence< OUString > SAL_CALL OAddConditionDialog::getSupportedServiceNames()
    {
        return OAddConditionDialog_GetSupportedServiceNames();
    }


    Reference<XPropertySetInfo>  SAL_CALL OAddConditionDialog::getPropertySetInfo()
    {
        return createPropertySetInfo( getInfoHelper() );
    }

    ::cppu::IPropertyArrayHelper& OAddConditionDialog::getInfoHelper()
    {
        return *getArrayHelper();
    }

    ::cppu::IPropertyArrayHelper* OAddConditionDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProperties;
        describeProperties( aProperties );
        return new ::cppu::OPropertyArrayHelper( aProperties );
    }

    svt::OGenericUnoDialog::Dialog OAddConditionDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
    {
        if ( !m_xBinding.is() || m_sFacetName.isEmpty() )
            throw RuntimeException( OUString(), *this );

        return svt::OGenericUnoDialog::Dialog(std::make_unique<AddConditionDialog>(Application::GetFrameWeld(rParent), m_sFacetName, m_xBinding));
    }

    void OAddConditionDialog::executedDialog( sal_Int16 _nExecutionResult )
    {
        OAddConditionDialogBase::executedDialog( _nExecutionResult );
        if ( _nExecutionResult == RET_OK )
            m_sConditionValue = static_cast< AddConditionDialog* >( m_aDialog.m_xWeldDialog.get() )->GetCondition();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
