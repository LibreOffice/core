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
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------
#include <tools/shl.hxx>
#include <svx/dialogs.hrc>
#include <cuires.hrc>
#include "page.hrc"

#include "bbdlg.hxx"
#include "border.hxx"
#include "backgrnd.hxx"
#include <dialmgr.hxx>

//UUUU
#include "cuitabarea.hxx"

// class SvxBorderBackgroundDlg ------------------------------------------

SvxBorderBackgroundDlg::SvxBorderBackgroundDlg(Window *pParent,
    const SfxItemSet& rCoreSet,
    bool bEnableSelector,
    bool bEnableDrawingLayerFillStyles)
:   SfxTabDialog(
        pParent,
        CUI_RES(
            bEnableDrawingLayerFillStyles ? RID_SVXDLG_BBDLG_AREA_TRANS : RID_SVXDLG_BBDLG_BACKGROUND),
        &rCoreSet),
    mbEnableBackgroundSelector(bEnableSelector),
    mbEnableDrawingLayerFillStyles(bEnableDrawingLayerFillStyles)
{
    FreeResource();
    AddTabPage(RID_SVXPAGE_BORDER,SvxBorderTabPage::Create,0);

    if(mbEnableDrawingLayerFillStyles)
    {
        //UUUU Here we want full DrawingLayer FillStyle access, so add Area and Transparency TabPages
        AddTabPage( RID_SVXPAGE_AREA, SvxAreaTabPage::Create, 0 );
        AddTabPage( RID_SVXPAGE_TRANSPARENCE, SvxTransparenceTabPage::Create,  0);
    }
    else
    {
        //UUUU Use the more simple Background TabPage
        AddTabPage(RID_SVXPAGE_BACKGROUND, SvxBackgroundTabPage::Create, 0);
    }
}

// -----------------------------------------------------------------------

SvxBorderBackgroundDlg::~SvxBorderBackgroundDlg()
{
}

// -----------------------------------------------------------------------

void SvxBorderBackgroundDlg::PageCreated(sal_uInt16 nPageId, SfxTabPage& rTabPage)
{
    switch(nPageId)
    {
        case RID_SVXPAGE_BACKGROUND:
        {
            // allow switching between Color/graphic
            if(mbEnableBackgroundSelector)
            {
                static_cast< SvxBackgroundTabPage& >(rTabPage).ShowSelector();
            }
            break;
        }

        //UUUU inits for Area and Transparency TabPages
        // The selection attribute lists (XPropertyList derivates, e.g. XColorList for
        // the color table) need to be added as items (e.g. SvxColorTableItem) to make
        // these pages find the needed attributes for fill style suggestions.
        // These are added in SwDocStyleSheet::GetItemSet() for the SFX_STYLE_FAMILY_PARA on
        // demand, but could also be directly added from the DrawModel.
        case RID_SVXPAGE_AREA:
        {
            SfxItemSet aNew(
                *GetInputSetImpl()->GetPool(),
                SID_COLOR_TABLE, SID_BITMAP_LIST,
                SID_OFFER_IMPORT, SID_OFFER_IMPORT,
                0, 0);

            aNew.Put(*GetInputSetImpl());

            // add flag for direct graphic content selection
            aNew.Put(SfxBoolItem(SID_OFFER_IMPORT, true));

            rTabPage.PageCreated(aNew);
            break;
        }
        case RID_SVXPAGE_TRANSPARENCE:
        {
            rTabPage.PageCreated(*GetInputSetImpl());
            break;
        }
    }
}

//eof
