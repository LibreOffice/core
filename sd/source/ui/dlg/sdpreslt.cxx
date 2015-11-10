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

#include <svl/itemset.hxx>
#include <sfx2/new.hxx>
#include <vcl/msgbox.hxx>

#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdpreslt.hxx"
#include "sdattr.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "DrawDocShell.hxx"
#include <memory>

SdPresLayoutDlg::SdPresLayoutDlg(::sd::DrawDocShell* pDocShell,
    vcl::Window* pWindow, const SfxItemSet& rInAttrs)
    : ModalDialog(pWindow, "SlideDesignDialog",
        "modules/simpress/ui/slidedesigndialog.ui")
    , mpDocSh(pDocShell)
    , mrOutAttrs(rInAttrs)
    , maStrNone(SD_RESSTR(STR_NULL))
{
    get(m_pVS, "select");
    Size aPref(LogicToPixel(Size(144 , 141), MAP_APPFONT));
    m_pVS->set_width_request(aPref.Width());
    m_pVS->set_height_request(aPref.Height());
    get(m_pCbxMasterPage, "masterpage");
    get(m_pCbxCheckMasters, "checkmasters");
    get(m_pBtnLoad, "load");

    m_pVS->SetDoubleClickHdl(LINK(this, SdPresLayoutDlg, ClickLayoutHdl));
    m_pBtnLoad->SetClickHdl(LINK(this, SdPresLayoutDlg, ClickLoadHdl));

    Reset();
}

SdPresLayoutDlg::~SdPresLayoutDlg()
{
    disposeOnce();
}

void SdPresLayoutDlg::dispose()
{
    m_pVS.clear();
    m_pCbxMasterPage.clear();
    m_pCbxCheckMasters.clear();
    m_pBtnLoad.clear();
    ModalDialog::dispose();
}

/**
 *    Initialize
 */
void SdPresLayoutDlg::Reset()
{
    const SfxPoolItem *pPoolItem = nullptr;
    long nName;

    // replace master page
    if( mrOutAttrs.GetItemState( ATTR_PRESLAYOUT_MASTER_PAGE, false, &pPoolItem ) == SfxItemState::SET )
    {
        bool bMasterPage = static_cast<const SfxBoolItem*>(pPoolItem)->GetValue();
        m_pCbxMasterPage->Enable( !bMasterPage );
        m_pCbxMasterPage->Check( bMasterPage );
    }

    // remove not used master pages
    m_pCbxCheckMasters->Check(false);

    if(mrOutAttrs.GetItemState(ATTR_PRESLAYOUT_NAME, true, &pPoolItem) == SfxItemState::SET)
        maName = static_cast<const SfxStringItem*>(pPoolItem)->GetValue();
    else
        maName.clear();

    FillValueSet();

    mnLayoutCount = maLayoutNames.size();
    for( nName = 0; nName < mnLayoutCount; nName++ )
    {
        if (maLayoutNames[nName] == maName)
            break;
    }
    DBG_ASSERT(nName < mnLayoutCount, "Layout not found");

    m_pVS->SelectItem((sal_uInt16)nName + 1);  // Indices of the ValueSets start at 1

}

/**
 * Fills the provided Item-Set with dialog box attributes
 */
void SdPresLayoutDlg::GetAttr(SfxItemSet& rOutAttrs)
{
    short nId = m_pVS->GetSelectItemId();
    bool bLoad = nId > mnLayoutCount;
    rOutAttrs.Put( SfxBoolItem( ATTR_PRESLAYOUT_LOAD, bLoad ) );

    OUString aLayoutName;

    if( bLoad )
    {
        aLayoutName = maName + "#" + maLayoutNames[ nId - 1 ];
    }
    else
    {
        aLayoutName = maLayoutNames[ nId - 1 ];
        if( aLayoutName == maStrNone )
            aLayoutName.clear(); // that way we encode "- nothing -" (see below)
    }

    rOutAttrs.Put( SfxStringItem( ATTR_PRESLAYOUT_NAME, aLayoutName ) );
    rOutAttrs.Put( SfxBoolItem( ATTR_PRESLAYOUT_MASTER_PAGE, m_pCbxMasterPage->IsChecked() ) );
    rOutAttrs.Put( SfxBoolItem( ATTR_PRESLAYOUT_CHECK_MASTERS, m_pCbxCheckMasters->IsChecked() ) );
}

/**
 * Fills ValueSet with bitmaps
 */
