/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cption.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-08 17:32:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#pragma hdrstop

#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _CPTION_HXX
#include <cption.hxx>
#endif
#ifndef _CAPTION_HXX
#include <caption.hxx>
#endif
#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _SWDOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _CALC_HXX
#include <calc.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_GRAPHICCROP_HPP_
#include <com/sun/star/text/GraphicCrop.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTGRAPHICOBJECTSSUPPLIER_HPP_
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTTABLECURSOR_HPP_
#include <com/sun/star/text/XTextTableCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTTABLESSUPPLIER_HPP_
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TABLECOLUMNSEPARATOR_HPP_
#include <com/sun/star/text/TableColumnSeparator.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTTABLE_HPP_
#include <com/sun/star/text/XTextTable.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTEMBEDDEDOBJECTSSUPPLIER_HPP_
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTEMBEDDEDOBJECT_HPP_
#include <com/sun/star/text/XTextEmbeddedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAMESSUPPLIER_HPP_
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAME_HPP_
#include <com/sun/star/text/XTextFrame.hpp>
#endif

#ifndef _FRMUI_HRC
#include <frmui.hrc>
#endif
#ifndef _CPTION_HRC
#include <cption.hrc>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
using namespace ::com::sun::star;

//CHINA001 extern String* pOldGrfCat;
//CHINA001 extern String* pOldTabCat;
//CHINA001 extern String* pOldFrmCat;
//CHINA001 extern String* pOldDrwCat;
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

    void    SetCharacterStyle(const String& rStyle);
    String  GetCharacterStyle() const;
};

SwCaptionDialog::SwCaptionDialog( Window *pParent, SwView &rV ) :

    SvxStandardDialog( pParent, SW_RES(DLG_CAPTION) ),

    aOKButton     (this, SW_RES(BTN_OK      )),
    aCancelButton (this, SW_RES(BTN_CANCEL  )),
    aHelpButton   (this, SW_RES(BTN_HELP    )),
    aAutoCaptionButton(this, SW_RES(BTN_AUTOCAPTION)),
    aOptionButton (this, SW_RES(BTN_OPTION  )),
    aCategoryText (this, SW_RES(TXT_CATEGORY)),
    aCategoryBox  (this, SW_RES(BOX_CATEGORY)),
    aFormatText   (this, SW_RES(TXT_FORMAT  )),
    aFormatBox    (this, SW_RES(BOX_FORMAT  )),
    aTextText     (this, SW_RES(TXT_TEXT    )),
    aTextEdit     (this, SW_RES(EDT_TEXT    )),
    aSepText      (this, SW_RES(TXT_SEP     )),
    aSepEdit      (this, SW_RES(EDT_SEP     )),
    aPosText      (this, SW_RES(TXT_POS     )),
    aPosBox       (this, SW_RES(BOX_POS     )),
    aPrevWin      (this, SW_RES(WIN_SAMPLE  )),
    sNone(      ResId( STR_CATEGORY_NONE )),
    aSettingsFL  (this, SW_RES(FL_SETTINGS)),
    rView( rV ),
    bCopyAttributes( FALSE ),
    pMgr( new SwFldMgr(rView.GetWrtShellPtr()) )

