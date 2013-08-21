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

#include <sfx2/docfile.hxx>
#include <unotools/pathoptions.hxx>

#include "sdpage.hxx"
#include "Outliner.hxx"
#include "res_bmp.hrc"

#include <vcl/svapp.hxx>
#include "svtools/svlbitm.hxx"
#include "svtools/treelistentry.hxx"

#include "dlgassim.hxx"

SdPageListControl::SdPageListControl(
    ::Window* pParent,
    const ResId& rResId )
    : SvTreeListBox(pParent, rResId)
{
    // add lines to tree listbox
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

SvTreeListEntry* SdPageListControl::InsertPage( const OUString& rPageName )
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;

    pEntry->AddItem( new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox,
                                       0, m_pCheckButton));
    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));    // otherwise boom!
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rPageName ) );

    GetModel()->Insert( pEntry );

    return pEntry;
}

void SdPageListControl::InsertTitle( SvTreeListEntry* pParent, const OUString& rTitle )
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;
    pEntry->AddItem( new SvLBoxString( pEntry, 0, OUString() ) );
    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));    // otherwise boom!
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
                // determines the SdrTextObject with the layout text of this page
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

                    sal_Int32 nCount = pOutliner->GetParagraphCount();

                    Paragraph* pPara = NULL;

                    for (sal_Int32 nPara = 0; nPara < nCount; nPara++)
                    {
                        pPara = pOutliner->GetParagraph(nPara);
                        if(pPara && pOutliner->GetDepth( nPara ) == 0 )
                        {
                            OUString aParaText = pOutliner->GetText(pPara);
                            if (!aParaText.isEmpty())
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
