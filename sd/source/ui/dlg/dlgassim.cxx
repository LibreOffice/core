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
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>

#include "sdpage.hxx"
#include "Outliner.hxx"
#include "res_bmp.hrc"

#include <vcl/svapp.hxx>

#include "dlgassim.hxx"

SdPageListControl::SdPageListControl(
    ::Window* pParent,
    const ResId& rResId )
    : SvTreeListBox(pParent, rResId)
{
    // Tree-ListBox mit Linien versehen
    SetStyle( GetStyle() | WB_TABSTOP | WB_BORDER | WB_HASLINES |
                            WB_HASBUTTONS |  WB_HASLINESATROOT |
                            WB_HSCROLL | // #31562#
                            WB_HASBUTTONSATROOT );

    SetNodeDefaultImages ();
    m_pCheckButton = new SvLBoxButtonData(this);
    EnableCheckButton (m_pCheckButton);

    SetCheckButtonHdl( LINK(this,SdPageListControl,CheckButtonClickHdl) );
}

IMPL_LINK( SdPageListControl, CheckButtonClickHdl, SvLBoxButtonData *, EMPTYARG )
{
    SvLBoxTreeList* pTreeModel = GetModel();
    SvLBoxEntry* pEntry = pTreeModel->First();

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

SvLBoxEntry* SdPageListControl::InsertPage( const String& rPageName )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;

    pEntry->AddItem( new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox,
                                       0, m_pCheckButton));
    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));    // Sonst Puff!
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rPageName ) );

    GetModel()->Insert( pEntry );

    return pEntry;
}

void SdPageListControl::InsertTitle( SvLBoxEntry* pParent, const String& rTitle )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;
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
            SvLBoxEntry* pEntry = InsertPage( pPage->GetName() );
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

sal_uInt32 SdPageListControl::GetSelectedPage()
{
    SvLBoxEntry* pSelEntry = GetCurEntry();
    sal_uInt32 nPage = 0;

    if ( pSelEntry )
    {
        SvLBoxTreeList* pTreeModel = GetModel();
        SvLBoxEntry* pEntry = pTreeModel->First();

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

bool SdPageListControl::IsPageChecked( sal_uInt32 nPage )
{
    SvLBoxEntry* pEntry = GetModel()->GetEntry(nPage);
    return pEntry ? (GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED) : false;
}

void SdPageListControl::DataChanged( const DataChangedEvent& rDCEvt )
{
    SvTreeListBox::DataChanged( rDCEvt );
}

