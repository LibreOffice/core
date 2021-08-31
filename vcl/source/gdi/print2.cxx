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

#include <sal/log.hxx>

#include <vcl/metaact.hxx>
#include <vcl/print.hxx>
#include <vcl/printer/Options.hxx>

#include <utility>
#include <vector>

void Printer::DrawGradientEx( OutputDevice* pOut, const tools::Rectangle& rRect, const Gradient& rGradient )
{
    const vcl::printer::Options& rPrinterOptions = GetPrinterOptions();

    if( rPrinterOptions.IsReduceGradients() )
    {
        if( vcl::printer::GradientMode::Stripes == rPrinterOptions.GetReducedGradientMode() )
        {
            if( !rGradient.GetSteps() || ( rGradient.GetSteps() > rPrinterOptions.GetReducedGradientStepCount() ) )
            {
                Gradient aNewGradient( rGradient );

                aNewGradient.SetSteps( rPrinterOptions.GetReducedGradientStepCount() );
                pOut->DrawGradient( rRect, aNewGradient );
            }
            else
                pOut->DrawGradient( rRect, rGradient );
        }
        else
        {
            const Color&    rStartColor = rGradient.GetStartColor();
            const Color&    rEndColor = rGradient.GetEndColor();
            const tools::Long      nR = ( ( static_cast<tools::Long>(rStartColor.GetRed()) * rGradient.GetStartIntensity() ) / 100 +
                                   ( static_cast<tools::Long>(rEndColor.GetRed()) * rGradient.GetEndIntensity() ) / 100 ) >> 1;
            const tools::Long      nG = ( ( static_cast<tools::Long>(rStartColor.GetGreen()) * rGradient.GetStartIntensity() ) / 100 +
                                   ( static_cast<tools::Long>(rEndColor.GetGreen()) * rGradient.GetEndIntensity() ) / 100 ) >> 1;
            const tools::Long      nB = ( ( static_cast<tools::Long>(rStartColor.GetBlue()) * rGradient.GetStartIntensity() ) / 100 +
                                   ( static_cast<tools::Long>(rEndColor.GetBlue()) * rGradient.GetEndIntensity() ) / 100 ) >> 1;
            const Color     aColor( static_cast<sal_uInt8>(nR), static_cast<sal_uInt8>(nG), static_cast<sal_uInt8>(nB) );

            pOut->Push( PushFlags::LINECOLOR | PushFlags::FILLCOLOR );
            pOut->SetLineColor( aColor );
            pOut->SetFillColor( aColor );
            pOut->DrawRect( rRect );
            pOut->Pop();
        }
    }
    else
        pOut->DrawGradient( rRect, rGradient );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
