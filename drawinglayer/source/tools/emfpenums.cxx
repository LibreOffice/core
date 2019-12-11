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
#include "emfpenums.hxx"

namespace emfplushelper
{
    OUString WrapModeToString(sal_uInt32 mode)
    {
        switch (mode)
        {
            case WrapModeTile: return "WrapModeTile";
            case WrapModeTileFlipX: return "WrapModeTileFlipX";
            case WrapModeTileFlipY: return "WrapModeTileFlipY";
            case WrapModeTileFlipXY: return "WrapModeTileFlipXY";
            case WrapModeClamp: return "WrapModeClamp";
        }
        return "";
    }

    OUString BrushDataFlagsToString(sal_uInt32 brush)
    {
        OUString sBrush;

        if (brush == BrushDataPath)
            sBrush = sBrush.concat("BrushDataPath");

        if (brush == BrushDataTransform)
        {
            if (sBrush.isEmpty())
                sBrush = sBrush.concat(", ");

            sBrush = sBrush.concat("BrushDataTransform");
        }

        if (brush == BrushDataPresetColors)
        {
            if (sBrush.isEmpty())
                sBrush = sBrush.concat(", ");

            sBrush = sBrush.concat("BrushDataPresetColors");
        }

        if (brush == BrushDataBlendFactorsH)
        {
            if (sBrush.isEmpty())
                sBrush = sBrush.concat(", ");

            sBrush = sBrush.concat("BrushDataBlendFactorsH");
        }

        if (brush == BrushDataBlendFactorsV)
        {
            if (sBrush.isEmpty())
                sBrush = sBrush.concat(", ");

            sBrush = sBrush.concat("BrushDataBlendFactorsV");
        }

        if (brush == BrushDataFocusScales)
        {
            if (sBrush.isEmpty())
                sBrush = sBrush.concat(", ");

            sBrush = sBrush.concat("BrushDataFocusScales");
        }

        if (brush == BrushDataGammaCorrected)
        {
            if (sBrush.isEmpty())
                sBrush = sBrush.concat(", ");

            sBrush = sBrush.concat("BrushDataGammaCorrected");
        }

        if (brush == BrushDataDoNotTransform)
        {
            if (sBrush.isEmpty())
                sBrush = sBrush.concat(", ");

            sBrush = sBrush.concat("BrushDataDoNotTransform");
        }

        return sBrush;
    }

    OUString ImageDataTypeToString(sal_uInt32 type)
    {
        switch (type)
        {
            case ImageDataTypeUnknown: return "ImageDataTypeUnknown";
            case ImageDataTypeBitmap: return "ImageDataTypeBitmap";
            case ImageDataTypeMetafile: return "ImageDataTypeMetafile";
        }

        return "";
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
