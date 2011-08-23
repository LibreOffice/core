 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

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

Dialog*	PDFDialog::createDialog( Window* pParent )
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
        if( maMediaDescriptor[ i ].Name.equalsAscii( "FilterData" ) ) 
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
        if( maMediaDescriptor[ i ].Name.equalsAscii( "FilterData" ) ) 
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
