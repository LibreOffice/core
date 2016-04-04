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

#include "GraphicExportDialog.hxx"

#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <svtools/GraphicExportOptionsDialog.hxx>
#include <unotools/syslocale.hxx>

using namespace css;
using namespace css::uno;
using namespace css::beans;
using namespace css::lang;

GraphicExportDialog::GraphicExportDialog( const Reference< XComponentContext >& )
    : meFieldUnit(FUNIT_NONE)
    , mbExportSelection( false )
{
}

GraphicExportDialog::~GraphicExportDialog()
{
}

// XInitialization
void SAL_CALL GraphicExportDialog::initialize( const Sequence<Any>& ) throw ( Exception, RuntimeException, std::exception )
{}

// XPropertyAccess
Sequence<PropertyValue> GraphicExportDialog::getPropertyValues() throw ( RuntimeException, std::exception )
{
    sal_Int32 i;
    sal_Int32 nCount = maMediaDescriptor.getLength();

    for ( i = 0; i < nCount; i++ )
    {
        if ( maMediaDescriptor[ i ].Name == "FilterData" )
        {
            break;
        }
    }

    if ( i >= nCount )
    {
        nCount++;
        maMediaDescriptor.realloc( nCount );
    }

    maMediaDescriptor[ i ].Name = "FilterData";
    maMediaDescriptor[ i ].Value <<= maFilterDataSequence;
    return maMediaDescriptor;
}

void GraphicExportDialog::setPropertyValues( const Sequence<PropertyValue>& aProps )
    throw ( UnknownPropertyException, PropertyVetoException, IllegalArgumentException,
            WrappedTargetException, RuntimeException, std::exception )
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
void GraphicExportDialog::setTitle( const OUString& aTitle )
    throw ( uno::RuntimeException, std::exception )
{
    maDialogTitle = aTitle;
}

sal_Int16 GraphicExportDialog::execute() throw ( RuntimeException, std::exception )
{
    sal_Int16 nReturn = ui::dialogs::ExecutableDialogResults::CANCEL;
    ScopedVclPtrInstance< GraphicExportOptionsDialog > graphicExportOptionsDialog( Application::GetDefDialogParent(), mxSourceDocument );
    if (graphicExportOptionsDialog->Execute() == RET_OK )
    {
        maFilterDataSequence = graphicExportOptionsDialog->getFilterData();
        nReturn = ui::dialogs::ExecutableDialogResults::OK;
    }
    return nReturn;
}

// XEmporter
void GraphicExportDialog::setSourceDocument( const Reference<XComponent>& xDocument )
    throw ( IllegalArgumentException, RuntimeException, std::exception )
{
    mxSourceDocument = xDocument;

    // try to set the corresponding metric unit
    OUString aConfigPath;
    Reference< XServiceInfo > xServiceInfo ( xDocument, UNO_QUERY );

    if ( xServiceInfo.is() )
    {
        if ( xServiceInfo->supportsService("com.sun.star.presentation.PresentationDocument") )
        {
            aConfigPath = "Office.Impress/Layout/Other/MeasureUnit";
        }
        else if ( xServiceInfo->supportsService("com.sun.star.drawing.DrawingDocument") )
        {
            aConfigPath = "Office.Draw/Layout/Other/MeasureUnit";
        }
        else if ( xServiceInfo->supportsService("com.sun.star.text.TextDocument") || xServiceInfo->supportsService("com.sun.star.text.WebDocument") )
        {
            aConfigPath = "Office.Writer/Layout/Other/MeasureUnit";
        }
        if ( !aConfigPath.isEmpty() )
        {
            FilterConfigItem aConfigItem( aConfigPath );
            OUString aPropertyName;
            SvtSysLocale aSysLocale;

            if ( aSysLocale.GetLocaleDataPtr()->getMeasurementSystemEnum() == MEASURE_METRIC )
            {
                aPropertyName = "Metric";
            }
            else
            {
                aPropertyName = "NonMetric";
            }
            meFieldUnit = (FieldUnit) aConfigItem.ReadInt32( aPropertyName, FUNIT_CM );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
