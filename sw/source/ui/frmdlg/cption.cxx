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
#include <frmui.hrc>
#include <globals.hrc>
#include <cption.hrc>
#include <SwStyleNameMapper.hxx>

using namespace ::com::sun::star;

extern String* GetOldGrfCat();
extern String* GetOldTabCat();
extern String* GetOldFrmCat();
extern String* GetOldDrwCat();

class SwSequenceOptionDialog : public SvxStandardDialog
{
    FixedLine       aFlHeader;
    FixedText       aFtLevel;
    ListBox         aLbLevel;
    FixedText       aFtDelim;
    Edit            aEdDelim;

    FixedLine       aFlCatAndFrame;
    FixedText       aFtCharStyle;
    ListBox         aLbCharStyle;
    CheckBox        aApplyBorderAndShadowCB;

    //#i61007# order of captions
    FixedLine       aFlCaptionOrder;
    FixedText       aFtCaptionOrder;
    ListBox         aLbCaptionOrder;

    OKButton        aOKButton;
    CancelButton    aCancelButton;
    HelpButton      aHelpButton;

    SwView&         rView;
    String          aFldTypeName;

public:
    SwSequenceOptionDialog( Window *pParent, SwView &rV,
                            const String& rSeqFldType );
    virtual ~SwSequenceOptionDialog();
    virtual void Apply();

    bool IsApplyBorderAndShadow( void ) { return aApplyBorderAndShadowCB.IsChecked(); }
    void SetApplyBorderAndShadow( bool bSet )  { aApplyBorderAndShadowCB.Check(bSet); }

    //#i61007# order of captions
    bool IsOrderNumberingFirst() const {return aLbCaptionOrder.GetSelectEntryPos() == 1;}
    void SetOrderNumberingFirst(bool bSet) { aLbCaptionOrder.SelectEntryPos( bSet ? 1 : 0 ); }

    void    SetCharacterStyle(const String& rStyle);
    String  GetCharacterStyle() const;
};

String SwCaptionDialog::our_aSepTextSave = rtl::OUString(": "); // Caption separator text