{
    SwWrtShell &rSh = rView.GetWrtShell();
     uno::Reference< frame::XModel >  xModel = rView.GetDocShell()->GetBaseModel();

    eType = (SwWrtShell::SelectionType)rSh.GetSelectionType();
    if ( eType & SwWrtShell::SEL_OLE )
    {
        eType = SwWrtShell::SEL_GRF;
         uno::Reference< text::XTextEmbeddedObjectsSupplier >  xObjs(xModel, uno::UNO_QUERY);
        xNameAccess = xObjs->getEmbeddedObjects();
    }

    Link aLk = LINK( this, SwCaptionDialog, ModifyHdl );
    aCategoryBox.SetModifyHdl( aLk );
    aTextEdit   .SetModifyHdl( aLk );

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
            ((SwSetExpFieldType *) pType)->GetType() & GSE_SEQ )
            aCategoryBox.InsertEntry(pType->GetName());
    }

    String* pString;
    sal_uInt16 nPoolId = 0;
    if (eType & SwWrtShell::SEL_GRF)
    {
        nPoolId = RES_POOLCOLL_LABEL_ABB;
        pString = ::GetOldGrfCat(); //CHINA001 pString = pOldGrfCat;
        bCopyAttributes = TRUE;
        sObjectName = rSh.GetFlyName();
        //if not OLE
        if(!xNameAccess.is())
        {
         uno::Reference< text::XTextGraphicObjectsSupplier >  xGraphics(xModel, uno::UNO_QUERY);
            xNameAccess = xGraphics->getGraphicObjects();
        }

    }
    else if( eType & SwWrtShell::SEL_TBL )
    {
        nPoolId = RES_POOLCOLL_LABEL_TABLE;
        pString = ::GetOldTabCat(); //CHINA001 pString = pOldTabCat;
        uno::Reference< text::XTextTablesSupplier >  xTables(xModel, uno::UNO_QUERY);
        xNameAccess = xTables->getTextTables();
        sObjectName = rSh.GetTableFmt()->GetName();
    }
    else if( eType & SwWrtShell::SEL_FRM )
    {
        nPoolId = RES_POOLCOLL_LABEL_FRAME;
        pString = ::GetOldFrmCat(); //CHINA001 pString = pOldFrmCat;
         uno::Reference< text::XTextFramesSupplier >  xFrms(xModel, uno::UNO_QUERY);
        xNameAccess = xFrms->getTextFrames();
        sObjectName = rSh.GetFlyName();
    }
    else if( eType == SwWrtShell::SEL_TXT )
    {
        nPoolId = RES_POOLCOLL_LABEL_FRAME;
        pString = ::GetOldFrmCat(); //CHINA001 pString = pOldFrmCat;
    }
    else if( eType & SwWrtShell::SEL_DRW )
    {
        nPoolId = RES_POOLCOLL_LABEL_DRAWING;
        pString = ::GetOldDrwCat(); //CHINA001 pString = pOldDrwCat;
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
        if( ( pFldType = pMgr->GetFldType(USHRT_MAX, --i))->GetName() ==
            aCategoryBox.GetText() )
        {
            nSelFmt = (sal_uInt16)((SwSetExpFieldType*)pFldType)->GetSeqFormat();
            break;
        }


    nCount = pMgr->GetFormatCount(TYP_SEQFLD, sal_False);
    for ( i = 0; i < nCount; ++i )
    {
        aFormatBox.InsertEntry( pMgr->GetFormatStr(TYP_SEQFLD, i) );
        sal_uInt16 nFmtId = pMgr->GetFormatId(TYP_SEQFLD, i);
        aFormatBox.SetEntryData( i, (void*)nFmtId );
        if( nFmtId == nSelFmt )
            aFormatBox.SelectEntryPos( i );
    }

    // aPosBox
    switch (eType)
    {
        case SwWrtShell::SEL_GRF:
        case SwWrtShell::SEL_TBL:
        case SwWrtShell::SEL_TBL | SwWrtShell::SEL_NUM:
        case SwWrtShell::SEL_TBL | SwWrtShell::SEL_TXT:
        case SwWrtShell::SEL_TBL | SwWrtShell::SEL_NUM | SwWrtShell::SEL_TXT:
        case SwWrtShell::SEL_DRW:
        case SwWrtShell::SEL_DRW | SwWrtShell::SEL_BEZ:
            aPosBox.InsertEntry(SW_RESSTR(STR_ABOVE));
            aPosBox.InsertEntry(SW_RESSTR(STR_CP_BELOW));
            break;
        case SwWrtShell::SEL_FRM:
        case SwWrtShell::SEL_TXT:
            aPosBox.InsertEntry(SW_RESSTR(STR_BEGINNING));
            aPosBox.InsertEntry(SW_RESSTR(STR_END     ));
            break;
    }
    aPosBox.SelectEntryPos(1);
    if (eType & (SwWrtShell::SEL_GRF|SwWrtShell::SEL_DRW))
    {
        aPosText.Enable( sal_False );
        aPosBox.Enable( sal_False );
    }

    aCategoryBox.GetModifyHdl().Call(&aCategoryBox);

    FreeResource();

    CheckButtonWidth();
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
    {
        aName.EraseLeadingChars( ' ' );
        aName.EraseTrailingChars( ' ' );
        aOpt.SetCategory( aName );
    }
    aOpt.SetNumType( (sal_uInt16)(sal_uIntPtr)aFormatBox.GetEntryData( aFormatBox.GetSelectEntryPos() ) );
    aOpt.SetSeparator( aSepEdit.IsEnabled() ? aSepEdit.GetText() : String() );
    aOpt.SetCaption( aTextEdit.GetText() );
    aOpt.SetPos( aPosBox.GetSelectEntryPos() );
    aOpt.IgnoreSeqOpts() = sal_True;
    aOpt.CopyAttributes() = bCopyAttributes;
    aOpt.SetCharacterStyle( sCharacterStyle );
    rView.InsertCaption( &aOpt );
}

