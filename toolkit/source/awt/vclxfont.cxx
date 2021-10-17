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

#include <memory>

#include <com/sun/star/awt/XDevice.hpp>

#include <comphelper/sequence.hxx>
#include <toolkit/awt/vclxfont.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <vcl/metric.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>



VCLXFont::VCLXFont()
{
    mpFontMetric = nullptr;
}

VCLXFont::~VCLXFont()
{
}

void VCLXFont::Init( css::awt::XDevice& rxDev, const vcl::Font& rFont )
{
    mxDevice = &rxDev;

    mpFontMetric.reset();

    maFont = rFont;
}

bool VCLXFont::ImplAssertValidFontMetric()
{
    if ( !mpFontMetric && mxDevice.is() )
    {
        OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
        if ( pOutDev )
        {
            vcl::Font aOldFont = pOutDev->GetFont();
            pOutDev->SetFont( maFont );
            mpFontMetric.reset( new FontMetric( pOutDev->GetFontMetric() ) );
            pOutDev->SetFont( aOldFont );
        }
    }
    return mpFontMetric != nullptr;
}

// css::lang::XUnoTunnel
UNO3_GETIMPLEMENTATION_IMPL( VCLXFont );

css::awt::FontDescriptor VCLXFont::getFontDescriptor(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return VCLUnoHelper::CreateFontDescriptor( maFont );

}

css::awt::SimpleFontMetric VCLXFont::getFontMetric(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::awt::SimpleFontMetric aFM;
    if ( ImplAssertValidFontMetric() )
        aFM = VCLUnoHelper::CreateFontMetric( *mpFontMetric );
    return aFM;
}

sal_Int16 VCLXFont::getCharWidth( sal_Unicode c )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int16 nRet = -1;
    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if ( pOutDev )
    {
        vcl::Font aOldFont = pOutDev->GetFont();
        pOutDev->SetFont( maFont );

        nRet = sal::static_int_cast< sal_Int16 >(
            pOutDev->GetTextWidth( OUString(c) ));

        pOutDev->SetFont( aOldFont );
    }
    return nRet;
}

css::uno::Sequence< sal_Int16 > VCLXFont::getCharWidths( sal_Unicode nFirst, sal_Unicode nLast )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::uno::Sequence<sal_Int16> aSeq;
    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if ( pOutDev )
    {
        vcl::Font aOldFont = pOutDev->GetFont();
        pOutDev->SetFont( maFont );

        sal_Int16 nCount = nLast-nFirst + 1;
        aSeq = css::uno::Sequence<sal_Int16>( nCount );
        for ( sal_uInt16 n = 0; n < nCount; n++ )
        {
            aSeq.getArray()[n] = sal::static_int_cast< sal_Int16 >(
                pOutDev->GetTextWidth(
                    OUString(static_cast< sal_Unicode >(nFirst+n)) ));
        }

        pOutDev->SetFont( aOldFont );
    }
    return aSeq;
}

sal_Int32 VCLXFont::getStringWidth( const OUString& str )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nRet = -1;
    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if ( pOutDev )
    {
        vcl::Font aOldFont = pOutDev->GetFont();
        pOutDev->SetFont( maFont );
        nRet = pOutDev->GetTextWidth( str );
        pOutDev->SetFont( aOldFont );
    }
    return nRet;
}

sal_Int32 VCLXFont::getStringWidthArray( const OUString& str, css::uno::Sequence< sal_Int32 >& rDXArray )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nRet = -1;
    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if ( pOutDev )
    {
        vcl::Font aOldFont = pOutDev->GetFont();
        pOutDev->SetFont( maFont );
        std::vector<tools::Long> aDXA;
        nRet = pOutDev->GetTextArray( str, &aDXA );
        // I don't know if size of aDXA is guaranteed same as length of str, so use arrayToSequence
        rDXArray = comphelper::arrayToSequence<sal_Int32>(aDXA.data(), str.getLength());
        pOutDev->SetFont( aOldFont );
    }
    return nRet;
}

void VCLXFont::getKernPairs( css::uno::Sequence< sal_Unicode >& /*rnChars1*/, css::uno::Sequence< sal_Unicode >& /*rnChars2*/, css::uno::Sequence< sal_Int16 >& /*rnKerns*/ )
{
    // NOTE: this empty method is just used for keeping the related UNO-API stable
}

// css::awt::XFont2
sal_Bool VCLXFont::hasGlyphs( const OUString& aText )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    SolarMutexGuard aSolarGuard;

    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if ( pOutDev )
    {
        if ( pOutDev->HasGlyphs( maFont, aText ) == -1 )
        {
            return true;
        }
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
