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

#include <utility>
#include <view.hxx>
#include <wrtsh.hxx>
#include <cption.hxx>
#include <fldmgr.hxx>
#include <expfld.hxx>
#include <numrule.hxx>
#include <poolfmt.hxx>
#include <docsh.hxx>
#include <calc.hxx>
#include <uitool.hxx>
#include <doc.hxx>
#include <modcfg.hxx>
#include <swmodule.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <comphelper/string.hxx>
#include <vcl/weld.hxx>
#include <strings.hrc>
#include <SwStyleNameMapper.hxx>

using namespace ::com::sun::star;

namespace {

class SwSequenceOptionDialog : public weld::GenericDialogController
{
    SwView&         m_rView;
    OUString        m_aFieldTypeName;

    std::unique_ptr<weld::ComboBox> m_xLbLevel;
    std::unique_ptr<weld::Entry> m_xEdDelim;

    std::unique_ptr<weld::ComboBox> m_xLbCharStyle;
    std::unique_ptr<weld::CheckButton> m_xApplyBorderAndShadowCB;

    //#i61007# order of captions
    std::unique_ptr<weld::ComboBox> m_xLbCaptionOrder;

public:
    SwSequenceOptionDialog(weld::Window *pParent, SwView &rV, OUString aSeqFieldType);
    void Apply();

    bool IsApplyBorderAndShadow() const { return m_xApplyBorderAndShadowCB->get_active(); }
    void SetApplyBorderAndShadow( bool bSet )  { m_xApplyBorderAndShadowCB->set_active(bSet); }

    //#i61007# order of captions
    bool IsOrderNumberingFirst() const { return m_xLbCaptionOrder->get_active() == 1; }
    void SetOrderNumberingFirst(bool bSet) { m_xLbCaptionOrder->set_active(bSet ? 1 : 0); }

    void      SetCharacterStyle(const OUString& rStyle);
    OUString  GetCharacterStyle() const;
};

}

OUString SwCaptionDialog::s_aSepTextSave(u": "_ustr); // Caption separator text

//Resolves: tdf#47427 disallow typing *or* pasting invalid content into the category box
OUString TextFilterAutoConvert::filter(const OUString &rText)
{
    if (!rText.isEmpty() && rText != m_sNone && !SwCalc::IsValidVarName(rText))
        return m_sLastGoodText;
    m_sLastGoodText = rText;
    return rText;
}

