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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <vcl/svapp.hxx>
#include <svl/zforlist.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/viewfrm.hxx>
#include <comphelper/processfactory.hxx>

#include "sc.hrc"
#include "scmod.hxx"
#include "attrib.hxx"
#include "zforauto.hxx"
#include "scitems.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "autoform.hxx"
//CHINA001 #include "strindlg.hxx"
#include "miscdlgs.hrc"
#include "autofmt.hxx"
#include "scresid.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"

#define FRAME_OFFSET 4


//CHINA001 //========================================================================
//CHINA001 // AutoFormat-Dialog:
//CHINA001
//CHINA001 ScAutoFormatDlg::ScAutoFormatDlg( Window*                 pParent,
//CHINA001                               ScAutoFormat*             pAutoFormat,
//CHINA001                                 const ScAutoFormatData*   pSelFormatData,
//CHINA001                                 ScDocument*               pDoc ) :
//CHINA001
//CHINA001   ModalDialog     ( pParent, ScResId( RID_SCDLG_AUTOFORMAT ) ),
//CHINA001   //
//CHINA001   aLbFormat       ( this, ScResId( LB_FORMAT ) ),
//CHINA001   aFlFormat       ( this, ScResId( FL_FORMAT ) ),
//CHINA001   pWndPreview     ( new ScAutoFmtPreview( this, ScResId( WND_PREVIEW ), pDoc ) ),
//CHINA001   aBtnNumFormat   ( this, ScResId( BTN_NUMFORMAT ) ),
//CHINA001   aBtnBorder      ( this, ScResId( BTN_BORDER ) ),
//CHINA001   aBtnFont        ( this, ScResId( BTN_FONT ) ),
//CHINA001   aBtnPattern     ( this, ScResId( BTN_PATTERN ) ),
//CHINA001   aBtnAlignment   ( this, ScResId( BTN_ALIGNMENT ) ),
//CHINA001   aBtnAdjust      ( this, ScResId( BTN_ADJUST ) ),
//CHINA001   aFlFormatting   ( this, ScResId( FL_FORMATTING ) ),
//CHINA001   aBtnOk          ( this, ScResId( BTN_OK ) ),
//CHINA001   aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
//CHINA001   aBtnHelp        ( this, ScResId( BTN_HELP ) ),
//CHINA001   aBtnAdd         ( this, ScResId( BTN_ADD ) ),
//CHINA001   aBtnRemove      ( this, ScResId( BTN_REMOVE ) ),
//CHINA001   aBtnMore        ( this, ScResId( BTN_MORE ) ),
//CHINA001   aBtnRename      ( this, ScResId( BTN_RENAME ) ),
//CHINA001   aStrTitle       ( ScResId( STR_ADD_TITLE ) ),
//CHINA001   aStrLabel       ( ScResId( STR_ADD_LABEL ) ),
//CHINA001   aStrRename      ( ScResId( STR_RENAME_TITLE ) ),
//CHINA001   aStrClose       ( ScResId( STR_BTN_CLOSE ) ),
//CHINA001   aStrDelTitle    ( ScResId( STR_DEL_TITLE ) ),
//CHINA001   aStrDelMsg      ( ScResId( STR_DEL_MSG ) ) ,
//CHINA001   //
//CHINA001   nIndex          ( 0 ),
//CHINA001   bFmtInserted    ( sal_False ),
//CHINA001   bCoreDataChanged( sal_False ),
//CHINA001   pFormat         ( pAutoFormat ),
//CHINA001   pSelFmtData     ( pSelFormatData )
//CHINA001 {
//CHINA001   Init();
//CHINA001   pWndPreview->NotifyChange( (*pFormat)[0] );
//CHINA001   FreeResource();
//CHINA001 }
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001
//CHINA001 ScAutoFormatDlg::~ScAutoFormatDlg()
//CHINA001 {
//CHINA001   delete pWndPreview;
//CHINA001 }
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001
//CHINA001 void ScAutoFormatDlg::Init()
//CHINA001 {
//CHINA001   sal_uInt16 nCount;
//CHINA001   String aEntry;
//CHINA001
//CHINA001   aLbFormat    .SetSelectHdl( LINK( this, ScAutoFormatDlg, SelFmtHdl ) );
//CHINA001   aBtnNumFormat.SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
//CHINA001   aBtnBorder   .SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
//CHINA001   aBtnFont     .SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
//CHINA001   aBtnPattern  .SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
//CHINA001   aBtnAlignment.SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
//CHINA001   aBtnAdjust   .SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
//CHINA001   aBtnAdd      .SetClickHdl ( LINK( this, ScAutoFormatDlg, AddHdl ) );
//CHINA001   aBtnRemove   .SetClickHdl ( LINK( this, ScAutoFormatDlg, RemoveHdl ) );
//CHINA001   aBtnOk       .SetClickHdl ( LINK( this, ScAutoFormatDlg, CloseHdl ) );
//CHINA001   aBtnCancel   .SetClickHdl ( LINK( this, ScAutoFormatDlg, CloseHdl ) );
//CHINA001   aBtnRename   .SetClickHdl ( LINK( this, ScAutoFormatDlg, RenameHdl ) );
//CHINA001   aLbFormat    .SetDoubleClickHdl( LINK( this, ScAutoFormatDlg, DblClkHdl ) );
//CHINA001
//CHINA001   aBtnMore.AddWindow( &aBtnRename );
//CHINA001   aBtnMore.AddWindow( &aBtnNumFormat );
//CHINA001   aBtnMore.AddWindow( &aBtnBorder );
//CHINA001   aBtnMore.AddWindow( &aBtnFont );
//CHINA001   aBtnMore.AddWindow( &aBtnPattern );
//CHINA001   aBtnMore.AddWindow( &aBtnAlignment );
//CHINA001   aBtnMore.AddWindow( &aBtnAdjust );
//CHINA001   aBtnMore.AddWindow( &aFlFormatting );
//CHINA001
//CHINA001   nCount = pFormat->GetCount();
//CHINA001
//CHINA001   for ( sal_uInt16 i = 0; i < nCount; i++ )
//CHINA001   {
//CHINA001     ((*pFormat)[i])->GetName( aEntry );
//CHINA001     aLbFormat.InsertEntry( aEntry );
//CHINA001   }
//CHINA001
//CHINA001   if ( nCount == 1 )
//CHINA001     aBtnRemove.Disable();
//CHINA001
//CHINA001   aLbFormat.SelectEntryPos( 0 );
//CHINA001   aBtnRename.Disable();
//CHINA001   aBtnRemove.Disable();
//CHINA001
//CHINA001   nIndex = 0;
//CHINA001   UpdateChecks();
//CHINA001
//CHINA001   if ( !pSelFmtData )
//CHINA001   {
//CHINA001     aBtnAdd.Disable();
//CHINA001     aBtnRemove.Disable();
//CHINA001     bFmtInserted = sal_True;
//CHINA001   }
//CHINA001 }
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001
//CHINA001 void ScAutoFormatDlg::UpdateChecks()
//CHINA001 {
//CHINA001   ScAutoFormatData* pData = (*pFormat)[nIndex];
//CHINA001
//CHINA001   aBtnNumFormat.Check( pData->GetIncludeValueFormat() );
//CHINA001   aBtnBorder   .Check( pData->GetIncludeFrame() );
//CHINA001   aBtnFont     .Check( pData->GetIncludeFont() );
//CHINA001   aBtnPattern  .Check( pData->GetIncludeBackground() );
//CHINA001   aBtnAlignment.Check( pData->GetIncludeJustify() );
//CHINA001   aBtnAdjust   .Check( pData->GetIncludeWidthHeight() );
//CHINA001 }
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001 // Handler:
//CHINA001 //---------
//CHINA001
//CHINA001 IMPL_LINK( ScAutoFormatDlg, CloseHdl, PushButton *, pBtn )
//CHINA001 {
//CHINA001   if ( pBtn == &aBtnOk || pBtn == &aBtnCancel )
//CHINA001   {
//CHINA001     if ( bCoreDataChanged )
//CHINA001         ScGlobal::GetAutoFormat()->Save();
//CHINA001
//CHINA001     EndDialog( (pBtn == &aBtnOk) ? RET_OK : RET_CANCEL );
//CHINA001   }
//CHINA001   return 0;
//CHINA001 }
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001
//CHINA001 IMPL_LINK_INLINE_START( ScAutoFormatDlg, DblClkHdl, void *, EMPTYARG )
//CHINA001 {
//CHINA001   if ( bCoreDataChanged )
//CHINA001     ScGlobal::GetAutoFormat()->Save();
//CHINA001
//CHINA001   EndDialog( RET_OK );
//CHINA001   return 0;
//CHINA001 }
//CHINA001 IMPL_LINK_INLINE_END( ScAutoFormatDlg, DblClkHdl, void *, EMPTYARG )
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001
//CHINA001 IMPL_LINK( ScAutoFormatDlg, CheckHdl, Button *, pBtn )
//CHINA001 {
//CHINA001   ScAutoFormatData* pData  = (*pFormat)[nIndex];
//CHINA001   sal_Bool              bCheck = ((CheckBox*)pBtn)->IsChecked();
//CHINA001
//CHINA001   if ( pBtn == &aBtnNumFormat )
//CHINA001     pData->SetIncludeValueFormat( bCheck );
//CHINA001   else if ( pBtn == &aBtnBorder )
//CHINA001     pData->SetIncludeFrame( bCheck );
//CHINA001   else if ( pBtn == &aBtnFont )
//CHINA001     pData->SetIncludeFont( bCheck );
//CHINA001   else if ( pBtn == &aBtnPattern )
//CHINA001     pData->SetIncludeBackground( bCheck );
//CHINA001   else if ( pBtn == &aBtnAlignment )
//CHINA001     pData->SetIncludeJustify( bCheck );
//CHINA001   else if ( pBtn == &aBtnAdjust )
//CHINA001     pData->SetIncludeWidthHeight( bCheck );
//CHINA001
//CHINA001   if ( !bCoreDataChanged )
//CHINA001   {
//CHINA001     aBtnCancel.SetText( aStrClose );
//CHINA001     bCoreDataChanged = sal_True;
//CHINA001   }
//CHINA001
//CHINA001   pWndPreview->NotifyChange( pData );
//CHINA001
//CHINA001   return 0;
//CHINA001 }
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001
//CHINA001 IMPL_LINK( ScAutoFormatDlg, AddHdl, void *, EMPTYARG )
//CHINA001 {
//CHINA001   if ( !bFmtInserted && pSelFmtData )
//CHINA001   {
//CHINA001     String              aStrStandard( ScResId(STR_STANDARD) );
//CHINA001     String              aFormatName;
//CHINA001     ScStringInputDlg*   pDlg;
//CHINA001     sal_Bool                bOk = sal_False;
//CHINA001
//CHINA001     while ( !bOk )
//CHINA001     {
//CHINA001         pDlg = new ScStringInputDlg( this,
//CHINA001                                      aStrTitle,
//CHINA001                                      aStrLabel,
//CHINA001                                      aFormatName,
//CHINA001                                      HID_SC_ADD_AUTOFMT );
//CHINA001
//CHINA001         if ( pDlg->Execute() == RET_OK )
//CHINA001         {
//CHINA001             pDlg->GetInputString( aFormatName );
//CHINA001
//CHINA001             if ( (aFormatName.Len() > 0) && (aFormatName != aStrStandard) )
//CHINA001             {
//CHINA001                 ScAutoFormatData* pNewData
//CHINA001                     = new ScAutoFormatData( *pSelFmtData );
//CHINA001
//CHINA001                 pNewData->SetName( aFormatName );
//CHINA001                 bFmtInserted = pFormat->Insert( pNewData );
//CHINA001
//CHINA001                 if ( bFmtInserted )
//CHINA001                 {
//CHINA001                     sal_uInt16 nAt = pFormat->IndexOf( pNewData );
//CHINA001
//CHINA001                     aLbFormat.InsertEntry( aFormatName, nAt );
//CHINA001                     aLbFormat.SelectEntry( aFormatName );
//CHINA001                     aBtnAdd.Disable();
//CHINA001
//CHINA001                     if ( !bCoreDataChanged )
//CHINA001                     {
//CHINA001                         aBtnCancel.SetText( aStrClose );
//CHINA001                         bCoreDataChanged = sal_True;
//CHINA001                     }
//CHINA001
//CHINA001                     SelFmtHdl( 0 );
//CHINA001                     bOk = sal_True;
//CHINA001                 }
//CHINA001                 else
//CHINA001                     delete pNewData;
//CHINA001
//CHINA001             }
//CHINA001
//CHINA001             if ( !bFmtInserted )
//CHINA001             {
//CHINA001                 sal_uInt16 nRet = ErrorBox( this,
//CHINA001                                         WinBits( WB_OK_CANCEL | WB_DEF_OK),
//CHINA001                                         ScGlobal::GetRscString(STR_INVALID_AFNAME)
//CHINA001                                       ).Execute();
//CHINA001
//CHINA001                 bOk = ( nRet == RET_CANCEL );
//CHINA001             }
//CHINA001         }
//CHINA001         else
//CHINA001             bOk = sal_True;
//CHINA001
//CHINA001         delete pDlg;
//CHINA001     }
//CHINA001   }
//CHINA001
//CHINA001   return 0;
//CHINA001 }
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001
//CHINA001 IMPL_LINK( ScAutoFormatDlg, RemoveHdl, void *, EMPTYARG )
//CHINA001 {
//CHINA001   if ( (nIndex > 0) && (aLbFormat.GetEntryCount() > 0) )
//CHINA001   {
//CHINA001     String aMsg( aStrDelMsg.GetToken( 0, '#' ) );
//CHINA001
//CHINA001     aMsg += aLbFormat.GetSelectEntry();
//CHINA001     aMsg += aStrDelMsg.GetToken( 1, '#' );
//CHINA001
//CHINA001     if ( RET_YES ==
//CHINA001          QueryBox( this, WinBits( WB_YES_NO | WB_DEF_YES ), aMsg ).Execute() )
//CHINA001     {
//CHINA001         aLbFormat.RemoveEntry( nIndex );
//CHINA001         aLbFormat.SelectEntryPos( nIndex-1 );
//CHINA001
//CHINA001         if ( nIndex-1 == 0 )
//CHINA001             aBtnRemove.Disable();
//CHINA001
//CHINA001         if ( !bCoreDataChanged )
//CHINA001         {
//CHINA001             aBtnCancel.SetText( aStrClose );
//CHINA001             bCoreDataChanged = sal_True;
//CHINA001         }
//CHINA001
//CHINA001         pFormat->AtFree( nIndex ); // in der Core loeschen
//CHINA001         nIndex--;
//CHINA001
//CHINA001         SelFmtHdl( 0 );
//CHINA001     }
//CHINA001   }
//CHINA001
//CHINA001   SelFmtHdl( 0 );
//CHINA001
//CHINA001   return 0;
//CHINA001 }
//CHINA001
//CHINA001 IMPL_LINK( ScAutoFormatDlg, RenameHdl, void *, pBtn)
//CHINA001 {
//CHINA001   sal_Bool bOk = sal_False;
//CHINA001   while( !bOk )
//CHINA001   {
//CHINA001
//CHINA001     String aFormatName=aLbFormat.GetSelectEntry();
//CHINA001     String aEntry;
//CHINA001
//CHINA001     ScStringInputDlg* pDlg = new ScStringInputDlg( this,
//CHINA001                                      aStrRename,
//CHINA001                                      aStrLabel,
//CHINA001                                      aFormatName,
//CHINA001                                      HID_SC_RENAME_AUTOFMT );
//CHINA001     if( pDlg->Execute() == RET_OK )
//CHINA001     {
//CHINA001         sal_Bool bFmtRenamed = sal_False;
//CHINA001         pDlg->GetInputString( aFormatName );
//CHINA001         sal_uInt16 n;
//CHINA001
//CHINA001         if ( aFormatName.Len() > 0 )
//CHINA001         {
//CHINA001             for( n = 0; n < pFormat->GetCount(); ++n )
//CHINA001             {
//CHINA001                 (*pFormat)[n]->GetName(aEntry);
//CHINA001                 if ( aEntry== aFormatName)
//CHINA001                     break;
//CHINA001             }
//CHINA001             if( n >= pFormat->GetCount() )
//CHINA001             {
//CHINA001                 // Format mit dem Namen noch nicht vorhanden, also
//CHINA001                 // umbenennen
//CHINA001
//CHINA001                 aLbFormat.RemoveEntry(nIndex );
//CHINA001                 ScAutoFormatData* p=(*pFormat)[ nIndex ];
//CHINA001                 ScAutoFormatData* pNewData
//CHINA001                     = new ScAutoFormatData(*p);
//CHINA001
//CHINA001                 pFormat->AtFree( nIndex );
//CHINA001
//CHINA001                 pNewData->SetName( aFormatName );
//CHINA001
//CHINA001                 pFormat->Insert( pNewData);
//CHINA001
//CHINA001                 sal_uInt16 nCount = pFormat->GetCount();
//CHINA001
//CHINA001                 aLbFormat.SetUpdateMode(sal_False);
//CHINA001                 aLbFormat.Clear();
//CHINA001                 for ( sal_uInt16 i = 0; i < nCount; i++ )
//CHINA001                 {
//CHINA001                     ((*pFormat)[i])->GetName( aEntry );
//CHINA001                     aLbFormat.InsertEntry( aEntry );
//CHINA001                 }
//CHINA001
//CHINA001                 aLbFormat.SetUpdateMode( sal_True);
//CHINA001                 aLbFormat.SelectEntry( aFormatName);
//CHINA001
//CHINA001                 if ( !bCoreDataChanged )
//CHINA001                 {
//CHINA001                     aBtnCancel.SetText( aStrClose );
//CHINA001                     bCoreDataChanged = sal_True;
//CHINA001                 }
//CHINA001
//CHINA001
//CHINA001                 SelFmtHdl( 0 );
//CHINA001                 bOk = sal_True;
//CHINA001                 bFmtRenamed = sal_True;
//CHINA001             }
//CHINA001         }
//CHINA001         if( !bFmtRenamed )
//CHINA001         {
//CHINA001             bOk = RET_CANCEL == ErrorBox( this,
//CHINA001                                 WinBits( WB_OK_CANCEL | WB_DEF_OK),
//CHINA001                                 ScGlobal::GetRscString(STR_INVALID_AFNAME)
//CHINA001                                 ).Execute();
//CHINA001         }
//CHINA001     }
//CHINA001     else
//CHINA001         bOk = sal_True;
//CHINA001     delete pDlg;
//CHINA001   }
//CHINA001
//CHINA001   return 0;
//CHINA001 }
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001
//CHINA001 IMPL_LINK( ScAutoFormatDlg, SelFmtHdl, void *, EMPTYARG )
//CHINA001 {
//CHINA001  nIndex = aLbFormat.GetSelectEntryPos();
//CHINA001  UpdateChecks();
//CHINA001
//CHINA001  if ( nIndex == 0 )
//CHINA001  {
//CHINA001      aBtnRename.Disable();
//CHINA001      aBtnRemove.Disable();
//CHINA001  }
//CHINA001  else
//CHINA001  {
//CHINA001      aBtnRename.Enable();
//CHINA001      aBtnRemove.Enable();
//CHINA001  }
//CHINA001
//CHINA001  pWndPreview->NotifyChange( (*pFormat)[nIndex] );
//CHINA001
//CHINA001  return 0;
//CHINA001 }
//CHINA001
//CHINA001 //------------------------------------------------------------------------
//CHINA001
//CHINA001 String ScAutoFormatDlg::GetCurrFormatName()
//CHINA001 {
//CHINA001  String  aResult;
//CHINA001
//CHINA001  ((*pFormat)[nIndex])->GetName( aResult );
//CHINA001
//CHINA001  return aResult;
//CHINA001 }
//CHINA001
//========================================================================
// ScAutoFmtPreview