IMPL_LINK_INLINE_START( SwCaptionDialog, OptionHdl, Button*, pButton )
{
    String sFldTypeName = aCategoryBox.GetText();
    if(sFldTypeName == sNone)
        sFldTypeName = aEmptyStr;
    SwSequenceOptionDialog  aDlg( pButton, rView, sFldTypeName );
    aDlg.SetApplyBorderAndShadow(bCopyAttributes);
    aDlg.SetCharacterStyle( sCharacterStyle );
    aDlg.Execute();
    bCopyAttributes = aDlg.IsApplyBorderAndShadow();
    sCharacterStyle = aDlg.GetCharacterStyle();
    DrawSample();
    return 0;
}
IMPL_LINK_INLINE_END( SwCaptionDialog, OptionHdl, Button*, EMPTYARG )

IMPL_LINK_INLINE_START( SwCaptionDialog, SelectHdl, ListBox *, pBox )
{
    DrawSample();
    return 0;
}
IMPL_LINK_INLINE_END( SwCaptionDialog, SelectHdl, ListBox *, EMPTYARG )



IMPL_LINK( SwCaptionDialog, ModifyHdl, Edit *, pEdit )
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
                            ((SwSetExpFieldType*)pType)->GetType() == GSE_SEQ)
                                && 0 != sFldTypeName.Len() );
    aOptionButton.Enable( aOKButton.IsEnabled() && !bNone );
    aFormatText.Enable( !bNone );
    aFormatBox.Enable( !bNone );
    aSepText.Enable( !bNone );
    aSepEdit.Enable( !bNone );
    DrawSample();
    return 0;
}

IMPL_LINK(SwCaptionDialog, CaptionHdl, PushButton*, EMPTYARG)
{
    SfxItemSet  aSet( rView.GetDocShell()->GetDoc()->GetAttrPool() );
    SwCaptionOptDlg aDlg( this, aSet );
    aDlg.Execute();

    return 0;
}