SwCaptionDialog::SwCaptionDialog(weld::Window *pParent, SwView &rV)
    : SfxDialogController(pParent, u"modules/swriter/ui/insertcaption.ui"_ustr, u"InsertCaptionDialog"_ustr)
    , m_sNone(SwResId(SW_STR_NONE))
    , m_aTextFilter(m_sNone)
    , m_rView(rV)
    , m_pMgr(new SwFieldMgr(m_rView.GetWrtShellPtr()))
    , m_bCopyAttributes(false)
    , m_bOrderNumberingFirst(SW_MOD()->GetModuleConfig()->IsCaptionOrderNumberingFirst())
    , m_xTextEdit(m_xBuilder->weld_entry(u"caption_edit"_ustr))
    , m_xCategoryBox(m_xBuilder->weld_combo_box(u"category"_ustr))
    , m_xFormatText(m_xBuilder->weld_label(u"numbering_label"_ustr))
    , m_xFormatBox(m_xBuilder->weld_combo_box(u"numbering"_ustr))
    , m_xNumberingSeparatorFT(m_xBuilder->weld_label(u"num_separator"_ustr))
    , m_xNumberingSeparatorED(m_xBuilder->weld_entry(u"num_separator_edit"_ustr))
    , m_xSepText(m_xBuilder->weld_label(u"separator_label"_ustr))
    , m_xSepEdit(m_xBuilder->weld_entry(u"separator_edit"_ustr))
    , m_xPosBox(m_xBuilder->weld_combo_box(u"position"_ustr))
    , m_xOKButton(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xAutoCaptionButton(m_xBuilder->weld_button(u"auto"_ustr))
    , m_xOptionButton(m_xBuilder->weld_button(u"options"_ustr))
    , m_xPreview(new weld::CustomWeld(*m_xBuilder, u"preview"_ustr, m_aPreview))
{
    //#i61007# order of captions
    ApplyCaptionOrder();
    SwWrtShell &rSh = m_rView.GetWrtShell();
    uno::Reference< frame::XModel >  xModel = m_rView.GetDocShell()->GetBaseModel();

    SelectionType eType = rSh.GetSelectionType();
    if ( eType & SelectionType::Ole )
    {
        eType = SelectionType::Graphic;
        uno::Reference< text::XTextEmbeddedObjectsSupplier >  xObjs(xModel, uno::UNO_QUERY);
        m_xNameAccess = xObjs->getEmbeddedObjects();
    }

    m_xCategoryBox->connect_changed(LINK(this, SwCaptionDialog, ModifyComboHdl));
    Link<weld::Entry&,void> aLk = LINK(this, SwCaptionDialog, ModifyEntryHdl);
    m_xTextEdit->connect_changed(aLk);
    m_xNumberingSeparatorED->connect_changed(aLk);
    m_xSepEdit->connect_changed(aLk);

    m_xFormatBox->connect_changed(LINK(this, SwCaptionDialog, SelectListBoxHdl));
    m_xOKButton->connect_clicked(LINK(this, SwCaptionDialog, OKHdl));
    m_xOptionButton->connect_clicked(LINK(this, SwCaptionDialog, OptionHdl));
    m_xAutoCaptionButton->connect_clicked(LINK(this, SwCaptionDialog, CaptionHdl));
    m_xAutoCaptionButton->set_accessible_description(SwResId(STR_A11Y_DESC_AUTO));

    m_xCategoryBox->append_text(m_sNone);
    size_t nCount = m_pMgr->GetFieldTypeCount();
    for (size_t i = 0; i < nCount; ++i)
    {
        SwFieldType *pType = m_pMgr->GetFieldType( SwFieldIds::Unknown, i );
        if( pType->Which() == SwFieldIds::SetExp &&
            static_cast<SwSetExpFieldType *>( pType)->GetType() & nsSwGetSetExpType::GSE_SEQ )
            m_xCategoryBox->append_text(pType->GetName());
    }

    OUString sString;
    sal_uInt16 nPoolId = 0;
    if (eType & SelectionType::Graphic)
    {
        nPoolId = RES_POOLCOLL_LABEL_FIGURE;

        SwSetExpFieldType* pTypeIll= static_cast<SwSetExpFieldType*>(rSh.GetFieldType(SwFieldIds::SetExp, SwResId(STR_POOLCOLL_LABEL_ABB)));
        if(rSh.IsUsed(*pTypeIll)) //default to illustration for legacy docs
        {
            nPoolId = RES_POOLCOLL_LABEL_ABB;

        }

        sString = m_rView.GetOldGrfCat();
        m_bCopyAttributes = true;
        //if not OLE
        if(!m_xNameAccess.is())
        {
            uno::Reference< text::XTextGraphicObjectsSupplier >  xGraphics(xModel, uno::UNO_QUERY);
            m_xNameAccess = xGraphics->getGraphicObjects();
        }

    }
    else if( eType & SelectionType::Table )
    {
        nPoolId = RES_POOLCOLL_LABEL_TABLE;
        sString = m_rView.GetOldTabCat();
        uno::Reference< text::XTextTablesSupplier >  xTables(xModel, uno::UNO_QUERY);
        m_xNameAccess = xTables->getTextTables();
    }
    else if( eType & SelectionType::Frame )
    {
        nPoolId = RES_POOLCOLL_LABEL_FRAME;
        sString = m_rView.GetOldFrameCat();
        uno::Reference< text::XTextFramesSupplier >  xFrames(xModel, uno::UNO_QUERY);
        m_xNameAccess = xFrames->getTextFrames();
    }
    else if( eType == SelectionType::Text )
    {
        nPoolId = RES_POOLCOLL_LABEL_FRAME;
        sString = m_rView.GetOldFrameCat();
    }
    else if( eType & SelectionType::DrawObject )
    {
        nPoolId = RES_POOLCOLL_LABEL_DRAWING;
        sString = m_rView.GetOldDrwCat();
    }
    if( nPoolId )
    {
        if (sString.isEmpty())
            sString = SwStyleNameMapper::GetUIName(nPoolId, OUString());
        auto nIndex = m_xCategoryBox->find_text(sString);
        if (nIndex != -1)
            m_xCategoryBox->set_active(nIndex);
        else
            m_xCategoryBox->set_entry_text(sString);
    }

    // aFormatBox
    sal_uInt16 nSelFormat = SVX_NUM_ARABIC;
    nCount = m_pMgr->GetFieldTypeCount();
    for ( size_t i = nCount; i; )
    {
        SwFieldType* pFieldType = m_pMgr->GetFieldType(SwFieldIds::Unknown, --i);
        if (pFieldType->GetName() == m_xCategoryBox->get_active_text())
        {
            nSelFormat = o3tl::narrowing<sal_uInt16>(static_cast<SwSetExpFieldType*>(pFieldType)->GetSeqFormat());
            break;
        }
    }

    sal_uInt16 nFormatCount = m_pMgr->GetFormatCount(SwFieldTypesEnum::Sequence, false);
    for ( sal_uInt16 i = 0; i < nFormatCount; ++i )
    {
        const sal_uInt16 nFormatId = m_pMgr->GetFormatId(SwFieldTypesEnum::Sequence, i);
        m_xFormatBox->append(OUString::number(nFormatId), m_pMgr->GetFormatStr(SwFieldTypesEnum::Sequence, i));
        if (nFormatId == nSelFormat)
            m_xFormatBox->set_active(i);
    }

    // aPosBox
    if (eType == SelectionType::Graphic
        || eType == SelectionType::Table
        || eType == (SelectionType::Table | SelectionType::NumberList)
        || eType == (SelectionType::Table | SelectionType::Text)
        || eType == (SelectionType::Table | SelectionType::NumberList | SelectionType::Text)
        || eType == SelectionType::DrawObject
        || eType == (SelectionType::DrawObject | SelectionType::Ornament))
    {
        m_xPosBox->append_text(SwResId(STR_CAPTION_ABOVE));
        m_xPosBox->append_text(SwResId(STR_CAPTION_BELOW));
    }
    else if(eType == SelectionType::Frame
            || eType == SelectionType::Text)
    {
        m_xPosBox->append_text(SwResId(STR_CAPTION_BEGINNING));
        m_xPosBox->append_text(SwResId(STR_CAPTION_END));
    }

    if (eType & SelectionType::Table)
    {
        m_xPosBox->set_active(0);
    }
    else
    {
        m_xPosBox->set_active(1);
    }

    ModifyHdl();

    m_xSepEdit->set_text(s_aSepTextSave);
    m_xTextEdit->grab_focus();
    DrawSample();
}

IMPL_LINK_NOARG(SwCaptionDialog, OKHdl, weld::Button&, void)
{
    Apply();
    m_xDialog->response(RET_OK);
}

void SwCaptionDialog::Apply()
{
    InsCaptionOpt aOpt;
    aOpt.UseCaption() = true;
    OUString aName(m_xCategoryBox->get_active_text());
    if ( aName == m_sNone )
    {
        aOpt.SetCategory( OUString() );
        aOpt.SetNumSeparator( OUString() );
    }
    else
    {
        aOpt.SetCategory(comphelper::string::strip(aName, ' '));
        aOpt.SetNumSeparator(m_xNumberingSeparatorED->get_text());
    }
    aOpt.SetNumType(m_xFormatBox->get_active_id().toUInt32());
    aOpt.SetSeparator(m_xSepEdit->get_sensitive() ? m_xSepEdit->get_text() : OUString());
    aOpt.SetCaption(m_xTextEdit->get_text());
    aOpt.SetPos(m_xPosBox->get_active());
    aOpt.IgnoreSeqOpts() = true;
    aOpt.CopyAttributes() = m_bCopyAttributes;
    aOpt.SetCharacterStyle( m_sCharacterStyle );
    m_rView.InsertCaption( &aOpt );
    s_aSepTextSave = m_xSepEdit->get_text();
}

short SwCaptionDialog::run()
{
    short nRet = SfxDialogController::run();
    if (nRet == RET_OK)
        Apply();
    return nRet;
}

IMPL_LINK_NOARG(SwCaptionDialog, OptionHdl, weld::Button&, void)
{
    OUString sFieldTypeName = m_xCategoryBox->get_active_text();
    if(sFieldTypeName == m_sNone)
        sFieldTypeName.clear();
    auto pDlg = std::make_shared<SwSequenceOptionDialog>(m_xDialog.get(), m_rView, sFieldTypeName);
    pDlg->SetApplyBorderAndShadow(m_bCopyAttributes);
    pDlg->SetCharacterStyle( m_sCharacterStyle );
    pDlg->SetOrderNumberingFirst( m_bOrderNumberingFirst );

    GenericDialogController::runAsync(pDlg, [pDlg, this](sal_Int32 nResult){
        if (nResult == RET_OK) {
            pDlg->Apply();
            m_bCopyAttributes = pDlg->IsApplyBorderAndShadow();
            m_sCharacterStyle = pDlg->GetCharacterStyle();
            //#i61007# order of captions
            if( m_bOrderNumberingFirst != pDlg->IsOrderNumberingFirst() )
            {
                m_bOrderNumberingFirst = pDlg->IsOrderNumberingFirst();
                SW_MOD()->GetModuleConfig()->SetCaptionOrderNumberingFirst(m_bOrderNumberingFirst);
                ApplyCaptionOrder();
            }
            DrawSample();
        }
    });
}

IMPL_LINK_NOARG(SwCaptionDialog, SelectListBoxHdl, weld::ComboBox&, void)
{
    DrawSample();
}

void SwCaptionDialog::ModifyHdl()
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    OUString sFieldTypeName = m_xCategoryBox->get_active_text();
    bool bCorrectFieldName = !sFieldTypeName.isEmpty();
    bool bNone = sFieldTypeName == m_sNone;
    SwFieldType* pType = (bCorrectFieldName && !bNone)
                    ? rSh.GetFieldType( SwFieldIds::SetExp, sFieldTypeName )
                    : nullptr;
    m_xOKButton->set_sensitive( bCorrectFieldName &&
                        (!pType ||
                            static_cast<SwSetExpFieldType*>(pType)->GetType() == nsSwGetSetExpType::GSE_SEQ) );
    m_xOptionButton->set_sensitive(m_xOKButton->get_sensitive() && !bNone);
    m_xNumberingSeparatorFT->set_sensitive(m_bOrderNumberingFirst && !bNone);
    m_xNumberingSeparatorED->set_sensitive(m_bOrderNumberingFirst && !bNone);
    m_xFormatText->set_sensitive(!bNone);
    m_xFormatBox->set_sensitive(!bNone);
    m_xSepText->set_sensitive(!bNone);
    m_xSepEdit->set_sensitive(!bNone);
    DrawSample();
}

