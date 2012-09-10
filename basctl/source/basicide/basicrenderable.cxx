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

#include "com/sun/star/awt/XDevice.hpp"
#include "toolkit/awt/vclxdevice.hxx"
#include "vcl/print.hxx"
#include "tools/multisel.hxx"
#include "tools/resary.hxx"

namespace basctl
{

using namespace com::sun::star;
using namespace com::sun::star::uno;

Renderable::Renderable (BaseWindow* pWin)
: cppu::WeakComponentImplHelper1< com::sun::star::view::XRenderable >( maMutex )
, mpWindow( pWin )
{
    ResStringArray aStrings( IDEResId( RID_PRINTDLG_STRLIST )  );
    DBG_ASSERT( aStrings.Count() >= 3, "resource incomplete" );
    if( aStrings.Count() < 3 ) // bad resource ?
        return;

    m_aUIProperties.realloc( 3 );

    // show Subgroup for print range
    vcl::PrinterOptionsHelper::UIControlOptions aPrintRangeOpt;
    aPrintRangeOpt.maGroupHint = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintRange" ) );
    aPrintRangeOpt.mbInternalOnly = true;
    m_aUIProperties[0].Value = setSubgroupControlOpt("printrange",
        rtl::OUString(aStrings.GetString(0)), rtl::OUString(), aPrintRangeOpt);

    // create a choice for the range to print
    rtl::OUString aPrintContentName( RTL_CONSTASCII_USTRINGPARAM( "PrintContent" ) );
    Sequence< rtl::OUString > aChoices( 2 );
    Sequence< rtl::OUString > aHelpIds( 2 );
    Sequence< rtl::OUString > aWidgetIds( 2 );
    aChoices[0] = aStrings.GetString( 1 );
    aHelpIds[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:0" ) );
    aChoices[1] = aStrings.GetString( 2 );
    aHelpIds[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:1" ) );
    aWidgetIds[0] = rtl::OUString("printallpages");
    aWidgetIds[1] = rtl::OUString("printpages");
    m_aUIProperties[1].Value = setChoiceRadiosControlOpt(aWidgetIds, rtl::OUString(),
                                                   aHelpIds, aPrintContentName,
                                                   aChoices, 0);

    // create a an Edit dependent on "Pages" selected
    vcl::PrinterOptionsHelper::UIControlOptions aPageRangeOpt(aPrintContentName, 1, true);
    m_aUIProperties[2].Value = setEditControlOpt("pagerange", rtl::OUString(),
                                                 rtl::OUString(), "PageRange",
                                                 rtl::OUString(), aPageRangeOpt);
}

Renderable::~Renderable()
{
}

Printer* Renderable::getPrinter()
{
    Printer* pPrinter = NULL;
    Any aValue( getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RenderDevice" ) ) ) );
    Reference<awt::XDevice> xRenderDevice;

    if( aValue >>= xRenderDevice )
    {
        VCLXDevice* pDevice = VCLXDevice::GetImplementation(xRenderDevice);
        OutputDevice* pOut = pDevice ? pDevice->GetOutputDevice() : NULL;
        pPrinter = dynamic_cast<Printer*>(pOut);
    }
    return pPrinter;
}

sal_Int32 SAL_CALL Renderable::getRendererCount (
        const Any&, const Sequence<beans::PropertyValue >& i_xOptions
        ) throw (lang::IllegalArgumentException, RuntimeException)
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
                rtl::OUString aPageRange( getStringValue( "PageRange" ) );
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
        ) throw (lang::IllegalArgumentException, RuntimeException)
{
    processProperties( i_xOptions );

    Sequence< beans::PropertyValue > aVals;
    // insert page size here
    Printer* pPrinter = getPrinter();
    // no renderdevice is legal; the first call is to get our print ui options
    if( pPrinter )
    {
        Size aPageSize( pPrinter->PixelToLogic( pPrinter->GetPaperSizePixel(), MapMode( MAP_100TH_MM ) ) );

        aVals.realloc( 1 );
        aVals[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) );
        awt::Size aSize;
        aSize.Width  = aPageSize.Width();
        aSize.Height = aPageSize.Height();
        aVals[0].Value <<= aSize;
    }

    appendPrintUIOptions( aVals );

    return aVals;
}

void SAL_CALL Renderable::render (
        sal_Int32 nRenderer, const Any&,
        const Sequence<beans::PropertyValue>& i_xOptions
        ) throw (lang::IllegalArgumentException, RuntimeException)
{
    processProperties( i_xOptions );

    if( mpWindow )
    {
        if (Printer* pPrinter = getPrinter())
        {
            sal_Int64 nContent = getIntValue( "PrintContent", -1 );
            if( nContent == 1 )
            {
                rtl::OUString aPageRange( getStringValue( "PageRange" ) );
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
