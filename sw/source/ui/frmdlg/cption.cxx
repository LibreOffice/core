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

#include <view.hxx>
#include <wrtsh.hxx>
#include <cption.hxx>
#include <fldmgr.hxx>
#include <expfld.hxx>
#include <numrule.hxx>
#include <poolfmt.hxx>
#include <docsh.hxx>
#include <frmfmt.hxx>
#include <calc.hxx>
#include <uitool.hxx>
#include <doc.hxx>
#include <modcfg.hxx>
#include <swmodule.hxx>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextTableCursor.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <comphelper/string.hxx>
#include <vcl/weld.hxx>
#include <initui.hxx>
#include <globals.hrc>
#include <strings.hrc>
#include <SwStyleNameMapper.hxx>

using namespace ::com::sun::star;

class SwSequenceOptionDialog : public weld::GenericDialogController
{
    SwView&         m_rView;
    OUString const  m_aFieldTypeName;

    std::unique_ptr<weld::ComboBox> m_xLbLevel;
    std::unique_ptr<weld::Entry> m_xEdDelim;

    std::unique_ptr<weld::ComboBox> m_xLbCharStyle;
    std::unique_ptr<weld::CheckButton> m_xApplyBorderAndShadowCB;

    //#i61007# order of captions
    std::unique_ptr<weld::ComboBox> m_xLbCaptionOrder;

public:
    SwSequenceOptionDialog(weld::Window *pParent, SwView &rV, const OUString& rSeqFieldType);
    void Apply();

    bool IsApplyBorderAndShadow() { return m_xApplyBorderAndShadowCB->get_active(); }
    void SetApplyBorderAndShadow( bool bSet )  { m_xApplyBorderAndShadowCB->set_active(bSet); }

    //#i61007# order of captions
    bool IsOrderNumberingFirst() const { return m_xLbCaptionOrder->get_active() == 1; }
    void SetOrderNumberingFirst(bool bSet) { m_xLbCaptionOrder->set_active(bSet ? 1 : 0); }

    void      SetCharacterStyle(const OUString& rStyle);
    OUString  GetCharacterStyle() const;

    virtual short run() override
    {
        int nRet = GenericDialogController::run();
        if (nRet == RET_OK)
            Apply();
        return nRet;
    }
};

OUString SwCaptionDialog::our_aSepTextSave(": "); // Caption separator text

//Resolves: tdf#47427 disallow typing *or* pasting invalid content into the category box
OUString TextFilterAutoConvert::filter(const OUString &rText)
{
    if (!rText.isEmpty() && rText != m_sNone && !SwCalc::IsValidVarName(rText))
        return m_sLastGoodText;
    m_sLastGoodText = rText;
    return rText;
}

IMPL_LINK(SwCaptionDialog, TextFilterHdl, OUString&, rTest, bool)
{
    rTest = m_aTextFilter.filter(rTest);
    return true;
}