IMPL_LINK_NOARG(SwCaptionDialog, ModifyEntryHdl, weld::Entry&, void)
{
    ModifyHdl();
}

IMPL_LINK_NOARG(SwCaptionDialog, ModifyComboHdl, weld::ComboBox&, void)
{
    OUString sText = m_xCategoryBox->get_active_text();
    OUString sAllowedText = m_aTextFilter.filter(sText);
    if (sText != sAllowedText)
    {
        m_xCategoryBox->set_entry_text(sAllowedText);
        m_xCategoryBox->select_entry_region(sAllowedText.getLength(), sAllowedText.getLength());
    }
    ModifyHdl();
}

IMPL_LINK_NOARG(SwCaptionDialog, CaptionHdl, weld::Button&, void)
{
    SfxItemSet aSet(m_rView.GetDocShell()->GetDoc()->GetAttrPool());
    SwCaptionOptDlg aDlg(m_xDialog.get(), aSet);
    aDlg.run();
}

void SwCaptionDialog::DrawSample()
{
    OUString aStr;
    OUString sCaption = m_xTextEdit->get_text();

    // number
    OUString sFieldTypeName = m_xCategoryBox->get_active_text();
    bool bNone = sFieldTypeName == m_sNone;
    if( !bNone )
    {
        const sal_uInt16 nNumFormat = m_xFormatBox->get_active_id().toUInt32();
        if (SVX_NUM_NUMBER_NONE != nNumFormat)
        {
            // category
            //#i61007# order of captions
            if( !m_bOrderNumberingFirst )
            {
                aStr = sFieldTypeName;
                if ( !aStr.isEmpty() )
                    aStr += " ";
            }

            SwWrtShell &rSh = m_rView.GetWrtShell();
            SwSetExpFieldType* pFieldType = static_cast<SwSetExpFieldType*>(rSh.GetFieldType(
                                            SwFieldIds::SetExp, sFieldTypeName ));
            if( pFieldType && pFieldType->GetOutlineLvl() < MAXLEVEL )
            {
                SwNumberTree::tNumberVector aNumVector;
                aNumVector.insert(aNumVector.end(), pFieldType->GetOutlineLvl() + 1, 1);

                OUString sNumber( rSh.GetOutlineNumRule()->
                                MakeNumString(aNumVector, false ));
                if( !sNumber.isEmpty() )
                    aStr += sNumber + pFieldType->GetDelimiter();
            }

            switch( nNumFormat )
            {
            case SVX_NUM_CHARS_UPPER_LETTER:    aStr += "A"; break;
            case SVX_NUM_CHARS_UPPER_LETTER_N:  aStr += "A"; break;
            case SVX_NUM_CHARS_LOWER_LETTER:    aStr += "a"; break;
            case SVX_NUM_CHARS_LOWER_LETTER_N:  aStr += "a"; break;
            case SVX_NUM_ROMAN_UPPER:           aStr += "I"; break;
            case SVX_NUM_ROMAN_LOWER:           aStr += "i"; break;
            default:                    aStr += "1"; break;
            }
            //#i61007# order of captions
            if( m_bOrderNumberingFirst )
            {
                aStr += m_xNumberingSeparatorED->get_text() + sFieldTypeName;
            }

        }
        if( !sCaption.isEmpty() )
        {
            aStr += m_xSepEdit->get_text();
        }
    }
    aStr += sCaption;
    // do preview!
    m_aPreview.SetPreviewText(aStr);
}

