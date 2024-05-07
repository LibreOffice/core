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

#include <sal/log.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/svapp.hxx>
#include "MasterDetailLinkDialog.hxx"
#include "formlinkdialog.hxx"

namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    MasterDetailLinkDialog::MasterDetailLinkDialog(const Reference< XComponentContext >& _rxContext )
        :OGenericUnoDialog( _rxContext )
    {
    }

    Sequence<sal_Int8> SAL_CALL MasterDetailLinkDialog::getImplementationId(  )
    {
        return css::uno::Sequence<sal_Int8>();
    }


    OUString SAL_CALL MasterDetailLinkDialog::getImplementationName()
    {
        return u"org.openoffice.comp.form.ui.MasterDetailLinkDialog"_ustr;
    }


    css::uno::Sequence<OUString> SAL_CALL MasterDetailLinkDialog::getSupportedServiceNames()
    {
        return { u"com.sun.star.form.MasterDetailLinkDialog"_ustr };
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

    std::unique_ptr<weld::DialogController> MasterDetailLinkDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
    {
        return std::make_unique<FormLinkDialog>(Application::GetFrameWeld(rParent), m_xDetail,
                                                m_xMaster, m_aContext, m_sExplanation,
                                                m_sDetailLabel, m_sMasterLabel);
    }

    void MasterDetailLinkDialog::implInitialize(const Any& _rValue)
    {
        PropertyValue aProperty;
        if (_rValue >>= aProperty)
        {
            if (aProperty.Name == "Detail")
            {
                if ( ! (aProperty.Value >>= m_xDetail) )
                    SAL_WARN("extensions.propctrlr", "implInitialize: unable to get property Detail");
                return;
            }
            else if (aProperty.Name == "Master")
            {
                if ( ! (aProperty.Value >>= m_xMaster) )
                    SAL_WARN("extensions.propctrlr", "implInitialize: unable to get property Master");
                return;
            }
            else if (aProperty.Name == "Explanation")
            {
                if ( ! (aProperty.Value >>= m_sExplanation) )
                    SAL_WARN("extensions.propctrlr", "implInitialize: unable to get property Explanation");
                return;
            }
            else if (aProperty.Name == "DetailLabel")
            {
                if ( ! (aProperty.Value >>= m_sDetailLabel) )
                    SAL_WARN("extensions.propctrlr", "implInitialize: unable to get property DetailLabel");
                return;
            }
            else if (aProperty.Name == "MasterLabel")
            {
                if ( ! (aProperty.Value >>= m_sMasterLabel) )
                    SAL_WARN("extensions.propctrlr", "implInitialize: unable to get property MasterLabel");
                return;
            }
        }
        MasterDetailLinkDialog_DBase::implInitialize(_rValue);
    }


}   // namespace pcr

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
extensions_propctrlr_MasterDetailLinkDialog_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new pcr::MasterDetailLinkDialog(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
