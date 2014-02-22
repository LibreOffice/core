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
#include <svl/smplhint.hxx>

#include "prevwsh.hxx"
#include "docsh.hxx"
#include "preview.hxx"
#include "hints.hxx"
#include "sc.hrc"

// STATIC DATA -----------------------------------------------------------



void ScPreviewShell::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    sal_Bool bDataChanged = false;

    if (rHint.ISA(SfxSimpleHint))
    {
        sal_uLong nSlot = ((const SfxSimpleHint&)rHint).GetId();
        switch ( nSlot )
        {
            case FID_DATACHANGED:
            case SID_SCPRINTOPTIONS:
                bDataChanged = sal_True;
                break;
            case SC_HINT_DRWLAYER_NEW:
                {
                    SfxBroadcaster* pDrawBC = pDocShell->GetDocument()->GetDrawBroadcaster();
                    if (pDrawBC)
                        StartListening(*pDrawBC);
                }
                break;
        }
    }
    else if (rHint.ISA(ScPaintHint))
    {
        if ( ((const ScPaintHint&)rHint).GetPrintFlag() )
        {
            sal_uInt16 nParts = ((const ScPaintHint&)rHint).GetParts();
            if (nParts & ( PAINT_GRID | PAINT_LEFT | PAINT_TOP | PAINT_SIZE ))
                bDataChanged = sal_True;
        }
    }
    else if (rHint.ISA(SdrHint))
    {
        // SdrHints are no longer used for invalidating, thus react on objectchange instead
        if(HINT_OBJCHG == ((const SdrHint&)rHint).GetKind())
            bDataChanged = sal_True;
    }

    if (bDataChanged)
        pPreview->DataChanged(true);
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
