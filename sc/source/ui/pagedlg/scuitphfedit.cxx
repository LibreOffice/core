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
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <osl/time.h>
#include <sfx2/tabdlg.hxx>
#include <svtools/ctrltool.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/Dialog.hxx>

#include <unotools/useroptions.hxx>

#include <editutil.hxx>
#include <attrib.hxx>
#include <patattr.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

#include <scuitphfedit.hxx>
#include <memory>


ScHFEditPage::ScHFEditPage(weld::Container* pPage, weld::DialogController* pController,
                           const SfxItemSet& rCoreAttrs,
                           TypedWhichId<ScPageHFItem> nWhichId)
    : SfxTabPage(pPage, pController, u"modules/scalc/ui/headerfootercontent.ui"_ustr, u"HeaderFooterContent"_ustr, &rCoreAttrs)
    , nWhich( nWhichId )
    , m_bDropDownActive(false)
    , m_nTimeToggled(-1)
    , m_xLbDefined(m_xBuilder->weld_combo_box(u"comboLB_DEFINED"_ustr))
    , m_xLbFontFamily(m_xBuilder->weld_combo_box(u"comboLB_FONT_FAMILY"_ustr))
    , m_xLbFontHeight(m_xBuilder->weld_combo_box(u"comboLB_FONT_HEIGHT"_ustr))
    , m_xBtnText(m_xBuilder->weld_button(u"buttonBTN_TEXT"_ustr))
    , m_xBtnFile(m_xBuilder->weld_menu_button(u"buttonBTN_FILE"_ustr))
    , m_xBtnTable(m_xBuilder->weld_button(u"buttonBTN_TABLE"_ustr))
    , m_xBtnPage(m_xBuilder->weld_button(u"buttonBTN_PAGE"_ustr))
    , m_xBtnLastPage(m_xBuilder->weld_button(u"buttonBTN_PAGES"_ustr))
    , m_xBtnDate(m_xBuilder->weld_button(u"buttonBTN_DATE"_ustr))
    , m_xBtnTime(m_xBuilder->weld_button(u"buttonBTN_TIME"_ustr))
    , m_xBtnBold(m_xBuilder->weld_toggle_button(u"buttonBTN_BOLD"_ustr))
    , m_xBtnItalic(m_xBuilder->weld_toggle_button(u"buttonBTN_ITALIC"_ustr))
    , m_xBtnUnderline(m_xBuilder->weld_toggle_button(u"buttonBTN_UNDERLINE"_ustr))
    , m_xEnableHF(m_xBuilder->weld_check_button(u"checkBTN_ENABLE_HF"_ustr))
    , m_xFtConfidential(m_xBuilder->weld_label(u"labelSTR_HF_CONFIDENTIAL"_ustr))
    , m_xFtPage(m_xBuilder->weld_label(u"labelSTR_PAGE"_ustr))
    , m_xFtOfQuestion(m_xBuilder->weld_label(u"labelSTR_HF_OF_QUESTION"_ustr))
    , m_xFtOf(m_xBuilder->weld_label(u"labelSTR_HF_OF"_ustr))
    , m_xFtNone(m_xBuilder->weld_label(u"labelSTR_HF_NONE_IN_BRACKETS"_ustr))
    , m_xFtCreatedBy(m_xBuilder->weld_label(u"labelSTR_HF_CREATED_BY"_ustr))
    , m_xFtCustomized(m_xBuilder->weld_label(u"labelSTR_HF_CUSTOMIZED"_ustr))
    , m_xFtPreset(m_xBuilder->weld_label(u"labelFT_PRESET"_ustr))
    , m_xFtFormat(m_xBuilder->weld_label(u"labelFT_FORMAT"_ustr))
    , m_xFtField(m_xBuilder->weld_label(u"labelFT_FIELD"_ustr))
    , m_xAreaGrid(m_xBuilder->weld_grid(u"areagrid"_ustr))
    , m_xLeftScrolledWindow(m_xBuilder->weld_widget(u"scrolledwindow_LEFT"_ustr))
    , m_xLeft(m_xBuilder->weld_widget(u"labelFT_LEFT"_ustr))
    , m_xRightScrolledWindow(m_xBuilder->weld_widget(u"scrolledwindow_RIGHT"_ustr))
    , m_xRight(m_xBuilder->weld_widget(u"labelFT_RIGHT"_ustr))
    , m_xWndLeft(new ScEditWindow(Left, pController->getDialog()))
    , m_xWndCenter(new ScEditWindow(Center, pController->getDialog()))
    , m_xWndRight(new ScEditWindow(Right, pController->getDialog()))
    , m_xWndLeftWnd(new weld::CustomWeld(*m_xBuilder, u"textviewWND_LEFT"_ustr, *m_xWndLeft))
    , m_xWndCenterWnd(new weld::CustomWeld(*m_xBuilder, u"textviewWND_CENTER"_ustr, *m_xWndCenter))
    , m_xWndRightWnd(new weld::CustomWeld(*m_xBuilder, u"textviewWND_RIGHT"_ustr, *m_xWndRight))
    , m_pEditFocus(nullptr)
{
    // tdf#114695 override natural size with a small value
    // we expect this to get stretched to some larger but
    // limited size based on surrounding widgets
    m_xLbDefined->set_size_request(m_xLbDefined->get_approximate_digit_width() * 20, -1);

    //! use default style from current document?
    //! if font color is used, header/footer background color must be set
    const CellAttributeHelper aTempHelper(*rCoreAttrs.GetPool());
    const ScPatternAttr& rDefaultCellAttribute(aTempHelper.getDefaultCellAttribute());

    m_xLbDefined->connect_popup_toggled( LINK( this, ScHFEditPage, ListToggleHdl_Impl) );
    m_xLbDefined->connect_changed( LINK( this, ScHFEditPage, ListHdl_Impl ) );
    m_xLbFontFamily->connect_changed( LINK( this, ScHFEditPage, FontFamilyListHdl ) );
    m_xLbFontHeight->connect_changed( LINK( this, ScHFEditPage, FontHeightListHdl ) );
    m_xBtnFile->connect_selected( LINK( this, ScHFEditPage, MenuHdl ) );
    m_xBtnText->connect_clicked( LINK( this, ScHFEditPage, ClickHdl ) );
    m_xBtnPage->connect_clicked( LINK( this, ScHFEditPage, ClickHdl ) );
    m_xBtnLastPage->connect_clicked( LINK( this, ScHFEditPage, ClickHdl ) );
    m_xBtnDate->connect_clicked( LINK( this, ScHFEditPage, ClickHdl ) );
    m_xBtnTime->connect_clicked( LINK( this, ScHFEditPage, ClickHdl ) );
    m_xBtnTable->connect_clicked( LINK( this, ScHFEditPage, ClickHdl ) );
    m_xBtnBold->connect_toggled( LINK( this, ScHFEditPage, ToggleHdl ) );
    m_xBtnItalic->connect_toggled( LINK( this, ScHFEditPage, ToggleHdl ) );
    m_xBtnUnderline->connect_toggled( LINK( this, ScHFEditPage, ToggleHdl ) );

    m_xFtPreset->show();
    m_xFtFormat->show();
    m_xFtField->show();

    //swap left/right areas and their labels in RTL mode
    if( AllSettings::GetLayoutRTL() )
    {
        sal_Int32 nOldLeftAttach = m_xAreaGrid->get_child_left_attach(*m_xLeft);
        sal_Int32 nOldRightAttach = m_xAreaGrid->get_child_left_attach(*m_xRight);
        m_xAreaGrid->set_child_left_attach(*m_xLeft, nOldRightAttach);
        m_xAreaGrid->set_child_left_attach(*m_xRight, nOldLeftAttach);

        nOldLeftAttach = m_xAreaGrid->get_child_left_attach(*m_xLeftScrolledWindow);
        nOldRightAttach = m_xAreaGrid->get_child_left_attach(*m_xRightScrolledWindow);
        m_xAreaGrid->set_child_left_attach(*m_xLeftScrolledWindow, nOldRightAttach);
        m_xAreaGrid->set_child_left_attach(*m_xRightScrolledWindow, nOldLeftAttach);
    }
    m_xWndLeft->SetFont( rDefaultCellAttribute );
    m_xWndCenter->SetFont( rDefaultCellAttribute );
    m_xWndRight->SetFont( rDefaultCellAttribute );

    m_xWndLeft->SetObjectSelectHdl( LINK(this,ScHFEditPage,ObjectSelectHdl) );
    m_xWndCenter->SetObjectSelectHdl( LINK(this,ScHFEditPage,ObjectSelectHdl) );
    m_xWndRight->SetObjectSelectHdl( LINK(this,ScHFEditPage,ObjectSelectHdl) );
    auto setEditFocus = [this](ScEditWindow & rEdit) {
        UpdateFontAttributes();
        this->m_pEditFocus = &rEdit;
    };
    m_xWndLeft->SetGetFocusHdl(setEditFocus);
    m_xWndCenter->SetGetFocusHdl(setEditFocus);
    m_xWndRight->SetGetFocusHdl(setEditFocus);

    m_xWndLeft->GrabFocus();
    m_pEditFocus = m_xWndLeft.get(); // there's no event from grab_focus()

    InitPreDefinedList();

    FontList aFontList( Application::GetDefaultDevice() );
    sal_uInt16 nFontCount = aFontList.GetFontNameCount();
    for (sal_uInt16 i = 0; i < nFontCount; ++i)
    {
        const FontMetric &rFontMetric = aFontList.GetFontName(i);
        m_xLbFontFamily->append_text(rFontMetric.GetFamilyName());
    }

    if (const SvxFontItem *pFontItem = m_pEditFocus->GetEditView()->GetAttribs().GetItemIfSet( EE_CHAR_FONTINFO ))
        m_xLbFontFamily->set_active_text(pFontItem->GetFamilyName());

    sal_uInt16 nFontHeight;
    for (nFontHeight = 6; nFontHeight <= 16; ++nFontHeight)
        m_xLbFontHeight->append_text( OUString::number(nFontHeight) );
    for (nFontHeight = 18; nFontHeight <= 28; nFontHeight += 2)
        m_xLbFontHeight->append_text( OUString::number(nFontHeight) );
    for (nFontHeight = 32; nFontHeight <= 48; nFontHeight += 4)
        m_xLbFontHeight->append_text( OUString::number(nFontHeight) );
    for (nFontHeight = 54; nFontHeight <= 72; nFontHeight += 6)
        m_xLbFontHeight->append_text( OUString::number(nFontHeight) );
    for (nFontHeight = 80; nFontHeight <= 96; nFontHeight += 8)
        m_xLbFontHeight->append_text( OUString::number(nFontHeight) );

    if (const SvxFontHeightItem *pFontHeightItem = m_pEditFocus->GetEditView()->GetAttribs().GetItemIfSet( EE_CHAR_FONTHEIGHT ))
        m_xLbFontHeight->set_active_text( OUString::number(pFontHeightItem->GetHeight() / 20) );

    bool bHeader = (nWhich == SID_SCATTR_PAGE_HEADERFIRST ||
                    nWhich == SID_SCATTR_PAGE_HEADERRIGHT ||
                    nWhich == SID_SCATTR_PAGE_HEADERLEFT);

    const bool bOn = rCoreAttrs.Get( bHeader ? ATTR_PAGE_HEADERSET : ATTR_PAGE_FOOTERSET ).GetItemSet().Get( ATTR_PAGE_ON ).GetValue();

    m_xEnableHF->set_label( ScResId( bHeader ? STR_HF_ENABLE_HEADER : STR_HF_ENABLE_FOOTER ) );
    m_xEnableHF->set_active( bOn );
    m_xEnableHF->connect_toggled( LINK( this, ScHFEditPage, ToggleEnableHdl ) );

    ToggleEnableHdl( *m_xEnableHF );
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
    const ScPageHFItem* pItem = rCoreSet->GetItemIfSet(nWhich);
    if ( !pItem )
        return;

    if( const EditTextObject* pLeft = pItem->GetLeftArea() )
        m_xWndLeft->SetText( *pLeft );
    if( const EditTextObject* pCenter = pItem->GetCenterArea() )
        m_xWndCenter->SetText( *pCenter );
    if( const EditTextObject* pRight = pItem->GetRightArea() )
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

    bool bHeader = (nWhich == SID_SCATTR_PAGE_HEADERFIRST ||
                    nWhich == SID_SCATTR_PAGE_HEADERRIGHT ||
                    nWhich == SID_SCATTR_PAGE_HEADERLEFT);

    bool bIsOn = m_xEnableHF->get_active();
    sal_uInt16 nSetId = bHeader ? ATTR_PAGE_HEADERSET : ATTR_PAGE_FOOTERSET;

    const SfxPoolItem* pItem = nullptr;
    std::unique_ptr<SvxSetItem> pNewItem;

    if ( rCoreSet->GetItemState( nSetId, false, &pItem ) == SfxItemState::SET )
        pNewItem.reset( static_cast<SvxSetItem*>( pItem->Clone() ) );
    else
        pNewItem.reset( static_cast<SvxSetItem*>( GetItemSet().Get( nSetId ).Clone() ) );

    bool bOldOn = pNewItem->GetItemSet().Get( ATTR_PAGE_ON ).GetValue();
    if ( bIsOn != bOldOn )
    {
        pNewItem->GetItemSet().Put( SfxBoolItem( ATTR_PAGE_ON, bIsOn ) );
        rCoreSet->Put( *pNewItem );
    }

    return true;
}

