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
#include "bastypes.hxx"
#include "basidesh.hrc"

#include <toolkit/awt/vclxdevice.hxx>
#include <tools/multisel.hxx>
#include <tools/resary.hxx>
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
        IDE_RESSTR( RID_STR_PRINTDLG_RANGE ), OUString(), aPrintRangeOpt);

    // create a choice for the range to print
    OUString aPrintContentName( "PrintContent" );
    Sequence< OUString > aChoices( 2 );
    Sequence< OUString > aHelpIds( 2 );
    Sequence< OUString > aWidgetIds( 2 );
    aChoices[0] = IDE_RESSTR( RID_STR_PRINTDLG_ALLPAGES );
    aHelpIds[0] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:0" ;
    aChoices[1] = IDE_RESSTR( RID_STR_PRINTDLG_PAGES );
    aHelpIds[1] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:1" ;
    aWidgetIds[0] = "printallpages" ;
    aWidgetIds[1] = "printpages" ;
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

sal_Int32 SAL_CALL Renderable::getRendererCount (
        const Any&, const Sequence<beans::PropertyValue >& i_xOptions
        ) throw (lang::IllegalArgumentException, RuntimeException, std::exception)
{
    processProperties( i_xOptions );

    sal_Int32 nCount = 0;
    if( mpWindow )
    {
        if (Printer* pPrinter = getPrinter())
        {
            nCount = mpWindow->countPages( pPrinter );
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
        }
        else
            throw lang::IllegalArgumentException();
    }

    return nCount;
}

Sequence<beans::PropertyValue> SAL_CALL Renderable::getRenderer (
        sal_Int32, const Any&, const Sequence<beans::PropertyValue>& i_xOptions
        ) throw (lang::IllegalArgumentException, RuntimeException, std::exception)
{
    processProperties( i_xOptions );

    Sequence< beans::PropertyValue > aVals;
    // insert page size here
    Printer* pPrinter = getPrinter();
    // no renderdevice is legal; the first call is to get our print ui options
    if( pPrinter )
    {
        Size aPageSize( pPrinter->PixelToLogic( pPrinter->GetPaperSizePixel(), MapMode( MAP_100TH_MM ) ) );

        awt::Size aSize;
        aSize.Width  = aPageSize.Width();
        aSize.Height = aPageSize.Height();
        aVals = ::comphelper::InitPropertySequence({
            { "PageSize", makeAny(aSize) }
        });
    }

    appendPrintUIOptions( aVals );

    return aVals;
}

void SAL_CALL Renderable::render (
        sal_Int32 nRenderer, const Any&,
        const Sequence<beans::PropertyValue>& i_xOptions
        ) throw (lang::IllegalArgumentException, RuntimeException, std::exception)
{
    processProperties( i_xOptions );

    if( mpWindow )
    {
        if (Printer* pPrinter = getPrinter())
        {
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
                mpWindow->printPage( nRenderer, pPrinter );
        }
        else
            throw lang::IllegalArgumentException();
    }
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
