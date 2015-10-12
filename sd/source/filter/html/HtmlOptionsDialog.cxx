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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>

#include <facreg.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::document;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::frame;
using namespace com::sun::star::ui::dialogs;

#include "pres.hxx"
#include "sdabstdlg.hxx"
#include <memory>

class SdHtmlOptionsDialog : public cppu::WeakImplHelper
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
    virtual ~SdHtmlOptionsDialog();

    // XInterface
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any > & aArguments ) throw ( Exception, RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw ( RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw ( RuntimeException, std::exception ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw ( RuntimeException, std::exception ) override;

    // XPropertyAccess
    virtual Sequence< PropertyValue > SAL_CALL getPropertyValues() throw ( RuntimeException, std::exception ) override;
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & aProps )
        throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XExecuteDialog
    virtual sal_Int16 SAL_CALL execute()
        throw ( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setTitle( const OUString& aTitle )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw ( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

};

// - SdHtmlOptionsDialog -
SdHtmlOptionsDialog::SdHtmlOptionsDialog() :
    meDocType   ( DOCUMENT_TYPE_DRAW )
{
}

SdHtmlOptionsDialog::~SdHtmlOptionsDialog()
{
}

void SAL_CALL SdHtmlOptionsDialog::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL SdHtmlOptionsDialog::release() throw()
{
    OWeakObject::release();
}

// XInitialization
void SAL_CALL SdHtmlOptionsDialog::initialize( const Sequence< Any > & )
    throw ( Exception, RuntimeException, std::exception )
{
}

// XServiceInfo
OUString SAL_CALL SdHtmlOptionsDialog::getImplementationName()
    throw( RuntimeException, std::exception )
{
    return OUString( "com.sun.star.comp.draw.SdHtmlOptionsDialog" );
}

sal_Bool SAL_CALL SdHtmlOptionsDialog::supportsService( const OUString& rServiceName )
    throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL SdHtmlOptionsDialog::getSupportedServiceNames()
    throw ( RuntimeException, std::exception )
{
    Sequence< OUString > aRet(1);
    aRet[0] = "com.sun.star.ui.dialog.FilterOptionsDialog";
    return aRet;
}

// XPropertyAccess
Sequence< PropertyValue > SdHtmlOptionsDialog::getPropertyValues()
        throw ( RuntimeException, std::exception )
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
                RuntimeException, std::exception )
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
    throw ( RuntimeException, std::exception )
{
    aDialogTitle = aTitle;
}

sal_Int16 SdHtmlOptionsDialog::execute()
    throw ( RuntimeException, std::exception )
{
    sal_Int16 nRet = ExecutableDialogResults::CANCEL;

    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    if( pFact )
    {
        std::unique_ptr<AbstractSdPublishingDlg> pDlg(pFact->CreateSdPublishingDlg( Application::GetDefDialogParent(), meDocType ));
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
        }
    }
    return nRet;
}

// XEmporter
void SdHtmlOptionsDialog::setSourceDocument( const Reference< XComponent >& xDoc )
        throw ( IllegalArgumentException, RuntimeException, std::exception )
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


extern "C" SAL_DLLPUBLIC_EXPORT ::com::sun::star::uno::XInterface* SAL_CALL
com_sun_star_comp_draw_SdHtmlOptionsDialog_get_implementation(::com::sun::star::uno::XComponentContext*,
                                                              ::com::sun::star::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SdHtmlOptionsDialog());
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