void ScHFEditPage::InitPreDefinedList()
{
    SvtUserOptions aUserOpt;

    std::optional<Color> pTxtColour;
    std::optional<Color> pFldColour;
    std::optional<FontLineStyle> pFldLineStyle;

    // Get all the field values at the outset.
    OUString aPageFieldValue(m_xWndLeft->GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), 0,0, pTxtColour, pFldColour, pFldLineStyle));
    OUString aSheetFieldValue(m_xWndLeft->GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxTableField(), EE_FEATURE_FIELD), 0,0, pTxtColour, pFldColour, pFldLineStyle));
    OUString aFileFieldValue(m_xWndLeft->GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxFileField(), EE_FEATURE_FIELD), 0,0, pTxtColour, pFldColour, pFldLineStyle));
    OUString aExtFileFieldValue(m_xWndLeft->GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxExtFileField(), EE_FEATURE_FIELD), 0,0, pTxtColour, pFldColour, pFldLineStyle));
    OUString aDateFieldValue(m_xWndLeft->GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxDateField(), EE_FEATURE_FIELD), 0,0, pTxtColour, pFldColour, pFldLineStyle));

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
                if(aLeftEntry.isEmpty() && aCenterEntry.isEmpty()
                    && aRightEntry.isEmpty())
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
                if(aLeftEntry.isEmpty() && aRightEntry.isEmpty())
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
                if(aLeftEntry.isEmpty() && aRightEntry.isEmpty())
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
                if(IsExtFileNameEntry(pCenterObj.get()) && aLeftEntry.isEmpty()
                    && aRightEntry.isEmpty())
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
                    IsExtFileNameEntry(pRightObj.get()) && aLeftEntry.isEmpty())
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
            ESelection aSel;
            aSel.end.nIndex = aPageEntry.getLength();
            if(aPageEntry == pEngine->GetText(aSel))
            {
                aSel.start.nIndex = aSel.end.nIndex;
                aSel.end.nIndex++;
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
            ESelection aSel;
            OUString aPageEntry( m_xFtPage->get_label() + " ");
            m_xWndCenter->GetEditEngine()->SetTextCurrentDefaults(aPageEntry);
            aSel.end.nIndex = aPageEntry.getLength();
            m_xWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), ESelection(aSel.end));
            ++aSel.end.nIndex;

            OUString aPageOfEntry(" " + m_xFtOf->get_label() + " ");
            m_xWndCenter->GetEditEngine()->QuickInsertText(aPageOfEntry,ESelection(aSel.end));
            aSel.end.nIndex += aPageOfEntry.getLength();
            m_xWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPagesField(), EE_FEATURE_FIELD), ESelection(aSel.end));
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
            ESelection aSel;
            m_xWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem( SvxFileField(), EE_FEATURE_FIELD ), aSel );
            ++aSel.end.nIndex;
            OUString aPageEntry(", " + m_xFtPage->get_label() + " ");
            m_xWndCenter->GetEditEngine()->QuickInsertText(aPageEntry, ESelection(aSel.end));
            aSel.start.nIndex = aSel.end.nIndex;
            aSel.end.nIndex += aPageEntry.getLength();
            m_xWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), ESelection(aSel.end));
            pTextObj = m_xWndCenter->GetEditEngine()->CreateTextObject();
            m_xWndCenter->SetText(*pTextObj);
            if(!bTravelling)
                m_xWndCenter->GrabFocus();
        }
        break;

        case eExtFileNameEntry:
            ClearTextAreas();
            m_xWndCenter->InsertField( SvxFieldItem( SvxExtFileField(
                OUString(), SvxFileType::Var, SvxFileFormat::PathFull ), EE_FEATURE_FIELD ) );
            if(!bTravelling)
                m_xWndCenter->GrabFocus();
        break;

        case ePageSheetEntry:
        {
            ClearTextAreas();
            ESelection aSel;
            OUString aPageEntry( m_xFtPage->get_label() + " " );
            m_xWndCenter->GetEditEngine()->SetTextCurrentDefaults(aPageEntry);
            aSel.end.nIndex = aPageEntry.getLength();
            m_xWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), ESelection(aSel.end));
            ++aSel.end.nIndex;

            OUString aCommaSpace(u", "_ustr);
            m_xWndCenter->GetEditEngine()->QuickInsertText(aCommaSpace,ESelection(aSel.end));
            aSel.end.nIndex += aCommaSpace.getLength();
            m_xWndCenter->GetEditEngine()->QuickInsertField( SvxFieldItem(SvxTableField(), EE_FEATURE_FIELD), ESelection(aSel.end));
            pTextObj = m_xWndCenter->GetEditEngine()->CreateTextObject();
            m_xWndCenter->SetText(*pTextObj);
            if(!bTravelling)
                m_xWndCenter->GrabFocus();
        }
        break;

        case ePageFileNameEntry:
        {
            ClearTextAreas();
            ESelection aSel;
            OUString aPageEntry( m_xFtPage->get_label() + " " );
            m_xWndCenter->GetEditEngine()->SetTextCurrentDefaults(aPageEntry);
            aSel.end.nIndex = aPageEntry.getLength();
            m_xWndCenter->GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), ESelection(aSel.end));
            ++aSel.end.nIndex;
            OUString aCommaSpace(u", "_ustr);
            m_xWndCenter->GetEditEngine()->QuickInsertText(aCommaSpace,ESelection(aSel.end));
            aSel.end.nIndex += aCommaSpace.getLength();
            m_xWndCenter->GetEditEngine()->QuickInsertField( SvxFieldItem(SvxFileField(), EE_FEATURE_FIELD), ESelection(aSel.end));
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
                OUString(), SvxFileType::Var, SvxFileFormat::PathFull ), EE_FEATURE_FIELD ) );
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
    m_xWndLeft->GetEditEngine()->SetTextCurrentDefaults(OUString());
    m_xWndLeft->Invalidate();
    m_xWndCenter->GetEditEngine()->SetTextCurrentDefaults(OUString());
    m_xWndCenter->Invalidate();
    m_xWndRight->GetEditEngine()->SetTextCurrentDefaults(OUString());
    m_xWndRight->Invalidate();
}

