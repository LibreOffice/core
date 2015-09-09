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
#include <com/sun/star/frame/XStorable.hpp>
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
#include "initui.hxx"
#include <frmui.hrc>
#include <globals.hrc>
#include <SwStyleNameMapper.hxx>

using namespace ::com::sun::star;

class SwSequenceOptionDialog : public SvxStandardDialog
{
    VclPtr<ListBox>        m_pLbLevel;
    VclPtr<Edit>           m_pEdDelim;

    VclPtr<ListBox>        m_pLbCharStyle;
    VclPtr<CheckBox>       m_pApplyBorderAndShadowCB;

    //#i61007# order of captions
    VclPtr<ListBox>        m_pLbCaptionOrder;

    SwView&         rView;
    OUString        aFieldTypeName;

public:
    SwSequenceOptionDialog( vcl::Window *pParent, SwView &rV,
                            const OUString& rSeqFieldType );
    virtual ~SwSequenceOptionDialog();
    virtual void dispose() SAL_OVERRIDE;
    virtual void Apply() SAL_OVERRIDE;

    bool IsApplyBorderAndShadow() { return m_pApplyBorderAndShadowCB->IsChecked(); }
    void SetApplyBorderAndShadow( bool bSet )  { m_pApplyBorderAndShadowCB->Check(bSet); }

    //#i61007# order of captions
    bool IsOrderNumberingFirst() const {return m_pLbCaptionOrder->GetSelectEntryPos() == 1;}
    void SetOrderNumberingFirst(bool bSet) { m_pLbCaptionOrder->SelectEntryPos( bSet ? 1 : 0 ); }

    void      SetCharacterStyle(const OUString& rStyle);
    OUString  GetCharacterStyle() const;
};

OUString SwCaptionDialog::our_aSepTextSave(": "); // Caption separator text

//Resolves: fdo#47427 disallow typing *or* pasting content into the category box
OUString TextFilterAutoConvert::filter(const OUString &rText)
{
    if (rText != m_sNone && !SwCalc::IsValidVarName(rText))
        return m_sLastGoodText;
    m_sLastGoodText = rText;
    return rText;
}

