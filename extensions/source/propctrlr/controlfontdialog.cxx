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


#include <com/sun/star/beans/PropertyAttribute.hpp>
#include "controlfontdialog.hxx"

#include <comphelper/propertyvalue.hxx>
#include <vcl/svapp.hxx>
#include "fontdialog.hxx"
#include "formstrings.hxx"
#include "pcrcommon.hxx"

namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    OControlFontDialog::OControlFontDialog(const Reference< XComponentContext >& _rxContext )
    : OGenericUnoDialog( _rxContext )
    , maFontList(Application::GetDefaultDevice())
    {
        registerProperty(PROPERTY_INTROSPECTEDOBJECT, static_cast<sal_Int32>(OwnPropertyId::INTROSPECTEDOBJECT),
            PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT,
            &m_xControlModel, cppu::UnoType<decltype(m_xControlModel)>::get());
    }


    OControlFontDialog::~OControlFontDialog()
    {
        if (m_xDialog)
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            if (m_xDialog)
            {
                destroyDialog();
                ControlCharacterDialog::destroyItemSet(m_pFontItems, m_pItemPool);
            }
        }
    }


    Sequence<sal_Int8> SAL_CALL OControlFontDialog::getImplementationId(  )
    {
        return css::uno::Sequence<sal_Int8>();
    }


    OUString SAL_CALL OControlFontDialog::getImplementationName()
    {
        return u"org.openoffice.comp.form.ui.OControlFontDialog"_ustr;
    }


    css::uno::Sequence<OUString> SAL_CALL OControlFontDialog::getSupportedServiceNames()
    {
        return { u"com.sun.star.form.ControlFontDialog"_ustr };
    }

    void OControlFontDialog::initialize( const Sequence< Any >& aArguments )
    {
        Reference<XPropertySet> xGridModel;
        if (aArguments.getLength() == 1 && (aArguments[0] >>= xGridModel))
        {
            Sequence aNewArguments{ Any(comphelper::makePropertyValue(u"IntrospectedObject"_ustr,
                                                                      xGridModel)) };
            OControlFontDialog_DBase::initialize(aNewArguments);
        }
        else
            OControlFontDialog_DBase::initialize(aArguments);
    }


    Reference<XPropertySetInfo>  SAL_CALL OControlFontDialog::getPropertySetInfo()
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    ::cppu::IPropertyArrayHelper& OControlFontDialog::getInfoHelper()
    {
        return *getArrayHelper();
    }


    ::cppu::IPropertyArrayHelper* OControlFontDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }

    std::unique_ptr<weld::DialogController> OControlFontDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
    {
        ControlCharacterDialog::createItemSet(m_pFontItems, m_pItemPool, maFontList);

        OSL_ENSURE(m_xControlModel.is(), "OControlFontDialog::createDialog: no introspectee set!");
        if (m_xControlModel.is())
            ControlCharacterDialog::translatePropertiesToItems(m_xControlModel, m_pFontItems.get());
        // TODO: we need a mechanism to prevent that somebody creates us, sets an introspectee, executes us,
        // sets a new introspectee and re-executes us. In this case, the dialog returned here (upon the first
        // execute) will be re-used upon the second execute, and thus it won't be initialized correctly.

        return std::make_unique<ControlCharacterDialog>(Application::GetFrameWeld(rParent), *m_pFontItems);
    }

    void OControlFontDialog::executedDialog(sal_Int16 _nExecutionResult)
    {
        OSL_ENSURE(m_xDialog, "OControlFontDialog::executedDialog: no dialog anymore?!!");
        if (m_xDialog && (RET_OK == _nExecutionResult) && m_xControlModel.is())
        {
            const SfxItemSet* pOutput = static_cast<ControlCharacterDialog*>(m_xDialog.get())->GetOutputItemSet();
            if (pOutput)
                ControlCharacterDialog::translateItemsToProperties( *pOutput, m_xControlModel );
        }
    }


}   // namespace pcr

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
extensions_propctrlr_OControlFontDialog_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new pcr::OControlFontDialog(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
