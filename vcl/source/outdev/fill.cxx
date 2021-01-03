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

#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>

#include <drawmode.hxx>

void OutputDevice::SetFillColor(const Color& rColor)
{
    Color aColor(rColor);
    aColor = GetDrawModeFillColor(aColor, GetDrawMode(), GetSettings().GetStyleSettings());

    if (mpMetaFile)
    {
        if (aColor.IsTransparent())
            mpMetaFile->AddAction(new MetaFillColorAction(Color(), false));
        else
            mpMetaFile->AddAction(new MetaFillColorAction(aColor, true));
    }

    RenderContext2::SetFillColor(rColor);

    if (mpAlphaVDev)
    {
        if (aColor.IsTransparent())
            mpAlphaVDev->SetFillColor();
        else
            mpAlphaVDev->SetFillColor(COL_BLACK);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
