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

#include "basicrenderable.hxx"
#include <bastypes.hxx>
#include <strings.hrc>

#include <toolkit/awt/vclxdevice.hxx>
#include <tools/multisel.hxx>
#include <cppuhelper/compbase.hxx>
#include <comphelper/propertysequence.hxx>

namespace basctl
{

using namespace com::sun::star;
using namespace com::sun::star::uno;

Renderable::Renderable (BaseWindow* pWin)
: cppu::WeakComponentImplHelper< css::view::XRenderable >( maMutex )
, mpWindow( pWin )
{
    m_aUIProperties.resize( 3 );

    // show Subgroup for print range
    vcl::PrinterOptionsHelper::UIControlOptions aPrintRangeOpt;
    aPrintRangeOpt.maGroupHint = "PrintRange" ;
    aPrintRangeOpt.mbInternalOnly = true;

    m_aUIProperties[0].Value = setSubgroupControlOpt("printrange",
        IDEResId( RID_STR_PRINTDLG_PAGES ), OUString(), aPrintRangeOpt);

    // create a choice for the range to print
    OUString aPrintContentName( "PrintContent" );
    const Sequence<OUString> aChoices{IDEResId(RID_STR_PRINTDLG_PRINTALLPAGES),
                                      IDEResId(RID_STR_PRINTDLG_PRINTPAGES),
                                      IDEResId(RID_STR_PRINTDLG_PRINTEVENPAGES),
                                      IDEResId(RID_STR_PRINTDLG_PRINTODDPAGES)};
    const Sequence<OUString> aHelpIds{".HelpID:vcl:PrintDialog:PrintContent:RadioButton:0",
                                      ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:1",
                                      ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:2",
                                      ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:3"};
    const Sequence<OUString> aWidgetIds{"rbAllPages",
                                        "rbRangePages",
                                        "rbEvenPages",
                                        "rbOddPages"};
    m_aUIProperties[1].Value = setChoiceRadiosControlOpt(aWidgetIds, OUString(),
                                                   aHelpIds, aPrintContentName,
                                                   aChoices, 0);

    // create a an Edit dependent on "Pages" selected
    vcl::PrinterOptionsHelper::UIControlOptions aPageRangeOpt(aPrintContentName, 1, true);
    m_aUIProperties[2].Value = setEditControlOpt("pagerange", OUString(),
                                                 OUString(), "PageRange",
                                                 OUString(), aPageRangeOpt);
}

Renderable::~Renderable()
{
}

VclPtr< Printer > Renderable::getPrinter()
{
    VclPtr< Printer > pPrinter;
    Any aValue( getValue( "RenderDevice" ) );
    Reference<awt::XDevice> xRenderDevice;

    if( aValue >>= xRenderDevice )
    {
        VCLXDevice* pDevice = VCLXDevice::GetImplementation(xRenderDevice);
        VclPtr< OutputDevice > pOut = pDevice ? pDevice->GetOutputDevice() : VclPtr< OutputDevice >();
        pPrinter = dynamic_cast<Printer*>(pOut.get());
    }
    return pPrinter;
}

bool Renderable::isPrintOddPages()
{
    sal_Int64 nContent = getIntValue( "PrintContent", -1 );
    return nContent != 2;
}

bool Renderable::isPrintEvenPages()
{
    sal_Int64 nContent = getIntValue( "PrintContent", -1 );
    return nContent != 3;
}

sal_Int32 SAL_CALL Renderable::getRendererCount (
        const Any&, const Sequence<beans::PropertyValue >& i_xOptions
        )
{
    processProperties( i_xOptions );

    maValidPages.clear();

    sal_Int32 nCount = 0;
    if( mpWindow )
    {
        VclPtr<Printer> pPrinter = getPrinter();
        if (!pPrinter)
            throw lang::IllegalArgumentException();

        nCount = mpWindow->countPages( pPrinter );

        for (sal_Int32 nPage = 1; nPage <= nCount; nPage++)
        {
            if ( (isPrintEvenPages() && isOnEvenPage( nPage ))
                || (isPrintOddPages() && !isOnEvenPage( nPage )) )
            {
                maValidPages.push_back( nPage-1 );
            }
        }

        sal_Int64 nContent = getIntValue( "PrintContent", -1 );
        if( nContent == 1 )
        {
            OUString aPageRange( getStringValue( "PageRange" ) );
            if( !aPageRange.isEmpty() )
            {
                StringRangeEnumerator aRangeEnum( aPageRange, 0, nCount-1 );
                sal_Int32 nSelCount = aRangeEnum.size();
                if( nSelCount >= 0 )
                    nCount = nSelCount;
            }
        }
        else if ( nContent == 2 || nContent == 3 ) // even/odd pages
            return static_cast<sal_Int32>( maValidPages.size() );
    }

    return nCount;
}

Sequence<beans::PropertyValue> SAL_CALL Renderable::getRenderer (
        sal_Int32, const Any&, const Sequence<beans::PropertyValue>& i_xOptions
        )
{
    processProperties( i_xOptions );

    Sequence< beans::PropertyValue > aVals;
    // insert page size here
    VclPtr<Printer> pPrinter = getPrinter();
    // no renderdevice is legal; the first call is to get our print ui options
    if( pPrinter )
    {
        Size aPageSize( pPrinter->PixelToLogic( pPrinter->GetPaperSizePixel(), MapMode( MapUnit::Map100thMM ) ) );

        awt::Size aSize;
        aSize.Width  = aPageSize.Width();
        aSize.Height = aPageSize.Height();
        aVals = ::comphelper::InitPropertySequence({
            { "PageSize", Any(aSize) }
        });
    }

    appendPrintUIOptions( aVals );

    return aVals;
}

void SAL_CALL Renderable::render (
        sal_Int32 nRenderer, const Any&,
        const Sequence<beans::PropertyValue>& i_xOptions
        )
{
    processProperties( i_xOptions );

    if( mpWindow )
    {
        VclPtr<Printer> pPrinter = getPrinter();
        if (!pPrinter)
            throw lang::IllegalArgumentException();

        sal_Int64 nContent = getIntValue( "PrintContent", -1 );
        if( nContent == 1 )
        {
            OUString aPageRange( getStringValue( "PageRange" ) );
            if( !aPageRange.isEmpty() )
            {
                sal_Int32 nPageCount = mpWindow->countPages( pPrinter );
                StringRangeEnumerator aRangeEnum( aPageRange, 0, nPageCount-1 );
                StringRangeEnumerator::Iterator it = aRangeEnum.begin();
                for( ; it != aRangeEnum.end() && nRenderer; --nRenderer )
                    ++it;

                sal_Int32 nPage = ( it != aRangeEnum.end() ) ? *it : nRenderer;
                mpWindow->printPage( nPage, pPrinter );
            }
            else
                mpWindow->printPage( nRenderer, pPrinter );
        }
        else
            mpWindow->printPage( maValidPages.at( nRenderer ), pPrinter );
    }
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
