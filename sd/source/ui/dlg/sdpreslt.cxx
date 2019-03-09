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
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/new.hxx>
#include <svtools/valueset.hxx>
#include <tools/debug.hxx>

#include <strings.hrc>

#include <bitmaps.hlst>
#include <sdpreslt.hxx>
#include <sdattr.hrc>
#include <sdresid.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <DrawDocShell.hxx>
#include <memory>

SdPresLayoutDlg::SdPresLayoutDlg(::sd::DrawDocShell* pDocShell,
    weld::Window* pWindow, const SfxItemSet& rInAttrs)
    : GenericDialogController(pWindow, "modules/simpress/ui/slidedesigndialog.ui", "SlideDesignDialog")
    , mpDocSh(pDocShell)
    , mrOutAttrs(rInAttrs)
    , maStrNone(SdResId(STR_NULL))
    , m_xCbxMasterPage(m_xBuilder->weld_check_button("masterpage"))
    , m_xCbxCheckMasters(m_xBuilder->weld_check_button("checkmasters"))
    , m_xBtnLoad(m_xBuilder->weld_button("load"))
    , m_xVS(new SvtValueSet(m_xBuilder->weld_scrolled_window("selectwin")))
    , m_xVSWin(new weld::CustomWeld(*m_xBuilder, "select", *m_xVS))
{
    m_xVSWin->set_size_request(m_xBtnLoad->get_approximate_digit_width() * 60,
                               m_xBtnLoad->get_text_height() * 20);

    m_xVS->SetDoubleClickHdl(LINK(this, SdPresLayoutDlg, ClickLayoutHdl));
    m_xBtnLoad->connect_clicked(LINK(this, SdPresLayoutDlg, ClickLoadHdl));

    Reset();
}

SdPresLayoutDlg::~SdPresLayoutDlg()
{
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
        m_xCbxMasterPage->set_sensitive( !bMasterPage );
        m_xCbxMasterPage->set_active( bMasterPage );
    }

    // remove not used master pages
    m_xCbxCheckMasters->set_active(false);

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

    m_xVS->SelectItem(static_cast<sal_uInt16>(nName) + 1);  // Indices of the ValueSets start at 1

}

/**
 * Fills the provided Item-Set with dialog box attributes
 */
void SdPresLayoutDlg::GetAttr(SfxItemSet& rOutAttrs)
{
    short nId = m_xVS->GetSelectedItemId();
    bool bLoad = nId > mnLayoutCount;
    rOutAttrs.Put( SfxBoolItem( ATTR_PRESLAYOUT_LOAD, bLoad ) );

    OUString aLayoutName;

    if( bLoad )
    {
        aLayoutName = maName + "#" + maLayoutNames[ nId - 1 ];
    }
    else if (nId)
    {
        aLayoutName = maLayoutNames[ nId - 1 ];
        if( aLayoutName == maStrNone )
            aLayoutName.clear(); // that way we encode "- nothing -" (see below)
    }

    rOutAttrs.Put( SfxStringItem( ATTR_PRESLAYOUT_NAME, aLayoutName ) );
    rOutAttrs.Put( SfxBoolItem( ATTR_PRESLAYOUT_MASTER_PAGE, m_xCbxMasterPage->get_active() ) );
    rOutAttrs.Put( SfxBoolItem( ATTR_PRESLAYOUT_CHECK_MASTERS, m_xCbxCheckMasters->get_active() ) );
}

/**
 * Fills ValueSet with bitmaps
 */
void SdPresLayoutDlg::FillValueSet()
{
    m_xVS->SetStyle(m_xVS->GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER
                                      | WB_VSCROLL | WB_NAMEFIELD);

    m_xVS->SetColCount(2);
    m_xVS->SetLineCount(2);
    m_xVS->SetExtraSpacing(2);

    SdDrawDocument* pDoc = mpDocSh->GetDoc();

    sal_uInt16 nCount = pDoc->GetMasterPageCount();

    for (sal_uInt16 nLayout = 0; nLayout < nCount; nLayout++)
    {
        SdPage* pMaster = static_cast<SdPage*>(pDoc->GetMasterPage(nLayout));
        if (pMaster->GetPageKind() == PageKind::Standard)
        {
            OUString aLayoutName(pMaster->GetLayoutName());
            aLayoutName = aLayoutName.copy(0, aLayoutName.indexOf(SD_LT_SEPARATOR));
            maLayoutNames.push_back(aLayoutName);

            Image aBitmap(mpDocSh->GetPagePreviewBitmap(pMaster));
            m_xVS->InsertItem(static_cast<sal_uInt16>(maLayoutNames.size()), aBitmap, aLayoutName);
        }
    }

    m_xVS->Show();
}

/**
 * DoubleClick handler
 */
IMPL_LINK_NOARG(SdPresLayoutDlg, ClickLayoutHdl, SvtValueSet*, void)
{
    m_xDialog->response(RET_OK);
}

/**
 * Click handler for load button
 */
IMPL_LINK_NOARG(SdPresLayoutDlg, ClickLoadHdl, weld::Button&, void)
{
    SfxNewFileDialog aDlg(m_xDialog.get(), SfxNewFileDialogMode::Preview);
    aDlg.set_title(SdResId(STR_LOAD_PRESENTATION_LAYOUT));
    sal_uInt16 nResult = aDlg.run();

    bool   bCancel = false;

    switch (nResult)
    {
        case RET_OK:
        {
            if (aDlg.IsTemplate())
            {
                maName = aDlg.GetTemplateFileName();
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

    if( bCancel )
        return;

    // check if template already exists
    OUString aCompareStr(maName);
    if (aCompareStr.isEmpty())
        aCompareStr = maStrNone;

    auto it = std::find(maLayoutNames.begin(), maLayoutNames.end(), aCompareStr);
    if (it != maLayoutNames.end())
    {
        sal_uInt16 aPos = static_cast<sal_uInt16>(std::distance(maLayoutNames.begin(), it));
        // select template
        m_xVS->SelectItem( aPos + 1 );
    }
    else
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
                    if (pMaster->GetPageKind() == PageKind::Standard)
                    {
                        OUString aLayoutName(pMaster->GetLayoutName());
                        aLayoutName = aLayoutName.copy(0, aLayoutName.indexOf(SD_LT_SEPARATOR));
                        maLayoutNames.push_back(aLayoutName);

                        Image aBitmap(pTemplDocSh->GetPagePreviewBitmap(pMaster));
                        m_xVS->InsertItem(static_cast<sal_uInt16>(maLayoutNames.size()), aBitmap, aLayoutName);
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
            m_xVS->InsertItem( static_cast<sal_uInt16>(maLayoutNames.size()),
                    Image(BMP_FOIL_NONE), maStrNone );
        }

        if (!bCancel)
        {
            // select template
            m_xVS->SelectItem( static_cast<sal_uInt16>(maLayoutNames.size()) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
