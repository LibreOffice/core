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
#include <vcl/dialog.hxx>
#include <svl/solar.hrc>
#include <com/sun/star/view/XRenderable.hpp>

using namespace ::rtl;
using namespace ::vcl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

// -----------------------
// - PDFDialog functions -
// -----------------------

#define SERVICE_NAME "com.sun.star.document.PDFDialog"

// -----------------------------------------------------------------------------

OUString PDFDialog_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.PDF.PDFDialog" ) );
}

// -----------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL PDFDialog_getImplementationId()
    throw(RuntimeException)
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL PDFDialog_getSupportedServiceNames()
    throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}

// -----------------------------------------------------------------------------

Reference< XInterface > SAL_CALL PDFDialog_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new PDFDialog( rSMgr );
}

// -----------------------------------------------------------------------------

#undef SERVICE_NAME

// -------------
// - PDFDialog -
// -------------

PDFDialog::PDFDialog( const Reference< XMultiServiceFactory > &rxMSF )
: PDFDialog_Base( rxMSF )
{
}

// -----------------------------------------------------------------------------

PDFDialog::~PDFDialog()
{
}

// -----------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL PDFDialog::getImplementationId()
    throw(RuntimeException)
{
    return PDFDialog_getImplementationId();
}

// -----------------------------------------------------------------------------

OUString SAL_CALL PDFDialog::getImplementationName()
    throw (RuntimeException)
{
    return PDFDialog_getImplementationName();
}

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL PDFDialog::getSupportedServiceNames()
    throw (RuntimeException)
{
    return PDFDialog_getSupportedServiceNames();
}

// -----------------------------------------------------------------------------

Dialog* PDFDialog::createDialog( Window* pParent )
{
    Dialog* pRet = NULL;

    if( mxSrcDoc.is() )
    {
        ImpPDFTabDialog* pDlg = new ImpPDFTabDialog( pParent, maFilterData, mxSrcDoc, m_aContext.getLegacyServiceFactory() );
        pRet = pDlg;
    }

    return pRet;
}

// -----------------------------------------------------------------------------

void PDFDialog::executedDialog( sal_Int16 nExecutionResult )
{
    if( nExecutionResult && m_pDialog )
        maFilterData = static_cast< ImpPDFTabDialog* >( m_pDialog )->GetFilterData();
    destroyDialog();
}

// -----------------------------------------------------------------------------

Reference< XPropertySetInfo > SAL_CALL PDFDialog::getPropertySetInfo()
    throw(RuntimeException)
{
    Reference< XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//-------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& PDFDialog::getInfoHelper()
{
    return *const_cast<PDFDialog*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* PDFDialog::createArrayHelper() const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper( aProps );
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > SAL_CALL PDFDialog::getPropertyValues()
    throw ( RuntimeException )
{
    sal_Int32 i, nCount;

    for( i = 0, nCount = maMediaDescriptor.getLength(); i < nCount; i++ )
    {
        if ( maMediaDescriptor[ i ].Name == "FilterData" )
            break;
    }

    if( i == nCount )
        maMediaDescriptor.realloc( ++nCount );

    maMediaDescriptor[ i ].Name = String( RTL_CONSTASCII_USTRINGPARAM( "FilterData" ) );
    maMediaDescriptor[ i ].Value <<= maFilterData;

    return maMediaDescriptor;
}

// -----------------------------------------------------------------------------

void SAL_CALL PDFDialog::setPropertyValues( const Sequence< PropertyValue >& rProps )
    throw ( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException )
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

// -----------------------------------------------------------------------------

void SAL_CALL PDFDialog::setSourceDocument( const Reference< XComponent >& xDoc )
    throw(IllegalArgumentException, RuntimeException)
{
    mxSrcDoc = xDoc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
