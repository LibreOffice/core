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

#undef SC_DLLIMPLEMENTATION

#include <scitems.hxx>
#include <editeng/eeitem.hxx>

#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/editview.hxx>
#include <editeng/flditem.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <unotools/useroptions.hxx>

#include <editutil.hxx>
#include <global.hxx>
#include <attrib.hxx>
#include <patattr.hxx>
#include <sc.hrc>
#include <tabvwsh.hxx>
#include <prevwsh.hxx>
#include <textdlgs.hxx>
#include <AccessibleEditObject.hxx>

#include <scuitphfedit.hxx>
#include <memory>

// class ScHFEditPage

ScHFEditPage::ScHFEditPage( vcl::Window*             pParent,
                            const SfxItemSet&   rCoreAttrs,
                            sal_uInt16          nWhichId,
                            bool                bHeader  )
    : SfxTabPage( pParent, "HeaderFooterContent", "modules/scalc/ui/headerfootercontent.ui", &rCoreAttrs )
    , m_pEditFocus(nullptr)
    , nWhich( nWhichId )
{
    get(m_pWndLeft,"textviewWND_LEFT");
    m_pWndLeft->SetLocation(Left);
    get(m_pWndCenter,"textviewWND_CENTER");
    m_pWndCenter->SetLocation(Center);
    get(m_pWndRight,"textviewWND_RIGHT");
    m_pWndRight->SetLocation(Right);

    get(m_pLbDefined,"comboLB_DEFINED");
    // tdf#114695 override natural size with a small value
    // we expect this to get stretched to some larger but
    // limited size based on surrounding widgets
    m_pLbDefined->set_width_request(m_pLbDefined->approximate_digit_width() * 20);

    get(m_pBtnText,"buttonBTN_TEXT");
    get(m_pBtnTable,"buttonBTN_TABLE");
    get(m_pBtnPage,"buttonBTN_PAGE");
    get(m_pBtnLastPage,"buttonBTN_PAGES");
    get(m_pBtnDate,"buttonBTN_DATE");
    get(m_pBtnTime,"buttonBTN_TIME");

    get(m_pBtnFile,"buttonBTN_FILE");

    get(m_pFtConfidential,"labelSTR_HF_CONFIDENTIAL");
    get(m_pFtPage,"labelSTR_PAGE");
    get(m_pFtOfQuestion,"labelSTR_HF_OF_QUESTION");
    get(m_pFtOf,"labelSTR_HF_OF");
    get(m_pFtNone,"labelSTR_HF_NONE_IN_BRACKETS");
    get(m_pFtCreatedBy,"labelSTR_HF_CREATED_BY");
    get(m_pFtCustomized,"labelSTR_HF_CUSTOMIZED");

    //! use default style from current document?
    //! if font color is used, header/footer background color must be set

    ScPatternAttr   aPatAttr( rCoreAttrs.GetPool() );

    m_pBtnFile->SetPopupMenu(get_menu("popup"));

    m_pLbDefined->SetSelectHdl( LINK( this, ScHFEditPage, ListHdl_Impl ) );
    m_pBtnFile->SetMenuHdl( LINK( this, ScHFEditPage, MenuHdl ) );
    m_pBtnText->SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    m_pBtnPage->SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    m_pBtnLastPage->SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    m_pBtnDate->SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    m_pBtnTime->SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    m_pBtnFile->SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    m_pBtnTable->SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );

    get(m_pFtDefinedHF,!bHeader ? "labelFT_F_DEFINED" : "labelFT_H_DEFINED");
    get(m_pFtCustomHF, !bHeader ? "labelFT_F_CUSTOM" : "labelFT_H_CUSTOM");

    m_pFtDefinedHF->Show();
    m_pFtCustomHF->Show();

    //swap left/right areas and their labels in RTL mode
    if( AllSettings::GetLayoutRTL() )
    {
        vcl::Window *pLeft = get<vcl::Window>("labelFT_LEFT");
        vcl::Window *pRight = get<vcl::Window>("labelFT_RIGHT");
        sal_Int32 nOldLeftAttach = pLeft->get_grid_left_attach();
        sal_Int32 nOldRightAttach = pRight->get_grid_left_attach();
        pLeft->set_grid_left_attach(nOldRightAttach);
        pRight->set_grid_left_attach(nOldLeftAttach);

        pLeft = m_pWndLeft;
        pRight = m_pWndRight;
        nOldLeftAttach = pLeft->get_grid_left_attach();
        nOldRightAttach = pRight->get_grid_left_attach();
        pLeft->set_grid_left_attach(nOldRightAttach);
        pRight->set_grid_left_attach(nOldLeftAttach);
    }
    m_pWndLeft->SetFont( aPatAttr );
    m_pWndCenter->SetFont( aPatAttr );
    m_pWndRight->SetFont( aPatAttr );

    // Set size request for all 3 widgets
    Size aSize = LogicToPixel(Size(80, 120), MapMode(MapUnit::MapAppFont));
    VclPtr<ScEditWindow> aEditWindows[] = {m_pWndLeft, m_pWndCenter, m_pWndRight};

    for (auto &pEditWindow : aEditWindows)
    {
        pEditWindow->set_width_request(aSize.Width());
        pEditWindow->set_height_request(aSize.Height());
    }

    m_pWndLeft->SetObjectSelectHdl( LINK(this,ScHFEditPage,ObjectSelectHdl) );
    m_pWndCenter->SetObjectSelectHdl( LINK(this,ScHFEditPage,ObjectSelectHdl) );
    m_pWndRight->SetObjectSelectHdl( LINK(this,ScHFEditPage,ObjectSelectHdl) );
    auto setEditFocus = [this](ScEditWindow & rEdit) { this->m_pEditFocus = &rEdit; };
    m_pWndLeft->SetGetFocusHdl(setEditFocus);
    m_pWndCenter->SetGetFocusHdl(setEditFocus);
    m_pWndRight->SetGetFocusHdl(setEditFocus);

    m_pWndLeft->GrabFocus();
    m_pEditFocus = m_pWndLeft; // there's no event from GrabFocus()

    InitPreDefinedList();

}

