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


#include "pdfdialog.hxx"
#include "impdialog.hxx"
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

PDFDialog::PDFDialog( const Reference< XComponentContext > &rxContext )
: PDFDialog_Base( rxContext )
{
}


PDFDialog::~PDFDialog()
{
}


Sequence< sal_Int8 > SAL_CALL PDFDialog::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


OUString SAL_CALL PDFDialog::getImplementationName()
{
    return u"com.sun.star.comp.PDF.PDFDialog"_ustr;
}


Sequence< OUString > SAL_CALL PDFDialog::getSupportedServiceNames()
{
    return { u"com.sun.star.document.PDFDialog"_ustr };
}

std::unique_ptr<weld::DialogController> PDFDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
{
    if( mxSrcDoc.is() )
        return std::make_unique<ImpPDFTabDialog>(Application::GetFrameWeld(rParent), maFilterData, mxSrcDoc);
    return nullptr;
}

std::shared_ptr<SfxTabDialogController> PDFDialog::createAsyncDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
{
    if( mxSrcDoc.is() )
        return std::make_shared<ImpPDFTabDialog>(Application::GetFrameWeld(rParent), maFilterData, mxSrcDoc);
    return nullptr;
}

void PDFDialog::executedDialog( sal_Int16 nExecutionResult )
{
    if (nExecutionResult && m_xDialog)
        maFilterData = static_cast<ImpPDFTabDialog*>(m_xDialog.get())->GetFilterData();
    destroyDialog();
}

void PDFDialog::runAsync(const css::uno::Reference< css::ui::dialogs::XDialogClosedListener >& xListener)
{
    SfxTabDialogController::runAsync(m_xAsyncDialog, [this, xListener](sal_Int32 nResponse) {
        rtl::Reference<PDFDialog> xThis(this); // keep alive for scope, dialogClosed can cause owner to drop this
        executedAsyncDialog( m_xAsyncDialog, nResponse );
        css::ui::dialogs::DialogClosedEvent aEvent;
        aEvent.DialogResult = nResponse;
        xListener->dialogClosed( aEvent );
        destroyAsyncDialog();
    });
}

void PDFDialog::executedAsyncDialog( std::shared_ptr<SfxTabDialogController> xAsyncDialog, sal_Int32 nExecutionResult )
{
    if (nExecutionResult && xAsyncDialog)
        maFilterData = static_cast<ImpPDFTabDialog*>(xAsyncDialog.get())->GetFilterData();
}

Reference< XPropertySetInfo > SAL_CALL PDFDialog::getPropertySetInfo()
{
    Reference< XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

::cppu::IPropertyArrayHelper& PDFDialog::getInfoHelper()
{
    return *getArrayHelper();
}

::cppu::IPropertyArrayHelper* PDFDialog::createArrayHelper() const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper( aProps );
}


Sequence< PropertyValue > SAL_CALL PDFDialog::getPropertyValues()
{
    sal_Int32 i, nCount;

    for( i = 0, nCount = maMediaDescriptor.getLength(); i < nCount; i++ )
    {
        if ( maMediaDescriptor[ i ].Name == "FilterData" )
            break;
    }

    if( i == nCount )
        maMediaDescriptor.realloc( ++nCount );
    auto pMediaDescriptor = maMediaDescriptor.getArray();

    pMediaDescriptor[ i ].Name = "FilterData";
    pMediaDescriptor[ i ].Value <<= maFilterData;

    return maMediaDescriptor;
}


void SAL_CALL PDFDialog::setPropertyValues( const Sequence< PropertyValue >& rProps )
{
    maMediaDescriptor = rProps;

    for (const PropertyValue& rProp : maMediaDescriptor)
    {
        if ( rProp.Name == "FilterData" )
        {
            rProp.Value >>= maFilterData;
            break;
        }
    }
}


void SAL_CALL PDFDialog::setSourceDocument( const Reference< XComponent >& xDoc )
{
    mxSrcDoc = xDoc;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
filter_PDFDialog_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new PDFDialog(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
