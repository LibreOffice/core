/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif


#include <svl/itemset.hxx>
#include <sfx2/new.hxx>
#include <vcl/msgbox.hxx>

#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdpreslt.hxx"
#include "sdpreslt.hrc"
#include "sdattr.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "DrawDocShell.hxx"
#include "ViewShell.hxx"

#define DOCUMENT_TOKEN (sal_Unicode('#'))

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

SdPresLayoutDlg::SdPresLayoutDlg(
    ::sd::DrawDocShell* pDocShell,
    ::sd::ViewShell* pViewShell,
    ::Window* pWindow,
    const SfxItemSet& rInAttrs ):
                ModalDialog         (pWindow, SdResId(DLG_PRESLT)),
                mpDocSh              ( pDocShell ),
                mpViewSh             ( pViewShell ),
                maFtLayout           (this, SdResId(FT_LAYOUT)),
                maVS                 (this, SdResId(VS_LAYOUT)),
                maBtnOK              (this, SdResId(BTN_OK)),
                maBtnCancel          (this, SdResId(BTN_CANCEL)),
                maBtnHelp            (this, SdResId(BTN_HELP)),
                maCbxMasterPage     (this, SdResId(CBX_MASTER_PAGE)),
                maCbxCheckMasters   (this, SdResId(CBX_CHECK_MASTERS)),
                maBtnLoad            (this, SdResId(BTN_LOAD)),
                mrOutAttrs           (rInAttrs),
                maStrNone           ( SdResId( STR_NULL ) )
{
    FreeResource();

    mpLayoutNames = new List;

    maVS.SetDoubleClickHdl(LINK(this, SdPresLayoutDlg, ClickLayoutHdl));
    maBtnLoad.SetClickHdl(LINK(this, SdPresLayoutDlg, ClickLoadHdl));

    Reset();
}

/*************************************************************************
|*
|*  Dtor
|*
*************************************************************************/

SdPresLayoutDlg::~SdPresLayoutDlg()
{
    String* pName = (String*)mpLayoutNames->First();
    while (pName)
    {
        delete pName;
        pName = (String*)mpLayoutNames->Next();
    }

    delete mpLayoutNames;
}

/*************************************************************************
|*
|*    Initialisierung
|*
*************************************************************************/

void SdPresLayoutDlg::Reset()
{
    const SfxPoolItem *pPoolItem = NULL;
    long nName;

    // MasterPage austauschen
    if( mrOutAttrs.GetItemState( ATTR_PRESLAYOUT_MASTER_PAGE, FALSE, &pPoolItem ) == SFX_ITEM_SET )
    {
        BOOL bMasterPage = ( (const SfxBoolItem*) pPoolItem)->GetValue();
        maCbxMasterPage.Enable( !bMasterPage );
        maCbxMasterPage.Check( bMasterPage );
    }

    // Nicht verwendete MasterPages entfernen
    maCbxCheckMasters.Check(FALSE);

    if(mrOutAttrs.GetItemState(ATTR_PRESLAYOUT_NAME, TRUE, &pPoolItem) == SFX_ITEM_SET)
        maName = ((const SfxStringItem*)pPoolItem)->GetValue();
    else
        maName.Erase();

    FillValueSet();

    mnLayoutCount = mpLayoutNames->Count();
    for( nName = 0; nName < mnLayoutCount; nName++ )
    {
        if (*((String*)mpLayoutNames->GetObject(nName)) == maName)
            break;
    }
    DBG_ASSERT(nName < mnLayoutCount, "Layout nicht gefunden");

    maVS.SelectItem((USHORT)nName + 1);  // Inizes des ValueSets beginnen bei 1

}

/*************************************************************************
|*
|*    Fuellt uebergebenen Item-Set mit Dialogbox-Attributen
|*
*************************************************************************/

void SdPresLayoutDlg::GetAttr(SfxItemSet& rOutAttrs)
{
    short nId = maVS.GetSelectItemId();
    BOOL bLoad = nId > mnLayoutCount;
    rOutAttrs.Put( SfxBoolItem( ATTR_PRESLAYOUT_LOAD, bLoad ) );

    String aLayoutName;

    if( bLoad )
    {
        aLayoutName = maName;
        aLayoutName.Append( DOCUMENT_TOKEN );
        aLayoutName.Append( *(String*)mpLayoutNames->GetObject( nId - 1 ) );
    }
    else
    {
        aLayoutName = *(String*)mpLayoutNames->GetObject( nId - 1 );
        if( aLayoutName == maStrNone )
            aLayoutName.Erase(); //  so wird "- keine -" codiert (s.u.)
    }

    rOutAttrs.Put( SfxStringItem( ATTR_PRESLAYOUT_NAME, aLayoutName ) );
    rOutAttrs.Put( SfxBoolItem( ATTR_PRESLAYOUT_MASTER_PAGE, maCbxMasterPage.IsChecked() ) );
    rOutAttrs.Put( SfxBoolItem( ATTR_PRESLAYOUT_CHECK_MASTERS, maCbxCheckMasters.IsChecked() ) );
}