IMPL_LINK_NOARG( ScHFEditPage, ObjectSelectHdl, ScEditWindow&, void )
{
    m_pBtnText->GrabFocus();
}

ScHFEditPage::~ScHFEditPage()
{
    disposeOnce();
}

void ScHFEditPage::dispose()
{
    m_pWndLeft.clear();
    m_pWndCenter.clear();
    m_pWndRight.clear();
    m_pFtDefinedHF.clear();
    m_pLbDefined.clear();
    m_pFtCustomHF.clear();
    m_pBtnText.clear();
    m_pBtnFile.clear();
    m_pBtnTable.clear();
    m_pBtnPage.clear();
    m_pBtnLastPage.clear();
    m_pBtnDate.clear();
    m_pBtnTime.clear();
    m_pFtConfidential.clear();
    m_pFtPage.clear();
    m_pFtOfQuestion.clear();
    m_pFtOf.clear();
    m_pFtNone.clear();
    m_pFtCreatedBy.clear();
    m_pFtCustomized.clear();
    SfxTabPage::dispose();
}

void ScHFEditPage::SetNumType(SvxNumType eNumType)
{
    m_pWndLeft->SetNumType(eNumType);
    m_pWndCenter->SetNumType(eNumType);
    m_pWndRight->SetNumType(eNumType);
}

void ScHFEditPage::Reset( const SfxItemSet* rCoreSet )
{
    const SfxPoolItem* pItem = nullptr;
    if ( rCoreSet->HasItem(nWhich, &pItem) )
    {
        const ScPageHFItem& rItem = static_cast<const ScPageHFItem&>(*pItem);

        if( const EditTextObject* pLeft = rItem.GetLeftArea() )
            m_pWndLeft->SetText( *pLeft );
        if( const EditTextObject* pCenter = rItem.GetCenterArea() )
            m_pWndCenter->SetText( *pCenter );
        if( const EditTextObject* pRight = rItem.GetRightArea() )
            m_pWndRight->SetText( *pRight );

        SetSelectDefinedList();
    }
}

bool ScHFEditPage::FillItemSet( SfxItemSet* rCoreSet )
{
    ScPageHFItem    aItem( nWhich );
    std::unique_ptr<EditTextObject> pLeft   = m_pWndLeft->CreateTextObject();
    std::unique_ptr<EditTextObject> pCenter = m_pWndCenter->CreateTextObject();
    std::unique_ptr<EditTextObject> pRight  = m_pWndRight->CreateTextObject();

    aItem.SetLeftArea  ( *pLeft );
    aItem.SetCenterArea( *pCenter );
    aItem.SetRightArea ( *pRight );

    rCoreSet->Put( aItem );

    return true;
}