ScAutoFmtPreview::ScAutoFmtPreview( Window* pParent, const ResId& rRes, ScDocument* pDoc ) :
        Window          ( pParent, rRes ),
        pCurData        ( NULL ),
        aVD             ( *this ),
        aScriptedText   ( aVD ),
        xBreakIter      ( pDoc->GetBreakIterator() ),
        bFitWidth       ( sal_False ),
        mbRTL           ( false ),
        aPrvSize        ( GetSizePixel().Width() - 6, GetSizePixel().Height() - 30 ),
        mnLabelColWidth ( (aPrvSize.Width() - 4) / 4 - 12 ),
        mnDataColWidth1 ( (aPrvSize.Width() - 4 - 2 * mnLabelColWidth) / 3 ),
        mnDataColWidth2 ( (aPrvSize.Width() - 4 - 2 * mnLabelColWidth) / 4 ),
        mnRowHeight     ( (aPrvSize.Height() - 4) / 5 ),
        aStrJan         ( ScResId( STR_JAN ) ),
        aStrFeb         ( ScResId( STR_FEB ) ),
        aStrMar         ( ScResId( STR_MAR ) ),
        aStrNorth       ( ScResId( STR_NORTH ) ),
        aStrMid         ( ScResId( STR_MID ) ),
        aStrSouth       ( ScResId( STR_SOUTH ) ),
        aStrSum         ( ScResId( STR_SUM ) ),
        pNumFmt         ( new SvNumberFormatter( ::comphelper::getProcessServiceFactory(), ScGlobal::eLnge ) )
{
    Init();
}

