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
#include <com/sun/star/text/XTextEmbeddedObject.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <comphelper/string.hxx>
#include <frmui.hrc>
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

    SvxStandardDialog( pParent, SW_RES(DLG_CAPTION) ),

    aTextText     (this, SW_RES(TXT_TEXT    )),
    aTextEdit     (this, SW_RES(EDT_TEXT    )),
    aSettingsFL  (this, SW_RES(FL_SETTINGS)),
    aCategoryText (this, SW_RES(TXT_CATEGORY)),
    aCategoryBox  (this, SW_RES(BOX_CATEGORY)),
    aFormatText   (this, SW_RES(TXT_FORMAT  )),
    aFormatBox    (this, SW_RES(BOX_FORMAT  )),
    aNumberingSeparatorFT(this, SW_RES(FT_NUM_SEP  )),
    aNumberingSeparatorED(this, SW_RES(ED_NUM_SEP  )),
    aSepText      (this, SW_RES(TXT_SEP     )),
    aSepEdit      (this, SW_RES(EDT_SEP     )),
    aPosText      (this, SW_RES(TXT_POS     )),
    aPosBox       (this, SW_RES(BOX_POS     )),
    aOKButton     (this, SW_RES(BTN_OK      )),
    aCancelButton (this, SW_RES(BTN_CANCEL  )),
    aHelpButton   (this, SW_RES(BTN_HELP    )),
    aAutoCaptionButton(this, SW_RES(BTN_AUTOCAPTION)),
    aOptionButton (this, SW_RES(BTN_OPTION  )),
    sNone(      SW_RES( STR_CATEGORY_NONE )),
    aPrevWin      (this, SW_RES(WIN_SAMPLE  )),
    rView( rV ),
    pMgr( new SwFldMgr(rView.GetWrtShellPtr()) ),
    bCopyAttributes( sal_False ),
    bOrderNumberingFirst( SW_MOD()->GetModuleConfig()->IsCaptionOrderNumberingFirst() )
{
    //#i61007# order of captions
    if( bOrderNumberingFirst )
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
    aCategoryBox.SetModifyHdl( aLk );
    aTextEdit   .SetModifyHdl( aLk );
    aNumberingSeparatorED.SetModifyHdl ( aLk );
    aSepEdit    .SetModifyHdl( aLk );

    aLk = LINK(this, SwCaptionDialog, SelectHdl);
    aCategoryBox.SetSelectHdl( aLk );
    aFormatBox  .SetSelectHdl( aLk );
    aOptionButton.SetClickHdl( LINK( this, SwCaptionDialog, OptionHdl ) );
    aAutoCaptionButton.SetClickHdl(LINK(this, SwCaptionDialog, CaptionHdl));

    aCategoryBox.InsertEntry( sNone );
    sal_uInt16 i, nCount = pMgr->GetFldTypeCount();
    for (i = 0; i < nCount; i++)
    {
        SwFieldType *pType = pMgr->GetFldType( USHRT_MAX, i );
        if( pType->Which() == RES_SETEXPFLD &&
            ((SwSetExpFieldType *) pType)->GetType() & nsSwGetSetExpType::GSE_SEQ )
            aCategoryBox.InsertEntry(pType->GetName());
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
            aCategoryBox.SetText( *pString );
        else
            aCategoryBox.SetText(
                    SwStyleNameMapper::GetUIName( nPoolId, aEmptyStr ));
    }

    // aFormatBox
    sal_uInt16 nSelFmt = SVX_NUM_ARABIC;
    nCount = pMgr->GetFldTypeCount();
    SwFieldType* pFldType;
    for ( i = nCount; i; )
    {
        pFldType = pMgr->GetFldType(USHRT_MAX, --i);
        if( pFldType->GetName().equals(aCategoryBox.GetText()) )
        {
            nSelFmt = (sal_uInt16)((SwSetExpFieldType*)pFldType)->GetSeqFormat();
            break;
        }
    }

    nCount = pMgr->GetFormatCount(TYP_SEQFLD, sal_False);
    for ( i = 0; i < nCount; ++i )
    {
        aFormatBox.InsertEntry( pMgr->GetFormatStr(TYP_SEQFLD, i) );
        sal_uInt16 nFmtId = pMgr->GetFormatId(TYP_SEQFLD, i);
        aFormatBox.SetEntryData( i, reinterpret_cast<void*>( nFmtId ) );
        if( nFmtId == nSelFmt )
            aFormatBox.SelectEntryPos( i );
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
            aPosBox.InsertEntry(SW_RESSTR(STR_ABOVE));
            aPosBox.InsertEntry(SW_RESSTR(STR_CP_BELOW));
            break;
        case nsSelectionType::SEL_FRM:
        case nsSelectionType::SEL_TXT:
            aPosBox.InsertEntry(SW_RESSTR(STR_BEGINNING));
            aPosBox.InsertEntry(SW_RESSTR(STR_END     ));
            break;
    }
    aPosBox.SelectEntryPos(1);
    if (eType & (nsSelectionType::SEL_GRF|nsSelectionType::SEL_DRW))
    {
        aPosText.Enable( sal_False );
        aPosBox.Enable( sal_False );
    }

    aCategoryBox.GetModifyHdl().Call(&aCategoryBox);

    FreeResource();

    CheckButtonWidth();
    aSepEdit.SetText(our_aSepTextSave);
    aTextEdit.GrabFocus();
    DrawSample();
}