void ScHFEditPage::InitPreDefinedList()
{
    SvtUserOptions aUserOpt;

    boost::optional<Color> pTxtColour;
    boost::optional<Color> pFldColour;

    // Get the all field values at the outset.
    OUString aPageFieldValue(m_pWndLeft->GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), 0,0, pTxtColour, pFldColour));
    OUString aSheetFieldValue(m_pWndLeft->GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxTableField(), EE_FEATURE_FIELD), 0,0, pTxtColour, pFldColour));
    OUString aFileFieldValue(m_pWndLeft->GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxFileField(), EE_FEATURE_FIELD), 0,0, pTxtColour, pFldColour));
    OUString aExtFileFieldValue(m_pWndLeft->GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxExtFileField(), EE_FEATURE_FIELD), 0,0, pTxtColour, pFldColour));
    OUString aDateFieldValue(m_pWndLeft->GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxDateField(), EE_FEATURE_FIELD), 0,0, pTxtColour, pFldColour));

    m_pLbDefined->Clear();

    m_pLbDefined->InsertEntry(m_pFtNone->GetText());

    OUString aPageEntry(m_pFtPage->GetText() + " " + aPageFieldValue);
    m_pLbDefined->InsertEntry(aPageEntry);

    OUString aPageOfEntry(aPageEntry + " " + m_pFtOfQuestion->GetText());
    m_pLbDefined->InsertEntry( aPageOfEntry);

    m_pLbDefined->InsertEntry(aSheetFieldValue);

    OUString aConfidentialEntry(aUserOpt.GetCompany() + " " + m_pFtConfidential->GetText() + ", " + aDateFieldValue + ", " + aPageEntry);
    m_pLbDefined->InsertEntry( aConfidentialEntry);

    OUString aFileNamePageEntry(aFileFieldValue + ", " + aPageEntry);
    m_pLbDefined->InsertEntry( aFileNamePageEntry);

    m_pLbDefined->InsertEntry( aExtFileFieldValue);

    OUString aPageSheetNameEntry(aPageEntry + ", " + aSheetFieldValue);
    m_pLbDefined->InsertEntry( aPageSheetNameEntry);

    OUString aPageFileNameEntry(aPageEntry + ", " + aFileFieldValue);
    m_pLbDefined->InsertEntry( aPageFileNameEntry);

    OUString aPagePathNameEntry(aPageEntry + ", " + aExtFileFieldValue);
    m_pLbDefined->InsertEntry( aPagePathNameEntry);

    OUString aUserNameEntry(aUserOpt.GetFirstName() + " " + aUserOpt.GetLastName() + ", " + aPageEntry + ", " + aDateFieldValue);
    m_pLbDefined->InsertEntry( aUserNameEntry);

    OUString aCreatedByEntry( m_pFtCreatedBy->GetText() + " " + aUserOpt.GetFirstName() + " " + aUserOpt.GetLastName() + ", ");
    aCreatedByEntry += aDateFieldValue + ", " + aPageEntry;
    m_pLbDefined->InsertEntry( aCreatedByEntry);
}

void ScHFEditPage::InsertToDefinedList()
{
    const sal_Int32 nCount =  m_pLbDefined->GetEntryCount();
    if(nCount == eEntryCount)
    {
        m_pLbDefined->InsertEntry( m_pFtCustomized->GetText() );
        m_pLbDefined->SelectEntryPos(eEntryCount);
    }
}

void ScHFEditPage::RemoveFromDefinedList()
{
    const sal_Int32 nCount =  m_pLbDefined->GetEntryCount();
    if(nCount > eEntryCount )
        m_pLbDefined->RemoveEntry( nCount-1);
}

