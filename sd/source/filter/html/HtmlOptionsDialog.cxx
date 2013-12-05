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

#include <osl/file.hxx>
#include <osl/module.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::document;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::frame;
using namespace com::sun::star::ui::dialogs;

#include "pres.hxx"
#include "sdabstdlg.hxx"
class SdHtmlOptionsDialog : public cppu::WeakImplHelper5
<
    XExporter,
    XExecutableDialog,
    XPropertyAccess,
    XInitialization,
    XServiceInfo
>
{
    Sequence< PropertyValue > maMediaDescriptor;
    Sequence< PropertyValue > maFilterDataSequence;
    OUString aDialogTitle;
    DocumentType meDocType;

public:

    SdHtmlOptionsDialog();
    ~SdHtmlOptionsDialog();

    // XInterface
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any > & aArguments ) throw ( Exception, RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw ( RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw ( RuntimeException );
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw ( RuntimeException );

    // XPropertyAccess
    virtual Sequence< PropertyValue > SAL_CALL getPropertyValues() throw ( RuntimeException );
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & aProps )
        throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    // XExecuteDialog
    virtual sal_Int16 SAL_CALL execute()
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setTitle( const OUString& aTitle )
        throw ( ::com::sun::star::uno::RuntimeException );

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw ( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

};

// - SdHtmlOptionsDialog -
Reference< XInterface >
    SAL_CALL SdHtmlOptionsDialog_CreateInstance(
        SAL_UNUSED_PARAMETER const Reference< XMultiServiceFactory > & )
{
    return static_cast< ::cppu::OWeakObject* > ( new SdHtmlOptionsDialog );
}

OUString SdHtmlOptionsDialog_getImplementationName()
    throw( RuntimeException )
{
    return OUString( "com.sun.star.comp.draw.SdHtmlOptionsDialog" );
}

Sequence< OUString > SAL_CALL SdHtmlOptionsDialog_getSupportedServiceNames()
    throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = "com.sun.star.ui.dialog.FilterOptionsDialog";
    return aRet;
}

// -----------------------------------------------------------------------------

SdHtmlOptionsDialog::SdHtmlOptionsDialog() :
    meDocType   ( DOCUMENT_TYPE_DRAW )
{
}

// -----------------------------------------------------------------------------

SdHtmlOptionsDialog::~SdHtmlOptionsDialog()
{
}

// -----------------------------------------------------------------------------

void SAL_CALL SdHtmlOptionsDialog::acquire() throw()
{
    OWeakObject::acquire();
}

// -----------------------------------------------------------------------------

void SAL_CALL SdHtmlOptionsDialog::release() throw()
{
    OWeakObject::release();
}

// XInitialization
void SAL_CALL SdHtmlOptionsDialog::initialize( const Sequence< Any > & )
    throw ( Exception, RuntimeException )
{
}

// XServiceInfo
OUString SAL_CALL SdHtmlOptionsDialog::getImplementationName()
    throw( RuntimeException )
{
    return SdHtmlOptionsDialog_getImplementationName();
}
sal_Bool SAL_CALL SdHtmlOptionsDialog::supportsService( const OUString& rServiceName )
    throw( RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL SdHtmlOptionsDialog::getSupportedServiceNames()
    throw ( RuntimeException )
{
    return SdHtmlOptionsDialog_getSupportedServiceNames();
}


// XPropertyAccess
Sequence< PropertyValue > SdHtmlOptionsDialog::getPropertyValues()
        throw ( RuntimeException )
{
    sal_Int32 i, nCount;
    for ( i = 0, nCount = maMediaDescriptor.getLength(); i < nCount; i++ )
    {
        if ( maMediaDescriptor[ i ].Name == "FilterData" )
            break;
    }
    if ( i == nCount )
        maMediaDescriptor.realloc( ++nCount );

    // the "FilterData" Property is an Any that will contain our PropertySequence of Values
    maMediaDescriptor[ i ].Name = "FilterData";
    maMediaDescriptor[ i ].Value <<= maFilterDataSequence;
    return maMediaDescriptor;
}

void SdHtmlOptionsDialog::setPropertyValues( const Sequence< PropertyValue > & aProps )
        throw ( UnknownPropertyException, PropertyVetoException,
                IllegalArgumentException, WrappedTargetException,
                RuntimeException )
{
    maMediaDescriptor = aProps;

    sal_Int32 i, nCount;
    for ( i = 0, nCount = maMediaDescriptor.getLength(); i < nCount; i++ )
    {
        if ( maMediaDescriptor[ i ].Name == "FilterData" )
        {
            maMediaDescriptor[ i ].Value >>= maFilterDataSequence;
            break;
        }
    }
}

// XExecutableDialog
void SdHtmlOptionsDialog::setTitle( const OUString& aTitle )
    throw ( RuntimeException )
{
    aDialogTitle = aTitle;
}

sal_Int16 SdHtmlOptionsDialog::execute()
    throw ( RuntimeException )
{
    sal_Int16 nRet = ExecutableDialogResults::CANCEL;

    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    if( pFact )
    {
        AbstractSdPublishingDlg* pDlg = pFact->CreateSdPublishingDlg( Application::GetDefDialogParent(), meDocType );
        if( pDlg )
        {
            if( pDlg->Execute() )
            {
                pDlg->GetParameterSequence( maFilterDataSequence );
                nRet = ExecutableDialogResults::OK;
            }
            else
            {
                nRet = ExecutableDialogResults::CANCEL;
            }
            delete pDlg;
        }
    }
    return nRet;
}

// XEmporter
void SdHtmlOptionsDialog::setSourceDocument( const Reference< XComponent >& xDoc )
        throw ( IllegalArgumentException, RuntimeException )
{
    // try to set the corresponding metric unit
    Reference< XServiceInfo > xServiceInfo(xDoc, UNO_QUERY);
    if ( xServiceInfo.is() )
    {
        if ( xServiceInfo->supportsService( "com.sun.star.presentation.PresentationDocument" ) )
        {
            meDocType = DOCUMENT_TYPE_IMPRESS;
            return;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.drawing.DrawingDocument" ) )
        {
            meDocType = DOCUMENT_TYPE_DRAW;
            return;
        }
    }
    throw IllegalArgumentException();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