//------------------------------------------------------------------------

ScAutoFmtPreview::~ScAutoFmtPreview()
{
    delete pNumFmt;
}

//------------------------------------------------------------------------

void lcl_SetFontProperties(
        Font& rFont,
        const SvxFontItem& rFontItem,
        const SvxWeightItem& rWeightItem,
        const SvxPostureItem& rPostureItem )
{
    rFont.SetFamily     ( rFontItem.GetFamily() );
    rFont.SetName       ( rFontItem.GetFamilyName() );
    rFont.SetStyleName  ( rFontItem.GetStyleName() );
    rFont.SetCharSet    ( rFontItem.GetCharSet() );
    rFont.SetPitch      ( rFontItem.GetPitch() );
    rFont.SetWeight     ( (FontWeight)rWeightItem.GetValue() );
    rFont.SetItalic     ( (FontItalic)rPostureItem.GetValue() );
}

void ScAutoFmtPreview::MakeFonts( sal_uInt16 nIndex, Font& rFont, Font& rCJKFont, Font& rCTLFont )
{
    if ( pCurData )
    {
        rFont = rCJKFont = rCTLFont = GetFont();
        Size aFontSize( rFont.GetSize().Width(), 10 );

        const SvxFontItem*        pFontItem       = (const SvxFontItem*)      pCurData->GetItem( nIndex, ATTR_FONT );
        const SvxWeightItem*      pWeightItem     = (const SvxWeightItem*)    pCurData->GetItem( nIndex, ATTR_FONT_WEIGHT );
        const SvxPostureItem*     pPostureItem    = (const SvxPostureItem*)   pCurData->GetItem( nIndex, ATTR_FONT_POSTURE );
        const SvxFontItem*        pCJKFontItem    = (const SvxFontItem*)      pCurData->GetItem( nIndex, ATTR_CJK_FONT );
        const SvxWeightItem*      pCJKWeightItem  = (const SvxWeightItem*)    pCurData->GetItem( nIndex, ATTR_CJK_FONT_WEIGHT );
        const SvxPostureItem*     pCJKPostureItem = (const SvxPostureItem*)   pCurData->GetItem( nIndex, ATTR_CJK_FONT_POSTURE );
        const SvxFontItem*        pCTLFontItem    = (const SvxFontItem*)      pCurData->GetItem( nIndex, ATTR_CTL_FONT );
        const SvxWeightItem*      pCTLWeightItem  = (const SvxWeightItem*)    pCurData->GetItem( nIndex, ATTR_CTL_FONT_WEIGHT );
        const SvxPostureItem*     pCTLPostureItem = (const SvxPostureItem*)   pCurData->GetItem( nIndex, ATTR_CTL_FONT_POSTURE );
        const SvxUnderlineItem*   pUnderlineItem  = (const SvxUnderlineItem*) pCurData->GetItem( nIndex, ATTR_FONT_UNDERLINE );
        const SvxOverlineItem*    pOverlineItem   = (const SvxOverlineItem*)  pCurData->GetItem( nIndex, ATTR_FONT_OVERLINE );
        const SvxCrossedOutItem*  pCrossedOutItem = (const SvxCrossedOutItem*)pCurData->GetItem( nIndex, ATTR_FONT_CROSSEDOUT );
        const SvxContourItem*     pContourItem    = (const SvxContourItem*)   pCurData->GetItem( nIndex, ATTR_FONT_CONTOUR );
        const SvxShadowedItem*    pShadowedItem   = (const SvxShadowedItem*)  pCurData->GetItem( nIndex, ATTR_FONT_SHADOWED );
        const SvxColorItem*       pColorItem      = (const SvxColorItem*)     pCurData->GetItem( nIndex, ATTR_FONT_COLOR );

        lcl_SetFontProperties( rFont, *pFontItem, *pWeightItem, *pPostureItem );
        lcl_SetFontProperties( rCJKFont, *pCJKFontItem, *pCJKWeightItem, *pCJKPostureItem );
        lcl_SetFontProperties( rCTLFont, *pCTLFontItem, *pCTLWeightItem, *pCTLPostureItem );

        Color aColor( pColorItem->GetValue() );
        if( aColor.GetColor() == COL_TRANSPARENT )
            aColor = GetSettings().GetStyleSettings().GetWindowTextColor();

#define SETONALLFONTS( MethodName, Value ) \
rFont.MethodName( Value ); rCJKFont.MethodName( Value ); rCTLFont.MethodName( Value );

        SETONALLFONTS( SetUnderline,    (FontUnderline)pUnderlineItem->GetValue() )
        SETONALLFONTS( SetOverline,     (FontUnderline)pOverlineItem->GetValue() )
        SETONALLFONTS( SetStrikeout,    (FontStrikeout)pCrossedOutItem->GetValue() )
        SETONALLFONTS( SetOutline,      pContourItem->GetValue() )
        SETONALLFONTS( SetShadow,       pShadowedItem->GetValue() )
        SETONALLFONTS( SetColor,        aColor )
        SETONALLFONTS( SetSize,         aFontSize )
        SETONALLFONTS( SetTransparent,  sal_True )

#undef SETONALLFONTS
    }
}

