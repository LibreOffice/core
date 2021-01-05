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
#include <editeng/flditem.hxx>
#include <osl/time.h>
#include <sfx2/tabdlg.hxx>
#include <vcl/settings.hxx>

#include <unotools/useroptions.hxx>

#include <editutil.hxx>
#include <global.hxx>
#include <attrib.hxx>
#include <patattr.hxx>

#include <scuitphfedit.hxx>
#include <memory>


ScHFEditPage::ScHFEditPage(weld::Container* pPage, weld::DialogController* pController,
                           const SfxItemSet& rCoreAttrs,
                           sal_uInt16 nWhichId,
                           bool bHeader)
    : SfxTabPage(pPage, pController, "modules/scalc/ui/headerfootercontent.ui", "HeaderFooterContent", &rCoreAttrs)
    , nWhich( nWhichId )
    , m_bDropDownActive(false)
    , m_nTimeToggled(-1)
    , m_xFtDefinedHF(m_xBuilder->weld_label(!bHeader ? "labelFT_F_DEFINED" : "labelFT_H_DEFINED"))
    , m_xLbDefined(m_xBuilder->weld_combo_box("comboLB_DEFINED"))
    , m_xFtCustomHF(m_xBuilder->weld_label(!bHeader ? "labelFT_F_CUSTOM" : "labelFT_H_CUSTOM"))
    , m_xBtnText(m_xBuilder->weld_button("buttonBTN_TEXT"))
    , m_xBtnFile(m_xBuilder->weld_menu_button("buttonBTN_FILE"))
    , m_xBtnTable(m_xBuilder->weld_button("buttonBTN_TABLE"))
    , m_xBtnPage(m_xBuilder->weld_button("buttonBTN_PAGE"))
    , m_xBtnLastPage(m_xBuilder->weld_button("buttonBTN_PAGES"))
    , m_xBtnDate(m_xBuilder->weld_button("buttonBTN_DATE"))
    , m_xBtnTime(m_xBuilder->weld_button("buttonBTN_TIME"))
    , m_xFtConfidential(m_xBuilder->weld_label("labelSTR_HF_CONFIDENTIAL"))
    , m_xFtPage(m_xBuilder->weld_label("labelSTR_PAGE"))
    , m_xFtOfQuestion(m_xBuilder->weld_label("labelSTR_HF_OF_QUESTION"))
    , m_xFtOf(m_xBuilder->weld_label("labelSTR_HF_OF"))
    , m_xFtNone(m_xBuilder->weld_label("labelSTR_HF_NONE_IN_BRACKETS"))
    , m_xFtCreatedBy(m_xBuilder->weld_label("labelSTR_HF_CREATED_BY"))
    , m_xFtCustomized(m_xBuilder->weld_label("labelSTR_HF_CUSTOMIZED"))
    , m_xLeft(m_xBuilder->weld_widget("labelFT_LEFT"))
    , m_xRight(m_xBuilder->weld_widget("labelFT_RIGHT"))
    , m_xWndLeft(new ScEditWindow(Left, pController->getDialog()))
    , m_xWndCenter(new ScEditWindow(Center, pController->getDialog()))
    , m_xWndRight(new ScEditWindow(Right, pController->getDialog()))
    , m_xWndLeftWnd(new weld::CustomWeld(*m_xBuilder, "textviewWND_LEFT", *m_xWndLeft))
    , m_xWndCenterWnd(new weld::CustomWeld(*m_xBuilder, "textviewWND_CENTER", *m_xWndCenter))
    , m_xWndRightWnd(new weld::CustomWeld(*m_xBuilder, "textviewWND_RIGHT", *m_xWndRight))
    , m_pEditFocus(nullptr)
{
    // tdf#114695 override natural size with a small value
    // we expect this to get stretched to some larger but
    // limited size based on surrounding widgets
    m_xLbDefined->set_size_request(m_xLbDefined->get_approximate_digit_width() * 20, -1);

    //! use default style from current document?
    //! if font color is used, header/footer background color must be set

    ScPatternAttr aPatAttr( rCoreAttrs.GetPool() );

    m_xLbDefined->connect_popup_toggled( LINK( this, ScHFEditPage, ListToggleHdl_Impl) );
    m_xLbDefined->connect_changed( LINK( this, ScHFEditPage, ListHdl_Impl ) );
    m_xBtnFile->connect_selected( LINK( this, ScHFEditPage, MenuHdl ) );
    m_xBtnText->connect_clicked( LINK( this, ScHFEditPage, ClickHdl ) );
    m_xBtnPage->connect_clicked( LINK( this, ScHFEditPage, ClickHdl ) );
    m_xBtnLastPage->connect_clicked( LINK( this, ScHFEditPage, ClickHdl ) );
    m_xBtnDate->connect_clicked( LINK( this, ScHFEditPage, ClickHdl ) );
    m_xBtnTime->connect_clicked( LINK( this, ScHFEditPage, ClickHdl ) );
    m_xBtnTable->connect_clicked( LINK( this, ScHFEditPage, ClickHdl ) );

    m_xFtDefinedHF->show();
    m_xFtCustomHF->show();

    //swap left/right areas and their labels in RTL mode
    if( AllSettings::GetLayoutRTL() )
    {
        sal_Int32 nOldLeftAttach = m_xLeft->get_grid_left_attach();
        sal_Int32 nOldRightAttach = m_xRight->get_grid_left_attach();
        m_xLeft->set_grid_left_attach(nOldRightAttach);
        m_xRight->set_grid_left_attach(nOldLeftAttach);

        nOldLeftAttach = m_xWndLeftWnd->get_grid_left_attach();
        nOldRightAttach = m_xWndRightWnd->get_grid_left_attach();
        m_xWndLeftWnd->set_grid_left_attach(nOldRightAttach);
        m_xWndRightWnd->set_grid_left_attach(nOldLeftAttach);
    }
    m_xWndLeft->SetFont( aPatAttr );
    m_xWndCenter->SetFont( aPatAttr );
    m_xWndRight->SetFont( aPatAttr );

    m_xWndLeft->SetObjectSelectHdl( LINK(this,ScHFEditPage,ObjectSelectHdl) );
    m_xWndCenter->SetObjectSelectHdl( LINK(this,ScHFEditPage,ObjectSelectHdl) );
    m_xWndRight->SetObjectSelectHdl( LINK(this,ScHFEditPage,ObjectSelectHdl) );
    auto setEditFocus = [this](ScEditWindow & rEdit) { this->m_pEditFocus = &rEdit; };
    m_xWndLeft->SetGetFocusHdl(setEditFocus);
    m_xWndCenter->SetGetFocusHdl(setEditFocus);
    m_xWndRight->SetGetFocusHdl(setEditFocus);

    m_xWndLeft->GrabFocus();
    m_pEditFocus = m_xWndLeft.get(); // there's no event from grab_focus()

    InitPreDefinedList();

}