SwCaptionDialog::~SwCaptionDialog()
{
}

SwSequenceOptionDialog::SwSequenceOptionDialog(weld::Window *pParent, SwView &rV, OUString aSeqFieldType )
    : GenericDialogController(pParent, u"modules/swriter/ui/captionoptions.ui"_ustr, u"CaptionOptionsDialog"_ustr)
    , m_rView(rV)
    , m_aFieldTypeName(std::move(aSeqFieldType))
    , m_xLbLevel(m_xBuilder->weld_combo_box(u"level"_ustr))
    , m_xEdDelim(m_xBuilder->weld_entry(u"separator"_ustr))
    , m_xLbCharStyle(m_xBuilder->weld_combo_box(u"style"_ustr))
    , m_xApplyBorderAndShadowCB(m_xBuilder->weld_check_button(u"border_and_shadow"_ustr))
    , m_xLbCaptionOrder(m_xBuilder->weld_combo_box(u"caption_order"_ustr))
{
    SwWrtShell &rSh = m_rView.GetWrtShell();

    const OUString sNone(SwResId(SW_STR_NONE));

    m_xLbLevel->append_text(sNone);
    for (sal_uInt16 n = 0; n < MAXLEVEL; ++n)
        m_xLbLevel->append_text(OUString::number(n + 1));

    SwSetExpFieldType* pFieldType = static_cast<SwSetExpFieldType*>(rSh.GetFieldType(
                                        SwFieldIds::SetExp, m_aFieldTypeName ));

    sal_Unicode nLvl = MAXLEVEL;
    OUString sDelim(u": "_ustr);
    if( pFieldType )
    {
        sDelim = pFieldType->GetDelimiter();
        nLvl = pFieldType->GetOutlineLvl();
    }

    m_xLbLevel->set_active(nLvl < MAXLEVEL ? nLvl + 1 : 0);
    m_xEdDelim->set_text(sDelim);

    m_xLbCharStyle->append_text(sNone);
    ::FillCharStyleListBox(*m_xLbCharStyle, m_rView.GetDocShell(), true, true);
    m_xLbCharStyle->set_active(0);
}

