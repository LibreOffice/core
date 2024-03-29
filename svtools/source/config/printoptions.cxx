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

#include <svtools/printoptions.hxx>
#include <vcl/printer/Options.hxx>
#include <officecfg/Office/Common.hxx>
#include <sal/macros.h>
#include <tools/long.hxx>

const sal_uInt16 aDPIArray[] = { 72, 96, 150, 200, 300, 600 };

#define DPI_COUNT (SAL_N_ELEMENTS(aDPIArray))

namespace svtools
{

void GetPrinterOptions( vcl::printer::Options& rOptions, bool bFile )
{
    if (bFile)
    {
        rOptions.SetReduceTransparency( officecfg::Office::Common::Print::Option::File::ReduceTransparency::get() );
        rOptions.SetReducedTransparencyMode( static_cast<vcl::printer::TransparencyMode>(
                officecfg::Office::Common::Print::Option::File::ReducedTransparencyMode::get() ) );
        rOptions.SetReduceGradients( officecfg::Office::Common::Print::Option::File::ReduceGradients::get() );
        rOptions.SetReducedGradientMode( static_cast<vcl::printer::GradientMode>(
                officecfg::Office::Common::Print::Option::File::ReducedGradientMode::get()) );
        rOptions.SetReducedGradientStepCount( officecfg::Office::Common::Print::Option::File::ReducedGradientStepCount::get() );
        rOptions.SetReduceBitmaps( officecfg::Office::Common::Print::Option::File::ReduceBitmaps::get() );
        rOptions.SetReducedBitmapMode( static_cast<vcl::printer::BitmapMode>(
                officecfg::Office::Common::Print::Option::File::ReducedBitmapMode::get()) );
        rOptions.SetReducedBitmapResolution( aDPIArray[ std::min( static_cast<sal_uInt16>(
                officecfg::Office::Common::Print::Option::File::ReducedBitmapResolution::get()), sal_uInt16( DPI_COUNT - 1 ) ) ] );
        rOptions.SetReducedBitmapIncludesTransparency(
                officecfg::Office::Common::Print::Option::File::ReducedBitmapIncludesTransparency::get() );
        rOptions.SetConvertToGreyscales( officecfg::Office::Common::Print::Option::File::ConvertToGreyscales::get() );
    }
    else
    {
        rOptions.SetReduceTransparency( officecfg::Office::Common::Print::Option::Printer::ReduceTransparency::get() );
        rOptions.SetReducedTransparencyMode( static_cast<vcl::printer::TransparencyMode>(
                officecfg::Office::Common::Print::Option::Printer::ReducedTransparencyMode::get() ) );
        rOptions.SetReduceGradients( officecfg::Office::Common::Print::Option::Printer::ReduceGradients::get() );
        rOptions.SetReducedGradientMode( static_cast<vcl::printer::GradientMode>(
                officecfg::Office::Common::Print::Option::Printer::ReducedGradientMode::get()) );
        rOptions.SetReducedGradientStepCount( officecfg::Office::Common::Print::Option::Printer::ReducedGradientStepCount::get() );
        rOptions.SetReduceBitmaps( officecfg::Office::Common::Print::Option::Printer::ReduceBitmaps::get() );
        rOptions.SetReducedBitmapMode( static_cast<vcl::printer::BitmapMode>(
                officecfg::Office::Common::Print::Option::Printer::ReducedBitmapMode::get()) );
        rOptions.SetReducedBitmapResolution( aDPIArray[ std::min( static_cast<sal_uInt16>(
                officecfg::Office::Common::Print::Option::Printer::ReducedBitmapResolution::get()), sal_uInt16( DPI_COUNT - 1 ) ) ] );
        rOptions.SetReducedBitmapIncludesTransparency(
                officecfg::Office::Common::Print::Option::Printer::ReducedBitmapIncludesTransparency::get() );
        rOptions.SetConvertToGreyscales( officecfg::Office::Common::Print::Option::Printer::ConvertToGreyscales::get() );
    }
}

void SetPrinterOptions( const vcl::printer::Options& rOptions, bool bFile )
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    if (bFile)
    {
        officecfg::Office::Common::Print::Option::File::ReduceTransparency::set(
                rOptions.IsReduceTransparency(), batch );
        officecfg::Office::Common::Print::Option::File::ReducedTransparencyMode::set(
                static_cast<sal_Int16>(rOptions.GetReducedTransparencyMode()), batch );
        officecfg::Office::Common::Print::Option::File::ReduceGradients::set(
                rOptions.IsReduceGradients(), batch );
        officecfg::Office::Common::Print::Option::File::ReducedGradientMode::set(
                static_cast<sal_Int16>(rOptions.GetReducedGradientMode()), batch );
        officecfg::Office::Common::Print::Option::File::ReducedGradientStepCount::set(
                rOptions.GetReducedGradientStepCount(), batch );
        officecfg::Office::Common::Print::Option::File::ReduceBitmaps::set(
                rOptions.IsReduceBitmaps(), batch );
        officecfg::Office::Common::Print::Option::File::ReducedBitmapMode::set(
                static_cast<sal_Int16>(rOptions.GetReducedBitmapMode()), batch );
        officecfg::Office::Common::Print::Option::File::ReducedBitmapIncludesTransparency::set(
                rOptions.IsReducedBitmapIncludesTransparency(), batch );
        officecfg::Office::Common::Print::Option::File::ConvertToGreyscales::set(
                rOptions.IsConvertToGreyscales(), batch );

        const sal_uInt16 nDPI = rOptions.GetReducedBitmapResolution();

        if( nDPI < aDPIArray[ 0 ] )
            officecfg::Office::Common::Print::Option::File::ReducedBitmapResolution::set( 0, batch );
        else
        {
            for( tools::Long i = DPI_COUNT - 1; i >= 0; i-- )
            {
                if( nDPI >= aDPIArray[ i ] )
                {
                    officecfg::Office::Common::Print::Option::File::ReducedBitmapResolution::set(
                            static_cast<sal_Int16>(i), batch );
                    i = -1;
                }
            }
        }
    }
    else
    {
        officecfg::Office::Common::Print::Option::Printer::ReduceTransparency::set(
                rOptions.IsReduceTransparency(), batch );
        officecfg::Office::Common::Print::Option::Printer::ReducedTransparencyMode::set(
                static_cast<sal_Int16>(rOptions.GetReducedTransparencyMode()), batch );
        officecfg::Office::Common::Print::Option::Printer::ReduceGradients::set(
                rOptions.IsReduceGradients(), batch );
        officecfg::Office::Common::Print::Option::Printer::ReducedGradientMode::set(
                static_cast<sal_Int16>(rOptions.GetReducedGradientMode()), batch );
        officecfg::Office::Common::Print::Option::Printer::ReducedGradientStepCount::set(
                rOptions.GetReducedGradientStepCount(), batch );
        officecfg::Office::Common::Print::Option::Printer::ReduceBitmaps::set(
                rOptions.IsReduceBitmaps(), batch );
        officecfg::Office::Common::Print::Option::Printer::ReducedBitmapMode::set(
                static_cast<sal_Int16>(rOptions.GetReducedBitmapMode()), batch );
        officecfg::Office::Common::Print::Option::Printer::ReducedBitmapIncludesTransparency::set(
                rOptions.IsReducedBitmapIncludesTransparency(), batch );
        officecfg::Office::Common::Print::Option::Printer::ConvertToGreyscales::set(
                rOptions.IsConvertToGreyscales(), batch );

        const sal_uInt16 nDPI = rOptions.GetReducedBitmapResolution();

        if( nDPI < aDPIArray[ 0 ] )
            officecfg::Office::Common::Print::Option::Printer::ReducedBitmapResolution::set( 0, batch );
        else
        {
            for( tools::Long i = DPI_COUNT - 1; i >= 0; i-- )
            {
                if( nDPI >= aDPIArray[ i ] )
                {
                    officecfg::Office::Common::Print::Option::Printer::ReducedBitmapResolution::set(
                            static_cast<sal_Int16>(i), batch );
                    i = -1;
                }
            }
        }
    }
    batch->commit();
}

} // namespace svtools






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