SwCaptionDialog::SwCaptionDialog( Window *pParent, SwView &rV ) :
    SvxStandardDialog( pParent, "InsertCaptionDialog", "modules/swriter/ui/insertcaption.ui" ),
    sNone( SW_RES( STR_CAPTION_CATEGORY_NONE )),
    rView( rV ),
    pMgr( new SwFldMgr(rView.GetWrtShellPtr()) ),
    bCopyAttributes( sal_False ),
    bOrderNumberingFirst( SW_MOD()->GetModuleConfig()->IsCaptionOrderNumberingFirst() )
{
    get(m_pTextEdit, "caption_edit");
    get(m_pCategoryBox, "category");
    get(m_pFormatText, "numbering_label");
    get(m_pFormatBox, "numbering");
    get(m_pNumberingSeparatorFT, "num_separator");
    get(m_pNumberingSeparatorED, "num_seperator_edit");
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

    Link aLk = LINK( this, SwCaptionDialog, ModifyHdl );
    m_pCategoryBox->SetModifyHdl( aLk );
    m_pTextEdit->SetModifyHdl( aLk );
    m_pNumberingSeparatorED->SetModifyHdl ( aLk );
    m_pSepEdit->SetModifyHdl( aLk );

    aLk = LINK(this, SwCaptionDialog, SelectHdl);
    m_pCategoryBox->SetSelectHdl( aLk );
    m_pFormatBox->SetSelectHdl( aLk );
    m_pOptionButton->SetClickHdl( LINK( this, SwCaptionDialog, OptionHdl ) );
    m_pAutoCaptionButton->SetClickHdl(LINK(this, SwCaptionDialog, CaptionHdl));

    m_pCategoryBox->InsertEntry( sNone );
    sal_uInt16 i, nCount = pMgr->GetFldTypeCount();
    for (i = 0; i < nCount; i++)
    {
        SwFieldType *pType = pMgr->GetFldType( USHRT_MAX, i );
        if( pType->Which() == RES_SETEXPFLD &&
            ((SwSetExpFieldType *) pType)->GetType() & nsSwGetSetExpType::GSE_SEQ )
            m_pCategoryBox->InsertEntry(pType->GetName());
    }

    String* pString = 0;
    sal_uInt16 nPoolId = 0;
    if (eType & nsSelectionType::SEL_GRF)
    {
        nPoolId = RES_POOLCOLL_LABEL_ABB;
        pString = ::GetOldGrfCat();
        bCopyAttributes = sal_True;
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
        pString = ::GetOldTabCat();
        uno::Reference< text::XTextTablesSupplier >  xTables(xModel, uno::UNO_QUERY);
        xNameAccess = xTables->getTextTables();
        sObjectName = rSh.GetTableFmt()->GetName();
    }
    else if( eType & nsSelectionType::SEL_FRM )
    {
        nPoolId = RES_POOLCOLL_LABEL_FRAME;
        pString = ::GetOldFrmCat();
         uno::Reference< text::XTextFramesSupplier >  xFrms(xModel, uno::UNO_QUERY);
        xNameAccess = xFrms->getTextFrames();
        sObjectName = rSh.GetFlyName();
    }
    else if( eType == nsSelectionType::SEL_TXT )
    {
        nPoolId = RES_POOLCOLL_LABEL_FRAME;
        pString = ::GetOldFrmCat();
    }
    else if( eType & nsSelectionType::SEL_DRW )
    {
        nPoolId = RES_POOLCOLL_LABEL_DRAWING;
        pString = ::GetOldDrwCat();
    }
    if( nPoolId )
    {
        if( pString && pString->Len())
            m_pCategoryBox->SetText( *pString );
        else
            m_pCategoryBox->SetText(
                    SwStyleNameMapper::GetUIName( nPoolId, aEmptyStr ));
    }

    // aFormatBox
    sal_uInt16 nSelFmt = SVX_NUM_ARABIC;
    nCount = pMgr->GetFldTypeCount();
    SwFieldType* pFldType;
    for ( i = nCount; i; )
    {
        pFldType = pMgr->GetFldType(USHRT_MAX, --i);
        if( pFldType->GetName().equals(m_pCategoryBox->GetText()) )
        {
            nSelFmt = (sal_uInt16)((SwSetExpFieldType*)pFldType)->GetSeqFormat();
            break;
        }
    }

    nCount = pMgr->GetFormatCount(TYP_SEQFLD, sal_False);
    for ( i = 0; i < nCount; ++i )
    {
        m_pFormatBox->InsertEntry( pMgr->GetFormatStr(TYP_SEQFLD, i) );
        sal_uInt16 nFmtId = pMgr->GetFormatId(TYP_SEQFLD, i);
        m_pFormatBox->SetEntryData( i, reinterpret_cast<void*>( nFmtId ) );
        if( nFmtId == nSelFmt )
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
    if (eType & (nsSelectionType::SEL_GRF|nsSelectionType::SEL_DRW))
    {
        m_pPosText->Enable( sal_False );
        m_pPosBox->Enable( sal_False );
    }

    m_pCategoryBox->GetModifyHdl().Call(m_pCategoryBox);

    m_pSepEdit->SetText(our_aSepTextSave);
    m_pTextEdit->GrabFocus();
    DrawSample();
}

void SwCaptionDialog::Apply()
{
    InsCaptionOpt aOpt;
    aOpt.UseCaption() = sal_True;
    String aName( m_pCategoryBox->GetText() );
    if ( aName == sNone )
    {
        aOpt.SetCategory( aEmptyStr );
        aOpt.SetNumSeparator( aEmptyStr );
    }
    else
    {
        aOpt.SetCategory(comphelper::string::strip(aName, ' '));
        aOpt.SetNumSeparator( m_pNumberingSeparatorED->GetText() );
    }
    aOpt.SetNumType( (sal_uInt16)(sal_uIntPtr)m_pFormatBox->GetEntryData( m_pFormatBox->GetSelectEntryPos() ) );
    aOpt.SetSeparator( m_pSepEdit->IsEnabled() ? m_pSepEdit->GetText() : String() );
    aOpt.SetCaption( m_pTextEdit->GetText() );
    aOpt.SetPos( m_pPosBox->GetSelectEntryPos() );
    aOpt.IgnoreSeqOpts() = sal_True;
    aOpt.CopyAttributes() = bCopyAttributes;
    aOpt.SetCharacterStyle( sCharacterStyle );
    rView.InsertCaption( &aOpt );
    our_aSepTextSave = m_pSepEdit->GetText();
}

IMPL_LINK_INLINE_START( SwCaptionDialog, OptionHdl, Button*, pButton )
{
    String sFldTypeName = m_pCategoryBox->GetText();
    if(sFldTypeName == sNone)
        sFldTypeName = aEmptyStr;
    SwSequenceOptionDialog  aDlg( pButton, rView, sFldTypeName );
    aDlg.SetApplyBorderAndShadow(bCopyAttributes);
    aDlg.SetCharacterStyle( sCharacterStyle );
    aDlg.SetOrderNumberingFirst( bOrderNumberingFirst );
    aDlg.Execute();
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
    return 0;
}
IMPL_LINK_INLINE_END( SwCaptionDialog, OptionHdl, Button*, pButton )

IMPL_LINK_NOARG_INLINE_START(SwCaptionDialog, SelectHdl)
{
    DrawSample();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwCaptionDialog, SelectHdl)

IMPL_LINK_NOARG(SwCaptionDialog, ModifyHdl)
{
    SwWrtShell &rSh = rView.GetWrtShell();
    String sFldTypeName = m_pCategoryBox->GetText();
    sal_Bool bCorrectFldName = sFldTypeName.Len() > 0;
    sal_Bool bNone = sFldTypeName == sNone;
    SwFieldType* pType = (bCorrectFldName && !bNone)
                    ? rSh.GetFldType( RES_SETEXPFLD, sFldTypeName )
                    : 0;
    m_pOKButton->Enable( bCorrectFldName &&
                        (!pType ||
                            ((SwSetExpFieldType*)pType)->GetType() == nsSwGetSetExpType::GSE_SEQ)
                                && 0 != sFldTypeName.Len() );
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
    SwCaptionOptDlg aDlg( this, aSet );
    aDlg.Execute();

    return 0;
}

void SwCaptionDialog::DrawSample()
{
    String aStr;
    String sCaption = m_pTextEdit->GetText();

    // number
    String sFldTypeName = m_pCategoryBox->GetText();
    sal_Bool bNone = sFldTypeName == sNone;
    if( !bNone )
    {
        sal_uInt16 nNumFmt = (sal_uInt16)(sal_uIntPtr)m_pFormatBox->GetEntryData(
                                        m_pFormatBox->GetSelectEntryPos() );
        if( SVX_NUM_NUMBER_NONE != nNumFmt )
        {
            // category
            //#i61007# order of captions
            if( !bOrderNumberingFirst )
            {
                aStr += sFldTypeName;
                if ( aStr.Len() > 0 )
                    aStr += ' ';
            }

            SwWrtShell &rSh = rView.GetWrtShell();
            SwSetExpFieldType* pFldType = (SwSetExpFieldType*)rSh.GetFldType(
                                            RES_SETEXPFLD, sFldTypeName );
            if( pFldType && pFldType->GetOutlineLvl() < MAXLEVEL )
            {
                sal_Int8 nLvl = pFldType->GetOutlineLvl();
                SwNumberTree::tNumberVector aNumVector;
                for( sal_Int8 i = 0; i <= nLvl; ++i )
                    aNumVector.push_back(1);

                String sNumber( rSh.GetOutlineNumRule()->
                                MakeNumString(aNumVector, sal_False ));
                if( sNumber.Len() )
                    (aStr += sNumber) += pFldType->GetDelimiter();
            }

            switch( nNumFmt )
            {
            case SVX_NUM_CHARS_UPPER_LETTER:    aStr += 'A'; break;
            case SVX_NUM_CHARS_UPPER_LETTER_N:  aStr += 'A'; break;
            case SVX_NUM_CHARS_LOWER_LETTER:    aStr += 'a'; break;
            case SVX_NUM_CHARS_LOWER_LETTER_N:  aStr += 'a'; break;
            case SVX_NUM_ROMAN_UPPER:           aStr += 'I'; break;
            case SVX_NUM_ROMAN_LOWER:           aStr += 'i'; break;
            default:                    aStr += '1'; break;
            }
            //#i61007# order of captions
            if( bOrderNumberingFirst )
            {
                aStr += m_pNumberingSeparatorED->GetText();
                aStr += sFldTypeName;
            }

        }
        if( sCaption.Len() > 0 )
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
    delete pMgr;
}

SwSequenceOptionDialog::SwSequenceOptionDialog( Window *pParent, SwView &rV,
                                            const String& rSeqFldType )
    : SvxStandardDialog( pParent, SW_RES(DLG_SEQUENCE_OPTION) ),
    aFlHeader       (this, SW_RES(FL_HEADER    )),
    aFtLevel        (this, SW_RES(FT_LEVEL     )),
    aLbLevel        (this, SW_RES(LB_LEVEL     )),
    aFtDelim        (this, SW_RES(FT_SEPARATOR )),
    aEdDelim        (this, SW_RES(ED_SEPARATOR )),
    aFlCatAndFrame  (this, SW_RES(FL_CATANDFRAME)),
    aFtCharStyle    (this, SW_RES(FT_CHARSTYLE )),
    aLbCharStyle    (this, SW_RES(LB_CHARSTYLE )),
    aApplyBorderAndShadowCB(this, SW_RES(CB_APPLYBAS)),
    aFlCaptionOrder(this, SW_RES( FL_ORDER )), //#i61007# order of captions
    aFtCaptionOrder(this, SW_RES( FT_ORDER )),
    aLbCaptionOrder(this, SW_RES( LB_ORDER )),
    aOKButton       (this, SW_RES(BTN_OK       )),
    aCancelButton   (this, SW_RES(BTN_CANCEL   )),
    aHelpButton     (this, SW_RES(BTN_HELP     )),

    rView( rV ),
    aFldTypeName( rSeqFldType )
{
    FreeResource();
    SwWrtShell &rSh = rView.GetWrtShell();

    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
        aLbLevel.InsertEntry( String::CreateFromInt32(n+1) );

    SwSetExpFieldType* pFldType = (SwSetExpFieldType*)rSh.GetFldType(
                                        RES_SETEXPFLD, aFldTypeName );

    sal_Unicode nLvl = MAXLEVEL;
    rtl::OUString sDelim(": ");
    if( pFldType )
    {
        sDelim = pFldType->GetDelimiter();
        nLvl = pFldType->GetOutlineLvl();
    }

    aLbLevel.SelectEntryPos( nLvl < MAXLEVEL ? nLvl + 1 : 0 );
    aEdDelim.SetText(sDelim);

    ::FillCharStyleListBox( aLbCharStyle, rView.GetDocShell(), sal_True, sal_True );
    aLbCharStyle.SelectEntryPos( 0 );
}

SwSequenceOptionDialog::~SwSequenceOptionDialog()
{
}

void SwSequenceOptionDialog::Apply()
{
    SwWrtShell &rSh = rView.GetWrtShell();
    SwSetExpFieldType* pFldType = (SwSetExpFieldType*)rSh.GetFldType(
                                        RES_SETEXPFLD, aFldTypeName );

    sal_Int8 nLvl = (sal_Int8)( aLbLevel.GetSelectEntryPos() - 1);
    sal_Unicode cDelim = aEdDelim.GetText().GetChar(0);

    sal_Bool bUpdate = sal_True;
    if( pFldType )
    {
        pFldType->SetDelimiter( rtl::OUString(cDelim) );
        pFldType->SetOutlineLvl( nLvl );
    }
    else if( aFldTypeName.Len() && nLvl < MAXLEVEL )
    {
        // then we have to insert that
        SwSetExpFieldType aFldType( rSh.GetDoc(), aFldTypeName, nsSwGetSetExpType::GSE_SEQ );
        aFldType.SetDelimiter( rtl::OUString(cDelim) );
        aFldType.SetOutlineLvl( nLvl );
        rSh.InsertFldType( aFldType );
    }
    else
        bUpdate = sal_False;

    if( bUpdate )
        rSh.UpdateExpFlds();
}

String  SwSequenceOptionDialog::GetCharacterStyle() const
{
    String sRet;
    if(aLbCharStyle.GetSelectEntryPos())
        sRet = aLbCharStyle.GetSelectEntry();
    return sRet;
}

void    SwSequenceOptionDialog::SetCharacterStyle(const String& rStyle)
{
    aLbCharStyle.SelectEntryPos(0);
    aLbCharStyle.SelectEntry(rStyle);
}

long CategoryBox::PreNotify( NotifyEvent& rNEvt )
{
    long nHandled = 0;
    if( rNEvt.GetType() == EVENT_KEYINPUT &&
        rNEvt.GetKeyEvent()->GetCharCode() )
    {
        const KeyEvent* pEvent = rNEvt.GetKeyEvent();
        const KeyCode&  rKeyCode = pEvent->GetKeyCode();
        sal_uInt16 nTmpCode = rKeyCode.GetFullCode() & ~KEY_ALLMODTYPE;

        if(nTmpCode != KEY_BACKSPACE && nTmpCode != KEY_RETURN
                && nTmpCode != KEY_TAB && nTmpCode != KEY_ESCAPE)
        {
            rtl::OUString sKey( pEvent->GetCharCode() );
            String sName( GetText() );
            Selection aSel( GetSelection() );
            aSel.Justify();
            if( aSel.Len() )
                sName.Erase( (xub_StrLen)aSel.Min(), (xub_StrLen)aSel.Len() );
            sName.Insert( sKey, (xub_StrLen)aSel.Min() );
            if( !SwCalc::IsValidVarName( sName ))
                nHandled = 1;
        }
    }
    if(!nHandled)
        nHandled = ComboBox::PreNotify( rNEvt );
    return nHandled;
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeCategoryBox(Window* pParent)
{
    CategoryBox* pCategoryBox = new CategoryBox(pParent, WB_LEFT | WB_DROPDOWN | WB_VCENTER | WB_3DLOOK);
    pCategoryBox->EnableAutoSize(true);
    return pCategoryBox;
}

/*-------------------------------------------------------------------------
    //#i61007# order of captions
  -----------------------------------------------------------------------*/
void SwCaptionDialog::ApplyCaptionOrder()
{
    m_pNumberingSeparatorFT->Enable(bOrderNumberingFirst);
    m_pNumberingSeparatorED->Enable(bOrderNumberingFirst);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
