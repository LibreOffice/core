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


#include <vcl/FilterConfigItem.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>
#include <osl/file.hxx>
#include <osl/module.hxx>
#include <rtl/ref.hxx>
#include <svl/solar.hrc>
#include <vcl/fltcall.hxx>
#include <vcl/settings.hxx>
#include "exportdialog.hxx"
#include <uno/mapping.hxx>
#include <tools/fldunit.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <unotools/syslocale.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <memory>

using namespace ::com::sun::star;

namespace {

class SvFilterOptionsDialog : public cppu::WeakImplHelper
<
    document::XExporter,
    ui::dialogs::XExecutableDialog,
    beans::XPropertyAccess,
    lang::XInitialization,
    lang::XServiceInfo
>
{
    const uno::Reference< uno::XComponentContext >
        mxContext;
    uno::Sequence< beans::PropertyValue >
        maMediaDescriptor;
    uno::Sequence< beans::PropertyValue >
        maFilterDataSequence;
    uno::Reference< lang::XComponent >
        mxSourceDocument;

    OUString        maDialogTitle;
    FieldUnit       meFieldUnit;
    bool        mbExportSelection;

public:

    explicit SvFilterOptionsDialog( const uno::Reference< uno::XComponentContext >& _rxORB );
    virtual ~SvFilterOptionsDialog();

    // XInterface
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XInitialization
    virtual void SAL_CALL initialize( const uno::Sequence< uno::Any > & aArguments )
        throw ( uno::Exception, uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw ( uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw ( uno::RuntimeException, std::exception ) override;
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw ( uno::RuntimeException, std::exception ) override;

    // XPropertyAccess
    virtual uno::Sequence< beans::PropertyValue > SAL_CALL getPropertyValues()
        throw ( uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setPropertyValues( const uno::Sequence< beans::PropertyValue > & aProps )
        throw ( beans::UnknownPropertyException, beans::PropertyVetoException,
                lang::IllegalArgumentException, lang::WrappedTargetException,
                uno::RuntimeException, std::exception ) override;

    // XExecuteDialog
    virtual sal_Int16 SAL_CALL execute()
        throw ( uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setTitle( const OUString& aTitle )
        throw ( uno::RuntimeException, std::exception ) override;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
        throw ( lang::IllegalArgumentException, uno::RuntimeException, std::exception ) override;

};

SvFilterOptionsDialog::SvFilterOptionsDialog( const uno::Reference< uno::XComponentContext >& rxContext ) :
    mxContext           ( rxContext ),
    meFieldUnit         ( FUNIT_CM ),
    mbExportSelection   ( false )
{
}


SvFilterOptionsDialog::~SvFilterOptionsDialog()
{
}


void SAL_CALL SvFilterOptionsDialog::acquire() throw()
{
    OWeakObject::acquire();
}


void SAL_CALL SvFilterOptionsDialog::release() throw()
{
    OWeakObject::release();
}

// XInitialization
void SAL_CALL SvFilterOptionsDialog::initialize( const uno::Sequence< uno::Any > & )
    throw ( uno::Exception, uno::RuntimeException, std::exception )
{
}

// XServiceInfo
OUString SAL_CALL SvFilterOptionsDialog::getImplementationName()
    throw( uno::RuntimeException, std::exception )
{
    return OUString( "com.sun.star.svtools.SvFilterOptionsDialog" );
}
sal_Bool SAL_CALL SvFilterOptionsDialog::supportsService( const OUString& rServiceName )
    throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}
uno::Sequence< OUString > SAL_CALL SvFilterOptionsDialog::getSupportedServiceNames()
    throw ( uno::RuntimeException, std::exception )
{
    uno::Sequence<OUString> aRet { "com.sun.star.ui.dialogs.FilterOptionsDialog" };
    return aRet;
}

// XPropertyAccess
uno::Sequence< beans::PropertyValue > SvFilterOptionsDialog::getPropertyValues()
        throw ( uno::RuntimeException, std::exception )
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

void SvFilterOptionsDialog::setPropertyValues( const uno::Sequence< beans::PropertyValue > & aProps )
        throw ( beans::UnknownPropertyException, beans::PropertyVetoException,
                lang::IllegalArgumentException, lang::WrappedTargetException,
                uno::RuntimeException, std::exception )
{
    maMediaDescriptor = aProps;

    sal_Int32 i, nCount;
    for ( i = 0, nCount = maMediaDescriptor.getLength(); i < nCount; i++ )
    {
        if ( maMediaDescriptor[ i ].Name == "FilterData" )
        {
            maMediaDescriptor[ i ].Value >>= maFilterDataSequence;
        }
        else if ( maMediaDescriptor[ i ].Name == "SelectionOnly" )
        {
            maMediaDescriptor[ i ].Value >>= mbExportSelection;
        }
    }
}

// XExecutableDialog
void SvFilterOptionsDialog::setTitle( const OUString& aTitle )
    throw ( uno::RuntimeException, std::exception )
{
    maDialogTitle = aTitle;
}

sal_Int16 SvFilterOptionsDialog::execute()
    throw ( uno::RuntimeException, std::exception )
{
    sal_Int16 nRet = ui::dialogs::ExecutableDialogResults::CANCEL;

    OUString aFilterNameStr( "FilterName" );
    OUString aInternalFilterName;
    sal_Int32 j, nCount = maMediaDescriptor.getLength();
    for ( j = 0; j < nCount; j++ )
    {
        if ( maMediaDescriptor[ j ].Name.equals( aFilterNameStr ) )
        {
            OUString aStr;
            maMediaDescriptor[ j ].Value >>= aStr;
            aInternalFilterName = aStr;
            aInternalFilterName = aInternalFilterName.replaceAll( "draw_", "" );
            aInternalFilterName = aInternalFilterName.replaceAll( "impress_", "" );
            break;
       }
    }
    if ( !aInternalFilterName.isEmpty() )
    {
        GraphicFilter aGraphicFilter( true );

        sal_uInt16 nFormat, nFilterCount = aGraphicFilter.GetExportFormatCount();
        for ( nFormat = 0; nFormat < nFilterCount; nFormat++ )
        {
            if ( aGraphicFilter.GetExportInternalFilterName( nFormat ) == aInternalFilterName )
                break;
        }
        if ( nFormat < nFilterCount )
        {
            FltCallDialogParameter aFltCallDlgPara( Application::GetDefDialogParent(), nullptr, meFieldUnit );
            aFltCallDlgPara.aFilterData = maFilterDataSequence;

            std::unique_ptr<ResMgr> pResMgr(ResMgr::CreateResMgr( "svt", Application::GetSettings().GetUILanguageTag() ));
            aFltCallDlgPara.pResMgr = pResMgr.get();

            aFltCallDlgPara.aFilterExt = aGraphicFilter.GetExportFormatShortName( nFormat );
            bool bIsPixelFormat( aGraphicFilter.IsExportPixelFormat( nFormat ) );
            if ( ScopedVclPtrInstance<ExportDialog>( aFltCallDlgPara, mxContext, mxSourceDocument, mbExportSelection, bIsPixelFormat )->Execute() == RET_OK )
                nRet = ui::dialogs::ExecutableDialogResults::OK;

            pResMgr.reset();

            // taking the out parameter from the dialog
            maFilterDataSequence = aFltCallDlgPara.aFilterData;
        }
    }
    return nRet;
}

// XEmporter
void SvFilterOptionsDialog::setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
        throw ( lang::IllegalArgumentException, uno::RuntimeException, std::exception )
{
    mxSourceDocument = xDoc;

    // try to set the corresponding metric unit
    OUString aConfigPath;
    uno::Reference< lang::XServiceInfo > xServiceInfo
            ( xDoc, uno::UNO_QUERY );
    if ( xServiceInfo.is() )
    {
        if ( xServiceInfo->supportsService("com.sun.star.presentation.PresentationDocument") )
            aConfigPath = "Office.Impress/Layout/Other/MeasureUnit";
        else if ( xServiceInfo->supportsService("com.sun.star.drawing.DrawingDocument") )
            aConfigPath = "Office.Draw/Layout/Other/MeasureUnit";
        if ( !aConfigPath.isEmpty() )
        {
            FilterConfigItem aConfigItem( aConfigPath );
            OUString aPropertyName;
            SvtSysLocale aSysLocale;
            if ( aSysLocale.GetLocaleDataPtr()->getMeasurementSystemEnum() == MEASURE_METRIC )
                aPropertyName = "Metric";
            else
                aPropertyName = "NonMetric";
            meFieldUnit = (FieldUnit)aConfigItem.ReadInt32( aPropertyName, FUNIT_CM );
        }
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_svtools_SvFilterOptionsDialog_get_implementation(
    css::uno::XComponentContext * context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SvFilterOptionsDialog(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