IMPL_LINK_NOARG( ScHFEditPage, ObjectSelectHdl, ScEditWindow&, void )
{
    m_xBtnText->grab_focus();
}

ScHFEditPage::~ScHFEditPage()
{
}

void ScHFEditPage::SetNumType(SvxNumType eNumType)
{
    m_xWndLeft->SetNumType(eNumType);
    m_xWndCenter->SetNumType(eNumType);
    m_xWndRight->SetNumType(eNumType);
}

void ScHFEditPage::Reset( const SfxItemSet* rCoreSet )
{
    const SfxPoolItem* pItem = nullptr;
    if ( !rCoreSet->HasItem(nWhich, &pItem) )
        return;

    const ScPageHFItem& rItem = static_cast<const ScPageHFItem&>(*pItem);

    if( const EditTextObject* pLeft = rItem.GetLeftArea() )
        m_xWndLeft->SetText( *pLeft );
    if( const EditTextObject* pCenter = rItem.GetCenterArea() )
        m_xWndCenter->SetText( *pCenter );
    if( const EditTextObject* pRight = rItem.GetRightArea() )
        m_xWndRight->SetText( *pRight );

    SetSelectDefinedList();
}

bool ScHFEditPage::FillItemSet( SfxItemSet* rCoreSet )
{
    ScPageHFItem    aItem( nWhich );
    std::unique_ptr<EditTextObject> pLeft   = m_xWndLeft->CreateTextObject();
    std::unique_ptr<EditTextObject> pCenter = m_xWndCenter->CreateTextObject();
    std::unique_ptr<EditTextObject> pRight  = m_xWndRight->CreateTextObject();

    aItem.SetLeftArea  ( *pLeft );
    aItem.SetCenterArea( *pCenter );
    aItem.SetRightArea ( *pRight );

    rCoreSet->Put( aItem );

    return true;
}

