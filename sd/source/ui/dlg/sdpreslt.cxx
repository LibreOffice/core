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
}

/*************************************************************************
|*
|*    Initialisierung
|*
*************************************************************************/

void SdPresLayoutDlg::Reset()
{
    const SfxPoolItem *pPoolItem = NULL;
    sal_uInt32 nName(0);

    // MasterPage austauschen
    if( mrOutAttrs.GetItemState( ATTR_PRESLAYOUT_MASTER_PAGE, false, &pPoolItem ) == SFX_ITEM_SET )
    {
        bool bMasterPage = ( (const SfxBoolItem*) pPoolItem)->GetValue();
        maCbxMasterPage.Enable( !bMasterPage );
        maCbxMasterPage.Check( bMasterPage );
    }

    // Nicht verwendete MasterPages entfernen
    maCbxCheckMasters.Check(false);

    if(mrOutAttrs.GetItemState(ATTR_PRESLAYOUT_NAME, true, &pPoolItem) == SFX_ITEM_SET)
        maName = ((const SfxStringItem*)pPoolItem)->GetValue();
    else
        maName.Erase();

    FillValueSet();

    mnLayoutCount = maLayoutNames.size();
    for( nName = 0; nName < mnLayoutCount; nName++ )
    {
        if(maLayoutNames[nName] == maName)
        {
            break;
        }
    }
    DBG_ASSERT(nName < mnLayoutCount, "Layout nicht gefunden");

    maVS.SelectItem(static_cast< sal_uInt16 >(nName + 1));  // Inizes des ValueSets beginnen bei 1

}

/*************************************************************************
|*
|*    Fuellt uebergebenen Item-Set mit Dialogbox-Attributen
|*
*************************************************************************/

void SdPresLayoutDlg::GetAttr(SfxItemSet& rOutAttrs)
{
    short nId = maVS.GetSelectItemId();
    bool bLoad = nId > mnLayoutCount;
    rOutAttrs.Put( SfxBoolItem( ATTR_PRESLAYOUT_LOAD, bLoad ) );

    String aLayoutName;

    if( bLoad )
    {
        aLayoutName = maName;
        aLayoutName.Append( DOCUMENT_TOKEN );
        aLayoutName.Append(maLayoutNames[nId - 1]);
    }
    else
    {
        aLayoutName = maLayoutNames[nId - 1];
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

    sal_uInt32 nCount = pDoc->GetMasterPageCount();

    for (sal_uInt32 nLayout = 0; nLayout < nCount; nLayout++)
    {
        SdPage* pMaster = (SdPage*)pDoc->GetMasterPage(nLayout);
        if (pMaster->GetPageKind() == PK_STANDARD)
        {
            String aLayoutName(pMaster->GetLayoutName());
            aLayoutName.Erase( aLayoutName.SearchAscii( SD_LT_SEPARATOR ) );
            maLayoutNames.push_back(aLayoutName);

            Bitmap aBitmap(mpDocSh->GetPagePreviewBitmap(pMaster, 90));
            maVS.InsertItem(maLayoutNames.size(), aBitmap, aLayoutName);
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

    sal_uInt16 nResult = pDlg->Execute();
    // #96072# OJ: Inserted update to force repaint
    Update();

    String aFile;
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
                // so wird "- keine -" codiert
                maName.Erase();
            }
        }
        break;

        default:
            bCancel = true;
    }
    delete pDlg;

//    if (!bCancel)
//        EndDialog(RET_OK);

    if( !bCancel )
    {
        // Pruefen, ob Vorlage schon vorhanden
        bool bExists = false;
        String aCompareStr( maName );
        if( maName.Len() == 0 )
            aCompareStr = maStrNone;

        for(sal_uInt32 a(0); !bExists && a < maLayoutNames.size(); a++)
        {
            if(aCompareStr == maLayoutNames[a])
            {
                bExists = true;

                // select template
                // const sal_uInt16 nId(static_cast< sal_uInt16 >(a + 1);
                maVS.SelectItem(static_cast< sal_uInt16 >(a + 1));
            }
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
                    sal_uInt32 nCount = pTemplDoc->GetMasterPageCount();

                    for (sal_uInt32 nLayout = 0; nLayout < nCount; nLayout++)
                    {
                        SdPage* pMaster = (SdPage*) pTemplDoc->GetMasterPage(nLayout);
                        if (pMaster->GetPageKind() == PK_STANDARD)
                        {
                            String aLayoutName(pMaster->GetLayoutName());
                            aLayoutName.Erase( aLayoutName.SearchAscii( SD_LT_SEPARATOR ) );
                            maLayoutNames.push_back(aLayoutName);

                            Bitmap aBitmap(pTemplDocSh->GetPagePreviewBitmap(pMaster, 90));
                            maVS.InsertItem(maLayoutNames.size(), aBitmap, aLayoutName);
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
                // leeres Layout
                maLayoutNames.push_back(maStrNone);
                maVS.InsertItem(maLayoutNames.size(), Bitmap( SdResId( BMP_FOIL_NONE ) ), maStrNone );
            }

            if (!bCancel)
            {
                // Vorlage selektieren
                maVS.SelectItem(static_cast< sal_uInt16 >(maLayoutNames.size()));
            }
        }
    }

    return( 0 );
}
