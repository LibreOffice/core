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
#include <FltCallDialogParameter.hxx>
#include <vcl/settings.hxx>
#include "exportdialog.hxx"
#include <uno/mapping.hxx>
#include <tools/fldunit.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
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

    css::uno::Reference<css::awt::XWindow> mxParent;
    FieldUnit       meFieldUnit;
    bool            mbExportSelection;
    bool            mbGraphicsSource;

public:

    explicit SvFilterOptionsDialog( const uno::Reference< uno::XComponentContext >& _rxORB );

    // XInterface
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XInitialization
    virtual void SAL_CALL initialize( const uno::Sequence< uno::Any > & aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XPropertyAccess
    virtual uno::Sequence< beans::PropertyValue > SAL_CALL getPropertyValues() override;
    virtual void SAL_CALL setPropertyValues( const uno::Sequence< beans::PropertyValue > & aProps ) override;

    // XExecuteDialog
    virtual sal_Int16 SAL_CALL execute() override;
    virtual void SAL_CALL setTitle( const OUString& aTitle ) override;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const uno::Reference< lang::XComponent >& xDoc ) override;

};

SvFilterOptionsDialog::SvFilterOptionsDialog( const uno::Reference< uno::XComponentContext >& rxContext ) :
    mxContext           ( rxContext ),
    meFieldUnit         ( FieldUnit::CM ),
    mbExportSelection   ( false ),
    mbGraphicsSource    ( true )
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
void SAL_CALL SvFilterOptionsDialog::initialize(const uno::Sequence<uno::Any>& rArguments)
{
    for(const uno::Any& rArgument : rArguments)
    {
        beans::PropertyValue aProperty;
        if (rArgument >>= aProperty)
        {
            if( aProperty.Name == "ParentWindow" )
            {
                aProperty.Value >>= mxParent;
            }
        }
    }
}

// XServiceInfo
OUString SAL_CALL SvFilterOptionsDialog::getImplementationName()
{
    return OUString( "com.sun.star.svtools.SvFilterOptionsDialog" );
}
sal_Bool SAL_CALL SvFilterOptionsDialog::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}
uno::Sequence< OUString > SAL_CALL SvFilterOptionsDialog::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet { "com.sun.star.ui.dialogs.FilterOptionsDialog" };
    return aRet;
}

// XPropertyAccess
uno::Sequence< beans::PropertyValue > SvFilterOptionsDialog::getPropertyValues()
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
void SvFilterOptionsDialog::setTitle( const OUString& )
{
}

sal_Int16 SvFilterOptionsDialog::execute()
{
    sal_Int16 nRet = ui::dialogs::ExecutableDialogResults::CANCEL;

    OUString aInternalFilterName;
    uno::Reference<graphic::XGraphic> xGraphic;
    sal_Int32 j, nCount = maMediaDescriptor.getLength();
    for ( j = 0; j < nCount; j++ )
    {
        const OUString& rName = maMediaDescriptor[ j ].Name;
        if ( rName == "FilterName" )
        {
            OUString aStr;
            maMediaDescriptor[ j ].Value >>= aStr;
            aInternalFilterName = aStr;
            aInternalFilterName = aInternalFilterName.replaceFirst( "draw_", "" );
            aInternalFilterName = aInternalFilterName.replaceFirst( "impress_", "" );
            aInternalFilterName = aInternalFilterName.replaceFirst( "calc_", "" );
            aInternalFilterName = aInternalFilterName.replaceFirst( "writer_", "" );
            break;
       }
        else if ( rName == "Graphic" )
        {
            maMediaDescriptor[ j ].Value >>= xGraphic;
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
            FltCallDialogParameter aFltCallDlgPara(Application::GetFrameWeld(mxParent), meFieldUnit);
            aFltCallDlgPara.aFilterData = maFilterDataSequence;
            aFltCallDlgPara.aFilterExt = aGraphicFilter.GetExportFormatShortName( nFormat );
            bool bIsPixelFormat( aGraphicFilter.IsExportPixelFormat( nFormat ) );

            ExportDialog aDialog(aFltCallDlgPara, mxContext, mxSourceDocument, mbExportSelection,
                        bIsPixelFormat, mbGraphicsSource, xGraphic);
            if (aDialog.run() == RET_OK)
                nRet = ui::dialogs::ExecutableDialogResults::OK;

            // taking the out parameter from the dialog
            maFilterDataSequence = aFltCallDlgPara.aFilterData;
        }
    }
    return nRet;
}

// XEmporter
void SvFilterOptionsDialog::setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
{
    mxSourceDocument = xDoc;

    mbGraphicsSource = true;    // default Draw and Impress like it was before

    // try to set the corresponding metric unit
    OUString aConfigPath;
    uno::Reference< lang::XServiceInfo > xServiceInfo
            ( xDoc, uno::UNO_QUERY );
    if ( !xServiceInfo.is() )
        return;

    if ( xServiceInfo->supportsService("com.sun.star.presentation.PresentationDocument") )
        aConfigPath = "Office.Impress/Layout/Other/MeasureUnit";
    else if ( xServiceInfo->supportsService("com.sun.star.drawing.DrawingDocument") )
        aConfigPath = "Office.Draw/Layout/Other/MeasureUnit";
    else
    {
        mbGraphicsSource = false;
        if ( xServiceInfo->supportsService("com.sun.star.sheet.SpreadsheetDocument") )
            aConfigPath = "Office.Calc/Layout/Other/MeasureUnit";
        else if ( xServiceInfo->supportsService("com.sun.star.text.TextDocument") )
            aConfigPath = "Office.Writer/Layout/Other/MeasureUnit";
    }
    if ( !aConfigPath.isEmpty() )
    {
        FilterConfigItem aConfigItem( aConfigPath );
        OUString aPropertyName;
        SvtSysLocale aSysLocale;
        if ( aSysLocale.GetLocaleDataPtr()->getMeasurementSystemEnum() == MeasurementSystem::Metric )
            aPropertyName = "Metric";
        else
            aPropertyName = "NonMetric";
        meFieldUnit = static_cast<FieldUnit>(
            aConfigItem.ReadInt32(aPropertyName, sal_Int32(FieldUnit::CM)));
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_svtools_SvFilterOptionsDialog_get_implementation(
    css::uno::XComponentContext * context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SvFilterOptionsDialog(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
