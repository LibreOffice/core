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
#include <vcl/svapp.hxx>
#include <vcl/dialog.hxx>
#include <svl/solar.hrc>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/view/XRenderable.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::document;


// - PDFDialog functions -


#define SERVICE_NAME "com.sun.star.Impress.FlashExportDialog"



OUString SWFDialog_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( SERVICE_NAME );
}

Sequence< OUString > SAL_CALL SWFDialog_getSupportedServiceNames()
    throw (RuntimeException)
{
    Sequence<OUString> aRet { SERVICE_NAME };
    return aRet;
}



Reference< XInterface > SAL_CALL SWFDialog_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return static_cast<cppu::OWeakObject*>(new SWFDialog( comphelper::getComponentContext(rSMgr) ));
}



#undef SERVICE_NAME


// - SWFDialog -


SWFDialog::SWFDialog( const Reference< XComponentContext> &rxContext ) :
    OGenericUnoDialog( rxContext )
{
}



SWFDialog::~SWFDialog()
{
}



Any SAL_CALL SWFDialog::queryInterface( const Type& rType )
    throw (RuntimeException, std::exception)
{
    Any aReturn = OGenericUnoDialog::queryInterface( rType );

    if( !aReturn.hasValue() )
        aReturn = ::cppu::queryInterface( rType,
                                          static_cast< XPropertyAccess* >( this ),
                                          static_cast< XExporter* >( this ) );

    return aReturn;
}



void SAL_CALL SWFDialog::acquire()
    throw ()
{
    OWeakObject::acquire();
}



void SAL_CALL SWFDialog::release()
    throw ()
{
    OWeakObject::release();
}



Sequence< sal_Int8 > SAL_CALL SWFDialog::getImplementationId()
    throw(RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}



OUString SAL_CALL SWFDialog::getImplementationName()
    throw (RuntimeException, std::exception)
{
    return SWFDialog_getImplementationName();
}



Sequence< OUString > SAL_CALL SWFDialog::getSupportedServiceNames()
    throw (RuntimeException, std::exception)
{
    return SWFDialog_getSupportedServiceNames();
}



VclPtr<Dialog> SWFDialog::createDialog( vcl::Window* pParent )
{
    VclPtr<Dialog> pRet;

    if (mxSrcDoc.is())
    {
/*      TODO: From the controller we may get information what page is visible and what shapes
              are selected, if we optionaly want to limit output to that
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
        pRet.reset( VclPtr<ImpSWFDialog>::Create( pParent, maFilterData ) );
    }

    return pRet;
}



void SWFDialog::executedDialog( sal_Int16 nExecutionResult )
{
    if( nExecutionResult && m_pDialog )
        maFilterData = static_cast< ImpSWFDialog* >( m_pDialog.get() )->GetFilterData();

    destroyDialog();
}



Reference< XPropertySetInfo > SAL_CALL SWFDialog::getPropertySetInfo()
    throw(RuntimeException, std::exception)
{
    Reference< XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


::cppu::IPropertyArrayHelper& SWFDialog::getInfoHelper()
{
    return *getArrayHelper();
}


::cppu::IPropertyArrayHelper* SWFDialog::createArrayHelper() const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper( aProps );
}



Sequence< PropertyValue > SAL_CALL SWFDialog::getPropertyValues()
    throw ( RuntimeException, std::exception )
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



void SAL_CALL SWFDialog::setPropertyValues( const Sequence< PropertyValue >& rProps )
    throw ( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception )
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



void SAL_CALL SWFDialog::setSourceDocument( const Reference< XComponent >& xDoc )
    throw(IllegalArgumentException, RuntimeException, std::exception)
{
    mxSrcDoc = xDoc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
