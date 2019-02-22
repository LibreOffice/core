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

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::document;
using namespace com::sun::star::beans;
using namespace com::sun::star::ui::dialogs;

#include <pres.hxx>
#include <sdabstdlg.hxx>

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
    DocumentType meDocType;

public:

    SdHtmlOptionsDialog();

    // XInterface
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any > & aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XPropertyAccess
    virtual Sequence< PropertyValue > SAL_CALL getPropertyValues() override;
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< css::beans::PropertyValue > & aProps ) override;

    // XExecuteDialog
    virtual sal_Int16 SAL_CALL execute() override;
    virtual void SAL_CALL setTitle( const OUString& aTitle ) override;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;

};

SdHtmlOptionsDialog::SdHtmlOptionsDialog() :
    meDocType   ( DocumentType::Draw )
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
{
}

// XServiceInfo
OUString SAL_CALL SdHtmlOptionsDialog::getImplementationName()
{
    return OUString( "com.sun.star.comp.draw.SdHtmlOptionsDialog" );
}

sal_Bool SAL_CALL SdHtmlOptionsDialog::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL SdHtmlOptionsDialog::getSupportedServiceNames()
{
    Sequence< OUString > aRet { "com.sun.star.ui.dialog.FilterOptionsDialog" };
    return aRet;
}

// XPropertyAccess
Sequence< PropertyValue > SdHtmlOptionsDialog::getPropertyValues()
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
void SdHtmlOptionsDialog::setTitle( const OUString& )
{
}

sal_Int16 SdHtmlOptionsDialog::execute()
{
    sal_Int16 nRet = ExecutableDialogResults::CANCEL;

    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSdPublishingDlg> pDlg(pFact->CreateSdPublishingDlg( Application::GetDefDialogParent(), meDocType ));
    if( pDlg->Execute() )
    {
        pDlg->GetParameterSequence( maFilterDataSequence );
        nRet = ExecutableDialogResults::OK;
    }
    else
    {
        nRet = ExecutableDialogResults::CANCEL;
    }
    return nRet;
}

// XEmporter
void SdHtmlOptionsDialog::setSourceDocument( const Reference< XComponent >& xDoc )
{
    // try to set the corresponding metric unit
    Reference< XServiceInfo > xServiceInfo(xDoc, UNO_QUERY);
    if ( xServiceInfo.is() )
    {
        if ( xServiceInfo->supportsService( "com.sun.star.presentation.PresentationDocument" ) )
        {
            meDocType = DocumentType::Impress;
            return;
        }
        else if ( xServiceInfo->supportsService( "com.sun.star.drawing.DrawingDocument" ) )
        {
            meDocType = DocumentType::Draw;
            return;
        }
    }
    throw IllegalArgumentException();
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_draw_SdHtmlOptionsDialog_get_implementation(css::uno::XComponentContext*,
                                                              css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SdHtmlOptionsDialog());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
