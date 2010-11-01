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

#include "precompiled_vcl.hxx"

#include "vcl/oldprintadaptor.hxx"
#include "vcl/gdimtf.hxx"

#include "com/sun/star/awt/Size.hpp"

#include <vector>

namespace vcl
{
    struct AdaptorPage
    {
        GDIMetaFile                     maPage;
        com::sun::star::awt::Size       maPageSize;
    };

    struct ImplOldStyleAdaptorData
    {
        std::vector< AdaptorPage >  maPages;
    };
}

using namespace vcl;
using namespace cppu;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

OldStylePrintAdaptor::OldStylePrintAdaptor( const boost::shared_ptr< Printer >& i_pPrinter )
    : PrinterController( i_pPrinter )
    , mpData( new ImplOldStyleAdaptorData() )
{
}

OldStylePrintAdaptor::~OldStylePrintAdaptor()
{
}

void OldStylePrintAdaptor::StartPage()
{
    Size aPaperSize( getPrinter()->PixelToLogic( getPrinter()->GetPaperSizePixel(), MapMode( MAP_100TH_MM ) ) );
    mpData->maPages.push_back( AdaptorPage() );
    mpData->maPages.back().maPageSize.Width = aPaperSize.getWidth();
    mpData->maPages.back().maPageSize.Height = aPaperSize.getHeight();
    getPrinter()->SetConnectMetaFile( &mpData->maPages.back().maPage );

    // copy state into metafile
    boost::shared_ptr<Printer> pPrinter( getPrinter() );
    pPrinter->SetMapMode( pPrinter->GetMapMode() );
    pPrinter->SetFont( pPrinter->GetFont() );
    pPrinter->SetDrawMode( pPrinter->GetDrawMode() );
    pPrinter->SetLineColor( pPrinter->GetLineColor() );
    pPrinter->SetFillColor( pPrinter->GetFillColor() );
}

void OldStylePrintAdaptor::EndPage()
{
    getPrinter()->SetConnectMetaFile( NULL );
    mpData->maPages.back().maPage.WindStart();
}

int  OldStylePrintAdaptor::getPageCount() const
{
    return int(mpData->maPages.size());
}

Sequence< PropertyValue > OldStylePrintAdaptor::getPageParameters( int i_nPage ) const
{
    Sequence< PropertyValue > aRet( 1 );
    aRet[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageSize") );
    if( i_nPage < int(mpData->maPages.size() ) )
        aRet[0].Value = makeAny( mpData->maPages[i_nPage].maPageSize );
    else
    {
        awt::Size aEmpty( 0, 0  );
        aRet[0].Value = makeAny( aEmpty );
    }
    return aRet;
}

void OldStylePrintAdaptor::printPage( int i_nPage ) const
{
    if( i_nPage < int(mpData->maPages.size()) )
   {
       mpData->maPages[ i_nPage ].maPage.WindStart();
       mpData->maPages[ i_nPage ].maPage.Play( getPrinter().get() );
   }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