SwCaptionDialog::SwCaptionDialog( vcl::Window *pParent, SwView &rV ) :
    SvxStandardDialog( pParent, "InsertCaptionDialog", "modules/swriter/ui/insertcaption.ui" ),
    m_sNone( SW_RESSTR(SW_STR_NONE) ),
    m_aTextFilter(m_sNone),
    rView( rV ),
    pMgr( new SwFieldMgr(rView.GetWrtShellPtr()) ),
    bCopyAttributes( false ),
    bOrderNumberingFirst( SW_MOD()->GetModuleConfig()->IsCaptionOrderNumberingFirst() )
{
    get(m_pTextEdit, "caption_edit");
    get(m_pCategoryBox, "category");
    m_pCategoryBox->SetTextFilter(&m_aTextFilter);
    get(m_pFormatText, "numbering_label");
    get(m_pFormatBox, "numbering");
    get(m_pNumberingSeparatorFT, "num_separator");
    get(m_pNumberingSeparatorED, "num_separator_edit");
    get(m_pSepText, "separator_label");
    get(m_pSepEdit, "separator_edit");
    get(m_pPosText, "position_label");
    get(m_pPosBox, "position");
    get(m_pPreview, "preview");
    get(m_pOKButton, "ok");
    get(m_pAutoCaptionButton, "auto");
    get(m_pOptionButton, "options");

    //#i61007# order of captions
    ApplyCaptionOrder();
    SwWrtShell &rSh = rView.GetWrtShell();
     uno::Reference< frame::XModel >  xModel = rView.GetDocShell()->GetBaseModel();

    eType = rSh.GetSelectionType();
    if ( eType & nsSelectionType::SEL_OLE )
    {
        eType = nsSelectionType::SEL_GRF;
         uno::Reference< text::XTextEmbeddedObjectsSupplier >  xObjs(xModel, uno::UNO_QUERY);
        xNameAccess = xObjs->getEmbeddedObjects();
    }

    Link<> aLk = LINK( this, SwCaptionDialog, ModifyHdl );
    m_pCategoryBox->SetModifyHdl( aLk );
    m_pTextEdit->SetModifyHdl( aLk );
    m_pNumberingSeparatorED->SetModifyHdl ( aLk );
    m_pSepEdit->SetModifyHdl( aLk );

    aLk = LINK(this, SwCaptionDialog, SelectHdl);
    m_pCategoryBox->SetSelectHdl( aLk );
    m_pFormatBox->SetSelectHdl( aLk );
    m_pOptionButton->SetClickHdl( LINK( this, SwCaptionDialog, OptionHdl ) );
    m_pAutoCaptionButton->SetClickHdl(LINK(this, SwCaptionDialog, CaptionHdl));

    m_pCategoryBox->InsertEntry( m_sNone );
    sal_uInt16 nCount = pMgr->GetFieldTypeCount();
    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        SwFieldType *pType = pMgr->GetFieldType( USHRT_MAX, i );
        if( pType->Which() == RES_SETEXPFLD &&
            static_cast<SwSetExpFieldType *>( pType)->GetType() & nsSwGetSetExpType::GSE_SEQ )
            m_pCategoryBox->InsertEntry(pType->GetName());
    }

    OUString sString;
    sal_uInt16 nPoolId = 0;
    if (eType & nsSelectionType::SEL_GRF)
    {
        nPoolId = RES_POOLCOLL_LABEL_ABB;
        sString = ::GetOldGrfCat();
        bCopyAttributes = true;
        sObjectName = rSh.GetFlyName();
        //if not OLE
        if(!xNameAccess.is())
        {
         uno::Reference< text::XTextGraphicObjectsSupplier >  xGraphics(xModel, uno::UNO_QUERY);
            xNameAccess = xGraphics->getGraphicObjects();
        }

    }
    else if( eType & nsSelectionType::SEL_TBL )
    {
        nPoolId = RES_POOLCOLL_LABEL_TABLE;
        sString = ::GetOldTabCat();
        uno::Reference< text::XTextTablesSupplier >  xTables(xModel, uno::UNO_QUERY);
        xNameAccess = xTables->getTextTables();
        sObjectName = rSh.GetTableFormat()->GetName();
    }
    else if( eType & nsSelectionType::SEL_FRM )
    {
        nPoolId = RES_POOLCOLL_LABEL_FRAME;
        sString = ::GetOldFrmCat();
         uno::Reference< text::XTextFramesSupplier >  xFrms(xModel, uno::UNO_QUERY);
        xNameAccess = xFrms->getTextFrames();
        sObjectName = rSh.GetFlyName();
    }
    else if( eType == nsSelectionType::SEL_TXT )
    {
        nPoolId = RES_POOLCOLL_LABEL_FRAME;
        sString = ::GetOldFrmCat();
    }
    else if( eType & nsSelectionType::SEL_DRW )
    {
        nPoolId = RES_POOLCOLL_LABEL_DRAWING;
        sString = ::GetOldDrwCat();
    }
    if( nPoolId )
    {
        if (!sString.isEmpty())
            m_pCategoryBox->SetText( sString );
        else
            m_pCategoryBox->SetText(
                    SwStyleNameMapper::GetUIName( nPoolId, OUString() ));
    }

    // aFormatBox
    sal_uInt16 nSelFormat = SVX_NUM_ARABIC;
    nCount = pMgr->GetFieldTypeCount();
    for ( sal_uInt16 i = nCount; i; )
    {
        SwFieldType* pFieldType = pMgr->GetFieldType(USHRT_MAX, --i);
        if( pFieldType->GetName().equals(m_pCategoryBox->GetText()) )
        {
            nSelFormat = (sal_uInt16)static_cast<SwSetExpFieldType*>(pFieldType)->GetSeqFormat();
            break;
        }
    }

    nCount = pMgr->GetFormatCount(TYP_SEQFLD, false);
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        m_pFormatBox->InsertEntry( pMgr->GetFormatStr(TYP_SEQFLD, i) );
        const sal_uInt16 nFormatId = pMgr->GetFormatId(TYP_SEQFLD, i);
        m_pFormatBox->SetEntryData( i, reinterpret_cast<void*>( nFormatId ) );
        if( nFormatId == nSelFormat )
            m_pFormatBox->SelectEntryPos( i );
    }

    // aPosBox
    switch (eType)
    {
        case nsSelectionType::SEL_GRF:
        case nsSelectionType::SEL_TBL:
        case nsSelectionType::SEL_TBL | nsSelectionType::SEL_NUM:
        case nsSelectionType::SEL_TBL | nsSelectionType::SEL_TXT:
        case nsSelectionType::SEL_TBL | nsSelectionType::SEL_NUM | nsSelectionType::SEL_TXT:
        case nsSelectionType::SEL_DRW:
        case nsSelectionType::SEL_DRW | nsSelectionType::SEL_BEZ:
            m_pPosBox->InsertEntry(SW_RESSTR(STR_CAPTION_ABOVE));
            m_pPosBox->InsertEntry(SW_RESSTR(STR_CAPTION_BELOW));
            break;
        case nsSelectionType::SEL_FRM:
        case nsSelectionType::SEL_TXT:
            m_pPosBox->InsertEntry(SW_RESSTR(STR_CAPTION_BEGINNING));
            m_pPosBox->InsertEntry(SW_RESSTR(STR_CAPTION_END     ));
            break;
    }
    m_pPosBox->SelectEntryPos(1);

    m_pCategoryBox->GetModifyHdl().Call(m_pCategoryBox);

    m_pSepEdit->SetText(our_aSepTextSave);
    m_pTextEdit->GrabFocus();
    DrawSample();
}