void ScHFEditPage::UpdateFontAttributes()
{
    const SfxItemSet aSet = m_pEditFocus->GetEditView()->GetAttribs();

    if (const SvxFontItem* pFontItem = aSet.GetItemIfSet( EE_CHAR_FONTINFO ))
        m_xLbFontFamily->set_active_text(pFontItem->GetFamilyName());
    if (const SvxFontHeightItem* pFontHeightItem = aSet.GetItemIfSet( EE_CHAR_FONTHEIGHT ))
        m_xLbFontHeight->set_active_text(OUString::number(pFontHeightItem->GetHeight() / 20));
    if (const SvxWeightItem* pWeightItem = aSet.GetItemIfSet( EE_CHAR_WEIGHT ))
        m_xBtnBold->set_active(pWeightItem->GetValue() == WEIGHT_BOLD);
    if (const SvxUnderlineItem* pUnderlineItem = aSet.GetItemIfSet( EE_CHAR_UNDERLINE ))
        m_xBtnUnderline->set_active(pUnderlineItem->GetValue() != LINESTYLE_NONE);
    if (const SvxPostureItem* pItalicItem = aSet.GetItemIfSet( EE_CHAR_ITALIC ))
        m_xBtnItalic->set_active(pItalicItem->GetValue() == ITALIC_NORMAL);

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

IMPL_LINK_NOARG( ScHFEditPage, FontHeightListHdl, weld::ComboBox&, void)
{
    SfxItemSet aSet(m_pEditFocus->GetEditView()->GetEmptyItemSet());

    aSet.Put( SvxFontHeightItem( m_xLbFontHeight->get_active_text().toInt32() * 20, 100, EE_CHAR_FONTHEIGHT ) );
    m_pEditFocus->GetEditView()->SetAttribs(aSet);
    m_pEditFocus->GrabFocus();
}

IMPL_LINK_NOARG( ScHFEditPage, FontFamilyListHdl, weld::ComboBox&, void)
{
    OUString sFontName(m_xLbFontFamily->get_active_text());
    vcl::Font aFont;
    aFont.SetFamilyName(sFontName);

    SfxItemSet aSet(m_pEditFocus->GetEditView()->GetEmptyItemSet());

    aSet.Put( SvxFontItem( FAMILY_DONTKNOW, aFont.GetFamilyName(), u""_ustr, PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, EE_CHAR_FONTINFO ) );
    m_pEditFocus->GetEditView()->SetAttribs(aSet);
    m_pEditFocus->GrabFocus();
}

IMPL_LINK_NOARG( ScHFEditPage, ToggleEnableHdl, weld::Toggleable&, void )
{
    bool bActive = m_xEnableHF->get_active();

    m_xLbDefined->set_sensitive( bActive );
    m_xLbFontFamily->set_sensitive( bActive );
    m_xLbFontHeight->set_sensitive( bActive );
    m_xBtnBold->set_sensitive( bActive );
    m_xBtnItalic->set_sensitive( bActive );
    m_xBtnUnderline->set_sensitive( bActive );

    m_xBtnText->set_sensitive( bActive );
    m_xBtnFile->set_sensitive( bActive );
    m_xBtnTable->set_sensitive( bActive );
    m_xBtnPage->set_sensitive( bActive );
    m_xBtnLastPage->set_sensitive( bActive );
    m_xBtnDate->set_sensitive( bActive );
    m_xBtnTime->set_sensitive( bActive );

    m_xWndLeftWnd->set_sensitive( bActive );
    m_xWndCenterWnd->set_sensitive( bActive );
    m_xWndRightWnd->set_sensitive( bActive );
    m_xLeft->set_sensitive( bActive );
    m_xRight->set_sensitive( bActive );
}

IMPL_LINK( ScHFEditPage, ClickHdl, weld::Button&, rBtn, void )
{
    if (!m_pEditFocus)
        return;

    if (&rBtn == m_xBtnText.get())
    {
        if ( !m_pEditFocus->HasSelection() )
        {
            ESelection aSel;
            aSel.start.nIndex = 0;
            aSel.end.nIndex = m_pEditFocus->GetEditEngine()->GetTextLen(0);
            m_pEditFocus->GetEditView()->SetSelection(aSel);
        }
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

IMPL_LINK(ScHFEditPage, ToggleHdl, weld::Toggleable&, rBtn, void)
{
    if (&rBtn == m_xBtnBold.get())
    {
        const SvxWeightItem* pItem = m_pEditFocus->GetEditView()->GetAttribs().GetItemIfSet( EE_CHAR_WEIGHT );
        SfxItemSet aSet(m_pEditFocus->GetEditView()->GetEmptyItemSet());

        if ( pItem && pItem->GetValue() == WEIGHT_BOLD )
            aSet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
        else
            aSet.Put( SvxWeightItem( WEIGHT_BOLD, EE_CHAR_WEIGHT ) );
        m_pEditFocus->GetEditView()->SetAttribs(aSet);
    }
    else if (&rBtn == m_xBtnItalic.get())
    {
        const SvxPostureItem* pItem = m_pEditFocus->GetEditView()->GetAttribs().GetItemIfSet( EE_CHAR_ITALIC );
        SfxItemSet aSet(m_pEditFocus->GetEditView()->GetEmptyItemSet());

        if ( pItem && pItem->GetValue() == ITALIC_NORMAL )
            aSet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC ) );
        else
            aSet.Put( SvxPostureItem( ITALIC_NORMAL, EE_CHAR_ITALIC ) );
        m_pEditFocus->GetEditView()->SetAttribs(aSet);
    }
    else if (&rBtn == m_xBtnUnderline.get())
    {
        const SvxUnderlineItem* pItem = m_pEditFocus->GetEditView()->GetAttribs().GetItemIfSet( EE_CHAR_UNDERLINE );
        SfxItemSet aSet(m_pEditFocus->GetEditView()->GetEmptyItemSet());

        if ( pItem && pItem->GetValue() == LINESTYLE_SINGLE )
            aSet.Put( SvxUnderlineItem( LINESTYLE_NONE, EE_CHAR_UNDERLINE ) );
        else
            aSet.Put( SvxUnderlineItem( LINESTYLE_SINGLE, EE_CHAR_UNDERLINE ) );
        m_pEditFocus->GetEditView()->SetAttribs(aSet);
    }
    InsertToDefinedList();
    UpdateFontAttributes();
    m_pEditFocus->GrabFocus();
}

IMPL_LINK(ScHFEditPage, MenuHdl, const OUString&, rSelectedId, void)
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


ScFirstHeaderEditPage::ScFirstHeaderEditPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pPage, pController,
                    rCoreSet,
                    SID_SCATTR_PAGE_HEADERFIRST)
    {}

