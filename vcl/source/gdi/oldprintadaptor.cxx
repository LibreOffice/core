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

#include <sal/config.h>

#include <comphelper/propertyvalue.hxx>
#include <vcl/oldprintadaptor.hxx>
#include <vcl/gdimtf.hxx>

#include <com/sun/star/awt/Size.hpp>

#include <vector>

using namespace vcl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

namespace vcl
{
    namespace {

    struct AdaptorPage
    {
        GDIMetaFile          maPage;
        css::awt::Size       maPageSize;
    };

    }

    struct ImplOldStyleAdaptorData
    {
        std::vector< AdaptorPage >  maPages;
    };
}

OldStylePrintAdaptor::OldStylePrintAdaptor(const VclPtr<Printer>& i_xPrinter, weld::Window* i_pWindow)
    : PrinterController(i_xPrinter, i_pWindow)
    , mpData(new ImplOldStyleAdaptorData)
{
}

OldStylePrintAdaptor::~OldStylePrintAdaptor()
{
}

void OldStylePrintAdaptor::StartPage()
{
    Size aPaperSize( getPrinter()->PixelToLogic( getPrinter()->GetPaperSizePixel(), MapMode( MapUnit::Map100thMM ) ) );
    mpData->maPages.emplace_back( );
    mpData->maPages.back().maPageSize.Width = aPaperSize.getWidth();
    mpData->maPages.back().maPageSize.Height = aPaperSize.getHeight();
    getPrinter()->SetConnectMetaFile( &mpData->maPages.back().maPage );

    // copy state into metafile
    VclPtr<Printer> xPrinter( getPrinter() );
    xPrinter->SetMapMode(xPrinter->GetMapMode());
    xPrinter->SetFont(xPrinter->GetFont());
    xPrinter->SetDrawMode(xPrinter->GetDrawMode());
    xPrinter->SetLineColor(xPrinter->GetLineColor());
    xPrinter->SetFillColor(xPrinter->GetFillColor());
}

void OldStylePrintAdaptor::EndPage()
{
    getPrinter()->SetConnectMetaFile( nullptr );
    mpData->maPages.back().maPage.WindStart();
}

int  OldStylePrintAdaptor::getPageCount() const
{
    return int(mpData->maPages.size());
}

Sequence< PropertyValue > OldStylePrintAdaptor::getPageParameters( int i_nPage ) const
{
    css::awt::Size aSize;
    if( i_nPage < int(mpData->maPages.size() ) )
        aSize = mpData->maPages[i_nPage].maPageSize;
    return { comphelper::makePropertyValue("PageSize", css::uno::Any(aSize)) };
}

void OldStylePrintAdaptor::printPage( int i_nPage ) const
{
    if( i_nPage < int(mpData->maPages.size()) )
    {
        mpData->maPages[ i_nPage ].maPage.WindStart();
        mpData->maPages[ i_nPage ].maPage.Play(*getPrinter());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