void SwCaptionDialog::Apply()
{
    InsCaptionOpt aOpt;
    aOpt.UseCaption() = true;
    OUString aName( m_pCategoryBox->GetText() );
    if ( aName == m_sNone )
    {
        aOpt.SetCategory( OUString() );
        aOpt.SetNumSeparator( OUString() );
    }
    else
    {
        aOpt.SetCategory(comphelper::string::strip(aName, ' '));
        aOpt.SetNumSeparator( m_pNumberingSeparatorED->GetText() );
    }
    aOpt.SetNumType( (sal_uInt16)reinterpret_cast<sal_uIntPtr>(m_pFormatBox->GetSelectEntryData()) );
    aOpt.SetSeparator( m_pSepEdit->IsEnabled() ? m_pSepEdit->GetText() : OUString() );
    aOpt.SetCaption( m_pTextEdit->GetText() );
    aOpt.SetPos( m_pPosBox->GetSelectEntryPos() );
    aOpt.IgnoreSeqOpts() = true;
    aOpt.CopyAttributes() = bCopyAttributes;
    aOpt.SetCharacterStyle( sCharacterStyle );
    rView.InsertCaption( &aOpt );
    our_aSepTextSave = m_pSepEdit->GetText();
}

IMPL_LINK( SwCaptionDialog, OptionHdl, Button*, pButton )
{
    OUString sFieldTypeName = m_pCategoryBox->GetText();
    if(sFieldTypeName == m_sNone)
        sFieldTypeName.clear();
    ScopedVclPtrInstance< SwSequenceOptionDialog > aDlg( pButton, rView, sFieldTypeName );
    aDlg->SetApplyBorderAndShadow(bCopyAttributes);
    aDlg->SetCharacterStyle( sCharacterStyle );
    aDlg->SetOrderNumberingFirst( bOrderNumberingFirst );
    aDlg->Execute();
    bCopyAttributes = aDlg->IsApplyBorderAndShadow();
    sCharacterStyle = aDlg->GetCharacterStyle();
    //#i61007# order of captions
    if( bOrderNumberingFirst != aDlg->IsOrderNumberingFirst() )
    {
        bOrderNumberingFirst = aDlg->IsOrderNumberingFirst();
        SW_MOD()->GetModuleConfig()->SetCaptionOrderNumberingFirst(bOrderNumberingFirst);
        ApplyCaptionOrder();
    }
    DrawSample();
    return 0;
}

