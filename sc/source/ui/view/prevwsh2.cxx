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

#include <svx/svdmodel.hxx>
#include <svl/hint.hxx>

#include <prevwsh.hxx>
#include <docsh.hxx>
#include <preview.hxx>
#include <hints.hxx>

void ScPreviewShell::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    bool bDataChanged = false;

    if (rHint.GetId() == SfxHintId::ThisIsAnSdrHint)
    {
        const SdrHint* pSdrHint = static_cast<const SdrHint*>(&rHint);
        // SdrHints are no longer used for invalidating, thus react on objectchange instead
        if(SdrHintKind::ObjectChange == pSdrHint->GetKind())
            bDataChanged = true;
    }
    else if (const ScPaintHint* pPaintHint = dynamic_cast<const ScPaintHint*>(&rHint))
    {
        PaintPartFlags nParts = pPaintHint->GetParts();
        if (nParts & ( PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top | PaintPartFlags::Size ))
            bDataChanged = true;
    }
    else
    {
        switch ( rHint.GetId() )
        {
            case SfxHintId::ScDataChanged:
            case SfxHintId::ScPrintOptions:
                bDataChanged = true;
                break;
            case SfxHintId::ScDrawLayerNew:
                {
                    SfxBroadcaster* pDrawBC = pDocShell->GetDocument().GetDrawBroadcaster();
                    if (pDrawBC)
                        StartListening(*pDrawBC);
                }
                break;
            default: break;
        }
    }

    if (bDataChanged)
        pPreview->DataChanged(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
