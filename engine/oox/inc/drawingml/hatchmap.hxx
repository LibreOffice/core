/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/drawing/Hatch.hpp>
#include <oox/token/tokens.hxx>

#include <tools/color.hxx>

static css::drawing::Hatch createHatch(sal_Int32 nHatchToken, ::Color nColor)
{
    css::drawing::Hatch aHatch;
    aHatch.Color = sal_Int32(nColor);
    // best-effort mapping; we do not support all the styles in core
    switch (nHatchToken)
    {
        case oox::XML_pct5:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 250;
            aHatch.Angle = 450;
            break;
        case oox::XML_pct10:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 200;
            aHatch.Angle = 450;
            break;
        case oox::XML_pct20:
        case oox::XML_dashUpDiag:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 150;
            aHatch.Angle = 450;
            break;
        case oox::XML_pct25:
            aHatch.Style = css::drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 200;
            aHatch.Angle = 450;
            break;
        case oox::XML_pct30:
            aHatch.Style = css::drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 175;
            aHatch.Angle = 450;
            break;
        case oox::XML_pct40:
        case oox::XML_weave:
            aHatch.Style = css::drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 150;
            aHatch.Angle = 450;
            break;
        case oox::XML_pct50:
            aHatch.Style = css::drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 125;
            aHatch.Angle = 450;
            break;
        case oox::XML_pct60:
            aHatch.Style = css::drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 150;
            aHatch.Angle = 450;
            break;
        case oox::XML_pct70:
            aHatch.Style = css::drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 125;
            aHatch.Angle = 450;
            break;
        case oox::XML_pct75:
            aHatch.Style = css::drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 100;
            aHatch.Angle = 450;
            break;
        case oox::XML_pct80:
            aHatch.Style = css::drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 75;
            aHatch.Angle = 450;
            break;
        case oox::XML_pct90:
            aHatch.Style = css::drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 50;
            aHatch.Angle = 450;
            break;
        case oox::XML_horz:
        case oox::XML_wave:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 100;
            aHatch.Angle = 0;
            break;
        case oox::XML_vert:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 100;
            aHatch.Angle = 900;
            break;
        case oox::XML_ltHorz:
        case oox::XML_narHorz:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 50;
            aHatch.Angle = 0;
            break;
        case oox::XML_ltVert:
        case oox::XML_narVert:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 50;
            aHatch.Angle = 900;
            break;
        case oox::XML_dkHorz:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 25;
            aHatch.Angle = 0;
            break;
        case oox::XML_dkVert:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 25;
            aHatch.Angle = 900;
            break;
        case oox::XML_dashHorz:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 150;
            aHatch.Angle = 0;
            break;
        case oox::XML_dashVert:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 150;
            aHatch.Angle = 900;
            break;
        case oox::XML_cross:
        case oox::XML_lgGrid:
            aHatch.Style = css::drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 100;
            aHatch.Angle = 0;
            break;
        case oox::XML_dnDiag:
        case oox::XML_wdDnDiag:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 100;
            aHatch.Angle = 1350;
            break;
        case oox::XML_upDiag:
        case oox::XML_wdUpDiag:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 100;
            aHatch.Angle = 450;
            break;
        case oox::XML_ltDnDiag:
        case oox::XML_dkDnDiag:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 50;
            aHatch.Angle = 1350;
            break;
        case oox::XML_ltUpDiag:
        case oox::XML_dkUpDiag:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 50;
            aHatch.Angle = 450;
            break;
        case oox::XML_dashDnDiag:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 150;
            aHatch.Angle = 1350;
            break;
        case oox::XML_diagCross:
            aHatch.Style = css::drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 100;
            aHatch.Angle = 450;
            break;
        case oox::XML_smCheck:
            aHatch.Style = css::drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 50;
            aHatch.Angle = 450;
            break;
        case oox::XML_lgCheck:
            aHatch.Style = css::drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 100;
            aHatch.Angle = 450;
            break;
        case oox::XML_smGrid:
            aHatch.Style = css::drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 50;
            aHatch.Angle = 0;
            break;
        case oox::XML_dotGrid:
            aHatch.Style = css::drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 400;
            aHatch.Angle = 0;
            break;
        case oox::XML_smConfetti:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 200;
            aHatch.Angle = 600;
            break;
        case oox::XML_lgConfetti:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 100;
            aHatch.Angle = 600;
            break;
        case oox::XML_horzBrick:
            aHatch.Style = css::drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 300;
            aHatch.Angle = 0;
            break;
        case oox::XML_diagBrick:
        case oox::XML_dotDmnd:
            aHatch.Style = css::drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 300;
            aHatch.Angle = 450;
            break;
        case oox::XML_solidDmnd:
            aHatch.Style = css::drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 100;
            aHatch.Angle = 450;
            break;
        case oox::XML_openDmnd:
            aHatch.Style = css::drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 100;
            aHatch.Angle = 450;
            break;
        case oox::XML_plaid:
            aHatch.Style = css::drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 200;
            aHatch.Angle = 900;
            break;
        case oox::XML_sphere:
            aHatch.Style = css::drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 100;
            aHatch.Angle = 0;
            break;
        case oox::XML_divot:
            aHatch.Style = css::drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 400;
            aHatch.Angle = 450;
            break;
        case oox::XML_shingle:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 200;
            aHatch.Angle = 1350;
            break;
        case oox::XML_trellis:
            aHatch.Style = css::drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 75;
            aHatch.Angle = 450;
            break;
        case oox::XML_zigZag:
            aHatch.Style = css::drawing::HatchStyle_SINGLE;
            aHatch.Distance = 75;
            aHatch.Angle = 0;
            break;
    }

    return aHatch;
}
