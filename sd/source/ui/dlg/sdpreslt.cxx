/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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


SdPresLayoutDlg::SdPresLayoutDlg(::sd::DrawDocShell* pDocShell,
    ::Window* pWindow, const SfxItemSet& rInAttrs)
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
}

/**
 *    Initialize
 */
void SdPresLayoutDlg::Reset()
{
    const SfxPoolItem *pPoolItem = NULL;
    long nName;

    
    if( mrOutAttrs.GetItemState( ATTR_PRESLAYOUT_MASTER_PAGE, false, &pPoolItem ) == SFX_ITEM_SET )
    {
        sal_Bool bMasterPage = ( (const SfxBoolItem*) pPoolItem)->GetValue();
        m_pCbxMasterPage->Enable( !bMasterPage );
        m_pCbxMasterPage->Check( bMasterPage );
    }

    
    m_pCbxCheckMasters->Check(false);

    if(mrOutAttrs.GetItemState(ATTR_PRESLAYOUT_NAME, true, &pPoolItem) == SFX_ITEM_SET)
        maName = ((const SfxStringItem*)pPoolItem)->GetValue();
    else
        maName = "";

    FillValueSet();

    mnLayoutCount = maLayoutNames.size();
    for( nName = 0; nName < mnLayoutCount; nName++ )
    {
        if (maLayoutNames[nName] == maName)
            break;
    }
    DBG_ASSERT(nName < mnLayoutCount, "Layout not found");

    m_pVS->SelectItem((sal_uInt16)nName + 1);  

}

/**
 * Fills the provided Item-Set with dialog box attributes
 */
void SdPresLayoutDlg::GetAttr(SfxItemSet& rOutAttrs)
{
    short nId = m_pVS->GetSelectItemId();
    sal_Bool bLoad = nId > mnLayoutCount;
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
            aLayoutName = ""; 
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
        SdPage* pMaster = (SdPage*)pDoc->GetMasterPage(nLayout);
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
IMPL_LINK_NOARG(SdPresLayoutDlg, ClickLayoutHdl)
{
    EndDialog(RET_OK);
    return 0;
}

/**
 * Click handler for load button
 */
IMPL_LINK_NOARG(SdPresLayoutDlg, ClickLoadHdl)
{
    SfxNewFileDialog* pDlg = new SfxNewFileDialog(this, SFXWB_PREVIEW);
    pDlg->SetText(SD_RESSTR(STR_LOAD_PRESENTATION_LAYOUT));

    if(!IsReallyVisible())
    {
        delete pDlg;
        return 0;
    }

    sal_uInt16 nResult = pDlg->Execute();
    
    Update();

    sal_Bool   bCancel = sal_False;

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
                
                maName = "";
            }
        }
        break;

        default:
            bCancel = sal_True;
    }
    delete pDlg;

    if( !bCancel )
    {
        
        sal_Bool bExists = sal_False;
        OUString aCompareStr(maName);
        if (aCompareStr.isEmpty())
            aCompareStr = maStrNone;

        sal_uInt16 aPos = 0;
        for (std::vector<OUString>::iterator it = maLayoutNames.begin();
	          it != maLayoutNames.end() && !bExists; ++it, ++aPos)
        {
            if( aCompareStr == *it )
            {
                bExists = sal_True;
                
                m_pVS->SelectItem( aPos + 1 );
            }
        }

        if( !bExists )
        {
            
            if (!maName.isEmpty())
            {
                
                SdDrawDocument* pDoc = mpDocSh->GetDoc();
                SdDrawDocument* pTemplDoc  = pDoc->OpenBookmarkDoc( maName );

                if (pTemplDoc)
                {
                    ::sd::DrawDocShell*  pTemplDocSh= pTemplDoc->GetDocSh();

                    sal_uInt16 nCount = pTemplDoc->GetMasterPageCount();

                    for (sal_uInt16 nLayout = 0; nLayout < nCount; nLayout++)
                    {
                        SdPage* pMaster = (SdPage*) pTemplDoc->GetMasterPage(nLayout);
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
                    bCancel = sal_True;
                }

                pDoc->CloseBookmarkDoc();
            }
            else
            {
                
                maLayoutNames.push_back(maStrNone);
                m_pVS->InsertItem( (sal_uInt16) maLayoutNames.size(),
                        Image(Bitmap(SdResId(BMP_FOIL_NONE))), maStrNone );
            }

            if (!bCancel)
            {
                
                m_pVS->SelectItem( (sal_uInt16) maLayoutNames.size() );
            }
        }
    }

    return( 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