IMPL_LINK_NOARG(SwCaptionDialog, SelectHdl)
{
    DrawSample();
    return 0;
}

IMPL_LINK_NOARG(SwCaptionDialog, ModifyHdl)
{
    SwWrtShell &rSh = rView.GetWrtShell();
    OUString sFieldTypeName = m_pCategoryBox->GetText();
    bool bCorrectFieldName = !sFieldTypeName.isEmpty();
    bool bNone = sFieldTypeName == m_sNone;
    SwFieldType* pType = (bCorrectFieldName && !bNone)
                    ? rSh.GetFieldType( RES_SETEXPFLD, sFieldTypeName )
                    : 0;
    m_pOKButton->Enable( bCorrectFieldName &&
                        (!pType ||
                            static_cast<SwSetExpFieldType*>(pType)->GetType() == nsSwGetSetExpType::GSE_SEQ) );
    m_pOptionButton->Enable( m_pOKButton->IsEnabled() && !bNone );
    m_pNumberingSeparatorFT->Enable( bOrderNumberingFirst && !bNone );
    m_pNumberingSeparatorED->Enable( bOrderNumberingFirst && !bNone );
    m_pFormatText->Enable( !bNone );
    m_pFormatBox->Enable( !bNone );
    m_pSepText->Enable( !bNone );
    m_pSepEdit->Enable( !bNone );
    DrawSample();
    return 0;
}

IMPL_LINK_NOARG(SwCaptionDialog, CaptionHdl)
{
    SfxItemSet  aSet( rView.GetDocShell()->GetDoc()->GetAttrPool() );
    ScopedVclPtrInstance< SwCaptionOptDlg > aDlg( this, aSet );
    aDlg->Execute();

    return 0;
}

void SwCaptionDialog::DrawSample()
{
    OUString aStr;
    OUString sCaption = m_pTextEdit->GetText();

    // number
    OUString sFieldTypeName = m_pCategoryBox->GetText();
    bool bNone = sFieldTypeName == m_sNone;
    if( !bNone )
    {
        const sal_uInt16 nNumFormat = (sal_uInt16)reinterpret_cast<sal_uIntPtr>(m_pFormatBox->GetSelectEntryData());
        if( SVX_NUM_NUMBER_NONE != nNumFormat )
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
                                            RES_SETEXPFLD, sFieldTypeName ));
            if( pFieldType && pFieldType->GetOutlineLvl() < MAXLEVEL )
            {
                sal_Int8 nLvl = pFieldType->GetOutlineLvl();
                SwNumberTree::tNumberVector aNumVector;
                for( sal_Int8 i = 0; i <= nLvl; ++i )
                    aNumVector.push_back(1);

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
                aStr += m_pNumberingSeparatorED->GetText() + sFieldTypeName;
            }

        }
        if( !sCaption.isEmpty() )
        {
            aStr += m_pSepEdit->GetText();
        }
    }
    aStr += sCaption;
    // do preview!
    m_pPreview->SetPreviewText( aStr );
}

SwCaptionDialog::~SwCaptionDialog()
{
    disposeOnce();
}

void SwCaptionDialog::dispose()
{
    delete pMgr;
    m_pTextEdit.clear();
    m_pCategoryBox.clear();
    m_pFormatText.clear();
    m_pFormatBox.clear();
    m_pNumberingSeparatorFT.clear();
    m_pNumberingSeparatorED.clear();
    m_pSepText.clear();
    m_pSepEdit.clear();
    m_pPosText.clear();
    m_pPosBox.clear();
    m_pOKButton.clear();
    m_pAutoCaptionButton.clear();
    m_pOptionButton.clear();
    m_pPreview.clear();
    SvxStandardDialog::dispose();
}

