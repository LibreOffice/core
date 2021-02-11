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

#include <svx/svxids.hrc>
#include <svx/drawitem.hxx>
#include <svx/svdoutl.hxx>
#include <svx/xtable.hxx>
#include <unotools/configmgr.hxx>
#include <docsh.hxx>
#include <drawdoc.hxx>
#include <swtypes.hxx>

using namespace ::com::sun::star;

// Load Document
void InitDrawModelAndDocShell(SwDocShell* pSwDocShell, SwDrawModel* pSwDrawDocument)
{
    if(pSwDrawDocument)
    {
        if(pSwDocShell == pSwDrawDocument->GetObjectShell())
        {
            // association already done, nothing to do
        }
        else
        {
            // set object shell (mainly for FormControl stuff), maybe zero
            pSwDrawDocument->SetObjectShell(pSwDocShell);

            // set persist, maybe zero
            pSwDrawDocument->SetPersist(pSwDocShell);

            // get and decide on the color table to use
            if(pSwDocShell)
            {
                const SvxColorListItem* pColItemFromDocShell = pSwDocShell->GetItem(SID_COLOR_TABLE);

                if(pColItemFromDocShell)
                {
                    // the DocShell has a ColorTable, use it also in DrawingLayer
                    const XColorListRef& xCol(pColItemFromDocShell->GetColorList());
                    pSwDrawDocument->SetPropertyList(static_cast<XPropertyList*>(xCol.get()));
                }
                else
                {
                    // Use the ColorTable which is used at the DrawingLayer's SdrModel
                    XColorListRef xColorList = pSwDrawDocument->GetColorList();
                    if (xColorList.is())
                    {
                        pSwDocShell->PutItem(SvxColorListItem(xColorList, SID_COLOR_TABLE));
                    }
                    else if (!utl::ConfigManager::IsFuzzing())
                    {
                        // there wasn't one, get the standard and set to the
                        // docshell and then to the drawdocument
                        xColorList = XColorList::GetStdColorList();
                        pSwDocShell->PutItem(SvxColorListItem(xColorList, SID_COLOR_TABLE));
                        pSwDrawDocument->SetPropertyList(xColorList);
                    }
                }

                // add other tables in SfxItemSet of the DocShell
                pSwDocShell->PutItem(SvxGradientListItem(pSwDrawDocument->GetGradientList(), SID_GRADIENT_LIST));
                pSwDocShell->PutItem(SvxHatchListItem(pSwDrawDocument->GetHatchList(), SID_HATCH_LIST));
                pSwDocShell->PutItem(SvxBitmapListItem(pSwDrawDocument->GetBitmapList(), SID_BITMAP_LIST));
                pSwDocShell->PutItem(SvxPatternListItem(pSwDrawDocument->GetPatternList(), SID_PATTERN_LIST));
                pSwDocShell->PutItem(SvxDashListItem(pSwDrawDocument->GetDashList(), SID_DASH_LIST));
                pSwDocShell->PutItem(SvxLineEndListItem(pSwDrawDocument->GetLineEndList(), SID_LINEEND_LIST));
            }

            // init hyphenator for DrawingLayer outliner
            uno::Reference<linguistic2::XHyphenator> xHyphenator(::GetHyphenator());
            Outliner& rOutliner = pSwDrawDocument->GetDrawOutliner();

            rOutliner.SetHyphenator(xHyphenator);
        }
    }
    else if(pSwDocShell)
    {
        // fallback: add the default color list to have one when someone requests it from the DocShell
        pSwDocShell->PutItem(SvxColorListItem(XColorList::GetStdColorList(), SID_COLOR_TABLE));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
