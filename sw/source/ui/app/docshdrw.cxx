/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <hintids.hxx>
#include <svx/svxids.hrc>
#include <svl/stritem.hxx>
#include <svx/drawitem.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoutl.hxx>
#include <svx/xtable.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <svx/fmmodel.hxx>
#include <drawdoc.hxx>

using namespace ::com::sun::star;

/*--------------------------------------------------------------------
    Beschreibung: Document laden
 --------------------------------------------------------------------*/

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
                const SvxColorTableItem* pColItemFromDocShell = static_cast< const SvxColorTableItem* >(pSwDocShell->GetItem(SID_COLOR_TABLE));

                if(pColItemFromDocShell)
                {
                    // the DocShell has a ColorTable, use it also in DrawingLayer
                    pSwDrawDocument->SetColorTableAtSdrModel(pColItemFromDocShell->GetColorTable());
                }
                else
                {
                    // Use the ColorTable which is used at the DrawingLayer's SdrModel
                    pSwDocShell->PutItem(SvxColorTableItem(pSwDrawDocument->GetColorTableFromSdrModel(), SID_COLOR_TABLE));
                }

                // add other tables in SfxItemSet of the DocShell
                pSwDocShell->PutItem(SvxGradientListItem(pSwDrawDocument->GetGradientListFromSdrModel(), SID_GRADIENT_LIST));
                pSwDocShell->PutItem(SvxHatchListItem(pSwDrawDocument->GetHatchListFromSdrModel(), SID_HATCH_LIST));
                pSwDocShell->PutItem(SvxBitmapListItem(pSwDrawDocument->GetBitmapListFromSdrModel(), SID_BITMAP_LIST));
                pSwDocShell->PutItem(SvxDashListItem(pSwDrawDocument->GetDashListFromSdrModel(), SID_DASH_LIST));
                pSwDocShell->PutItem(SvxLineEndListItem(pSwDrawDocument->GetLineEndListFromSdrModel(), SID_LINEEND_LIST));
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
        pSwDocShell->PutItem(SvxColorTableItem(XColorList::GetStdColorList(), SID_COLOR_TABLE));
    }
}

//eof