void SwCaptionDialog::Apply()
{
    InsCaptionOpt aOpt;
    aOpt.UseCaption() = sal_True;
    String aName( aCategoryBox.GetText() );
    if ( aName == sNone )
        aOpt.SetCategory( aEmptyStr );
    else
        aOpt.SetCategory(comphelper::string::strip(aName, ' '));
    aOpt.SetNumType( (sal_uInt16)(sal_uIntPtr)aFormatBox.GetEntryData( aFormatBox.GetSelectEntryPos() ) );
    aOpt.SetSeparator( aSepEdit.IsEnabled() ? aSepEdit.GetText() : String() );
    aOpt.SetNumSeparator( aNumberingSeparatorED.GetText() );
    aOpt.SetCaption( aTextEdit.GetText() );
    aOpt.SetPos( aPosBox.GetSelectEntryPos() );
    aOpt.IgnoreSeqOpts() = sal_True;
    aOpt.CopyAttributes() = bCopyAttributes;
    aOpt.SetCharacterStyle( sCharacterStyle );
    rView.InsertCaption( &aOpt );
    our_aSepTextSave = aSepEdit.GetText();
}

IMPL_LINK_INLINE_START( SwCaptionDialog, OptionHdl, Button*, pButton )
{
    String sFldTypeName = aCategoryBox.GetText();
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
    String sFldTypeName = aCategoryBox.GetText();
    sal_Bool bCorrectFldName = sFldTypeName.Len() > 0;
    sal_Bool bNone = sFldTypeName == sNone;
    SwFieldType* pType = (bCorrectFldName && !bNone)
                    ? rSh.GetFldType( RES_SETEXPFLD, sFldTypeName )
                    : 0;
    aOKButton.Enable( bCorrectFldName &&
                        (!pType ||
                            ((SwSetExpFieldType*)pType)->GetType() == nsSwGetSetExpType::GSE_SEQ)
                                && 0 != sFldTypeName.Len() );
    aOptionButton.Enable( aOKButton.IsEnabled() && !bNone );
    aFormatText.Enable( !bNone );
    aFormatBox.Enable( !bNone );
    aSepText.Enable( !bNone );
    aSepEdit.Enable( !bNone );
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
    String sCaption = aTextEdit.GetText();

    // number
    String sFldTypeName = aCategoryBox.GetText();
    sal_Bool bNone = sFldTypeName == sNone;
    if( !bNone )
    {
        sal_uInt16 nNumFmt = (sal_uInt16)(sal_uIntPtr)aFormatBox.GetEntryData(
                                        aFormatBox.GetSelectEntryPos() );
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
                aStr += aNumberingSeparatorED.GetText();
                aStr += sFldTypeName;
            }

        }
        if( sCaption.Len() > 0 )
        {
            aStr += aSepEdit.GetText();
        }
    }
    aStr += sCaption;
    // do preview!
    aPrevWin.SetPreviewText( aStr );
}

void SwCaptionDialog::CheckButtonWidth()
{
    // check if the text of the AutoCaption button is to wide
    const long nOffset = 10;
    String sText = aAutoCaptionButton.GetText();
    long nTxtW = aAutoCaptionButton.GetTextWidth( sText );
    if ( sText.Search( '~' ) == STRING_NOTFOUND )
        nTxtW += nOffset;
    long nBtnW = aAutoCaptionButton.GetSizePixel().Width();
    if ( nTxtW > nBtnW )
    {
        // then broaden all buttons
        Size aNewSize;
        long nDelta = Max( ( nTxtW - nBtnW ), nOffset );
        Button* pBtns[] =
        {
            &aOKButton, &aCancelButton, &aHelpButton, &aAutoCaptionButton, &aOptionButton
        };
        Button** pCurrent = pBtns;
        for ( sal_uInt32 i = 0; i < sizeof( pBtns ) / sizeof( pBtns[ 0 ] ); ++i, ++pCurrent )
        {
            aNewSize = (*pCurrent)->GetSizePixel();
            aNewSize.Width() += nDelta;
            (*pCurrent)->SetSizePixel( aNewSize );
        }
        // and the dialog
        aNewSize = GetOutputSizePixel();
        aNewSize.Width() += nDelta;
        SetOutputSizePixel( aNewSize );
    }
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

long SwCaptionDialog::CategoryBox::PreNotify( NotifyEvent& rNEvt )
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

/*-------------------------------------------------------------------------
    //#i61007# order of captions
  -----------------------------------------------------------------------*/
void lcl_MoveH( Window& rWin, sal_Int32 nMove )
{
    Point aPos( rWin.GetPosPixel() );
    aPos.Y() += nMove;
    rWin.SetPosPixel(aPos);
}

void SwCaptionDialog::ApplyCaptionOrder()
{
    //have the settings changed?
    bool bVisible = aNumberingSeparatorED.IsVisible() != 0;
    if( bOrderNumberingFirst != bVisible )
    {
        sal_Int32 nDiff = aPosBox.GetPosPixel().Y() - aSepEdit.GetPosPixel().Y();

        aNumberingSeparatorFT.Show( bOrderNumberingFirst );
        aNumberingSeparatorED.Show( bOrderNumberingFirst );
        if( !bOrderNumberingFirst )
        {
            nDiff = -nDiff;
        }
        lcl_MoveH( aCategoryText, 2 * nDiff);
        lcl_MoveH( aFormatText, -nDiff );
        lcl_MoveH( aFormatBox, -nDiff );
        lcl_MoveH( aCategoryBox, 2 * nDiff);
        lcl_MoveH( aSepText, nDiff );
        lcl_MoveH( aSepEdit, nDiff );
        lcl_MoveH( aPosText, nDiff );
        lcl_MoveH( aPosBox, nDiff );
        lcl_MoveH( aPrevWin, nDiff );
        Size aDlgSize( GetSizePixel() );
        aDlgSize.Height() += nDiff;
        SetSizePixel( aDlgSize );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