// determine if the header/footer exists in our predefined list and set select to it.
void ScHFEditPage::SetSelectDefinedList()
{
    SvtUserOptions aUserOpt;

    // default to customized
    ScHFEntryId eSelectEntry = eEntryCount;

    std::unique_ptr< EditTextObject > pLeftObj;
    std::unique_ptr< EditTextObject > pCenterObj;
    std::unique_ptr< EditTextObject > pRightObj;

    OUString aLeftEntry;
    OUString aCenterEntry;
    OUString aRightEntry;

    pLeftObj = m_pWndLeft->GetEditEngine()->CreateTextObject();
    pCenterObj = m_pWndCenter->GetEditEngine()->CreateTextObject();
    pRightObj = m_pWndRight->GetEditEngine()->CreateTextObject();

    bool bFound = false;

    const sal_Int32 nCount = m_pLbDefined->GetEntryCount();
    for(sal_Int32 i = 0; i < nCount && !bFound; ++i)
    {
        switch(static_cast<ScHFEntryId>(i))
        {
            case eNoneEntry:
            {
                aLeftEntry = pLeftObj->GetText(0);
                aCenterEntry = pCenterObj->GetText(0);
                aRightEntry = pRightObj->GetText(0);
                if(aLeftEntry == EMPTY_OUSTRING && aCenterEntry == EMPTY_OUSTRING
                    && aRightEntry == EMPTY_OUSTRING)
                {
                    eSelectEntry = eNoneEntry;
                    bFound = true;
                }
            }
            break;

            case ePageEntry:
            {
                aLeftEntry = pLeftObj->GetText(0);
                aRightEntry = pRightObj->GetText(0);
                if(aLeftEntry == EMPTY_OUSTRING && aRightEntry == EMPTY_OUSTRING)
                {
                    if(IsPageEntry(m_pWndCenter->GetEditEngine(), pCenterObj.get()))
                    {
                        eSelectEntry = ePageEntry;
                        bFound = true;
                    }
                }
            }
            break;

            //TODO
            case ePagesEntry:
            {
            }
            break;

            case eSheetEntry:
            {
                aLeftEntry = pLeftObj->GetText(0);
                aRightEntry = pRightObj->GetText(0);
                if(aLeftEntry == EMPTY_OUSTRING && aRightEntry == EMPTY_OUSTRING)
                {
                    if(pCenterObj->IsFieldObject())
                    {
                        const SvxFieldItem* pFieldItem = pCenterObj->GetField();
                        if(pFieldItem)
                        {
                            const SvxFieldData* pField = pFieldItem->GetField();
                            if(pField && dynamic_cast<const SvxTableField*>( pField) !=  nullptr)
                            {
                                eSelectEntry = eSheetEntry;
                                bFound = true;
                            }
                        }
                    }
                }
            }
            break;

            case eConfidentialEntry:
            {
                if(IsDateEntry(pCenterObj.get()) && IsPageEntry(m_pWndRight->GetEditEngine(), pRightObj.get()))
                {
                    OUString aConfidentialEntry(aUserOpt.GetCompany() + " " + m_pFtConfidential->GetText());
                    if(aConfidentialEntry == m_pWndLeft->GetEditEngine()->GetText(0))
                    {
                        eSelectEntry = eConfidentialEntry;
                        bFound = true;
                    }
                }
            }
            break;

            //TODO
            case eFileNamePageEntry:
            {
            }
            break;

            case eExtFileNameEntry:
            {
                aLeftEntry = pLeftObj->GetText(0);
                aRightEntry = pRightObj->GetText(0);
                if(IsExtFileNameEntry(pCenterObj.get()) && aLeftEntry == EMPTY_OUSTRING
                    && aRightEntry == EMPTY_OUSTRING)
                {
                    eSelectEntry = eExtFileNameEntry;
                    bFound = true;
                }
            }
            break;

            //TODO
            case ePageSheetEntry:
            {
            }
            break;

            //TODO
            case ePageFileNameEntry:
            {
            }
            break;

            case ePageExtFileNameEntry:
            {
                aLeftEntry = pLeftObj->GetText(0);
                if(IsPageEntry(m_pWndCenter->GetEditEngine(), pCenterObj.get()) &&
                    IsExtFileNameEntry(pRightObj.get()) && aLeftEntry == EMPTY_OUSTRING)
                {
                    eSelectEntry = ePageExtFileNameEntry;
                    bFound = true;
                }
            }
            break;

            case eUserNameEntry:
            {
                if(IsDateEntry(pRightObj.get()) && IsPageEntry(m_pWndCenter->GetEditEngine(), pCenterObj.get()))
                {
                    OUString aUserNameEntry(aUserOpt.GetFirstName() + " " + aUserOpt.GetLastName());

                    if(aUserNameEntry == m_pWndLeft->GetEditEngine()->GetText(0))
                    {
                        eSelectEntry = eUserNameEntry;
                        bFound = true;
                    }
                }
            }
            break;

            case eCreatedByEntry:
            {
                if(IsDateEntry(pCenterObj.get()) && IsPageEntry(m_pWndRight->GetEditEngine(), pRightObj.get()))
                {
                    OUString aCreatedByEntry(m_pFtCreatedBy->GetText() + " " + aUserOpt.GetFirstName() + " " + aUserOpt.GetLastName());

                    if(aCreatedByEntry == m_pWndLeft->GetEditEngine()->GetText(0))
                    {
                        eSelectEntry = eCreatedByEntry;
                        bFound = true;
                    }
                }
            }
            break;

            default:
            {
                // added to avoid warnings
            }
        }
    }

    if(eSelectEntry == eEntryCount)
        InsertToDefinedList();

    m_pLbDefined->SelectEntryPos( sal::static_int_cast<sal_uInt16>( eSelectEntry ) );
}