void ScHFEditPage::InitPreDefinedList()
{
    SvtUserOptions aUserOpt;

    std::optional<Color> pTxtColour;
    std::optional<Color> pFldColour;

    // Get the all field values at the outset.
    OUString aPageFieldValue(m_xWndLeft->GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), 0,0, pTxtColour, pFldColour));
    OUString aSheetFieldValue(m_xWndLeft->GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxTableField(), EE_FEATURE_FIELD), 0,0, pTxtColour, pFldColour));
    OUString aFileFieldValue(m_xWndLeft->GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxFileField(), EE_FEATURE_FIELD), 0,0, pTxtColour, pFldColour));
    OUString aExtFileFieldValue(m_xWndLeft->GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxExtFileField(), EE_FEATURE_FIELD), 0,0, pTxtColour, pFldColour));
    OUString aDateFieldValue(m_xWndLeft->GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxDateField(), EE_FEATURE_FIELD), 0,0, pTxtColour, pFldColour));

    m_xLbDefined->clear();

    m_xLbDefined->append_text(m_xFtNone->get_label());

    OUString aPageEntry(m_xFtPage->get_label() + " " + aPageFieldValue);
    m_xLbDefined->append_text(aPageEntry);

    OUString aPageOfEntry(aPageEntry + " " + m_xFtOfQuestion->get_label());
    m_xLbDefined->append_text( aPageOfEntry);

    m_xLbDefined->append_text(aSheetFieldValue);

    OUString aConfidentialEntry(aUserOpt.GetCompany() + " " + m_xFtConfidential->get_label() + ", " + aDateFieldValue + ", " + aPageEntry);
    m_xLbDefined->append_text( aConfidentialEntry);

    OUString aFileNamePageEntry(aFileFieldValue + ", " + aPageEntry);
    m_xLbDefined->append_text( aFileNamePageEntry);

    m_xLbDefined->append_text( aExtFileFieldValue);

    OUString aPageSheetNameEntry(aPageEntry + ", " + aSheetFieldValue);
    m_xLbDefined->append_text( aPageSheetNameEntry);

    OUString aPageFileNameEntry(aPageEntry + ", " + aFileFieldValue);
    m_xLbDefined->append_text( aPageFileNameEntry);

    OUString aPagePathNameEntry(aPageEntry + ", " + aExtFileFieldValue);
    m_xLbDefined->append_text( aPagePathNameEntry);

    OUString aUserNameEntry(aUserOpt.GetFirstName() + " " + aUserOpt.GetLastName() + ", " + aPageEntry + ", " + aDateFieldValue);
    m_xLbDefined->append_text( aUserNameEntry);

    OUString aCreatedByEntry = m_xFtCreatedBy->get_label() + " " + aUserOpt.GetFirstName() + " " + aUserOpt.GetLastName() + ", " +
        aDateFieldValue + ", " + aPageEntry;
    m_xLbDefined->append_text( aCreatedByEntry);
}

void ScHFEditPage::InsertToDefinedList()
{
    const sal_Int32 nCount =  m_xLbDefined->get_count();
    if(nCount == eEntryCount)
    {
        m_xLbDefined->append_text( m_xFtCustomized->get_label() );
        m_xLbDefined->set_active(eEntryCount);
    }
}

