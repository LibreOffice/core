/*************************************************************************
 *
 *  $RCSfile: sdpreslt.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2000-10-18 14:30:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXNEW_HXX //autogen
#include <sfx2/new.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdpreslt.hxx"
#include "sdpreslt.hrc"
#include "sdattr.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "docshell.hxx"
#include "viewshel.hxx"


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

SdPresLayoutDlg::SdPresLayoutDlg( SdDrawDocShell* pDocShell, SdViewShell* pViewShell, Window* pWindow, const SfxItemSet& rInAttrs ):
                ModalDialog         (pWindow, SdResId(DLG_PRESLT)),
                pDocSh              ( pDocShell ),
                pViewSh             ( pViewShell ),
                aFtLayout           (this, SdResId(FT_LAYOUT)),
                aVS                 (this, SdResId(VS_LAYOUT)),
                aBtnOK              (this, SdResId(BTN_OK)),
                aBtnCancel          (this, SdResId(BTN_CANCEL)),
                aBtnHelp            (this, SdResId(BTN_HELP)),
                aBtnLoad            (this, SdResId(BTN_LOAD)),
                aCbxMasterPage      (this, SdResId(CBX_MASTER_PAGE)),
                aCbxCheckMasters    (this, SdResId(CBX_CHECK_MASTERS)),
                rOutAttrs           (rInAttrs),
                aStrNone            ( SdResId( STR_NULL ) )
{
    FreeResource();

    pLayoutNames = new List;

    aVS.SetDoubleClickHdl(LINK(this, SdPresLayoutDlg, ClickLayoutHdl));
    aBtnLoad.SetClickHdl(LINK(this, SdPresLayoutDlg, ClickLoadHdl));

    Reset();
}

/*************************************************************************
|*
|*  Dtor
|*
*************************************************************************/

SdPresLayoutDlg::~SdPresLayoutDlg()
{
    String* pName = (String*)pLayoutNames->First();
    while (pName)
    {
        delete pName;
        pName = (String*)pLayoutNames->Next();
    }

    delete pLayoutNames;
}

/*************************************************************************
|*
|*    Initialisierung
|*
*************************************************************************/

void SdPresLayoutDlg::Reset()
{
    const SfxPoolItem *pPoolItem = NULL;

    // MasterPage austauschen
    if( rOutAttrs.GetItemState( ATTR_PRESLAYOUT_MASTER_PAGE, FALSE, &pPoolItem ) == SFX_ITEM_SET )
    {
        BOOL bMasterPage = ( (const SfxBoolItem*) pPoolItem)->GetValue();
        aCbxMasterPage.Enable( !bMasterPage );
        aCbxMasterPage.Check( bMasterPage );
    }

    // Nicht verwendete MasterPages entfernen
    aCbxCheckMasters.Check(FALSE);

    if(rOutAttrs.GetItemState(ATTR_PRESLAYOUT_NAME, TRUE, &pPoolItem) == SFX_ITEM_SET)
        aName = ((const SfxStringItem*)pPoolItem)->GetValue();
    else
        aName.Erase();

    FillValueSet();

    nLayoutCount = pLayoutNames->Count();
    for( long nName = 0; nName < nLayoutCount; nName++ )
    {
        if (*((String*)pLayoutNames->GetObject(nName)) == aName)
            break;
    }
    DBG_ASSERT(nName < nLayoutCount, "Layout nicht gefunden")

    aVS.SelectItem((USHORT)nName + 1);  // Inizes des ValueSets beginnen bei 1

}

/*************************************************************************
|*
|*    Fuellt uebergebenen Item-Set mit Dialogbox-Attributen
|*
*************************************************************************/

void SdPresLayoutDlg::GetAttr(SfxItemSet& rOutAttrs)
{
    short nId = aVS.GetSelectItemId();
    BOOL bLoad = nId > nLayoutCount;

    rOutAttrs.Put( SfxBoolItem( ATTR_PRESLAYOUT_LOAD, bLoad ) );
    rOutAttrs.Put( SfxBoolItem( ATTR_PRESLAYOUT_MASTER_PAGE, aCbxMasterPage.IsChecked() ) );
    rOutAttrs.Put( SfxBoolItem( ATTR_PRESLAYOUT_CHECK_MASTERS, aCbxCheckMasters.IsChecked() ) );

    String* pName = (String*)pLayoutNames->GetObject( nId - 1 );
    if( *pName == aStrNone )
        pName->Erase(); //  so wird "- keine -" codiert (s.u.)
    rOutAttrs.Put( SfxStringItem( ATTR_PRESLAYOUT_NAME, *pName ) );
}