/*************************************************************************
|*
|* Fuellt das ValueSet mit Bitmaps
|*
\************************************************************************/

void SdPresLayoutDlg::FillValueSet()
{
    maVS.SetStyle(maVS.GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER
                                      | WB_VSCROLL | WB_NAMEFIELD);

    maVS.SetColCount(2);
    maVS.SetLineCount(2);
    maVS.SetExtraSpacing(2);

    SdDrawDocument* pDoc = mpDocSh->GetDoc();

    USHORT nCount = pDoc->GetMasterPageCount();

    for (USHORT nLayout = 0; nLayout < nCount; nLayout++)
    {
        SdPage* pMaster = (SdPage*)pDoc->GetMasterPage(nLayout);
        if (pMaster->GetPageKind() == PK_STANDARD)
        {
            String aLayoutName(pMaster->GetLayoutName());
            aLayoutName.Erase( aLayoutName.SearchAscii( SD_LT_SEPARATOR ) );
            mpLayoutNames->Insert(new String(aLayoutName), LIST_APPEND);

            Bitmap aBitmap(mpDocSh->GetPagePreviewBitmap(pMaster, 90));
            maVS.InsertItem((USHORT)mpLayoutNames->Count(), aBitmap, aLayoutName);
        }
    }

    maVS.Show();
}


/*************************************************************************
|*
|* Doppelklick-Handler
|*
\************************************************************************/

IMPL_LINK(SdPresLayoutDlg, ClickLayoutHdl, void *, EMPTYARG)
{
    EndDialog(RET_OK);
    return 0;
}

/*************************************************************************
|*
|* Klick-Handler fuer Laden-Button
|*
\************************************************************************/

IMPL_LINK(SdPresLayoutDlg, ClickLoadHdl, void *, EMPTYARG)
{
    SfxNewFileDialog* pDlg = new SfxNewFileDialog(this, SFXWB_PREVIEW);
    pDlg->SetText(String(SdResId(STR_LOAD_PRESENTATION_LAYOUT)));

    if(!IsReallyVisible())
    {
        delete pDlg;
        return 0;
    }

    USHORT nResult = pDlg->Execute();
    // Inserted update to force repaint
    Update();

    String aFile;
    BOOL   bCancel = FALSE;

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
                // so wird "- keine -" codiert
                maName.Erase();
            }
        }
        break;

        default:
            bCancel = TRUE;
    }
    delete pDlg;

    if( !bCancel )
    {
        // Pruefen, ob Vorlage schon vorhanden
        BOOL bExists = FALSE;
        String* pName = (String*)mpLayoutNames->First();
        String aCompareStr( maName );
        if( maName.Len() == 0 )
            aCompareStr = maStrNone;

        while( pName && !bExists )
        {
            if( aCompareStr == *pName )
            {
                bExists = TRUE;
                // Vorlage selektieren
                USHORT nId = (USHORT) mpLayoutNames->GetCurPos() + 1;
                maVS.SelectItem( nId );
            }
            pName = (String*)mpLayoutNames->Next();
        }

        if( !bExists )
        {
            // Dokument laden um Preview-Bitmap zu ermitteln (wenn Vorlage ausgewaehlt)
            if( maName.Len() )
            {
                // Dokument ermitteln, um OpenBookmarkDoc rufen zu koennen
                SdDrawDocument* pDoc = mpDocSh->GetDoc();
                SdDrawDocument* pTemplDoc  = pDoc->OpenBookmarkDoc( maName );

                if (pTemplDoc)
                {
                    ::sd::DrawDocShell*  pTemplDocSh= pTemplDoc->GetDocSh();

                    USHORT nCount = pTemplDoc->GetMasterPageCount();

                    for (USHORT nLayout = 0; nLayout < nCount; nLayout++)
                    {
                        SdPage* pMaster = (SdPage*) pTemplDoc->GetMasterPage(nLayout);
                        if (pMaster->GetPageKind() == PK_STANDARD)
                        {
                            String aLayoutName(pMaster->GetLayoutName());
                            aLayoutName.Erase( aLayoutName.SearchAscii( SD_LT_SEPARATOR ) );
                            mpLayoutNames->Insert(new String(aLayoutName), LIST_APPEND);

                            Bitmap aBitmap(pTemplDocSh->GetPagePreviewBitmap(pMaster, 90));
                            maVS.InsertItem((USHORT)mpLayoutNames->Count(), aBitmap, aLayoutName);
                        }
                    }
                }
                else
                {
                    bCancel = TRUE;
                }

                pDoc->CloseBookmarkDoc();
            }
            else
            {
                // leeres Layout
                mpLayoutNames->Insert( new String( maStrNone ), LIST_APPEND );
                maVS.InsertItem( (USHORT) mpLayoutNames->Count(),
                        Bitmap( SdResId( BMP_FOIL_NONE ) ), maStrNone );
            }

            if (!bCancel)
            {
                // Vorlage selektieren
                maVS.SelectItem( (USHORT) mpLayoutNames->Count() );
            }
        }
    }

    return( 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