void SdPresLayoutDlg::FillValueSet()
{
    m_pVS->SetStyle(m_pVS->GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER
                                      | WB_VSCROLL | WB_NAMEFIELD);

    m_pVS->SetColCount(2);
    m_pVS->SetLineCount(2);
    m_pVS->SetExtraSpacing(2);

    SdDrawDocument* pDoc = mpDocSh->GetDoc();

    sal_uInt16 nCount = pDoc->GetMasterPageCount();

    for (sal_uInt16 nLayout = 0; nLayout < nCount; nLayout++)
    {
        SdPage* pMaster = static_cast<SdPage*>(pDoc->GetMasterPage(nLayout));
        if (pMaster->GetPageKind() == PK_STANDARD)
        {
            OUString aLayoutName(pMaster->GetLayoutName());
            aLayoutName = aLayoutName.copy(0, aLayoutName.indexOf(SD_LT_SEPARATOR));
            maLayoutNames.push_back(aLayoutName);

            Image aBitmap(Bitmap(mpDocSh->GetPagePreviewBitmap(pMaster, 90)));
            m_pVS->InsertItem((sal_uInt16)maLayoutNames.size(), aBitmap, aLayoutName);
        }
    }

    m_pVS->Show();
}

/**
 * DoubleClick handler
 */
IMPL_LINK_NOARG_TYPED(SdPresLayoutDlg, ClickLayoutHdl, ValueSet*, void)
{
    EndDialog(RET_OK);
}

/**
 * Click handler for load button
 */
IMPL_LINK_NOARG_TYPED(SdPresLayoutDlg, ClickLoadHdl, Button*, void)
{
    VclPtrInstance< SfxNewFileDialog > pDlg(this, SFXWB_PREVIEW);
    pDlg->SetText(SD_RESSTR(STR_LOAD_PRESENTATION_LAYOUT));

    if(!IsReallyVisible())
        return;

    sal_uInt16 nResult = pDlg->Execute();
    // Inserted update to force repaint
    Update();

    bool   bCancel = false;

    switch (nResult)
    {
        case RET_OK:
        {
            if (pDlg->IsTemplate())
            {
                maName = pDlg->GetTemplateFileName();
            }
            else
            {
                // that way we encode "- nothing -"
                maName.clear();
            }
        }
        break;

        default:
            bCancel = true;
    }
    pDlg.reset();

    if( !bCancel )
    {
        // check if template already exists
        bool bExists = false;
        OUString aCompareStr(maName);
        if (aCompareStr.isEmpty())
            aCompareStr = maStrNone;

        sal_uInt16 aPos = 0;
        for (std::vector<OUString>::iterator it = maLayoutNames.begin();
	          it != maLayoutNames.end() && !bExists; ++it, ++aPos)
        {
            if( aCompareStr == *it )
            {
                bExists = true;
                // select template
                m_pVS->SelectItem( aPos + 1 );
            }
        }

        if( !bExists )
        {
            // load document in order to determine preview bitmap (if template is selected)
            if (!maName.isEmpty())
            {
                // determine document in order to call OpenBookmarkDoc
                SdDrawDocument* pDoc = mpDocSh->GetDoc();
                SdDrawDocument* pTemplDoc  = pDoc->OpenBookmarkDoc( maName );

                if (pTemplDoc)
                {
                    ::sd::DrawDocShell*  pTemplDocSh= pTemplDoc->GetDocSh();

                    sal_uInt16 nCount = pTemplDoc->GetMasterPageCount();

                    for (sal_uInt16 nLayout = 0; nLayout < nCount; nLayout++)
                    {
                        SdPage* pMaster = static_cast<SdPage*>( pTemplDoc->GetMasterPage(nLayout) );
                        if (pMaster->GetPageKind() == PK_STANDARD)
                        {
                            OUString aLayoutName(pMaster->GetLayoutName());
                            aLayoutName = aLayoutName.copy(0, aLayoutName.indexOf(SD_LT_SEPARATOR));
                            maLayoutNames.push_back(aLayoutName);

                            Image aBitmap(Bitmap(pTemplDocSh->GetPagePreviewBitmap(pMaster, 90)));
                            m_pVS->InsertItem((sal_uInt16)maLayoutNames.size(), aBitmap, aLayoutName);
                        }
                    }
                }
                else
                {
                    bCancel = true;
                }

                pDoc->CloseBookmarkDoc();
            }
            else
            {
                // empty layout
                maLayoutNames.push_back(maStrNone);
                m_pVS->InsertItem( (sal_uInt16) maLayoutNames.size(),
                        Image(Bitmap(SdResId(BMP_FOIL_NONE))), maStrNone );
            }

            if (!bCancel)
            {
                // select template
                m_pVS->SelectItem( (sal_uInt16) maLayoutNames.size() );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