void SwSequenceOptionDialog::Apply()
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    SwSetExpFieldType* pFieldType = static_cast<SwSetExpFieldType*>(rSh.GetFieldType(
                                        SwFieldIds::SetExp, m_aFieldTypeName ));

    sal_Int8 nLvl = static_cast<sal_Int8>(m_xLbLevel->get_active() - 1);
    sal_Unicode cDelim = m_xEdDelim->get_text()[0];

    bool bUpdate = true;
    if( pFieldType )
    {
        pFieldType->SetDelimiter( OUString(cDelim) );
        pFieldType->SetOutlineLvl( nLvl );
    }
    else if( !m_aFieldTypeName.isEmpty() && nLvl < MAXLEVEL )
    {
        // then we have to insert that
        SwSetExpFieldType aFieldType( rSh.GetDoc(), m_aFieldTypeName, nsSwGetSetExpType::GSE_SEQ );
        aFieldType.SetDelimiter( OUString(cDelim) );
        aFieldType.SetOutlineLvl( nLvl );
        rSh.InsertFieldType( aFieldType );
    }
    else
        bUpdate = false;

    if( bUpdate )
        rSh.UpdateExpFields();
}

OUString  SwSequenceOptionDialog::GetCharacterStyle() const
{
    if (m_xLbCharStyle->get_active() != -1)
        return m_xLbCharStyle->get_active_text();
    return OUString();
}

void SwSequenceOptionDialog::SetCharacterStyle(const OUString& rStyle)
{
    const int nPos = m_xLbCharStyle->find_text(rStyle);
    if (nPos == -1)
        m_xLbCharStyle->set_active(0);
    else
        m_xLbCharStyle->set_active(nPos);
}

// #i61007# order of captions
void SwCaptionDialog::ApplyCaptionOrder()
{
    m_xNumberingSeparatorFT->set_sensitive(m_bOrderNumberingFirst);
    m_xNumberingSeparatorED->set_sensitive(m_bOrderNumberingFirst);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
