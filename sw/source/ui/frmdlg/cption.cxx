/*************************************************************************
 *
 *  $RCSfile: cption.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2001-03-28 14:55:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
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

using namespace ::com::sun::star;

extern String* pOldGrfCat;
extern String* pOldTabCat;
extern String* pOldFrmCat;
extern String* pOldDrwCat;

class SwSequenceOptionDialog : public SvxStandardDialog
{
    FixedText       aFtLevel;
    ListBox         aLbLevel;
    FixedText       aFtDelim;
    Edit            aEdDelim;
    OKButton        aOKButton;
    CancelButton    aCancelButton;
    HelpButton      aHelpButton;
    GroupBox        aGbHeader;
    SwView&         rView;
    String          aFldTypeName;

public:
    SwSequenceOptionDialog( Window *pParent, SwView &rV,
                            const String& rSeqFldType );
    virtual ~SwSequenceOptionDialog();
    virtual void Apply();
};


SwCaptionDialog::SwCaptionDialog( Window *pParent, SwView &rV ) :

    SvxStandardDialog( pParent, SW_RES(DLG_CAPTION) ),

    aOKButton     (this, SW_RES(BTN_OK     )),
    aCancelButton (this, SW_RES(BTN_CANCEL  )),
    aHelpButton   (this, SW_RES(BTN_HELP    )),
    aOptionButton (this, SW_RES(BTN_OPTION  )),
    aSampleText   (this, SW_RES(TXT_SAMPLE  )),
    aCategoryText (this, SW_RES(TXT_CATEGORY)),
    aCategoryBox  (this, SW_RES(BOX_CATEGORY)),
    aFormatText   (this, SW_RES(TXT_FORMAT  )),
    aFormatBox    (this, SW_RES(BOX_FORMAT  )),
    aTextText     (this, SW_RES(TXT_TEXT    )),
    aTextEdit     (this, SW_RES(EDT_TEXT    )),
    aPosText      (this, SW_RES(TXT_POS    )),
    aPosBox       (this, SW_RES(BOX_POS    )),
    aCopyAttributesCB(this, SW_RES(CB_COPY_ATTR    )),
    aObjectNameFT(this, SW_RES(FT_OBJECT_NAME )),
    aObjectNameED(this, SW_RES(ED_OBJECT_NAME )),
    aSettingsGroup(this, SW_RES(GRP_SETTINGS)),
    rView( rV ),
    pMgr( new SwFldMgr() )

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

    aLk = LINK(this, SwCaptionDialog, SelectHdl);
    aCategoryBox.SetSelectHdl( aLk );
    aFormatBox  .SetSelectHdl( aLk );
//  aPosBox     .SetSelectHdl( aLk );

    aOptionButton.SetClickHdl( LINK( this, SwCaptionDialog, OptionHdl ) );

    sal_uInt16 i, nCount = pMgr->GetFldTypeCount();
    for (i = 0; i < nCount; i++)
    {
        SwFieldType *pType = pMgr->GetFldType( USHRT_MAX, i );
        if( pType->Which() == RES_SETEXPFLD &&
            ((SwSetExpFieldType *) pType)->GetType() & GSE_SEQ )
            aCategoryBox.InsertEntry(pType->GetName(), i);
    }

    String* pString;
    sal_uInt16 nPoolId = 0;
    if (eType & SwWrtShell::SEL_GRF)
    {
        nPoolId = RES_POOLCOLL_LABEL_ABB;
        pString = pOldGrfCat;
        aCopyAttributesCB.Show();
        aCopyAttributesCB.Check(sal_True);
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
        pString = pOldTabCat;
     uno::Reference< text::XTextTablesSupplier >  xTables(xModel, uno::UNO_QUERY);
        xNameAccess = xTables->getTextTables();
        sObjectName = rSh.GetTableFmt()->GetName();
    }
    else if( eType & SwWrtShell::SEL_FRM )
    {
        nPoolId = RES_POOLCOLL_LABEL_FRAME;
        pString = pOldFrmCat;
         uno::Reference< text::XTextFramesSupplier >  xFrms(xModel, uno::UNO_QUERY);
        xNameAccess = xFrms->getTextFrames();
        sObjectName = rSh.GetFlyName();
    }
    else if( eType == SwWrtShell::SEL_TXT )
    {
        nPoolId = RES_POOLCOLL_LABEL_FRAME;
        pString = pOldFrmCat;
        aObjectNameED.Show(sal_False);
        aObjectNameFT.Show(sal_False);
    }
    else if( eType & SwWrtShell::SEL_DRW )
    {
        nPoolId = RES_POOLCOLL_LABEL_DRAWING;
        pString = pOldDrwCat;
        aObjectNameED.Show(sal_False);
        aObjectNameFT.Show(sal_False);
    }
    if( nPoolId )
    {
        if( pString && pString->Len())
            aCategoryBox.SetText( *pString );
        else
        {
            String sTmp;
            aCategoryBox.SetText( GetDocPoolNm( nPoolId, sTmp ));
        }
    }

    //
    if(aObjectNameED.IsVisible())
    {
        if(!aCopyAttributesCB.IsVisible())
        {
            long nHeightDiff = aObjectNameFT.GetPosPixel().Y() - aObjectNameED.GetPosPixel().Y();
             Point aPos(aCopyAttributesCB.GetPosPixel());
            aObjectNameFT.SetPosPixel(aPos);
            aPos.X() = aObjectNameED.GetPosPixel().X();
            aPos.Y() -= nHeightDiff;
            aObjectNameED.SetPosPixel(aPos);
        }
        if(xNameAccess.is())
        {
            uno::Any aObj = xNameAccess->getByName(sObjectName);
             uno::Reference< uno::XInterface >  xTmp = *(uno::Reference< uno::XInterface > *)aObj.getValue();
            xNamed = uno::Reference< container::XNamed >(xTmp, uno::UNO_QUERY);
        }
        aObjectNameED.SetText(sObjectName);
        aObjectNameED.SetForbiddenChars(' ');
        aObjectNameED.SetModifyHdl(LINK(this, SwCaptionDialog, ModifyHdl));
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
        aPosBox.Enable( sal_False );

    aCategoryBox.GetModifyHdl().Call(&aCategoryBox);

    FreeResource();
    aTextEdit.GrabFocus();
}



void SwCaptionDialog::Apply()
{
    String sNewName = aObjectNameED.GetText();
    if(aObjectNameED.IsVisible() && sNewName != sObjectName)
    {
        xNamed->setName(sNewName);
    }

    InsCaptionOpt aOpt;

    aOpt.UseCaption() = sal_True;
    String aName( aCategoryBox.GetText() );
    aName.EraseLeadingChars (' ');
    aName.EraseTrailingChars(' ');
    aOpt.SetCategory(aName);
    aOpt.SetNumType((sal_uInt16)(sal_uInt32)aFormatBox.GetEntryData(aFormatBox.GetSelectEntryPos()));
    aOpt.SetCaption(aTextEdit.GetText());
    aOpt.SetPos(aPosBox.GetSelectEntryPos());
    aOpt.IgnoreSeqOpts() = sal_True;
    aOpt.CopyAttributes() = aCopyAttributesCB.IsVisible() && aCopyAttributesCB.IsChecked();
    rView.InsertCaption(&aOpt);
}



IMPL_LINK_INLINE_START( SwCaptionDialog, OptionHdl, Button*, pButton )
{
    SwSequenceOptionDialog( pButton, rView, aCategoryBox.GetText() ).Execute();
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
    if(&aCategoryBox == pEdit)
    {
        String sName = pEdit->GetText();
        xub_StrLen nLen = sName.Len();
        SwCalc::IsValidVarName( sName, &sName );
        if( sName.Len() != nLen )
        {
            nLen = sName.Len();
            Selection aSel(pEdit->GetSelection());
            pEdit->SetText( sName );
            pEdit->SetSelection( aSel );   // Cursorpos restaurieren
        }
    }
    String sNewName = aObjectNameED.GetText();
    sal_Bool bCorrectName = !aObjectNameED.IsVisible() ||
        (sNewName.Len() &&
            (sNewName == sObjectName ||
                !xNameAccess.is() || !xNameAccess->hasByName(sNewName)));
    SwWrtShell &rSh = rView.GetWrtShell();
    String sFldTypeName = aCategoryBox.GetText();
    sal_Bool bCorrectFldName = sFldTypeName.Len() > 0;
    SwFieldType* pType = bCorrectFldName
                    ? rSh.GetFldType( RES_SETEXPFLD, sFldTypeName )
                    : 0;
    aOKButton.Enable( bCorrectName && bCorrectFldName &&
                        (!pType ||
                            ((SwSetExpFieldType*)pType)->GetType() == GSE_SEQ)
                                && 0 != sFldTypeName.Len() );
    aOptionButton.Enable( aOKButton.IsEnabled() );

    DrawSample();
    return 0;
}



void SwCaptionDialog::DrawSample()
{
    String aStr;

    // Nummer
    sal_uInt16 nNumFmt = (sal_uInt16)(sal_uInt32)aFormatBox.GetEntryData(
                                    aFormatBox.GetSelectEntryPos() );
    if( SVX_NUM_NUMBER_NONE != nNumFmt )
    {
        // Kategorie
        aStr += aCategoryBox.GetText();
        aStr += ' ';

        SwWrtShell &rSh = rView.GetWrtShell();
        String sFldTypeName( aCategoryBox.GetText() );
        SwSetExpFieldType* pFldType = (SwSetExpFieldType*)rSh.GetFldType(
                                        RES_SETEXPFLD, sFldTypeName );
        if( pFldType && pFldType->GetOutlineLvl() < MAXLEVEL )
        {
            sal_Int8 nLvl = pFldType->GetOutlineLvl();
            SwNodeNum aNum( nLvl );
            for( sal_Int8 i = 0; i <= nLvl; ++i )
                *(aNum.GetLevelVal() + i) = 1;

            String sNumber( rSh.GetOutlineNumRule()->MakeNumString(
                                                    aNum, sal_False ));
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

    aStr += aTextEdit.GetText();
    aSampleText.SetText(aStr);
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
    aFtLevel        (this, SW_RES(FT_LEVEL     )),
    aLbLevel        (this, SW_RES(LB_LEVEL     )),
    aFtDelim        (this, SW_RES(FT_SEPARATOR )),
    aEdDelim        (this, SW_RES(ED_SEPARATOR )),
    aGbHeader       (this, SW_RES(GB_HEADER    )),
    aFldTypeName( rSeqFldType ),
    rView( rV )
{
    FreeResource();
    SwWrtShell &rSh = rView.GetWrtShell();

    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
        aLbLevel.InsertEntry( String::CreateFromInt32(n+1) );

    SwSetExpFieldType* pFldType = (SwSetExpFieldType*)rSh.GetFldType(
                                        RES_SETEXPFLD, aFldTypeName );

    sal_Unicode cDelim = '.', nLvl = MAXLEVEL;
    if( pFldType )
    {
        cDelim = pFldType->GetDelimiter();
        nLvl = pFldType->GetOutlineLvl();
    }

    aLbLevel.SelectEntryPos( nLvl < MAXLEVEL ? nLvl + 1 : 0 );
    aEdDelim.SetText( cDelim );
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
    else if( nLvl < MAXLEVEL )
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