/*************************************************************************
|*
|* Fuellt das ValueSet mit Bitmaps
|*
\************************************************************************/

void SdPresLayoutDlg::FillValueSet()
{
    aVS.SetStyle(aVS.GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER
                                      | WB_VSCROLL | WB_NAMEFIELD);

    aVS.SetColCount(2);
    aVS.SetLineCount(2);
    aVS.SetExtraSpacing(2);

    SdView* pView = pViewSh->GetView();
    SdDrawDocument* pDoc = pDocSh->GetDoc();

    USHORT nCount = pDoc->GetMasterPageCount();

    for (USHORT nLayout = 0; nLayout < nCount; nLayout++)
    {
        SdPage* pMaster = (SdPage*)pDoc->GetMasterPage(nLayout);
        if (pMaster->GetPageKind() == PK_STANDARD)
        {
            String aLayoutName(pMaster->GetLayoutName());
            aLayoutName.Erase( aLayoutName.SearchAscii( SD_LT_SEPARATOR ) );
            pLayoutNames->Insert(new String(aLayoutName), LIST_APPEND);

            Bitmap aBitmap(pDocSh->GetPagePreviewBitmap(pMaster, 90));
            aVS.InsertItem((USHORT)pLayoutNames->Count(), aBitmap, aLayoutName);
        }
    }

    aVS.Show();
}


/*************************************************************************
|*
|* Doppelklick-Handler
|*
\************************************************************************/

IMPL_LINK(SdPresLayoutDlg, ClickLayoutHdl, void *, p)
{
    EndDialog(RET_OK);
    return 0;
}

/*************************************************************************
|*
|* Klick-Handler fuer Laden-Button
|*
\************************************************************************/

IMPL_LINK(SdPresLayoutDlg, ClickLoadHdl, void *, p)
{
    SfxNewFileDialog* pDlg = new SfxNewFileDialog(this, SFXWB_PREVIEW);
    pDlg->SetText(String(SdResId(STR_LOAD_PRESENTATION_LAYOUT)));

    if(!IsReallyVisible())
    {
        delete pDlg;
        return 0;
    }

    USHORT nResult = pDlg->Execute();
    String aFile;
    BOOL   bCancel = FALSE;

    switch (nResult)
    {
        case RET_OK:
        {
            if (pDlg->IsTemplate())
            {
                aName = pDlg->GetTemplateFileName();
            }
            else
            {
                // so wird "- keine -" codiert
                aName.Erase();
            }
        }
        break;

        default:
            bCancel = TRUE;
    }
    delete pDlg;

//    if (!bCancel)
//        EndDialog(RET_OK);

    if( !bCancel )
    {
        // Pruefen, ob Vorlage schon vorhanden
        BOOL bExists = FALSE;
        String* pName = (String*)pLayoutNames->First();
        String aCompareStr( aName );
        if( aName.Len() == 0 )
            aCompareStr = aStrNone;

        while( pName && !bExists )
        {
            if( aCompareStr == *pName )
            {
                bExists = TRUE;
                // Vorlage selektieren
                USHORT nId = (USHORT) pLayoutNames->GetCurPos() + 1;
                aVS.SelectItem( nId );
            }
            pName = (String*)pLayoutNames->Next();
        }

        if( !bExists )
        {
            // Dokument laden um Preview-Bitmap zu ermitteln (wenn Vorlage ausgewaehlt)
            if( aName.Len() )
            {
                // Dokument ermitteln, um OpenBookmarkDoc rufen zu koennen
                SdDrawDocument* pDoc = pDocSh->GetDoc();
                SdDrawDocument* pTemplDoc  = pDoc->OpenBookmarkDoc( aName );

                if (pTemplDoc)
                {
                    SdDrawDocShell*  pTemplDocSh= pTemplDoc->GetDocSh();

                    SdPage* pMaster = pTemplDoc->GetMasterSdPage( 0, PK_STANDARD );
                    pLayoutNames->Insert( new String( aName ), LIST_APPEND );

                    Bitmap aBitmap( pTemplDocSh->GetPagePreviewBitmap( pMaster, 90 ) );
                    aVS.InsertItem( (USHORT) pLayoutNames->Count(), aBitmap, aName);
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
                pLayoutNames->Insert( new String( aStrNone ), LIST_APPEND );
                aVS.InsertItem( (USHORT) pLayoutNames->Count(),
                        Bitmap( SdResId( BMP_FOIL_NONE ) ), aStrNone );
            }

            if (!bCancel)
            {
                // Vorlage selektieren
                aVS.SelectItem( (USHORT) pLayoutNames->Count() );
            }
        }
    }

    return( 0 );
}