//------------------------------------------------------------------------

sal_uInt16 ScAutoFmtPreview::GetFormatIndex( size_t nCol, size_t nRow ) const
{
    static const sal_uInt16 pnFmtMap[] =
    {
        0,  1,  2,  1,  3,
        4,  5,  6,  5,  7,
        8,  9,  10, 9,  11,
        4,  5,  6,  5,  7,
        12, 13, 14, 13, 15
    };
    return pnFmtMap[ maArray.GetCellIndex( nCol, nRow, mbRTL ) ];
}

const SvxBoxItem& ScAutoFmtPreview::GetBoxItem( size_t nCol, size_t nRow ) const
{
    DBG_ASSERT( pCurData, "ScAutoFmtPreview::GetBoxItem - no format data found" );
    return *static_cast< const SvxBoxItem* >( pCurData->GetItem( GetFormatIndex( nCol, nRow ), ATTR_BORDER ) );
}

const SvxLineItem& ScAutoFmtPreview::GetDiagItem( size_t nCol, size_t nRow, bool bTLBR ) const
{
    DBG_ASSERT( pCurData, "ScAutoFmtPreview::GetDiagItem - no format data found" );
    return *static_cast< const SvxLineItem* >( pCurData->GetItem( GetFormatIndex( nCol, nRow ), bTLBR ? ATTR_BORDER_TLBR : ATTR_BORDER_BLTR ) );
}

