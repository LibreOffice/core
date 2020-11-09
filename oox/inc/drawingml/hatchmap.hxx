/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/drawing/Hatch.hpp>
#include <oox/token/tokens.hxx>

#include <tools/color.hxx>

using namespace com::sun::star;
using namespace oox;

static drawing::Hatch createHatch(sal_Int32 nHatchToken, ::Color nColor)
{
    drawing::Hatch aHatch;
    aHatch.Color = sal_Int32(nColor);
    // best-effort mapping; we do not support all the styles in core
    switch (nHatchToken)
    {
        case XML_pct5:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 250;
            aHatch.Angle = 450;
            break;
        case XML_pct10:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 200;
            aHatch.Angle = 450;
            break;
        case XML_pct20:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 150;
            aHatch.Angle = 450;
            break;
        case XML_pct25:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 200;
            aHatch.Angle = 450;
            break;
        case XML_pct30:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 175;
            aHatch.Angle = 450;
            break;
        case XML_pct40:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 150;
            aHatch.Angle = 450;
            break;
        case XML_pct50:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 125;
            aHatch.Angle = 450;
            break;
        case XML_pct60:
            aHatch.Style = drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 150;
            aHatch.Angle = 450;
            break;
        case XML_pct70:
            aHatch.Style = drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 125;
            aHatch.Angle = 450;
            break;
        case XML_pct75:
            aHatch.Style = drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 100;
            aHatch.Angle = 450;
            break;
        case XML_pct80:
            aHatch.Style = drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 75;
            aHatch.Angle = 450;
            break;
        case XML_pct90:
            aHatch.Style = drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 50;
            aHatch.Angle = 450;
            break;
        case XML_horz:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 100;
            aHatch.Angle = 0;
            break;
        case XML_vert:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 100;
            aHatch.Angle = 900;
            break;
        case XML_ltHorz:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 50;
            aHatch.Angle = 0;
            break;
        case XML_ltVert:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 50;
            aHatch.Angle = 900;
            break;
        case XML_dkHorz:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 25;
            aHatch.Angle = 0;
            break;
        case XML_dkVert:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 25;
            aHatch.Angle = 900;
            break;
        case XML_narHorz:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 50;
            aHatch.Angle = 0;
            break;
        case XML_narVert:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 50;
            aHatch.Angle = 900;
            break;
        case XML_dashHorz:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 150;
            aHatch.Angle = 0;
            break;
        case XML_dashVert:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 150;
            aHatch.Angle = 900;
            break;
        case XML_cross:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 100;
            aHatch.Angle = 0;
            break;
        case XML_dnDiag:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 100;
            aHatch.Angle = 1350;
            break;
        case XML_upDiag:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 100;
            aHatch.Angle = 450;
            break;
        case XML_ltDnDiag:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 50;
            aHatch.Angle = 1350;
            break;
        case XML_ltUpDiag:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 50;
            aHatch.Angle = 450;
            break;
        case XML_dkDnDiag:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 50;
            aHatch.Angle = 1350;
            break;
        case XML_dkUpDiag:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 50;
            aHatch.Angle = 450;
            break;
        case XML_wdDnDiag:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 100;
            aHatch.Angle = 1350;
            break;
        case XML_wdUpDiag:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 100;
            aHatch.Angle = 450;
            break;
        case XML_dashDnDiag:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 150;
            aHatch.Angle = 1350;
            break;
        case XML_dashUpDiag:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 150;
            aHatch.Angle = 450;
            break;
        case XML_diagCross:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 100;
            aHatch.Angle = 450;
            break;
        case XML_smCheck:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 50;
            aHatch.Angle = 450;
            break;
        case XML_lgCheck:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 100;
            aHatch.Angle = 450;
            break;
        case XML_smGrid:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 50;
            aHatch.Angle = 0;
            break;
        case XML_lgGrid:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 100;
            aHatch.Angle = 0;
            break;
        case XML_dotGrid:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 400;
            aHatch.Angle = 0;
            break;
        case XML_smConfetti:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 200;
            aHatch.Angle = 600;
            break;
        case XML_lgConfetti:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 100;
            aHatch.Angle = 600;
            break;
        case XML_horzBrick:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 300;
            aHatch.Angle = 0;
            break;
        case XML_diagBrick:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 300;
            aHatch.Angle = 450;
            break;
        case XML_solidDmnd:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 100;
            aHatch.Angle = 450;
            break;
        case XML_openDmnd:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 100;
            aHatch.Angle = 450;
            break;
        case XML_dotDmnd:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 300;
            aHatch.Angle = 450;
            break;
        case XML_plaid:
            aHatch.Style = drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 200;
            aHatch.Angle = 900;
            break;
        case XML_sphere:
            aHatch.Style = drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 100;
            aHatch.Angle = 0;
            break;
        case XML_weave:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 150;
            aHatch.Angle = 450;
            break;
        case XML_divot:
            aHatch.Style = drawing::HatchStyle_TRIPLE;
            aHatch.Distance = 400;
            aHatch.Angle = 450;
            break;
        case XML_shingle:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 200;
            aHatch.Angle = 1350;
            break;
        case XML_wave:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 100;
            aHatch.Angle = 0;
            break;
        case XML_trellis:
            aHatch.Style = drawing::HatchStyle_DOUBLE;
            aHatch.Distance = 75;
            aHatch.Angle = 450;
            break;
        case XML_zigZag:
            aHatch.Style = drawing::HatchStyle_SINGLE;
            aHatch.Distance = 75;
            aHatch.Angle = 0;
            break;
    }

    return aHatch;
}