std::unique_ptr<SfxTabPage> ScFirstHeaderEditPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet )
{
    return std::make_unique<ScFirstHeaderEditPage>( pPage, pController, *rCoreSet );
}


ScRightHeaderEditPage::ScRightHeaderEditPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pPage, pController,
                    rCoreSet,
                    SID_SCATTR_PAGE_HEADERRIGHT )
    {}

std::unique_ptr<SfxTabPage> ScRightHeaderEditPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet )
{
    return std::make_unique<ScRightHeaderEditPage>( pPage, pController, *rCoreSet );
}


ScLeftHeaderEditPage::ScLeftHeaderEditPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pPage, pController,
                    rCoreSet,
                    SID_SCATTR_PAGE_HEADERLEFT )
    {}

std::unique_ptr<SfxTabPage> ScLeftHeaderEditPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet )
{
    return std::make_unique<ScLeftHeaderEditPage>( pPage, pController, *rCoreSet );
}


ScFirstFooterEditPage::ScFirstFooterEditPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pPage, pController,
                    rCoreSet,
                    SID_SCATTR_PAGE_FOOTERFIRST )
    {}

std::unique_ptr<SfxTabPage> ScFirstFooterEditPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet )
{
    return std::make_unique<ScFirstFooterEditPage>( pPage, pController, *rCoreSet );
}


ScRightFooterEditPage::ScRightFooterEditPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pPage, pController,
                    rCoreSet,
                    SID_SCATTR_PAGE_FOOTERRIGHT )
    {}

std::unique_ptr<SfxTabPage> ScRightFooterEditPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet )
{
    return std::make_unique<ScRightFooterEditPage>( pPage, pController, *rCoreSet );
}


ScLeftFooterEditPage::ScLeftFooterEditPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pPage, pController,
                    rCoreSet,
                    SID_SCATTR_PAGE_FOOTERLEFT )
    {}

std::unique_ptr<SfxTabPage> ScLeftFooterEditPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet )
{
    return std::make_unique<ScLeftFooterEditPage>( pPage, pController, *rCoreSet );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