//------------------------------------------------------------------------

void ScAutoFmtPreview::DrawString( size_t nCol, size_t nRow )
{
    if ( pCurData )
    {
        //------------------------
        // Ausgabe des Zelltextes:
        //------------------------

        String  cellString;
        sal_Bool    bNumFormat  = pCurData->GetIncludeValueFormat();
        sal_uLong   nNum;
        double  nVal;
        Color*  pDummy = NULL;
        sal_uInt16  nIndex = static_cast< sal_uInt16 >( maArray.GetCellIndex( nCol, nRow, mbRTL ) );

        switch( nIndex )
        {
            case  1: cellString = aStrJan;          break;
            case  2: cellString = aStrFeb;          break;
            case  3: cellString = aStrMar;          break;
            case  5: cellString = aStrNorth;        break;
            case 10: cellString = aStrMid;          break;
            case 15: cellString = aStrSouth;        break;
            case  4:
            case 20: cellString = aStrSum;          break;

            case  6:
            case  8:
            case 16:
            case 18: nVal = nIndex;
                     nNum = 5;
                     goto mknum;
            case 17:
            case  7: nVal = nIndex;
                     nNum = 6;
                     goto mknum;
            case 11:
            case 12:
            case 13: nVal = nIndex;
                     nNum = 12 == nIndex ? 10 : 9;
                     goto mknum;

            case  9: nVal = 21; nNum = 7; goto mknum;
            case 14: nVal = 36; nNum = 11; goto mknum;
            case 19: nVal = 51; nNum = 7; goto mknum;
            case 21: nVal = 33; nNum = 13; goto mknum;
            case 22: nVal = 36; nNum = 14; goto mknum;
            case 23: nVal = 39; nNum = 13; goto mknum;
            case 24: nVal = 108; nNum = 15;
            mknum:
                if( bNumFormat )
                {
                    ScNumFormatAbbrev& rNumFormat = (ScNumFormatAbbrev&)pCurData->GetNumFormat( (sal_uInt16) nNum );
                    nNum = rNumFormat.GetFormatIndex( *pNumFmt );
                }
                else
                    nNum = 0;
                pNumFmt->GetOutputString( nVal, nNum, cellString, &pDummy );
                break;
        }

        if ( cellString.Len() > 0 )
        {
            Size                aStrSize;
            sal_uInt16              nFmtIndex       = GetFormatIndex( nCol, nRow );
            Rectangle           cellRect        = maArray.GetCellRect( nCol, nRow );
            Point               aPos            = cellRect.TopLeft();
            sal_uInt16              nRightX         = 0;
            sal_Bool                bJustify        = pCurData->GetIncludeJustify();
            SvxHorJustifyItem    aHorJustifyItem( SVX_HOR_JUSTIFY_STANDARD, ATTR_HOR_JUSTIFY );
            SvxCellHorJustify      eJustification;

            //-------------
            // Ausrichtung:
            //-------------
            eJustification  = mbRTL ? SVX_HOR_JUSTIFY_RIGHT : bJustify ?
                (SvxCellHorJustify)(((const SvxHorJustifyItem*)pCurData->GetItem( nFmtIndex, ATTR_HOR_JUSTIFY ))->GetValue()) :
                SVX_HOR_JUSTIFY_STANDARD;

            if ( pCurData->GetIncludeFont() )
            {
                Font aFont, aCJKFont, aCTLFont;
                Size theMaxStrSize;

                MakeFonts( nFmtIndex, aFont, aCJKFont, aCTLFont );

                theMaxStrSize           = cellRect.GetSize();
                theMaxStrSize.Width()  -= FRAME_OFFSET;
                theMaxStrSize.Height() -= FRAME_OFFSET;

                aScriptedText.SetFonts( &aFont, &aCJKFont, &aCTLFont );
                aScriptedText.SetText( cellString, xBreakIter );
                aStrSize = aScriptedText.GetTextSize();

                if ( theMaxStrSize.Height() < aStrSize.Height() )
                {
                    // wenn der String in diesem Font nicht
                    // in die Zelle passt, wird wieder der
                    // Standard-Font genommen:
                    aScriptedText.SetDefaultFont();
                    aStrSize = aScriptedText.GetTextSize();
                }
                while ( ( theMaxStrSize.Width() <= aStrSize.Width() )
                    && ( cellString.Len() > 1 ) )
                {
                    if( eJustification == SVX_HOR_JUSTIFY_RIGHT )
                        cellString.Erase( 0, 1 );
                    else
                        cellString.Erase( cellString.Len() - 1 );

                    aScriptedText.SetText( cellString, xBreakIter );
                    aStrSize = aScriptedText.GetTextSize();
                }
            }
            else
            {
                aScriptedText.SetDefaultFont();
                aScriptedText.SetText( cellString, xBreakIter );
                aStrSize = aScriptedText.GetTextSize();
            }

            nRightX  = (sal_uInt16)(  cellRect.GetWidth()
                                  - aStrSize.Width()
                                  - FRAME_OFFSET );

            //-----------------------------
            // vertikal (immer zentrieren):
            //-----------------------------
            aPos.Y() += (mnRowHeight - (sal_uInt16)aStrSize.Height()) / 2;

            //-----------
            // horizontal
            //-----------
            if ( eJustification != SVX_HOR_JUSTIFY_STANDARD )
            {
                sal_uInt16 nHorPos = (sal_uInt16)
                                 ((cellRect.GetWidth()-aStrSize.Width())/2);

                switch ( eJustification )
                {
                    case SVX_HOR_JUSTIFY_LEFT:
                        aPos.X() += FRAME_OFFSET;
                        break;
                    case SVX_HOR_JUSTIFY_RIGHT:
                        aPos.X() += nRightX;
                        break;
                    case SVX_HOR_JUSTIFY_BLOCK:
                    case SVX_HOR_JUSTIFY_REPEAT:
                    case SVX_HOR_JUSTIFY_CENTER:
                        aPos.X() += nHorPos;
                        break;
                    case SVX_HOR_JUSTIFY_STANDARD:
                    default:
                        // Standard wird hier nicht behandelt
                        break;
                }
            }
            else
            {
                //---------------------
                // Standardausrichtung:
                //---------------------
                if ( (nCol == 0) || (nRow == 0) )
                {
                    // Text-Label links oder Summe linksbuendig
                    aPos.X() += FRAME_OFFSET;
                }
                else
                {
                     // Zahlen/Datum rechtsbuendig
                    aPos.X() += nRightX;
                }
            }

            //-------------------------------
            aScriptedText.DrawText( aPos );
            //-------------------------------
        }
    }
}