bool ScHFEditPage::IsPageEntry(EditEngine*pEngine, const EditTextObject* pTextObj)
{
    if(!pEngine || !pTextObj)
        return false;

    bool bReturn = false;

    if(!pTextObj->IsFieldObject())
    {
        std::vector<sal_Int32> aPosList;
        pEngine->GetPortions(0,aPosList);
        if(aPosList.size() == 2)
        {
            OUString aPageEntry(m_pFtPage->GetText() + " ");
            ESelection aSel(0,0,0,0);
            aSel.nEndPos = aPageEntry.getLength();
            if(aPageEntry == pEngine->GetText(aSel))
            {
                aSel.nStartPos = aSel.nEndPos;
                aSel.nEndPos++;
                std::unique_ptr< EditTextObject > pPageObj = pEngine->CreateTextObject(aSel);
                if(pPageObj.get() && pPageObj->IsFieldObject() )
                {
                    const SvxFieldItem* pFieldItem = pPageObj->GetField();
                    if(pFieldItem)
                    {
                        const SvxFieldData* pField = pFieldItem->GetField();
                        if(pField && dynamic_cast<const SvxPageField*>( pField) !=  nullptr)
                            bReturn = true;
                    }
                }
            }
        }
    }
    return bReturn;
}

bool ScHFEditPage::IsDateEntry(const EditTextObject* pTextObj)
{
    if(!pTextObj)
        return false;

    bool bReturn = false;
    if(pTextObj->IsFieldObject())
    {
        const SvxFieldItem* pFieldItem = pTextObj->GetField();
        if(pFieldItem)
        {
            const SvxFieldData* pField = pFieldItem->GetField();
            if(pField && dynamic_cast<const SvxDateField*>( pField) !=  nullptr)
                bReturn = true;
        }
    }
    return bReturn;
}

bool ScHFEditPage::IsExtFileNameEntry(const EditTextObject* pTextObj)
{
    if(!pTextObj)
        return false;
    bool bReturn = false;
    if(pTextObj->IsFieldObject())
    {
        const SvxFieldItem* pFieldItem = pTextObj->GetField();
        if(pFieldItem)
    {
            const SvxFieldData* pField = pFieldItem->GetField();
            if(pField && dynamic_cast<const SvxExtFileField*>( pField) !=  nullptr)
                bReturn = true;
        }
    }
    return bReturn;
}