void SwCaptionDialog::DrawSample()
{
    String aStr;

    // Nummer
    String sFldTypeName = aCategoryBox.GetText();
    sal_Bool bNone = sFldTypeName == sNone;
    if( !bNone )
    {
        sal_uInt16 nNumFmt = (sal_uInt16)(sal_uIntPtr)aFormatBox.GetEntryData(
                                        aFormatBox.GetSelectEntryPos() );
        if( SVX_NUM_NUMBER_NONE != nNumFmt )
        {
            // Kategorie
            aStr += sFldTypeName;
            if ( aStr.Len() > 0 )
                aStr += ' ';

            SwWrtShell &rSh = rView.GetWrtShell();
            String sFldTypeName( aCategoryBox.GetText() );
            SwSetExpFieldType* pFldType = (SwSetExpFieldType*)rSh.GetFldType(
                                            RES_SETEXPFLD, sFldTypeName );
            if( pFldType && pFldType->GetOutlineLvl() < MAXLEVEL )
            {
                sal_Int8 nLvl = pFldType->GetOutlineLvl();
                SwNodeNum::tNumberVector aNumVector;
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
            //case ARABIC:
            default:                    aStr += '1'; break;
            }
        }

        aStr += aSepEdit.GetText();
    }
    aStr += aTextEdit.GetText();

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
        for ( sal_Int32 i = 0; i < sizeof( pBtns ) / sizeof( pBtns[ 0 ] ); ++i, ++pCurrent )
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
/*  */


SwSequenceOptionDialog::SwSequenceOptionDialog( Window *pParent, SwView &rV,
                                            const String& rSeqFldType )
    : SvxStandardDialog( pParent, SW_RES(DLG_SEQUENCE_OPTION) ),
    aOKButton       (this, SW_RES(BTN_OK       )),
    aCancelButton   (this, SW_RES(BTN_CANCEL   )),
    aHelpButton     (this, SW_RES(BTN_HELP     )),
    aFlHeader       (this, SW_RES(FL_HEADER    )),
    aFtLevel        (this, SW_RES(FT_LEVEL     )),
    aLbLevel        (this, SW_RES(LB_LEVEL     )),
    aFtDelim        (this, SW_RES(FT_SEPARATOR )),
    aEdDelim        (this, SW_RES(ED_SEPARATOR )),
    aFlCatAndFrame  (this, SW_RES(FL_CATANDFRAME)),
    aFtCharStyle    (this, SW_RES(FT_CHARSTYLE )),
    aLbCharStyle    (this, SW_RES(LB_CHARSTYLE )),
    aApplyBorderAndShadowCB(this, SW_RES(CB_APPLYBAS)),

    aFldTypeName( rSeqFldType ),
    rView( rV )
{
    FreeResource();
    SwWrtShell &rSh = rView.GetWrtShell();

    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
        aLbLevel.InsertEntry( String::CreateFromInt32(n+1) );

    SwSetExpFieldType* pFldType = (SwSetExpFieldType*)rSh.GetFldType(
                                        RES_SETEXPFLD, aFldTypeName );

    sal_Unicode nLvl = MAXLEVEL;
    String sDelim( String::CreateFromAscii( ": " ) );
    if( pFldType )
    {
        sDelim = pFldType->GetDelimiter();
        nLvl = pFldType->GetOutlineLvl();
    }

    aLbLevel.SelectEntryPos( nLvl < MAXLEVEL ? nLvl + 1 : 0 );
    aEdDelim.SetText( sDelim );

    ::FillCharStyleListBox( aLbCharStyle, rView.GetDocShell(), TRUE, TRUE );
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
        pFldType->SetDelimiter( cDelim );
        pFldType->SetOutlineLvl( nLvl );
    }
    else if( aFldTypeName.Len() && nLvl < MAXLEVEL )
    {
        // dann muessen wir das mal einfuegen
        SwSetExpFieldType aFldType( rSh.GetDoc(), aFldTypeName, GSE_SEQ );
        aFldType.SetDelimiter( cDelim );
        aFldType.SetOutlineLvl( nLvl );
        rSh.InsertFldType( aFldType );
    }
    else
        bUpdate = sal_False;

    if( bUpdate )
        rSh.UpdateExpFlds();
}

/*-- 24.08.2004 16:13:53---------------------------------------------------

  -----------------------------------------------------------------------*/
String  SwSequenceOptionDialog::GetCharacterStyle() const
{
    String sRet;
    if(aLbCharStyle.GetSelectEntryPos())
        sRet = aLbCharStyle.GetSelectEntry();
    return sRet;
}

/*-- 24.08.2004 16:14:00---------------------------------------------------

  -----------------------------------------------------------------------*/
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
        USHORT nTmpCode = rKeyCode.GetFullCode() & ~KEY_ALLMODTYPE;

        if(nTmpCode != KEY_BACKSPACE && nTmpCode != KEY_RETURN
                && nTmpCode != KEY_TAB && nTmpCode != KEY_ESCAPE)
        {
            String sKey( pEvent->GetCharCode() ), sName( GetText() );
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