#undef FRAME_OFFSET

//------------------------------------------------------------------------

void ScAutoFmtPreview::DrawStrings()
{
    for( size_t nRow = 0; nRow < 5; ++nRow )
        for( size_t nCol = 0; nCol < 5; ++nCol )
            DrawString( nCol, nRow );
}

//------------------------------------------------------------------------

void ScAutoFmtPreview::DrawBackground()
{
    if( pCurData )
    {
        for( size_t nRow = 0; nRow < 5; ++nRow )
        {
            for( size_t nCol = 0; nCol < 5; ++nCol )
            {
                const SvxBrushItem* pItem = static_cast< const SvxBrushItem* >(
                    pCurData->GetItem( GetFormatIndex( nCol, nRow ), ATTR_BACKGROUND ) );

                aVD.Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
                aVD.SetLineColor();
                aVD.SetFillColor( pItem->GetColor() );
                aVD.DrawRect( maArray.GetCellRect( nCol, nRow ) );
                aVD.Pop();
            }
        }
    }
}

//------------------------------------------------------------------------

void ScAutoFmtPreview::PaintCells()
{
    if ( pCurData )
    {
        // 1) background
        if ( pCurData->GetIncludeBackground() )
            DrawBackground();

        // 2) values
        DrawStrings();

        // 3) border
        if ( pCurData->GetIncludeFrame() )
            maArray.DrawArray( aVD );
    }
}