void ScHFEditPage::ProcessDefinedListSel(ScHFEntryId eSel, bool bTravelling)
{
    SvtUserOptions aUserOpt;
    std::unique_ptr< EditTextObject > pTextObj;

    switch(eSel)
    {
        case eNoneEntry:
            ClearTextAreas();
            if(!bTravelling)
                m_pWndLeft->GrabFocus();
        break;

        case ePageEntry:
        {
            ClearTextAreas();
            OUString aPageEntry( m_pFtPage->GetText() + " " );
            m_pWndCenter->GetEditEngine()->SetText(aPageEntry);
            m_pWndCenter->InsertField( SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD) );
            if(!bTravelling)
                m_pWndCenter->GrabFocus();
        }
        break;

        case ePagesEntry:
        {
            ClearTextAreas();
            ESelection aSel(0,0,0,0);
            OUString aPageEntry( m_pFtPage->GetText() + " ");
            m_pWndCenter->GetEditEngine()->SetText(aPageEntry);
            aSel.nEndPos = aPageEntry.getLength();
            m_pWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            ++aSel.nEndPos;

            OUString aPageOfEntry(" " + m_pFtOf->GetText() + " ");
            m_pWndCenter->GetEditEngine()->QuickInsertText(aPageOfEntry,ESelection(aSel.nEndPara,aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            aSel.nEndPos = aSel.nEndPos + aPageOfEntry.getLength();
            m_pWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPagesField(), EE_FEATURE_FIELD), ESelection(aSel.nEndPara,aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            pTextObj = m_pWndCenter->GetEditEngine()->CreateTextObject();
            m_pWndCenter->SetText(*pTextObj);
            if(!bTravelling)
                m_pWndCenter->GrabFocus();
        }
        break;

        case eSheetEntry:
            ClearTextAreas();
            m_pWndCenter->InsertField( SvxFieldItem(SvxTableField(), EE_FEATURE_FIELD) );
            if(!bTravelling)
                m_pWndCenter->GrabFocus();
        break;

        case eConfidentialEntry:
        {
            ClearTextAreas();
            OUString aConfidentialEntry(aUserOpt.GetCompany() + " " + m_pFtConfidential->GetText());
            m_pWndLeft->GetEditEngine()->SetText(aConfidentialEntry);
            m_pWndCenter->InsertField( SvxFieldItem(SvxDateField(Date( Date::SYSTEM ),SvxDateType::Var), EE_FEATURE_FIELD) );

            OUString aPageEntry( m_pFtPage->GetText() + " ");
            m_pWndRight->GetEditEngine()->SetText(aPageEntry);
            m_pWndRight->InsertField( SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD) );
            if(!bTravelling)
                m_pWndRight->GrabFocus();
        }
        break;

        case eFileNamePageEntry:
        {
            ClearTextAreas();
            ESelection aSel(0,0,0,0);
            m_pWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem( SvxFileField(), EE_FEATURE_FIELD ), aSel );
            ++aSel.nEndPos;
            OUString aPageEntry(", " + m_pFtPage->GetText() + " ");
            m_pWndCenter->GetEditEngine()->QuickInsertText(aPageEntry, ESelection(aSel.nEndPara,aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            aSel.nStartPos = aSel.nEndPos;
            aSel.nEndPos = aSel.nEndPos + aPageEntry.getLength();
            m_pWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), ESelection(aSel.nEndPara,aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            pTextObj = m_pWndCenter->GetEditEngine()->CreateTextObject();
            m_pWndCenter->SetText(*pTextObj);
            if(!bTravelling)
                m_pWndCenter->GrabFocus();
        }
        break;

        case eExtFileNameEntry:
            ClearTextAreas();
            m_pWndCenter->InsertField( SvxFieldItem( SvxExtFileField(
                EMPTY_OUSTRING, SvxFileType::Var, SvxFileFormat::PathFull ), EE_FEATURE_FIELD ) );
            if(!bTravelling)
                m_pWndCenter->GrabFocus();
        break;

        case ePageSheetEntry:
        {
            ClearTextAreas();
            ESelection aSel(0,0,0,0);
            OUString aPageEntry( m_pFtPage->GetText() + " " );
            m_pWndCenter->GetEditEngine()->SetText(aPageEntry);
            aSel.nEndPos = aPageEntry.getLength();
            m_pWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            ++aSel.nEndPos;

            OUString aCommaSpace(", ");
            m_pWndCenter->GetEditEngine()->QuickInsertText(aCommaSpace,ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            aSel.nEndPos = aSel.nEndPos + aCommaSpace.getLength();
            m_pWndCenter->GetEditEngine()->QuickInsertField( SvxFieldItem(SvxTableField(), EE_FEATURE_FIELD), ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            pTextObj = m_pWndCenter->GetEditEngine()->CreateTextObject();
            m_pWndCenter->SetText(*pTextObj);
            if(!bTravelling)
                m_pWndCenter->GrabFocus();
        }
        break;

        case ePageFileNameEntry:
        {
            ClearTextAreas();
            ESelection aSel(0,0,0,0);
            OUString aPageEntry( m_pFtPage->GetText() + " " );
            m_pWndCenter->GetEditEngine()->SetText(aPageEntry);
            aSel.nEndPos = aPageEntry.getLength();
            m_pWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            ++aSel.nEndPos;
            OUString aCommaSpace(", ");
            m_pWndCenter->GetEditEngine()->QuickInsertText(aCommaSpace,ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            aSel.nEndPos = aSel.nEndPos + aCommaSpace.getLength();
            m_pWndCenter->GetEditEngine()->QuickInsertField( SvxFieldItem(SvxFileField(), EE_FEATURE_FIELD), ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            pTextObj = m_pWndCenter->GetEditEngine()->CreateTextObject();
            m_pWndCenter->SetText(*pTextObj);
            if(!bTravelling)
                m_pWndCenter->GrabFocus();
        }
        break;

        case ePageExtFileNameEntry:
        {
            ClearTextAreas();
            OUString aPageEntry( m_pFtPage->GetText() + " " );
            m_pWndCenter->GetEditEngine()->SetText(aPageEntry);
            m_pWndCenter->InsertField( SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD) );
            m_pWndRight->InsertField( SvxFieldItem( SvxExtFileField(
                EMPTY_OUSTRING, SvxFileType::Var, SvxFileFormat::PathFull ), EE_FEATURE_FIELD ) );
            if(!bTravelling)
                m_pWndRight->GrabFocus();
        }
        break;

        case eUserNameEntry:
        {
            ClearTextAreas();
            OUString aUserNameEntry(aUserOpt.GetFirstName() + " " + aUserOpt.GetLastName());
            m_pWndLeft->GetEditEngine()->SetText(aUserNameEntry);
            OUString aPageEntry( m_pFtPage->GetText() + " ");
            //aPageEntry += " ";
            m_pWndCenter->GetEditEngine()->SetText(aPageEntry);
            m_pWndCenter->InsertField( SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD) );
            m_pWndRight->InsertField( SvxFieldItem(SvxDateField(Date( Date::SYSTEM ),SvxDateType::Var), EE_FEATURE_FIELD) );
            if(!bTravelling)
                m_pWndRight->GrabFocus();
        }
        break;

        case eCreatedByEntry:
        {
            ClearTextAreas();
            OUString aCreatedByEntry( m_pFtCreatedBy->GetText() + " " + aUserOpt.GetFirstName() + " " + aUserOpt.GetLastName());
            m_pWndLeft->GetEditEngine()->SetText(aCreatedByEntry);
            m_pWndCenter->InsertField( SvxFieldItem(SvxDateField(Date( Date::SYSTEM ),SvxDateType::Var), EE_FEATURE_FIELD) );
            OUString aPageEntry( m_pFtPage->GetText() );
            aPageEntry += " ";
            m_pWndRight->GetEditEngine()->SetText(aPageEntry);
            m_pWndRight->InsertField( SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD) );
            if(!bTravelling)
                m_pWndRight->GrabFocus();
        }
        break;

        default :
            break;
    }
}

void ScHFEditPage::ClearTextAreas()
{
    m_pWndLeft->GetEditEngine()->SetText(EMPTY_OUSTRING);
    m_pWndLeft->Invalidate();
    m_pWndCenter->GetEditEngine()->SetText(EMPTY_OUSTRING);
    m_pWndCenter->Invalidate();
    m_pWndRight->GetEditEngine()->SetText(EMPTY_OUSTRING);
    m_pWndRight->Invalidate();
}

// Handler:

IMPL_LINK( ScHFEditPage, ListHdl_Impl, ListBox&, rList, void )
{
    if ( &rList == m_pLbDefined )
    {
        ScHFEntryId eSel = static_cast<ScHFEntryId>(m_pLbDefined->GetSelectedEntryPos());
        if(!m_pLbDefined->IsTravelSelect())
        {
            ProcessDefinedListSel(eSel, false);

            // check if we need to remove the customized entry.
            if(eSel < eEntryCount)
                RemoveFromDefinedList();
        }
        else
        {
            ProcessDefinedListSel(eSel, true);
        }
    }
}

IMPL_LINK( ScHFEditPage, ClickHdl, Button*, pBtn, void )
{
    if (!m_pEditFocus)
        return;

    if ( pBtn == m_pBtnText )
    {
        m_pEditFocus->SetCharAttributes();
    }
    else
    {
        if ( pBtn == m_pBtnPage )
            m_pEditFocus->InsertField(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD));
        else if ( pBtn == m_pBtnLastPage )
            m_pEditFocus->InsertField(SvxFieldItem(SvxPagesField(), EE_FEATURE_FIELD));
        else if ( pBtn == m_pBtnDate )
            m_pEditFocus->InsertField(SvxFieldItem(SvxDateField(Date(Date::SYSTEM),SvxDateType::Var), EE_FEATURE_FIELD));
        else if ( pBtn == m_pBtnTime )
            m_pEditFocus->InsertField(SvxFieldItem(SvxTimeField(), EE_FEATURE_FIELD));
        else if ( pBtn == m_pBtnFile )
        {
            m_pEditFocus->InsertField(SvxFieldItem(SvxFileField(), EE_FEATURE_FIELD));
        }
        else if ( pBtn == m_pBtnTable )
            m_pEditFocus->InsertField(SvxFieldItem(SvxTableField(), EE_FEATURE_FIELD));
    }
    InsertToDefinedList();
    m_pEditFocus->GrabFocus();
}

IMPL_LINK( ScHFEditPage, MenuHdl, ScExtIButton&, rBtn, void )
{
    if (!m_pEditFocus)
        return;

    SAL_WARN_IF(rBtn.GetSelected() == 0, "sc.ui", "nothing selected");
    OString sSelectedId = rBtn.GetSelectedIdent();

    if (sSelectedId == "title")
    {
        m_pEditFocus->InsertField(SvxFieldItem(SvxFileField(), EE_FEATURE_FIELD));
    }
    else if (sSelectedId == "filename")
    {
        m_pEditFocus->InsertField( SvxFieldItem( SvxExtFileField(
            OUString(), SvxFileType::Var, SvxFileFormat::NameAndExt ), EE_FEATURE_FIELD ) );
    }
    else if (sSelectedId == "pathname")
    {
        m_pEditFocus->InsertField( SvxFieldItem( SvxExtFileField(
            OUString(), SvxFileType::Var, SvxFileFormat::PathFull ), EE_FEATURE_FIELD ) );
    }
}

// class ScRightHeaderEditPage

ScRightHeaderEditPage::ScRightHeaderEditPage( vcl::Window* pParent, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pParent,
                    rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_HEADERRIGHT ),
                    true )
    {}

