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

#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include "MasterDetailLinkDialog.hxx"
#include "formlinkdialog.hxx"
#include "pcrservices.hxx"

extern "C" void createRegistryInfo_MasterDetailLinkDialog()
{
    ::pcr::OAutoRegistration< ::pcr::MasterDetailLinkDialog > aAutoRegistration;
}

namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    MasterDetailLinkDialog::MasterDetailLinkDialog(const Reference< XComponentContext >& _rxContext )
        :OGenericUnoDialog( _rxContext )
    {
    }

    Sequence<sal_Int8> SAL_CALL MasterDetailLinkDialog::getImplementationId(  )
    {
        return css::uno::Sequence<sal_Int8>();
    }


    Reference< XInterface > MasterDetailLinkDialog::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new MasterDetailLinkDialog( _rxContext ) );
    }


    OUString SAL_CALL MasterDetailLinkDialog::getImplementationName()
    {
        return getImplementationName_static();
    }


    OUString MasterDetailLinkDialog::getImplementationName_static()
    {
        return OUString("org.openoffice.comp.form.ui.MasterDetailLinkDialog");
    }


    css::uno::Sequence<OUString> SAL_CALL MasterDetailLinkDialog::getSupportedServiceNames()
    {
        return getSupportedServiceNames_static();
    }


    css::uno::Sequence<OUString> MasterDetailLinkDialog::getSupportedServiceNames_static()
    {
        css::uno::Sequence<OUString> aSupported { "com.sun.star.form.MasterDetailLinkDialog" };
        return aSupported;
    }


    Reference<XPropertySetInfo>  SAL_CALL MasterDetailLinkDialog::getPropertySetInfo()
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    ::cppu::IPropertyArrayHelper& MasterDetailLinkDialog::getInfoHelper()
    {
        return *getArrayHelper();
    }


    ::cppu::IPropertyArrayHelper* MasterDetailLinkDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }

    svt::OGenericUnoDialog::Dialog MasterDetailLinkDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
    {
        return svt::OGenericUnoDialog::Dialog(VclPtr<FormLinkDialog>::Create(VCLUnoHelper::GetWindow(rParent),m_xDetail,m_xMaster, m_aContext
            ,m_sExplanation,m_sDetailLabel,m_sMasterLabel));
    }

    void MasterDetailLinkDialog::implInitialize(const Any& _rValue)
    {
        PropertyValue aProperty;
        if (_rValue >>= aProperty)
        {
            if (aProperty.Name == "Detail")
            {
                OSL_VERIFY( aProperty.Value >>= m_xDetail );
                return;
            }
            else if (aProperty.Name == "Master")
            {
                OSL_VERIFY( aProperty.Value >>= m_xMaster );
                return;
            }
            else if (aProperty.Name == "Explanation")
            {
                OSL_VERIFY( aProperty.Value >>= m_sExplanation );
                return;
            }
            else if (aProperty.Name == "DetailLabel")
            {
                OSL_VERIFY( aProperty.Value >>= m_sDetailLabel );
                return;
            }
            else if (aProperty.Name == "MasterLabel")
            {
                OSL_VERIFY( aProperty.Value >>= m_sMasterLabel );
                return;
            }
        }
        MasterDetailLinkDialog_DBase::implInitialize(_rValue);
    }


}   // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
