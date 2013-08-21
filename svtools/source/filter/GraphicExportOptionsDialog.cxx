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

#include "GraphicExportOptionsDialog.hxx"

using namespace css::beans;
using namespace css::lang;
using namespace css::uno;

GraphicExportOptionsDialog::GraphicExportOptionsDialog(Window* pWindow, const Reference<XComponent>& rxSourceDocument) :
    ModalDialog(pWindow, "GraphicExporter", "svt/ui/GraphicExportOptionsDialog.ui"),
    mResolution(96.0),
    mRenderer(rxSourceDocument)
{
    get(mpWidth,          "spin-width");
    get(mpHeight,         "spin-height");
    get(mpResolution,     "combo-resolution");

    mpWidth->SetModifyHdl( LINK( this, GraphicExportOptionsDialog, widthModifiedHandle ));
    mpHeight->SetModifyHdl( LINK( this, GraphicExportOptionsDialog, heightModifiedHandle ));
    mpResolution->SetModifyHdl( LINK( this, GraphicExportOptionsDialog, resolutionModifiedHandle ));

    initialize();

    updateWidth();
    updateHeight();
    updateResolution();
}

GraphicExportOptionsDialog::~GraphicExportOptionsDialog()
{}

void GraphicExportOptionsDialog::initialize()
{
    mCurrentPage = mRenderer.getCurrentPageWriter();
    mSize100mm = mRenderer.getDocumentSizeIn100mm(mCurrentPage);
}

IMPL_LINK_NOARG( GraphicExportOptionsDialog, widthModifiedHandle )
{
    mResolution =  mpWidth->GetValue() / getViewWidthInch();

    updateHeight();
    updateResolution();

    return 0L;
}

IMPL_LINK_NOARG( GraphicExportOptionsDialog, heightModifiedHandle )
{
    mResolution =  mpHeight->GetValue() / getViewHeightInch();

    updateWidth();
    updateResolution();

    return 0L;
}

IMPL_LINK_NOARG( GraphicExportOptionsDialog, resolutionModifiedHandle )
{
    mResolution = mpResolution->GetText().toInt32();

    updateWidth();
    updateHeight();
    Update();

    return 0L;
}

double GraphicExportOptionsDialog::getViewWidthInch()
{
    return (double) MetricField::ConvertValue(mSize100mm.Width(),  2, MAP_100TH_MM, FUNIT_INCH) / 100.0;
}

double GraphicExportOptionsDialog::getViewHeightInch()
{
    return (double) MetricField::ConvertValue(mSize100mm.Height(),  2, MAP_100TH_MM, FUNIT_INCH) / 100.0;
}

void GraphicExportOptionsDialog::updateWidth()
{
    sal_Int32 aWidth = (sal_Int32)( getViewWidthInch() * mResolution );
    mpWidth->SetText( OUString::number( aWidth ));
}

void GraphicExportOptionsDialog::updateHeight()
{
    sal_Int32 aHeight = (sal_Int32)( getViewHeightInch() * mResolution );
    mpHeight->SetText( OUString::number( aHeight ));
}

void GraphicExportOptionsDialog::updateResolution()
{
    mpResolution->SetText( OUString::number( (sal_Int32) mResolution ) );
}

Sequence<PropertyValue> GraphicExportOptionsDialog::getFilterData()
{
    sal_Int32 aWidth = (sal_Int32)( getViewWidthInch() * mResolution );
    sal_Int32 aHeight = (sal_Int32)( getViewHeightInch() * mResolution );

    Sequence<PropertyValue> aFilterData;

    aFilterData.realloc( 2 );
    aFilterData[ 0 ].Name = "PixelWidth";
    aFilterData[ 0 ].Value <<= aWidth;
    aFilterData[ 1 ].Name = "PixelHeight";
    aFilterData[ 1 ].Value <<= aHeight;

    return aFilterData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
