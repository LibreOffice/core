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


#include "controlfontdialog.hxx"
#include <cppuhelper/typeprovider.hxx>
#include "fontdialog.hxx"
#include "formstrings.hxx"
#include "pcrcommon.hxx"
#include "pcrservices.hxx"

extern "C" void SAL_CALL createRegistryInfo_OControlFontDialog()
{
    ::pcr::OAutoRegistration< ::pcr::OControlFontDialog > aAutoRegistration;
}


namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    OControlFontDialog::OControlFontDialog(const Reference< XComponentContext >& _rxContext )
        :OGenericUnoDialog( _rxContext )
        ,m_pFontItems(nullptr)
        ,m_pItemPool(nullptr)
        ,m_pItemPoolDefaults(nullptr)
    {
        registerProperty(PROPERTY_INTROSPECTEDOBJECT, OWN_PROPERTY_ID_INTROSPECTEDOBJECT,
            PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT,
            &m_xControlModel, cppu::UnoType<decltype(m_xControlModel)>::get());
    }


    OControlFontDialog::~OControlFontDialog()
    {
        if (m_pDialog)
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            if (m_pDialog)
                destroyDialog();
        }
    }


    Sequence<sal_Int8> SAL_CALL OControlFontDialog::getImplementationId(  ) throw(RuntimeException, std::exception)
    {
        return css::uno::Sequence<sal_Int8>();
    }


    Reference< XInterface > SAL_CALL OControlFontDialog::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new OControlFontDialog( _rxContext ) );
    }


    OUString SAL_CALL OControlFontDialog::getImplementationName() throw(RuntimeException, std::exception)
    {
        return getImplementationName_static();
    }


    OUString OControlFontDialog::getImplementationName_static() throw(RuntimeException)
    {
        return OUString("org.openoffice.comp.form.ui.OControlFontDialog");
    }


    css::uno::Sequence<OUString> SAL_CALL OControlFontDialog::getSupportedServiceNames() throw(RuntimeException, std::exception)
    {
        return getSupportedServiceNames_static();
    }


    css::uno::Sequence<OUString> OControlFontDialog::getSupportedServiceNames_static() throw(RuntimeException)
    {
        css::uno::Sequence<OUString> aSupported { "com.sun.star.form.ControlFontDialog" };
        return aSupported;
    }

    void OControlFontDialog::initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException, std::exception)
    {
        Reference<XPropertySet> xGridModel;
        if (aArguments.getLength() == 1 && (aArguments[0] >>= xGridModel))
        {
            PropertyValue aArg;
            aArg.Name = "IntrospectedObject";
            aArg.Value <<= xGridModel;
            Sequence< Any > aNewArguments(1);
            aNewArguments[0] <<= aArg;
            OControlFontDialog_DBase::initialize(aNewArguments);
        }
        else
            OControlFontDialog_DBase::initialize(aArguments);
    }


    Reference<XPropertySetInfo>  SAL_CALL OControlFontDialog::getPropertySetInfo() throw(RuntimeException, std::exception)
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


    VclPtr<Dialog> OControlFontDialog::createDialog(vcl::Window* _pParent)
    {
        ControlCharacterDialog::createItemSet(m_pFontItems, m_pItemPool, m_pItemPoolDefaults);

        OSL_ENSURE(m_xControlModel.is(), "OControlFontDialog::createDialog: no introspectee set!");
        if (m_xControlModel.is())
            ControlCharacterDialog::translatePropertiesToItems(m_xControlModel, m_pFontItems);
        // TODO: we need a mechanism to prevent that somebody creates us, sets an introspectee, executes us,
        // sets a new introspectee and re-executes us. In this case, the dialog returned here (upon the first
        // execute) will be re-used upon the second execute, and thus it won't be initialized correctly.

        return VclPtr<ControlCharacterDialog>::Create(_pParent, *m_pFontItems);
    }


    void OControlFontDialog::destroyDialog()
    {
        OGenericUnoDialog::destroyDialog();
        ControlCharacterDialog::destroyItemSet(m_pFontItems, m_pItemPool, m_pItemPoolDefaults);
    }


    void OControlFontDialog::executedDialog(sal_Int16 _nExecutionResult)
    {
        OSL_ENSURE(m_pDialog, "OControlFontDialog::executedDialog: no dialog anymore?!!");
        if (m_pDialog && (RET_OK == _nExecutionResult) && m_xControlModel.is())
        {
            const SfxItemSet* pOutput = static_cast<ControlCharacterDialog*>(m_pDialog.get())->GetOutputItemSet();
            if (pOutput)
                ControlCharacterDialog::translateItemsToProperties( *pOutput, m_xControlModel );
        }
    }


}   // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
