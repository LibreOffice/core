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

extern "C" void SAL_CALL createRegistryInfo_OControlFontDialog()
{
    ::pcr::OAutoRegistration< ::pcr::OControlFontDialog > aAutoRegistration;
}

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    //====================================================================
    //= OControlFontDialog
    //====================================================================

    OControlFontDialog::OControlFontDialog(const Reference< XComponentContext >& _rxContext )
        :OGenericUnoDialog( _rxContext )
        ,m_pFontItems(NULL)
        ,m_pItemPool(NULL)
        ,m_pItemPoolDefaults(NULL)
    {
        registerProperty(PROPERTY_INTROSPECTEDOBJECT, OWN_PROPERTY_ID_INTROSPECTEDOBJECT,
            PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT,
            &m_xControlModel, ::getCppuType(&m_xControlModel));
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


    Sequence<sal_Int8> SAL_CALL OControlFontDialog::getImplementationId(  ) throw(RuntimeException)
    {
        static ::cppu::OImplementationId aId;
        return aId.getImplementationId();
    }


    Reference< XInterface > SAL_CALL OControlFontDialog::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new OControlFontDialog( _rxContext ) );
    }


    OUString SAL_CALL OControlFontDialog::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_static();
    }


    OUString OControlFontDialog::getImplementationName_static() throw(RuntimeException)
    {
        return OUString("org.openoffice.comp.form.ui.OControlFontDialog");
    }


    ::comphelper::StringSequence SAL_CALL OControlFontDialog::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_static();
    }


    ::comphelper::StringSequence OControlFontDialog::getSupportedServiceNames_static() throw(RuntimeException)
    {
        ::comphelper::StringSequence aSupported(1);
        aSupported[0] = "com.sun.star.form.ControlFontDialog";
        return aSupported;
    }

    void OControlFontDialog::initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException)
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


    Reference<XPropertySetInfo>  SAL_CALL OControlFontDialog::getPropertySetInfo() throw(RuntimeException)
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    ::cppu::IPropertyArrayHelper& OControlFontDialog::getInfoHelper()
    {
        return *const_cast<OControlFontDialog*>(this)->getArrayHelper();
    }


    ::cppu::IPropertyArrayHelper* OControlFontDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }


    Dialog* OControlFontDialog::createDialog(Window* _pParent)
    {
        ControlCharacterDialog::createItemSet(m_pFontItems, m_pItemPool, m_pItemPoolDefaults);

        OSL_ENSURE(m_xControlModel.is(), "OControlFontDialog::createDialog: no introspectee set!");
        if (m_xControlModel.is())
            ControlCharacterDialog::translatePropertiesToItems(m_xControlModel, m_pFontItems);
        // TODO: we need a mechanism to prevent that somebody creates us, sets an introspectee, executes us,
        // sets a new introspectee and re-executes us. In this case, the dialog returned here (upon the first
        // execute) will be re-used upon the second execute, and thus it won't be initialized correctly.

        ControlCharacterDialog* pDialog = new ControlCharacterDialog(_pParent, *m_pFontItems);
        return pDialog;
    }


    void OControlFontDialog::destroyDialog()
    {
        OGenericUnoDialog::destroyDialog();
        ControlCharacterDialog::destroyItemSet(m_pFontItems, m_pItemPool, m_pItemPoolDefaults);
    }


    void OControlFontDialog::executedDialog(sal_Int16 _nExecutionResult)
    {
        OSL_ENSURE(m_pDialog, "OControlFontDialog::executedDialog: no dialog anymore?!!");
        if (m_pDialog && (sal_True == _nExecutionResult) && m_xControlModel.is())
        {
            const SfxItemSet* pOutput = static_cast<ControlCharacterDialog*>(m_pDialog)->GetOutputItemSet();
            if (pOutput)
                ControlCharacterDialog::translateItemsToProperties( *pOutput, m_xControlModel );
        }
    }

//........................................................................
}   // namespace pcr
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