//------------------------------------------------------------------------

void ScAutoFmtPreview::Init()
{
    SetBorderStyle( WINDOW_BORDER_MONO );
    maArray.Initialize( 5, 5 );
    maArray.SetUseDiagDoubleClipping( false );
    CalcCellArray( sal_False );
    CalcLineMap();

    TypeId aType(TYPE(ScDocShell));
    ScDocShell* pDocShell = (ScDocShell*)SfxObjectShell::GetFirst(&aType);
    SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );
    SfxViewShell* p = pFrame->GetViewShell();
    ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell* >( p );
    ScViewData* pViewData = pViewSh->GetViewData();
    SCTAB nCurrentTab = pViewData->GetTabNo();
    ScDocument* pDoc = pViewData->GetDocument();
    mbRTL = pDoc->IsLayoutRTL( nCurrentTab );
}

//------------------------------------------------------------------------

void ScAutoFmtPreview::CalcCellArray( sal_Bool bFitWidthP )
{
    maArray.SetXOffset( 2 );
    maArray.SetAllColWidths( bFitWidthP ? mnDataColWidth2 : mnDataColWidth1 );
    maArray.SetColWidth( 0, mnLabelColWidth );
    maArray.SetColWidth( 4, mnLabelColWidth );

    maArray.SetYOffset( 2 );
    maArray.SetAllRowHeights( mnRowHeight );

    aPrvSize.Width() = maArray.GetWidth() + 4;
    aPrvSize.Height() = maArray.GetHeight() + 4;
}

