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


#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif
#include <sfx2/docfile.hxx>
#include <unotools/pathoptions.hxx>

#include "sdpage.hxx"
#include "Outliner.hxx"
#include "res_bmp.hrc"

#include <vcl/svapp.hxx>
#include "svtools/svlbitm.hxx"

#include "dlgassim.hxx"

SdPageListControl::SdPageListControl(
    ::Window* pParent,
    const ResId& rResId )
    : SvTreeListBox(pParent, rResId)
{
    // Tree-ListBox mit Linien versehen
    SetStyle( GetStyle() | WB_TABSTOP | WB_BORDER | WB_HASLINES |
                            WB_HASBUTTONS |  WB_HASLINESATROOT |
                            WB_HSCROLL |
                            WB_HASBUTTONSATROOT );

    SetNodeDefaultImages ();
    m_pCheckButton = new SvLBoxButtonData(this);
    EnableCheckButton (m_pCheckButton);

    SetCheckButtonHdl( LINK(this,SdPageListControl,CheckButtonClickHdl) );
}

IMPL_LINK_NOARG(SdPageListControl, CheckButtonClickHdl)
{
    SvTreeList* pTreeModel = GetModel();
    SvTreeListEntry* pEntry = pTreeModel->First();

    while( pEntry )
    {
        if(pTreeModel->IsAtRootDepth(pEntry) && GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED )
            return 0;
        pEntry = pTreeModel->Next( pEntry );
    }

    pEntry = pTreeModel->First();
    SetCheckButtonState( pEntry, SV_BUTTON_CHECKED );

    return 0;
}

SdPageListControl::~SdPageListControl()
{
    delete m_pCheckButton;
}

void SdPageListControl::Clear()
{
    SvTreeListBox::Clear();
}

SvTreeListEntry* SdPageListControl::InsertPage( const String& rPageName )
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;

    pEntry->AddItem( new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox,
                                       0, m_pCheckButton));
    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));    // Sonst Puff!
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rPageName ) );

    GetModel()->Insert( pEntry );

    return pEntry;
}

void SdPageListControl::InsertTitle( SvTreeListEntry* pParent, const String& rTitle )
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;
    pEntry->AddItem( new SvLBoxString( pEntry, 0, String() ) );
    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));    // Sonst Puff!
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rTitle ) );
    GetModel()->Insert( pEntry,pParent );
}

void SdPageListControl::Fill( SdDrawDocument* pDoc )
{
    Outliner* pOutliner = pDoc->GetInternalOutliner();

    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = pDoc->GetPageCount();
    while( nPage < nMaxPages )
    {
        SdPage* pPage = (SdPage*) pDoc->GetPage( nPage );
        if( pPage->GetPageKind() == PK_STANDARD )
        {
            SvTreeListEntry* pEntry = InsertPage( pPage->GetName() );
            SetCheckButtonState(pEntry, SvButtonState( SV_BUTTON_CHECKED ) );

            SdrTextObj* pTO = (SdrTextObj*)pPage->GetPresObj(PRESOBJ_TEXT);
            if(!pTO)
            {
                // Ermittelt das SdrTextObject mit dem Layout Text dieser Seite
                const sal_uLong nObjectCount = pPage->GetObjCount();
                for (sal_uLong nObject = 0; nObject < nObjectCount; nObject++)
                {
                    SdrObject* pObject = pPage->GetObj(nObject);
                    if (pObject->GetObjInventor() == SdrInventor && pObject->GetObjIdentifier() == OBJ_OUTLINETEXT)
                    {
                        pTO = (SdrTextObj*)pObject;
                        break;
                    }
                }
            }

            if (pTO && !pTO->IsEmptyPresObj())
            {
                OutlinerParaObject* pOPO = pTO->GetOutlinerParaObject();
                if (pOPO)
                {
                    pOutliner->Clear();
                    pOutliner->SetText( *pOPO );

                    sal_uLong nCount = pOutliner->GetParagraphCount();

                    Paragraph* pPara = NULL;

                    for (sal_uLong nPara = 0; nPara < nCount; nPara++)
                    {
                        pPara = pOutliner->GetParagraph(nPara);
                        if(pPara && pOutliner->GetDepth( (sal_uInt16) nPara ) == 0 )
                        {
                            String aParaText = pOutliner->GetText(pPara);
                            if(aParaText.Len() != 0)
                                InsertTitle( pEntry, aParaText );
                        }
                    }
                }
            }
        }
        nPage++;
    }

    pOutliner->Clear();
}

sal_uInt16 SdPageListControl::GetSelectedPage()
{
    SvTreeListEntry* pSelEntry = GetCurEntry();
    sal_uInt16 nPage = 0;

    if ( pSelEntry )
    {
        SvTreeList* pTreeModel = GetModel();
        SvTreeListEntry* pEntry = pTreeModel->First();

        while( pEntry && pEntry != pSelEntry )
        {
            if(pTreeModel->IsAtRootDepth(pEntry))
                nPage++;
            pEntry = pTreeModel->Next( pEntry );
        }

        if(!pTreeModel->IsAtRootDepth(pSelEntry))
            nPage--;
    }
    return nPage;
}

sal_Bool SdPageListControl::IsPageChecked( sal_uInt16 nPage )
{
    SvTreeListEntry* pEntry = GetModel()->GetEntry(nPage);
    return pEntry?(sal_Bool)(GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED): sal_False;
}

void SdPageListControl::DataChanged( const DataChangedEvent& rDCEvt )
{
    SvTreeListBox::DataChanged( rDCEvt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