SwCaptionDialog::SwCaptionDialog(weld::Window *pParent, SwView &rV)
    : SfxDialogController(pParent, "modules/swriter/ui/insertcaption.ui", "InsertCaptionDialog")
    , m_sNone(SwResId(SW_STR_NONE))
    , m_aTextFilter(m_sNone)
    , rView(rV)
    , pMgr(new SwFieldMgr(rView.GetWrtShellPtr()))
    , bCopyAttributes(false)
    , bOrderNumberingFirst(SW_MOD()->GetModuleConfig()->IsCaptionOrderNumberingFirst())
    , m_xTextEdit(m_xBuilder->weld_entry("caption_edit"))
    , m_xCategoryBox(m_xBuilder->weld_combo_box("category"))
    , m_xFormatText(m_xBuilder->weld_label("numbering_label"))
    , m_xFormatBox(m_xBuilder->weld_combo_box("numbering"))
    , m_xNumberingSeparatorFT(m_xBuilder->weld_label("num_separator"))
    , m_xNumberingSeparatorED(m_xBuilder->weld_entry("num_separator_edit"))
    , m_xSepText(m_xBuilder->weld_label("separator_label"))
    , m_xSepEdit(m_xBuilder->weld_entry("separator_edit"))
    , m_xPosText(m_xBuilder->weld_label("position_label"))
    , m_xPosBox(m_xBuilder->weld_combo_box("position"))
    , m_xOKButton(m_xBuilder->weld_button("ok"))
    , m_xAutoCaptionButton(m_xBuilder->weld_button("auto"))
    , m_xOptionButton(m_xBuilder->weld_button("options"))
    , m_xPreview(new weld::CustomWeld(*m_xBuilder, "preview", m_aPreview))
{
    m_xCategoryBox->connect_entry_insert_text(LINK(this, SwCaptionDialog, TextFilterHdl));

    //#i61007# order of captions
    ApplyCaptionOrder();
    SwWrtShell &rSh = rView.GetWrtShell();
    uno::Reference< frame::XModel >  xModel = rView.GetDocShell()->GetBaseModel();

    SelectionType eType = rSh.GetSelectionType();
    if ( eType & SelectionType::Ole )
    {
        eType = SelectionType::Graphic;
        uno::Reference< text::XTextEmbeddedObjectsSupplier >  xObjs(xModel, uno::UNO_QUERY);
        xNameAccess = xObjs->getEmbeddedObjects();
    }

    m_xCategoryBox->connect_changed(LINK(this, SwCaptionDialog, ModifyComboHdl));
    Link<weld::Entry&,void> aLk = LINK(this, SwCaptionDialog, ModifyEntryHdl);
    m_xTextEdit->connect_changed(aLk);
    m_xNumberingSeparatorED->connect_changed(aLk);
    m_xSepEdit->connect_changed(aLk);

    m_xFormatBox->connect_changed(LINK(this, SwCaptionDialog, SelectListBoxHdl));
    m_xOptionButton->connect_clicked(LINK(this, SwCaptionDialog, OptionHdl));
    m_xAutoCaptionButton->connect_clicked(LINK(this, SwCaptionDialog, CaptionHdl));

    m_xCategoryBox->append_text(m_sNone);
    size_t nCount = pMgr->GetFieldTypeCount();
    for (size_t i = 0; i < nCount; ++i)
    {
        SwFieldType *pType = pMgr->GetFieldType( SwFieldIds::Unknown, i );
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

        sString = rView.GetOldGrfCat();
        bCopyAttributes = true;
        //if not OLE
        if(!xNameAccess.is())
        {
            uno::Reference< text::XTextGraphicObjectsSupplier >  xGraphics(xModel, uno::UNO_QUERY);
            xNameAccess = xGraphics->getGraphicObjects();
        }

    }
    else if( eType & SelectionType::Table )
    {
        nPoolId = RES_POOLCOLL_LABEL_TABLE;
        sString = rView.GetOldTabCat();
        uno::Reference< text::XTextTablesSupplier >  xTables(xModel, uno::UNO_QUERY);
        xNameAccess = xTables->getTextTables();
    }
    else if( eType & SelectionType::Frame )
    {
        nPoolId = RES_POOLCOLL_LABEL_FRAME;
        sString = rView.GetOldFrameCat();
        uno::Reference< text::XTextFramesSupplier >  xFrames(xModel, uno::UNO_QUERY);
        xNameAccess = xFrames->getTextFrames();
    }
    else if( eType == SelectionType::Text )
    {
        nPoolId = RES_POOLCOLL_LABEL_FRAME;
        sString = rView.GetOldFrameCat();
    }
    else if( eType & SelectionType::DrawObject )
    {
        nPoolId = RES_POOLCOLL_LABEL_DRAWING;
        sString = rView.GetOldDrwCat();
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
    nCount = pMgr->GetFieldTypeCount();
    for ( size_t i = nCount; i; )
    {
        SwFieldType* pFieldType = pMgr->GetFieldType(SwFieldIds::Unknown, --i);
        if (pFieldType->GetName() == m_xCategoryBox->get_active_text())
        {
            nSelFormat = static_cast<sal_uInt16>(static_cast<SwSetExpFieldType*>(pFieldType)->GetSeqFormat());
            break;
        }
    }

    sal_uInt16 nFormatCount = pMgr->GetFormatCount(TYP_SEQFLD, false);
    for ( sal_uInt16 i = 0; i < nFormatCount; ++i )
    {
        const sal_uInt16 nFormatId = pMgr->GetFormatId(TYP_SEQFLD, i);
        m_xFormatBox->append(OUString::number(nFormatId), pMgr->GetFormatStr(TYP_SEQFLD, i));
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

    m_xSepEdit->set_text(our_aSepTextSave);
    m_xTextEdit->grab_focus();
    DrawSample();
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
    aOpt.CopyAttributes() = bCopyAttributes;
    aOpt.SetCharacterStyle( sCharacterStyle );
    rView.InsertCaption( &aOpt );
    our_aSepTextSave = m_xSepEdit->get_text();
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
    SwSequenceOptionDialog aDlg(m_xDialog.get(), rView, sFieldTypeName);
    aDlg.SetApplyBorderAndShadow(bCopyAttributes);
    aDlg.SetCharacterStyle( sCharacterStyle );
    aDlg.SetOrderNumberingFirst( bOrderNumberingFirst );
    aDlg.run();
    bCopyAttributes = aDlg.IsApplyBorderAndShadow();
    sCharacterStyle = aDlg.GetCharacterStyle();
    //#i61007# order of captions
    if( bOrderNumberingFirst != aDlg.IsOrderNumberingFirst() )
    {
        bOrderNumberingFirst = aDlg.IsOrderNumberingFirst();
        SW_MOD()->GetModuleConfig()->SetCaptionOrderNumberingFirst(bOrderNumberingFirst);
        ApplyCaptionOrder();
    }
    DrawSample();
}

IMPL_LINK_NOARG(SwCaptionDialog, SelectListBoxHdl, weld::ComboBox&, void)
{
    DrawSample();
}

void SwCaptionDialog::ModifyHdl()
{
    SwWrtShell &rSh = rView.GetWrtShell();
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
    m_xNumberingSeparatorFT->set_sensitive(bOrderNumberingFirst && !bNone);
    m_xNumberingSeparatorED->set_sensitive(bOrderNumberingFirst && !bNone);
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
    ModifyHdl();
}

IMPL_LINK_NOARG(SwCaptionDialog, CaptionHdl, weld::Button&, void)
{
    SfxItemSet aSet(rView.GetDocShell()->GetDoc()->GetAttrPool());
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
            if( !bOrderNumberingFirst )
            {
                aStr = sFieldTypeName;
                if ( !aStr.isEmpty() )
                    aStr += " ";
            }

            SwWrtShell &rSh = rView.GetWrtShell();
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
            if( bOrderNumberingFirst )
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

SwSequenceOptionDialog::SwSequenceOptionDialog(weld::Window *pParent, SwView &rV, const OUString& rSeqFieldType )
    : GenericDialogController(pParent, "modules/swriter/ui/captionoptions.ui", "CaptionOptionsDialog")
    , m_rView(rV)
    , m_aFieldTypeName(rSeqFieldType)
    , m_xLbLevel(m_xBuilder->weld_combo_box("level"))
    , m_xEdDelim(m_xBuilder->weld_entry("separator"))
    , m_xLbCharStyle(m_xBuilder->weld_combo_box("style"))
    , m_xApplyBorderAndShadowCB(m_xBuilder->weld_check_button("border_and_shadow"))
    , m_xLbCaptionOrder(m_xBuilder->weld_combo_box("caption_order"))
{
    SwWrtShell &rSh = m_rView.GetWrtShell();

    const OUString sNone(SwResId(SW_STR_NONE));

    m_xLbLevel->append_text(sNone);
    for (sal_uInt16 n = 0; n < MAXLEVEL; ++n)
        m_xLbLevel->append_text(OUString::number(n + 1));

    SwSetExpFieldType* pFieldType = static_cast<SwSetExpFieldType*>(rSh.GetFieldType(
                                        SwFieldIds::SetExp, m_aFieldTypeName ));

    sal_Unicode nLvl = MAXLEVEL;
    OUString sDelim(": ");
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
    m_xNumberingSeparatorFT->set_sensitive(bOrderNumberingFirst);
    m_xNumberingSeparatorED->set_sensitive(bOrderNumberingFirst);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