//------------------------------------------------------------------------

inline void lclSetStyleFromBorder( svx::frame::Style& rStyle, const SvxBorderLine* pBorder )
{
    rStyle.Set( pBorder, 1.0 / TWIPS_PER_POINT, 5 );
}

void ScAutoFmtPreview::CalcLineMap()
{
    if ( pCurData )
    {
        for( size_t nRow = 0; nRow < 5; ++nRow )
        {
            for( size_t nCol = 0; nCol < 5; ++nCol )
            {
                svx::frame::Style aStyle;

                const SvxBoxItem& rItem = GetBoxItem( nCol, nRow );
                lclSetStyleFromBorder( aStyle, rItem.GetLeft() );
                maArray.SetCellStyleLeft( nCol, nRow, aStyle );
                lclSetStyleFromBorder( aStyle, rItem.GetRight() );
                maArray.SetCellStyleRight( nCol, nRow, aStyle );
                lclSetStyleFromBorder( aStyle, rItem.GetTop() );
                maArray.SetCellStyleTop( nCol, nRow, aStyle );
                lclSetStyleFromBorder( aStyle, rItem.GetBottom() );
                maArray.SetCellStyleBottom( nCol, nRow, aStyle );

                lclSetStyleFromBorder( aStyle, GetDiagItem( nCol, nRow, true ).GetLine() );
                maArray.SetCellStyleTLBR( nCol, nRow, aStyle );
                lclSetStyleFromBorder( aStyle, GetDiagItem( nCol, nRow, false ).GetLine() );
                maArray.SetCellStyleBLTR( nCol, nRow, aStyle );
            }
        }
    }
}

//------------------------------------------------------------------------

void ScAutoFmtPreview::NotifyChange( ScAutoFormatData* pNewData )
{
    if ( pNewData != pCurData )
    {
        pCurData  = pNewData;
        bFitWidth = pNewData->GetIncludeWidthHeight();
        CalcCellArray( bFitWidth );
        CalcLineMap();
    }
    else if ( bFitWidth != pNewData->GetIncludeWidthHeight() )
    {
        bFitWidth = !bFitWidth;
        CalcCellArray( bFitWidth );
    }

    DoPaint( Rectangle( Point(0,0), GetSizePixel() ) );
}

//------------------------------------------------------------------------

void ScAutoFmtPreview::DoPaint( const Rectangle& /* rRect */ )
{
    sal_uInt32 nOldDrawMode = aVD.GetDrawMode();
    //  #105733# SvtAccessibilityOptions::GetIsForBorders is no longer used (always assumed sal_True)
    if( GetSettings().GetStyleSettings().GetHighContrastMode() )
        aVD.SetDrawMode( DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );

    Size aWndSize( GetSizePixel() );
    Font aFont( aVD.GetFont() );
    Color aBackCol( GetSettings().GetStyleSettings().GetWindowColor() );
    Point aTmpPoint;
    Rectangle aRect( aTmpPoint, aWndSize );

    aFont.SetTransparent( sal_True );
    aVD.SetFont( aFont );
    aVD.SetLineColor();
    aVD.SetFillColor( aBackCol );
    aVD.SetOutputSize( aWndSize );
    aVD.DrawRect( aRect );

    PaintCells();
    SetLineColor();
    SetFillColor( aBackCol );
    DrawRect( aRect );

    Point aPos( (aWndSize.Width() - aPrvSize.Width()) / 2, (aWndSize.Height() - aPrvSize.Height()) / 2 );
    if (Application::GetSettings().GetLayoutRTL())
       aPos.X() = -aPos.X();
    DrawOutDev( aPos, aWndSize, Point(), aWndSize, aVD );

    aVD.SetDrawMode( nOldDrawMode );
}

//------------------------------------------------------------------------

void ScAutoFmtPreview::Paint( const Rectangle& rRect )
{
    DoPaint( rRect );
}

