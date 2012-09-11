/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    ::rtl::OUString aDialogTitle;
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
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw ( RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw ( RuntimeException );
    virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw ( RuntimeException );

    // XPropertyAccess
    virtual Sequence< PropertyValue > SAL_CALL getPropertyValues() throw ( RuntimeException );
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & aProps )
        throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    // XExecuteDialog
    virtual sal_Int16 SAL_CALL execute()
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setTitle( const ::rtl::OUString& aTitle )
        throw ( ::com::sun::star::uno::RuntimeException );

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw ( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

};

// -------------------------
// - SdHtmlOptionsDialog -
// -------------------------

Reference< XInterface >
    SAL_CALL SdHtmlOptionsDialog_CreateInstance(
        SAL_UNUSED_PARAMETER const Reference< XMultiServiceFactory > & )
{
    return static_cast< ::cppu::OWeakObject* > ( new SdHtmlOptionsDialog );
}

::rtl::OUString SdHtmlOptionsDialog_getImplementationName()
    throw( RuntimeException )
{
    return ::rtl::OUString( "com.sun.star.comp.draw.SdHtmlOptionsDialog" );
}

sal_Bool SAL_CALL SdHtmlOptionsDialog_supportsService( const ::rtl::OUString& ServiceName )
    throw( RuntimeException )
{
    return ServiceName.equals("com.sun.star.ui.dialog.FilterOptionsDialog");
}

Sequence< ::rtl::OUString > SAL_CALL SdHtmlOptionsDialog_getSupportedServiceNames()
    throw( RuntimeException )
{
    Sequence< ::rtl::OUString > aRet(1);
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString("com.sun.star.ui.dialog.FilterOptionsDialog");
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
::rtl::OUString SAL_CALL SdHtmlOptionsDialog::getImplementationName()
    throw( RuntimeException )
{
    return SdHtmlOptionsDialog_getImplementationName();
}
sal_Bool SAL_CALL SdHtmlOptionsDialog::supportsService( const ::rtl::OUString& rServiceName )
    throw( RuntimeException )
{
    return SdHtmlOptionsDialog_supportsService( rServiceName );
}
Sequence< ::rtl::OUString > SAL_CALL SdHtmlOptionsDialog::getSupportedServiceNames()
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
void SdHtmlOptionsDialog::setTitle( const ::rtl::OUString& aTitle )
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
    String aConfigPath;
    Reference< XServiceInfo > xServiceInfo
            ( xDoc, UNO_QUERY );
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