SwSequenceOptionDialog::SwSequenceOptionDialog( vcl::Window *pParent, SwView &rV,
                                            const OUString& rSeqFieldType )
    : SvxStandardDialog( pParent, "CaptionOptionsDialog", "modules/swriter/ui/captionoptions.ui" ),
    rView( rV ),
    aFieldTypeName( rSeqFieldType )
{
    get(m_pLbLevel, "level");
    get(m_pEdDelim, "separator");
    get(m_pLbCharStyle, "style");
    get(m_pApplyBorderAndShadowCB, "border_and_shadow");
    get(m_pLbCaptionOrder, "caption_order");

    SwWrtShell &rSh = rView.GetWrtShell();

    const OUString sNone(SW_RESSTR(SW_STR_NONE));

    m_pLbLevel->InsertEntry(sNone);
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
        m_pLbLevel->InsertEntry( OUString::number(n+1) );

    SwSetExpFieldType* pFieldType = static_cast<SwSetExpFieldType*>(rSh.GetFieldType(
                                        RES_SETEXPFLD, aFieldTypeName ));

    sal_Unicode nLvl = MAXLEVEL;
    OUString sDelim(": ");
    if( pFieldType )
    {
        sDelim = pFieldType->GetDelimiter();
        nLvl = pFieldType->GetOutlineLvl();
    }

    m_pLbLevel->SelectEntryPos( nLvl < MAXLEVEL ? nLvl + 1 : 0 );
    m_pEdDelim->SetText(sDelim);

    m_pLbCharStyle->InsertEntry(sNone);
    ::FillCharStyleListBox( *m_pLbCharStyle, rView.GetDocShell(), true, true );
    m_pLbCharStyle->SelectEntryPos( 0 );
}

SwSequenceOptionDialog::~SwSequenceOptionDialog()
{
    disposeOnce();
}

void SwSequenceOptionDialog::dispose()
{
    m_pLbLevel.clear();
    m_pEdDelim.clear();
    m_pLbCharStyle.clear();
    m_pApplyBorderAndShadowCB.clear();
    m_pLbCaptionOrder.clear();
    SvxStandardDialog::dispose();
}


void SwSequenceOptionDialog::Apply()
{
    SwWrtShell &rSh = rView.GetWrtShell();
    SwSetExpFieldType* pFieldType = static_cast<SwSetExpFieldType*>(rSh.GetFieldType(
                                        RES_SETEXPFLD, aFieldTypeName ));

    sal_Int8 nLvl = (sal_Int8)( m_pLbLevel->GetSelectEntryPos() - 1);
    sal_Unicode cDelim = m_pEdDelim->GetText()[0];

    bool bUpdate = true;
    if( pFieldType )
    {
        pFieldType->SetDelimiter( OUString(cDelim) );
        pFieldType->SetOutlineLvl( nLvl );
    }
    else if( !aFieldTypeName.isEmpty() && nLvl < MAXLEVEL )
    {
        // then we have to insert that
        SwSetExpFieldType aFieldType( rSh.GetDoc(), aFieldTypeName, nsSwGetSetExpType::GSE_SEQ );
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
    if(m_pLbCharStyle->GetSelectEntryPos())
        return m_pLbCharStyle->GetSelectEntry();
    return OUString();
}

void    SwSequenceOptionDialog::SetCharacterStyle(const OUString& rStyle)
{
    m_pLbCharStyle->SelectEntryPos(0);
    m_pLbCharStyle->SelectEntry(rStyle);
}

// #i61007# order of captions
void SwCaptionDialog::ApplyCaptionOrder()
{
    m_pNumberingSeparatorFT->Enable(bOrderNumberingFirst);
    m_pNumberingSeparatorED->Enable(bOrderNumberingFirst);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
