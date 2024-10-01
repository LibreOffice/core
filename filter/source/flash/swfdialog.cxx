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

#include "swfdialog.hxx"
#include "swfuno.hxx"
#include "impswfdialog.hxx"
#include <comphelper/processfactory.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/view/XRenderable.hpp>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::document;

#define SERVICE_NAME "com.sun.star.Impress.FlashExportDialog"

OUString SWFDialog_getImplementationName() { return SERVICE_NAME; }

Sequence<OUString> SWFDialog_getSupportedServiceNames()
{
    Sequence<OUString> aRet{ SERVICE_NAME };
    return aRet;
}

Reference<XInterface> SWFDialog_createInstance(const Reference<XMultiServiceFactory>& rSMgr)
{
    return static_cast<cppu::OWeakObject*>(new SWFDialog(comphelper::getComponentContext(rSMgr)));
}

#undef SERVICE_NAME

SWFDialog::SWFDialog(const Reference<XComponentContext>& rxContext)
    : SWFDialog_Base(rxContext)
{
}

SWFDialog::~SWFDialog() {}

OUString SAL_CALL SWFDialog::getImplementationName() { return SWFDialog_getImplementationName(); }

Sequence<OUString> SAL_CALL SWFDialog::getSupportedServiceNames()
{
    return SWFDialog_getSupportedServiceNames();
}

std::unique_ptr<weld::DialogController>
SWFDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
{
    std::unique_ptr<weld::DialogController> xRet;

    if (mxSrcDoc.is())
    {
        /*      TODO: From the controller we may get information what page is visible and what shapes
              are selected, if we optionally want to limit output to that
        Any aSelection;

        try
        {
            Reference< XController > xController( Reference< XModel >( mxSrcDoc, UNO_QUERY )->getCurrentController() );

            if( xController.is() )
            {
                Reference< XSelectionSupplier > xView( xController, UNO_QUERY );

                if( xView.is() )
                    xView->getSelection() >>= aSelection;
            }
        }
        catch( RuntimeException )
        {
        }
*/
        xRet.reset(new ImpSWFDialog(Application::GetFrameWeld(rParent), maFilterData));
    }

    return xRet;
}

void SWFDialog::executedDialog(sal_Int16 nExecutionResult)
{
    if (nExecutionResult && m_xDialog)
        maFilterData = static_cast<ImpSWFDialog*>(m_xDialog.get())->GetFilterData();

    destroyDialog();
}

Reference<XPropertySetInfo> SAL_CALL SWFDialog::getPropertySetInfo()
{
    Reference<XPropertySetInfo> xInfo(createPropertySetInfo(getInfoHelper()));
    return xInfo;
}

::cppu::IPropertyArrayHelper& SWFDialog::getInfoHelper() { return *getArrayHelper(); }

::cppu::IPropertyArrayHelper* SWFDialog::createArrayHelper() const
{
    Sequence<Property> aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

Sequence<PropertyValue> SAL_CALL SWFDialog::getPropertyValues()
{
    sal_Int32 i, nCount;

    for (i = 0, nCount = maMediaDescriptor.getLength(); i < nCount; i++)
    {
        if (maMediaDescriptor[i].Name == "FilterData")
            break;
    }

    if (i == nCount)
        maMediaDescriptor.realloc(++nCount);

    auto pMediaDescriptor = maMediaDescriptor.getArray();

    pMediaDescriptor[i].Name = "FilterData";
    pMediaDescriptor[i].Value <<= maFilterData;

    return maMediaDescriptor;
}

void SAL_CALL SWFDialog::setPropertyValues(const Sequence<PropertyValue>& rProps)
{
    maMediaDescriptor = rProps;

    for (sal_Int32 i = 0, nCount = maMediaDescriptor.getLength(); i < nCount; i++)
    {
        if (maMediaDescriptor[i].Name == "FilterData")
        {
            maMediaDescriptor[i].Value >>= maFilterData;
            break;
        }
    }
}

void SAL_CALL SWFDialog::setSourceDocument(const Reference<XComponent>& xDoc) { mxSrcDoc = xDoc; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