VclPtr<SfxTabPage> ScRightHeaderEditPage::Create( TabPageParent pParent, const SfxItemSet* rCoreSet )
{
    return VclPtr<ScRightHeaderEditPage>::Create( pParent.pParent, *rCoreSet );
}

// class ScLeftHeaderEditPage

ScLeftHeaderEditPage::ScLeftHeaderEditPage( vcl::Window* pParent, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pParent,
                    rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_HEADERLEFT ),
                    true )
    {}

VclPtr<SfxTabPage> ScLeftHeaderEditPage::Create( TabPageParent pParent, const SfxItemSet* rCoreSet )
{
    return VclPtr<ScLeftHeaderEditPage>::Create( pParent.pParent, *rCoreSet );
}

// class ScRightFooterEditPage

ScRightFooterEditPage::ScRightFooterEditPage( vcl::Window* pParent, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pParent,
                    rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_FOOTERRIGHT ),
                    false )
    {}

VclPtr<SfxTabPage> ScRightFooterEditPage::Create( TabPageParent pParent, const SfxItemSet* rCoreSet )
{
    return VclPtr<ScRightFooterEditPage>::Create( pParent.pParent, *rCoreSet );
}

// class ScLeftFooterEditPage

ScLeftFooterEditPage::ScLeftFooterEditPage( vcl::Window* pParent, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pParent,
                    rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_FOOTERLEFT ),
                    false )
    {}

VclPtr<SfxTabPage> ScLeftFooterEditPage::Create( TabPageParent pParent, const SfxItemSet* rCoreSet )
{
    return VclPtr<ScLeftFooterEditPage>::Create( pParent.pParent, *rCoreSet );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
