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
#include <svl/solar.hrc>
#include <com/sun/star/view/XRenderable.hpp>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

#define SERVICE_NAME "com.sun.star.document.PDFDialog"


OUString PDFDialog_getImplementationName ()
{
    return OUString ( "com.sun.star.comp.PDF.PDFDialog" );
}


Sequence< OUString > PDFDialog_getSupportedServiceNames()
{
    Sequence<OUString> aRet { SERVICE_NAME };
    return aRet;
}


Reference< XInterface > PDFDialog_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
{
    return static_cast<cppu::OWeakObject*>(new PDFDialog( comphelper::getComponentContext(rSMgr) ));
}


#undef SERVICE_NAME


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
    return PDFDialog_getImplementationName();
}


Sequence< OUString > SAL_CALL PDFDialog::getSupportedServiceNames()
{
    return PDFDialog_getSupportedServiceNames();
}

svt::OGenericUnoDialog::Dialog PDFDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
{
    if( mxSrcDoc.is() )
        return svt::OGenericUnoDialog::Dialog(std::make_unique<ImpPDFTabDialog>(Application::GetFrameWeld(rParent), maFilterData, mxSrcDoc));
    return svt::OGenericUnoDialog::Dialog();
}

void PDFDialog::executedDialog( sal_Int16 nExecutionResult )
{
    if (nExecutionResult && m_aDialog)
        maFilterData = static_cast<ImpPDFTabDialog*>(m_aDialog.m_xWeldDialog.get())->GetFilterData();
    destroyDialog();
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

    maMediaDescriptor[ i ].Name = "FilterData";
    maMediaDescriptor[ i ].Value <<= maFilterData;

    return maMediaDescriptor;
}


void SAL_CALL PDFDialog::setPropertyValues( const Sequence< PropertyValue >& rProps )
{
    maMediaDescriptor = rProps;

    for( sal_Int32 i = 0, nCount = maMediaDescriptor.getLength(); i < nCount; i++ )
    {
        if ( maMediaDescriptor[ i ].Name == "FilterData" )
        {
            maMediaDescriptor[ i ].Value >>= maFilterData;
            break;
        }
    }
}


void SAL_CALL PDFDialog::setSourceDocument( const Reference< XComponent >& xDoc )
{
    mxSrcDoc = xDoc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