void ScHFEditPage::RemoveFromDefinedList()
{
    const sal_Int32 nCount =  m_xLbDefined->get_count();
    if(nCount > eEntryCount )
        m_xLbDefined->remove( nCount-1);
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

    pLeftObj = m_xWndLeft->GetEditEngine()->CreateTextObject();
    pCenterObj = m_xWndCenter->GetEditEngine()->CreateTextObject();
    pRightObj = m_xWndRight->GetEditEngine()->CreateTextObject();

    bool bFound = false;

    const sal_Int32 nCount = m_xLbDefined->get_count();
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
                    if(IsPageEntry(m_xWndCenter->GetEditEngine(), pCenterObj.get()))
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
                            if(dynamic_cast<const SvxTableField*>( pField))
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
                if(IsDateEntry(pCenterObj.get()) && IsPageEntry(m_xWndRight->GetEditEngine(), pRightObj.get()))
                {
                    OUString aConfidentialEntry(aUserOpt.GetCompany() + " " + m_xFtConfidential->get_label());
                    if(aConfidentialEntry == m_xWndLeft->GetEditEngine()->GetText(0))
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
                if(IsPageEntry(m_xWndCenter->GetEditEngine(), pCenterObj.get()) &&
                    IsExtFileNameEntry(pRightObj.get()) && aLeftEntry == EMPTY_OUSTRING)
                {
                    eSelectEntry = ePageExtFileNameEntry;
                    bFound = true;
                }
            }
            break;

            case eUserNameEntry:
            {
                if(IsDateEntry(pRightObj.get()) && IsPageEntry(m_xWndCenter->GetEditEngine(), pCenterObj.get()))
                {
                    OUString aUserNameEntry(aUserOpt.GetFirstName() + " " + aUserOpt.GetLastName());

                    if(aUserNameEntry == m_xWndLeft->GetEditEngine()->GetText(0))
                    {
                        eSelectEntry = eUserNameEntry;
                        bFound = true;
                    }
                }
            }
            break;

            case eCreatedByEntry:
            {
                if(IsDateEntry(pCenterObj.get()) && IsPageEntry(m_xWndRight->GetEditEngine(), pRightObj.get()))
                {
                    OUString aCreatedByEntry(m_xFtCreatedBy->get_label() + " " + aUserOpt.GetFirstName() + " " + aUserOpt.GetLastName());

                    if(aCreatedByEntry == m_xWndLeft->GetEditEngine()->GetText(0))
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

    m_xLbDefined->set_active( sal::static_int_cast<sal_uInt16>( eSelectEntry ) );
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
            OUString aPageEntry(m_xFtPage->get_label() + " ");
            ESelection aSel(0,0,0,0);
            aSel.nEndPos = aPageEntry.getLength();
            if(aPageEntry == pEngine->GetText(aSel))
            {
                aSel.nStartPos = aSel.nEndPos;
                aSel.nEndPos++;
                std::unique_ptr< EditTextObject > pPageObj = pEngine->CreateTextObject(aSel);
                if(pPageObj && pPageObj->IsFieldObject() )
                {
                    const SvxFieldItem* pFieldItem = pPageObj->GetField();
                    if(pFieldItem)
                    {
                        const SvxFieldData* pField = pFieldItem->GetField();
                        if(dynamic_cast<const SvxPageField*>( pField))
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
            if(dynamic_cast<const SvxDateField*>( pField))
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
            if(dynamic_cast<const SvxExtFileField*>( pField))
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
                m_xWndLeft->GrabFocus();
        break;

        case ePageEntry:
        {
            ClearTextAreas();
            OUString aPageEntry( m_xFtPage->get_label() + " " );
            m_xWndCenter->GetEditEngine()->SetTextCurrentDefaults(aPageEntry);
            m_xWndCenter->InsertField( SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD) );
            if(!bTravelling)
                m_xWndCenter->GrabFocus();
        }
        break;

        case ePagesEntry:
        {
            ClearTextAreas();
            ESelection aSel(0,0,0,0);
            OUString aPageEntry( m_xFtPage->get_label() + " ");
            m_xWndCenter->GetEditEngine()->SetTextCurrentDefaults(aPageEntry);
            aSel.nEndPos = aPageEntry.getLength();
            m_xWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            ++aSel.nEndPos;

            OUString aPageOfEntry(" " + m_xFtOf->get_label() + " ");
            m_xWndCenter->GetEditEngine()->QuickInsertText(aPageOfEntry,ESelection(aSel.nEndPara,aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            aSel.nEndPos = aSel.nEndPos + aPageOfEntry.getLength();
            m_xWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPagesField(), EE_FEATURE_FIELD), ESelection(aSel.nEndPara,aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            pTextObj = m_xWndCenter->GetEditEngine()->CreateTextObject();
            m_xWndCenter->SetText(*pTextObj);
            if(!bTravelling)
                m_xWndCenter->GrabFocus();
        }
        break;

        case eSheetEntry:
            ClearTextAreas();
            m_xWndCenter->InsertField( SvxFieldItem(SvxTableField(), EE_FEATURE_FIELD) );
            if(!bTravelling)
                m_xWndCenter->GrabFocus();
        break;

        case eConfidentialEntry:
        {
            ClearTextAreas();
            OUString aConfidentialEntry(aUserOpt.GetCompany() + " " + m_xFtConfidential->get_label());
            m_xWndLeft->GetEditEngine()->SetTextCurrentDefaults(aConfidentialEntry);
            m_xWndCenter->InsertField( SvxFieldItem(SvxDateField(Date( Date::SYSTEM ),SvxDateType::Var), EE_FEATURE_FIELD) );

            OUString aPageEntry( m_xFtPage->get_label() + " ");
            m_xWndRight->GetEditEngine()->SetTextCurrentDefaults(aPageEntry);
            m_xWndRight->InsertField( SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD) );
            if(!bTravelling)
                m_xWndRight->GrabFocus();
        }
        break;

        case eFileNamePageEntry:
        {
            ClearTextAreas();
            ESelection aSel(0,0,0,0);
            m_xWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem( SvxFileField(), EE_FEATURE_FIELD ), aSel );
            ++aSel.nEndPos;
            OUString aPageEntry(", " + m_xFtPage->get_label() + " ");
            m_xWndCenter->GetEditEngine()->QuickInsertText(aPageEntry, ESelection(aSel.nEndPara,aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            aSel.nStartPos = aSel.nEndPos;
            aSel.nEndPos = aSel.nEndPos + aPageEntry.getLength();
            m_xWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), ESelection(aSel.nEndPara,aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            pTextObj = m_xWndCenter->GetEditEngine()->CreateTextObject();
            m_xWndCenter->SetText(*pTextObj);
            if(!bTravelling)
                m_xWndCenter->GrabFocus();
        }
        break;

        case eExtFileNameEntry:
            ClearTextAreas();
            m_xWndCenter->InsertField( SvxFieldItem( SvxExtFileField(
                EMPTY_OUSTRING, SvxFileType::Var, SvxFileFormat::PathFull ), EE_FEATURE_FIELD ) );
            if(!bTravelling)
                m_xWndCenter->GrabFocus();
        break;

        case ePageSheetEntry:
        {
            ClearTextAreas();
            ESelection aSel(0,0,0,0);
            OUString aPageEntry( m_xFtPage->get_label() + " " );
            m_xWndCenter->GetEditEngine()->SetTextCurrentDefaults(aPageEntry);
            aSel.nEndPos = aPageEntry.getLength();
            m_xWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            ++aSel.nEndPos;

            OUString aCommaSpace(", ");
            m_xWndCenter->GetEditEngine()->QuickInsertText(aCommaSpace,ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            aSel.nEndPos = aSel.nEndPos + aCommaSpace.getLength();
            m_xWndCenter->GetEditEngine()->QuickInsertField( SvxFieldItem(SvxTableField(), EE_FEATURE_FIELD), ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            pTextObj = m_xWndCenter->GetEditEngine()->CreateTextObject();
            m_xWndCenter->SetText(*pTextObj);
            if(!bTravelling)
                m_xWndCenter->GrabFocus();
        }
        break;

        case ePageFileNameEntry:
        {
            ClearTextAreas();
            ESelection aSel(0,0,0,0);
            OUString aPageEntry( m_xFtPage->get_label() + " " );
            m_xWndCenter->GetEditEngine()->SetTextCurrentDefaults(aPageEntry);
            aSel.nEndPos = aPageEntry.getLength();
            m_xWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            ++aSel.nEndPos;
            OUString aCommaSpace(", ");
            m_xWndCenter->GetEditEngine()->QuickInsertText(aCommaSpace,ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            aSel.nEndPos = aSel.nEndPos + aCommaSpace.getLength();
            m_xWndCenter->GetEditEngine()->QuickInsertField( SvxFieldItem(SvxFileField(), EE_FEATURE_FIELD), ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            pTextObj = m_xWndCenter->GetEditEngine()->CreateTextObject();
            m_xWndCenter->SetText(*pTextObj);
            if(!bTravelling)
                m_xWndCenter->GrabFocus();
        }
        break;

        case ePageExtFileNameEntry:
        {
            ClearTextAreas();
            OUString aPageEntry( m_xFtPage->get_label() + " " );
            m_xWndCenter->GetEditEngine()->SetTextCurrentDefaults(aPageEntry);
            m_xWndCenter->InsertField( SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD) );
            m_xWndRight->InsertField( SvxFieldItem( SvxExtFileField(
                EMPTY_OUSTRING, SvxFileType::Var, SvxFileFormat::PathFull ), EE_FEATURE_FIELD ) );
            if(!bTravelling)
                m_xWndRight->GrabFocus();
        }
        break;

        case eUserNameEntry:
        {
            ClearTextAreas();
            OUString aUserNameEntry(aUserOpt.GetFirstName() + " " + aUserOpt.GetLastName());
            m_xWndLeft->GetEditEngine()->SetTextCurrentDefaults(aUserNameEntry);
            OUString aPageEntry( m_xFtPage->get_label() + " ");
            //aPageEntry += " ";
            m_xWndCenter->GetEditEngine()->SetTextCurrentDefaults(aPageEntry);
            m_xWndCenter->InsertField( SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD) );
            m_xWndRight->InsertField( SvxFieldItem(SvxDateField(Date( Date::SYSTEM ),SvxDateType::Var), EE_FEATURE_FIELD) );
            if(!bTravelling)
                m_xWndRight->GrabFocus();
        }
        break;

        case eCreatedByEntry:
        {
            ClearTextAreas();
            OUString aCreatedByEntry( m_xFtCreatedBy->get_label() + " " + aUserOpt.GetFirstName() + " " + aUserOpt.GetLastName());
            m_xWndLeft->GetEditEngine()->SetTextCurrentDefaults(aCreatedByEntry);
            m_xWndCenter->InsertField( SvxFieldItem(SvxDateField(Date( Date::SYSTEM ),SvxDateType::Var), EE_FEATURE_FIELD) );
            OUString aPageEntry( m_xFtPage->get_label() + " " );
            m_xWndRight->GetEditEngine()->SetTextCurrentDefaults(aPageEntry);
            m_xWndRight->InsertField( SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD) );
            if(!bTravelling)
                m_xWndRight->GrabFocus();
        }
        break;

        default :
            break;
    }
}

void ScHFEditPage::ClearTextAreas()
{
    m_xWndLeft->GetEditEngine()->SetTextCurrentDefaults(EMPTY_OUSTRING);
    m_xWndLeft->Invalidate();
    m_xWndCenter->GetEditEngine()->SetTextCurrentDefaults(EMPTY_OUSTRING);
    m_xWndCenter->Invalidate();
    m_xWndRight->GetEditEngine()->SetTextCurrentDefaults(EMPTY_OUSTRING);
    m_xWndRight->Invalidate();
}

// Handler:
IMPL_LINK_NOARG(ScHFEditPage, ListToggleHdl_Impl, weld::ComboBox&, void)
{
    m_bDropDownActive = !m_bDropDownActive;
    TimeValue aNow;
    osl_getSystemTime(&aNow);
    m_nTimeToggled = sal_Int64(aNow.Seconds) * 1000000000 + aNow.Nanosec;
}

IMPL_LINK_NOARG(ScHFEditPage, ListHdl_Impl, weld::ComboBox&, void)
{
    ScHFEntryId eSel = static_cast<ScHFEntryId>(m_xLbDefined->get_active());

    TimeValue aNow;
    osl_getSystemTime(&aNow);
    sal_Int64 nNow = sal_Int64(aNow.Seconds) * 1000000000 + aNow.Nanosec;

    // order of dropdown vs select not guaranteed
    bool bDiscrepancy = m_xLbDefined->get_popup_shown() != m_bDropDownActive;
    if (bDiscrepancy)
        ListToggleHdl_Impl(*m_xLbDefined);

    bool bFocusToTarget = !m_xLbDefined->get_popup_shown() && m_nTimeToggled != -1 && (nNow - m_nTimeToggled < 800000000);
    ProcessDefinedListSel(eSel, !bFocusToTarget);
    // check if we need to remove the customized entry.
    if (!m_bDropDownActive && eSel < eEntryCount)
        RemoveFromDefinedList();

    // keep balanced
    if (bDiscrepancy)
        ListToggleHdl_Impl(*m_xLbDefined);
}

IMPL_LINK( ScHFEditPage, ClickHdl, weld::Button&, rBtn, void )
{
    if (!m_pEditFocus)
        return;

    if (&rBtn == m_xBtnText.get())
    {
        m_pEditFocus->SetCharAttributes();
    }
    else
    {
        if ( &rBtn == m_xBtnPage.get() )
            m_pEditFocus->InsertField(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD));
        else if ( &rBtn == m_xBtnLastPage.get() )
            m_pEditFocus->InsertField(SvxFieldItem(SvxPagesField(), EE_FEATURE_FIELD));
        else if ( &rBtn == m_xBtnDate.get() )
            m_pEditFocus->InsertField(SvxFieldItem(SvxDateField(Date(Date::SYSTEM),SvxDateType::Var), EE_FEATURE_FIELD));
        else if ( &rBtn == m_xBtnTime.get() )
            m_pEditFocus->InsertField(SvxFieldItem(SvxTimeField(), EE_FEATURE_FIELD));
        else if ( &rBtn == m_xBtnTable.get() )
            m_pEditFocus->InsertField(SvxFieldItem(SvxTableField(), EE_FEATURE_FIELD));
    }
    InsertToDefinedList();
    m_pEditFocus->GrabFocus();
}

IMPL_LINK(ScHFEditPage, MenuHdl, const OString&, rSelectedId, void)
{
    if (!m_pEditFocus)
        return;

    if (rSelectedId == "title")
    {
        m_pEditFocus->InsertField(SvxFieldItem(SvxFileField(), EE_FEATURE_FIELD));
    }
    else if (rSelectedId == "filename")
    {
        m_pEditFocus->InsertField( SvxFieldItem( SvxExtFileField(
            OUString(), SvxFileType::Var, SvxFileFormat::NameAndExt ), EE_FEATURE_FIELD ) );
    }
    else if (rSelectedId == "pathname")
    {
        m_pEditFocus->InsertField( SvxFieldItem( SvxExtFileField(
            OUString(), SvxFileType::Var, SvxFileFormat::PathFull ), EE_FEATURE_FIELD ) );
    }
}


ScRightHeaderEditPage::ScRightHeaderEditPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pPage, pController,
                    rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_HEADERRIGHT ),
                    true )
    {}

std::unique_ptr<SfxTabPage> ScRightHeaderEditPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet )
{
    return std::make_unique<ScRightHeaderEditPage>( pPage, pController, *rCoreSet );
}


ScLeftHeaderEditPage::ScLeftHeaderEditPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pPage, pController,
                    rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_HEADERLEFT ),
                    true )
    {}

std::unique_ptr<SfxTabPage> ScLeftHeaderEditPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet )
{
    return std::make_unique<ScLeftHeaderEditPage>( pPage, pController, *rCoreSet );
}


ScRightFooterEditPage::ScRightFooterEditPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pPage, pController,
                    rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_FOOTERRIGHT ),
                    false )
    {}

std::unique_ptr<SfxTabPage> ScRightFooterEditPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet )
{
    return std::make_unique<ScRightFooterEditPage>( pPage, pController, *rCoreSet );
}


ScLeftFooterEditPage::ScLeftFooterEditPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pPage, pController,
                    rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_FOOTERLEFT ),
                    false )
    {}

std::unique_ptr<SfxTabPage> ScLeftFooterEditPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet )
{
    return std::make_unique<ScLeftFooterEditPage>( pPage, pController, *rCoreSet );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
