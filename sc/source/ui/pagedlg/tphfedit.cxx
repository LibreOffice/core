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

#define _TPHFEDIT_CXX
#include "scitems.hxx"
#include <editeng/eeitem.hxx>


//CHINA001 #include <svx/chardlg.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/editview.hxx>
#include <editeng/flditem.hxx>
#include <editeng/adjitem.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include "tphfedit.hxx"
#include "editutil.hxx"
#include "global.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "tabvwsh.hxx"
#include "prevwsh.hxx"
#include "hfedtdlg.hrc"
//CHINA001 #include "textdlgs.hxx"
#include "AccessibleEditObject.hxx"

#include "scabstdlg.hxx" //CHINA001


// STATIC DATA -----------------------------------------------------------
static ScEditWindow* pActiveEdWnd = NULL;

ScEditWindow* GetScEditWindow ()
{
    return pActiveEdWnd;
}

//CHINA001
//CHINA001 static sal_uInt16 pPageRightHeaderRanges[] = { SID_SCATTR_PAGE_HEADERRIGHT,
//CHINA001 SID_SCATTR_PAGE_HEADERRIGHT,
//CHINA001 0 };
//CHINA001
//CHINA001 static sal_uInt16 pPageRightFooterRanges[] = { SID_SCATTR_PAGE_FOOTERRIGHT,
//CHINA001 SID_SCATTR_PAGE_FOOTERRIGHT,
//CHINA001 0 };
//CHINA001
//CHINA001 static sal_uInt16 pPageLeftHeaderRanges[] =  { SID_SCATTR_PAGE_HEADERLEFT,
//CHINA001 SID_SCATTR_PAGE_HEADERLEFT,
//CHINA001 0 };
//CHINA001
//CHINA001 static sal_uInt16 pPageLeftFooterRanges[] =  { SID_SCATTR_PAGE_FOOTERLEFT,
//CHINA001 SID_SCATTR_PAGE_FOOTERLEFT,
//CHINA001 0 };
//CHINA001
//CHINA001 static ScEditWindow* pActiveEdWnd = NULL;
//CHINA001
//CHINA001
//CHINA001
//CHINA001 //========================================================================
//CHINA001 // class ScHFEditPage
//CHINA001 //
//CHINA001
//CHINA001 ScHFEditPage::ScHFEditPage( Window*          pParent,
//CHINA001 sal_uInt16               nResId,
//CHINA001 const SfxItemSet&    rCoreAttrs,
//CHINA001 sal_uInt16               nWhichId )
//CHINA001
//CHINA001 :    SfxTabPage      ( pParent, ScResId( nResId ), rCoreAttrs ),
//CHINA001
//CHINA001 aWndLeft     ( this, ScResId( WND_LEFT ), Left ),
//CHINA001 aWndCenter       ( this, ScResId( WND_CENTER ), Center ),
//CHINA001 aWndRight        ( this, ScResId( WND_RIGHT ), Right ),
//CHINA001 aFtLeft      ( this, ScResId( FT_LEFT ) ),
//CHINA001 aFtCenter        ( this, ScResId( FT_CENTER ) ),
//CHINA001 aFtRight     ( this, ScResId( FT_RIGHT ) ),
//CHINA001 aFlInfo      ( this, ScResId( FL_INFO ) ),
//CHINA001 aFtInfo      ( this, ScResId( FT_INFO ) ),
//CHINA001 aBtnText     ( this, ScResId( BTN_TEXT ) ),
//CHINA001 aBtnPage     ( this, ScResId( BTN_PAGE ) ),
//CHINA001 aBtnLastPage ( this, ScResId( BTN_PAGES ) ),
//CHINA001 aBtnDate     ( this, ScResId( BTN_DATE ) ),
//CHINA001 aBtnTime     ( this, ScResId( BTN_TIME ) ),
//CHINA001 aBtnFile     ( this, ScResId( BTN_FILE ) ),
//CHINA001 aBtnTable        ( this, ScResId( BTN_TABLE ) ),
//CHINA001 aPopUpFile       ( ScResId( RID_POPUP_FCOMMAND) ),
//CHINA001 nWhich           ( nWhichId )
//CHINA001 {
//CHINA001 //!  use default style from current document?
//CHINA001 //!  if font color is used, header/footer background color must be set
//CHINA001
//CHINA001 ScPatternAttr    aPatAttr( rCoreAttrs.GetPool() );
//CHINA001
//CHINA001 aBtnFile.SetPopupMenu(&aPopUpFile);
//CHINA001
//CHINA001 aBtnFile.SetMenuHdl( LINK( this, ScHFEditPage, MenuHdl ) );
//CHINA001 aBtnText .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
//CHINA001 aBtnPage .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
//CHINA001 aBtnLastPage.SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
//CHINA001 aBtnDate .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
//CHINA001 aBtnTime .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
//CHINA001 aBtnFile .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
//CHINA001 aBtnTable    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
//CHINA001
//CHINA001 aBtnText    .SetModeImage( Image( ScResId( IMG_TEXT_H  ) ), BMP_COLOR_HIGHCONTRAST );
//CHINA001 aBtnFile    .SetModeImage( Image( ScResId( IMG_FILE_H  ) ), BMP_COLOR_HIGHCONTRAST );
//CHINA001 aBtnTable   .SetModeImage( Image( ScResId( IMG_TABLE_H ) ), BMP_COLOR_HIGHCONTRAST );
//CHINA001 aBtnPage    .SetModeImage( Image( ScResId( IMG_PAGE_H  ) ), BMP_COLOR_HIGHCONTRAST );
//CHINA001 aBtnLastPage.SetModeImage( Image( ScResId( IMG_PAGES_H ) ), BMP_COLOR_HIGHCONTRAST );
//CHINA001 aBtnDate    .SetModeImage( Image( ScResId( IMG_DATE_H  ) ), BMP_COLOR_HIGHCONTRAST );
//CHINA001 aBtnTime    .SetModeImage( Image( ScResId( IMG_TIME_H  ) ), BMP_COLOR_HIGHCONTRAST );
//CHINA001
//CHINA001 aWndLeft.    SetFont( aPatAttr );
//CHINA001 aWndCenter. SetFont( aPatAttr );
//CHINA001 aWndRight.   SetFont( aPatAttr );
//CHINA001
//CHINA001 FillCmdArr();
//CHINA001
//CHINA001 aWndLeft.GrabFocus();
//CHINA001
//CHINA001 FreeResource();
//CHINA001 }
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 __EXPORT ScHFEditPage::~ScHFEditPage()
//CHINA001 {
//CHINA001 }
//CHINA001
//CHINA001 void ScHFEditPage::SetNumType(SvxNumType eNumType)
//CHINA001 {
//CHINA001 aWndLeft.SetNumType(eNumType);
//CHINA001 aWndCenter.SetNumType(eNumType);
//CHINA001 aWndRight.SetNumType(eNumType);
//CHINA001 }
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 #define IS_AVAILABLE(w)(rCoreSet.GetItemState( (w) ) >= SFX_ITEM_AVAILABLE)
//CHINA001
//CHINA001 void __EXPORT ScHFEditPage::Reset( const SfxItemSet& rCoreSet )
//CHINA001 {
//CHINA001 if ( IS_AVAILABLE( nWhich ) )
//CHINA001  {
//CHINA001 const ScPageHFItem& rItem = (const ScPageHFItem&)(rCoreSet.Get( nWhich ));
//CHINA001
//CHINA001 const EditTextObject* pLeft   = rItem.GetLeftArea();
//CHINA001 const EditTextObject* pCenter = rItem.GetCenterArea();
//CHINA001 const EditTextObject* pRight  = rItem.GetRightArea();
//CHINA001
//CHINA001 if ( pLeft && pCenter && pRight )
//CHINA001      {
//CHINA001 aWndLeft .SetText( *pLeft );
//CHINA001 aWndCenter   .SetText( *pCenter );
//CHINA001 aWndRight    .SetText( *pRight );
//CHINA001      }
//CHINA001  }
//CHINA001 }
//CHINA001
//CHINA001 #undef IS_AVAILABLE
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 sal_Bool __EXPORT ScHFEditPage::FillItemSet( SfxItemSet& rCoreSet )
//CHINA001 {
//CHINA001 ScPageHFItem aItem( nWhich );
//CHINA001 EditTextObject* pLeft    = aWndLeft  .CreateTextObject();
//CHINA001 EditTextObject* pCenter = aWndCenter.CreateTextObject();
//CHINA001 EditTextObject* pRight   = aWndRight .CreateTextObject();
//CHINA001
//CHINA001 aItem.SetLeftArea  ( *pLeft );
//CHINA001 aItem.SetCenterArea( *pCenter );
//CHINA001 aItem.SetRightArea ( *pRight );
//CHINA001 delete pLeft;
//CHINA001 delete pCenter;
//CHINA001 delete pRight;
//CHINA001
//CHINA001 rCoreSet.Put( aItem );
//CHINA001
//CHINA001 return sal_True;
//CHINA001 }
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 #define SET_CMD(i,id) x
//CHINA001 aCmd  = aDel;                            x
//CHINA001 aCmd += ScGlobal::GetRscString( id );    x
//CHINA001 aCmd += aDel;                            x
//CHINA001 aCmdArr[i] = aCmd;
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 void ScHFEditPage::FillCmdArr()
//CHINA001 {
//CHINA001 String aDel( ScGlobal::GetRscString( STR_HFCMD_DELIMITER ) );
//CHINA001 String aCmd;
//CHINA001
//CHINA001 SET_CMD( 0, STR_HFCMD_PAGE )
//CHINA001 SET_CMD( 1, STR_HFCMD_PAGES )
//CHINA001 SET_CMD( 2, STR_HFCMD_DATE )
//CHINA001 SET_CMD( 3, STR_HFCMD_TIME )
//CHINA001 SET_CMD( 4, STR_HFCMD_FILE )
//CHINA001 SET_CMD( 5, STR_HFCMD_TABLE )
//CHINA001 }
//CHINA001
//CHINA001 #undef SET_CMD
//CHINA001
//CHINA001 //-----------------------------------------------------------------------
//CHINA001 // Handler:
//CHINA001 //-----------------------------------------------------------------------
//CHINA001
//CHINA001 IMPL_LINK( ScHFEditPage, ClickHdl, ImageButton*, pBtn )
//CHINA001 {
//CHINA001 if ( !pActiveEdWnd )
//CHINA001 return 0;
//CHINA001
//CHINA001 if ( pBtn == &aBtnText )
//CHINA001  {
//CHINA001 pActiveEdWnd->SetCharAttriutes();
//CHINA001  }
//CHINA001  else
//CHINA001  {
//CHINA001 if ( pBtn == &aBtnPage )
//CHINA001 pActiveEdWnd->InsertField( SvxFieldItem(SvxPageField()) );
//CHINA001      else if ( pBtn == &aBtnLastPage )
//CHINA001 pActiveEdWnd->InsertField( SvxFieldItem(SvxPagesField()) );
//CHINA001      else if ( pBtn == &aBtnDate )
//CHINA001 pActiveEdWnd->InsertField( SvxFieldItem(SvxDateField(Date(),SVXDATETYPE_VAR)) );
//CHINA001      else if ( pBtn == &aBtnTime )
//CHINA001 pActiveEdWnd->InsertField( SvxFieldItem(SvxTimeField()) );
//CHINA001      else if ( pBtn == &aBtnFile )
//CHINA001      {
//CHINA001 pActiveEdWnd->InsertField( SvxFieldItem( SvxFileField() ) );
//CHINA001      }
//CHINA001      else if ( pBtn == &aBtnTable )
//CHINA001 pActiveEdWnd->InsertField( SvxFieldItem(SvxTableField()) );
//CHINA001  }
//CHINA001 pActiveEdWnd->GrabFocus();
//CHINA001
//CHINA001 return 0;
//CHINA001 }
//CHINA001
//CHINA001 IMPL_LINK( ScHFEditPage, MenuHdl, ScExtIButton*, pBtn )
//CHINA001 {
//CHINA001 if ( !pActiveEdWnd )
//CHINA001 return 0;
//CHINA001
//CHINA001 if(pBtn!=NULL)
//CHINA001  {
//CHINA001 switch(pBtn->GetSelected())
//CHINA001      {
//CHINA001          case FILE_COMMAND_TITEL:
//CHINA001 pActiveEdWnd->InsertField( SvxFieldItem( SvxFileField() ) );
//CHINA001 break;
//CHINA001          case FILE_COMMAND_FILENAME:
//CHINA001 pActiveEdWnd->InsertField( SvxFieldItem( SvxExtFileField(
//CHINA001 EMPTY_STRING, SVXFILETYPE_VAR, SVXFILEFORMAT_NAME_EXT ) ) );
//CHINA001 break;
//CHINA001          case FILE_COMMAND_PATH:
//CHINA001 pActiveEdWnd->InsertField( SvxFieldItem( SvxExtFileField(
//CHINA001 EMPTY_STRING, SVXFILETYPE_VAR, SVXFILEFORMAT_FULLPATH ) ) );
//CHINA001 break;
//CHINA001      }
//CHINA001  }
//CHINA001 return 0;
//CHINA001 }
//CHINA001
//CHINA001 //========================================================================
//CHINA001 // class ScRightHeaderEditPage
//CHINA001 //========================================================================
//CHINA001
//CHINA001 ScRightHeaderEditPage::ScRightHeaderEditPage( Window* pParent, const SfxItemSet& rCoreSet )
//CHINA001 : ScHFEditPage( pParent, RID_SCPAGE_HFED_HR, rCoreSet,
//CHINA001 rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_HEADERRIGHT) )
//CHINA001  {}
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 sal_uInt16* __EXPORT ScRightHeaderEditPage::GetRanges()
//CHINA001  { return pPageRightHeaderRanges; }
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 SfxTabPage* __EXPORT ScRightHeaderEditPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
//CHINA001  { return ( new ScRightHeaderEditPage( pParent, rCoreSet ) ); };
//CHINA001
//CHINA001
//CHINA001 //========================================================================
//CHINA001 // class ScLeftHeaderEditPage
//CHINA001 //========================================================================
//CHINA001
//CHINA001 ScLeftHeaderEditPage::ScLeftHeaderEditPage( Window* pParent, const SfxItemSet& rCoreSet )
//CHINA001 : ScHFEditPage( pParent, RID_SCPAGE_HFED_HL, rCoreSet,
//CHINA001 rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_HEADERLEFT) )
//CHINA001  {}
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 sal_uInt16* __EXPORT ScLeftHeaderEditPage::GetRanges()
//CHINA001  { return pPageLeftHeaderRanges; }
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 SfxTabPage* __EXPORT ScLeftHeaderEditPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
//CHINA001  { return ( new ScLeftHeaderEditPage( pParent, rCoreSet ) ); };
//CHINA001
//CHINA001 //========================================================================
//CHINA001 // class ScRightFooterEditPage
//CHINA001 //========================================================================
//CHINA001
//CHINA001 ScRightFooterEditPage::ScRightFooterEditPage( Window* pParent, const SfxItemSet& rCoreSet )
//CHINA001 : ScHFEditPage( pParent, RID_SCPAGE_HFED_FR, rCoreSet,
//CHINA001 rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_FOOTERRIGHT) )
//CHINA001  {}
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 sal_uInt16* __EXPORT ScRightFooterEditPage::GetRanges()
//CHINA001  { return pPageRightFooterRanges; }
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 SfxTabPage* __EXPORT ScRightFooterEditPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
//CHINA001  { return ( new ScRightFooterEditPage( pParent, rCoreSet ) ); };
//CHINA001
//CHINA001 //========================================================================
//CHINA001 // class ScLeftFooterEditPage
//CHINA001 //========================================================================
//CHINA001
//CHINA001 ScLeftFooterEditPage::ScLeftFooterEditPage( Window* pParent, const SfxItemSet& rCoreSet )
//CHINA001 : ScHFEditPage( pParent, RID_SCPAGE_HFED_FL, rCoreSet,
//CHINA001 rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_FOOTERLEFT) )
//CHINA001  {}
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 sal_uInt16* __EXPORT ScLeftFooterEditPage::GetRanges()
//CHINA001  { return pPageLeftFooterRanges; }
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 SfxTabPage* __EXPORT ScLeftFooterEditPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
//CHINA001  { return ( new ScLeftFooterEditPage( pParent, rCoreSet ) ); };

//========================================================================

void lcl_GetFieldData( ScHeaderFieldData& rData )
{
    SfxViewShell* pShell = SfxViewShell::Current();
    if (pShell)
    {
        if (pShell->ISA(ScTabViewShell))
            ((ScTabViewShell*)pShell)->FillFieldData(rData);
        else if (pShell->ISA(ScPreviewShell))
            ((ScPreviewShell*)pShell)->FillFieldData(rData);
    }
}

//========================================================================
// class ScEditWindow
//========================================================================

ScEditWindow::ScEditWindow( Window* pParent, const ResId& rResId, ScEditWindowLocation eLoc )
    :   Control( pParent, rResId ),
    eLocation(eLoc),
    pAcc(NULL)
{
    EnableRTL(sal_False);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    SetMapMode( MAP_TWIP );
    SetPointer( POINTER_TEXT );
    SetBackground( aBgColor );

    Size aSize( GetOutputSize() );
    aSize.Height() *= 4;

    pEdEngine = new ScHeaderEditEngine( EditEngine::CreatePool(), sal_True );
    pEdEngine->SetPaperSize( aSize );
    pEdEngine->SetRefDevice( this );

    ScHeaderFieldData aData;
    lcl_GetFieldData( aData );

        //  Feldbefehle:
    pEdEngine->SetData( aData );
    pEdEngine->SetControlWord( pEdEngine->GetControlWord() | EE_CNTRL_MARKFIELDS );
    mbRTL = ScGlobal::IsSystemRTL();
    if (mbRTL)
        pEdEngine->SetDefaultHorizontalTextDirection(EE_HTEXTDIR_R2L);

    pEdView = new EditView( pEdEngine, this );
    pEdView->SetOutputArea( Rectangle( Point(0,0), GetOutputSize() ) );

    pEdView->SetBackgroundColor( aBgColor );
    pEdEngine->InsertView( pEdView );
}

// -----------------------------------------------------------------------

__EXPORT ScEditWindow::~ScEditWindow()
{
    // delete Accessible object before deleting EditEngine and EditView
    if (pAcc)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xTemp = xAcc;
        if (xTemp.is())
            pAcc->dispose();
    }
    delete pEdEngine;
    delete pEdView;
}

// -----------------------------------------------------------------------

void ScEditWindow::SetNumType(SvxNumType eNumType)
{
    pEdEngine->SetNumType(eNumType);
    pEdEngine->UpdateFields();
}

// -----------------------------------------------------------------------

EditTextObject* __EXPORT ScEditWindow::CreateTextObject()
{
    //  wegen #38841# die Absatzattribute zuruecksetzen
    //  (GetAttribs beim Format-Dialog-Aufruf gibt immer gesetzte Items zurueck)

    const SfxItemSet& rEmpty = pEdEngine->GetEmptyItemSet();
    sal_uInt16 nParCnt = pEdEngine->GetParagraphCount();
    for (sal_uInt16 i=0; i<nParCnt; i++)
        pEdEngine->SetParaAttribs( i, rEmpty );

    return pEdEngine->CreateTextObject();
}

// -----------------------------------------------------------------------

void ScEditWindow::SetFont( const ScPatternAttr& rPattern )
{
    SfxItemSet* pSet = new SfxItemSet( pEdEngine->GetEmptyItemSet() );
    rPattern.FillEditItemSet( pSet );
    //  FillEditItemSet adjusts font height to 1/100th mm,
    //  but for header/footer twips is needed, as in the PatternAttr:
    pSet->Put( rPattern.GetItem(ATTR_FONT_HEIGHT), EE_CHAR_FONTHEIGHT );
    pSet->Put( rPattern.GetItem(ATTR_CJK_FONT_HEIGHT), EE_CHAR_FONTHEIGHT_CJK );
    pSet->Put( rPattern.GetItem(ATTR_CTL_FONT_HEIGHT), EE_CHAR_FONTHEIGHT_CTL );
    if (mbRTL)
        pSet->Put( SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ) );
    pEdEngine->SetDefaults( pSet );
}

// -----------------------------------------------------------------------

void ScEditWindow::SetText( const EditTextObject& rTextObject )
{
    pEdEngine->SetText( rTextObject );
}

// -----------------------------------------------------------------------

void ScEditWindow::InsertField( const SvxFieldItem& rFld )
{
    pEdView->InsertField( rFld );
}

// -----------------------------------------------------------------------

void ScEditWindow::SetCharAttriutes()
{
    SfxObjectShell* pDocSh  = SfxObjectShell::Current();

    SfxViewShell*       pViewSh = SfxViewShell::Current();

    ScTabViewShell* pTabViewSh = PTR_CAST(ScTabViewShell, SfxViewShell::Current());


    DBG_ASSERT( pDocSh,  "Current DocShell not found" );
    DBG_ASSERT( pViewSh, "Current ViewShell not found" );

    if ( pDocSh && pViewSh )
    {
        if(pTabViewSh!=NULL) pTabViewSh->SetInFormatDialog(sal_True);

        SfxItemSet aSet( pEdView->GetAttribs() );

        //CHINA001 ScCharDlg* pDlg = new ScCharDlg( GetParent(), &aSet, pDocSh );
        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

        SfxAbstractTabDialog* pDlg = pFact->CreateScCharDlg(  GetParent(),  &aSet,
                                                            pDocSh,RID_SCDLG_CHAR );
        DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
        pDlg->SetText( ScGlobal::GetRscString( STR_TEXTATTRS ) );
        if ( pDlg->Execute() == RET_OK )
        {
            aSet.ClearItem();
            aSet.Put( *pDlg->GetOutputItemSet() );
            pEdView->SetAttribs( aSet );
        }

        if(pTabViewSh!=NULL) pTabViewSh->SetInFormatDialog(sal_False);
        delete pDlg;
    }
}

// -----------------------------------------------------------------------

void __EXPORT ScEditWindow::Paint( const Rectangle& rRec )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    pEdView->SetBackgroundColor( aBgColor );

    SetBackground( aBgColor );

    Control::Paint( rRec );

    pEdView->Paint( rRec );
}

// -----------------------------------------------------------------------

void __EXPORT ScEditWindow::MouseMove( const MouseEvent& rMEvt )
{
    pEdView->MouseMove( rMEvt );
}

// -----------------------------------------------------------------------

void __EXPORT ScEditWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !HasFocus() )
        GrabFocus();

    pEdView->MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void __EXPORT ScEditWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    pEdView->MouseButtonUp( rMEvt );
}

// -----------------------------------------------------------------------

void __EXPORT ScEditWindow::KeyInput( const KeyEvent& rKEvt )
{
    sal_uInt16 nKey =  rKEvt.GetKeyCode().GetModifier()
                 + rKEvt.GetKeyCode().GetCode();

    if ( nKey == KEY_TAB || nKey == KEY_TAB + KEY_SHIFT )
    {
        Control::KeyInput( rKEvt );
    }
    else if ( !pEdView->PostKeyEvent( rKEvt ) )
    {
        Control::KeyInput( rKEvt );
    }
}

// -----------------------------------------------------------------------

void ScEditWindow::Command( const CommandEvent& rCEvt )
{
    pEdView->Command( rCEvt );
}

// -----------------------------------------------------------------------

void __EXPORT ScEditWindow::GetFocus()
{
    pActiveEdWnd = this;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xTemp = xAcc;
    if (xTemp.is() && pAcc)
    {
        pAcc->GotFocus();
    }
    else
        pAcc = NULL;
}

void __EXPORT ScEditWindow::LoseFocus()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xTemp = xAcc;
    if (xTemp.is() && pAcc)
    {
        pAcc->LostFocus();
    }
    else
        pAcc = NULL;
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > ScEditWindow::CreateAccessible()
{
    String sName;
    String sDescription(GetHelpText());
    switch (eLocation)
    {
    case Left:
        {
            sName = String(ScResId(STR_ACC_LEFTAREA_NAME));
//            sDescription = String(ScResId(STR_ACC_LEFTAREA_DESCR));
        }
        break;
    case Center:
        {
            sName = String(ScResId(STR_ACC_CENTERAREA_NAME));
//            sDescription = String(ScResId(STR_ACC_CENTERAREA_DESCR));
        }
        break;
    case Right:
        {
            sName = String(ScResId(STR_ACC_RIGHTAREA_NAME));
//            sDescription = String(ScResId(STR_ACC_RIGHTAREA_DESCR));
        }
        break;
    }
    pAcc = new ScAccessibleEditObject(GetAccessibleParentWindow()->GetAccessible(), pEdView, this,
        rtl::OUString(sName), rtl::OUString(sDescription), EditControl);
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAccessible = pAcc;
    xAcc = xAccessible;
    return pAcc;
}

/*
class ScExtIButton : public ImageButton
{
private:

    Timer           aTimer;
    ScPopupMenu*    pPopupMenu;

    DECL_LINK( TimerHdl, Timer*);

    void            DrawArrow();

protected:

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt);

    virtual void    StartPopup();

public:

    ScExtIButton(Window* pParent, const ResId& rResId );

    void            SetPopupMenu(ScPopupMenu* pPopUp);

    sal_uInt16          GetSelected();

    void            SetMenuHdl( const Link& rLink ) { aFxLink = rLink; }
    const Link&     GetMenuHdl() const { return aFxLink; }

}
*/
ScExtIButton::ScExtIButton(Window* pParent, const ResId& rResId )
:   ImageButton(pParent,rResId),
    pPopupMenu(NULL)
{
    nSelected=0;
    aTimer.SetTimeout(600);
    SetDropDown( sal_True);

//  DrawArrow();
}

void ScExtIButton::SetPopupMenu(ScPopupMenu* pPopUp)
{
    pPopupMenu=pPopUp;
}

sal_uInt16 ScExtIButton::GetSelected()
{
    return nSelected;
}

void ScExtIButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    if(!aTimer.IsActive())
    {
        aTimer.Start();
        aTimer.SetTimeoutHdl(LINK( this, ScExtIButton, TimerHdl));
    }

    ImageButton::MouseButtonDown(rMEvt );
}
void ScExtIButton::MouseButtonUp( const MouseEvent& rMEvt)
{
    aTimer.Stop();
    aTimer.SetTimeoutHdl(Link());
    ImageButton::MouseButtonUp(rMEvt );
}

void ScExtIButton::Click()
{
    aTimer.Stop();
    aTimer.SetTimeoutHdl(Link());
    ImageButton::Click();
}

void ScExtIButton::StartPopup()
{
    nSelected=0;

    if(pPopupMenu!=NULL)
    {
        SetPressed( sal_True );
        EndSelection();
        Point aPoint(0,0);
        aPoint.Y()=GetOutputSizePixel().Height();

        nSelected=pPopupMenu->Execute( this, aPoint );

        if(nSelected)
        {
            aMLink.Call(this);
        }
        SetPressed( sal_False);
    }
}

long ScExtIButton::PreNotify( NotifyEvent& rNEvt )
{
    sal_uInt16 nSwitch=rNEvt.GetType();
    if(nSwitch==EVENT_MOUSEBUTTONUP)
    {
        MouseButtonUp(*rNEvt.GetMouseEvent());
    }

    return ImageButton::PreNotify(rNEvt );
}

IMPL_LINK( ScExtIButton, TimerHdl, Timer*, EMPTYARG )
{
    StartPopup();
    return 0;
}

/*
static void ImplDrawToolArrow( ToolBox* pBox, long nX, long nY, sal_Bool bBlack,
                               sal_Bool bLeft = sal_False, sal_Bool bTop = sal_False )
{
    Color           aOldFillColor = pBox->GetFillColor();
    WindowAlign     eAlign = pBox->meAlign;
    if ( bLeft )
        eAlign = WINDOWALIGN_RIGHT;
    else if ( bTop )
        eAlign = WINDOWALIGN_BOTTOM;

    switch ( eAlign )
    {
        case WINDOWALIGN_LEFT:
            if ( bBlack )
                pBox->SetFillColor( Color( COL_BLACK ) );
            pBox->DrawRect( Rectangle( nX+0, nY+0, nX+0, nY+6 ) );
            pBox->DrawRect( Rectangle( nX+1, nY+1, nX+1, nY+5 ) );
            pBox->DrawRect( Rectangle( nX+2, nY+2, nX+2, nY+4 ) );
            pBox->DrawRect( Rectangle( nX+3, nY+3, nX+3, nY+3 ) );
            if ( bBlack )
            {
                pBox->SetFillColor( aOldFillColor );
                pBox->DrawRect( Rectangle( nX+1, nY+2, nX+1, nY+4 ) );
                pBox->DrawRect( Rectangle( nX+2, nY+3, nX+2, nY+3 ) );
            }
            break;
        case WINDOWALIGN_TOP:
            if ( bBlack )
                pBox->SetFillColor( Color( COL_BLACK ) );
            pBox->DrawRect( Rectangle( nX+0, nY+0, nX+6, nY+0 ) );
            pBox->DrawRect( Rectangle( nX+1, nY+1, nX+5, nY+1 ) );
            pBox->DrawRect( Rectangle( nX+2, nY+2, nX+4, nY+2 ) );
            pBox->DrawRect( Rectangle( nX+3, nY+3, nX+3, nY+3 ) );
            if ( bBlack )
            {
                pBox->SetFillColor( aOldFillColor );
                pBox->DrawRect( Rectangle( nX+2, nY+1, nX+4, nY+1 ) );
                pBox->DrawRect( Rectangle( nX+3, nY+2, nX+3, nY+2 ) );
            }
            break;
        case WINDOWALIGN_RIGHT:
            if ( bBlack )
                pBox->SetFillColor( Color( COL_BLACK ) );
            pBox->DrawRect( Rectangle( nX+3, nY+0, nX+3, nY+6 ) );
            pBox->DrawRect( Rectangle( nX+2, nY+1, nX+2, nY+5 ) );
            pBox->DrawRect( Rectangle( nX+1, nY+2, nX+1, nY+4 ) );
            pBox->DrawRect( Rectangle( nX+0, nY+3, nX+0, nY+3 ) );
            if ( bBlack )
            {
                pBox->SetFillColor( aOldFillColor );
                pBox->DrawRect( Rectangle( nX+2, nY+2, nX+2, nY+4 ) );
                pBox->DrawRect( Rectangle( nX+1, nY+3, nX+1, nY+3 ) );
            }
            break;
        case WINDOWALIGN_BOTTOM:
            if ( bBlack )
                pBox->SetFillColor( Color( COL_BLACK ) );
            pBox->DrawRect( Rectangle( nX+0, nY+3, nX+6, nY+3 ) );
            pBox->DrawRect( Rectangle( nX+1, nY+2, nX+5, nY+2 ) );
            pBox->DrawRect( Rectangle( nX+2, nY+1, nX+4, nY+1 ) );
            pBox->DrawRect( Rectangle( nX+3, nY+0, nX+3, nY+0 ) );
            if ( bBlack )
            {
                pBox->SetFillColor( aOldFillColor );
                pBox->DrawRect( Rectangle( nX+2, nY+2, nX+4, nY+2 ) );
                pBox->DrawRect( Rectangle( nX+3, nY+1, nX+3, nY+1 ) );
            }
            break;
    }
}
Down
    - Timer starten

Click
    - Timer abbrechen

Timer
    if ( ??? )
    {
    - SetPressed( sal_True );
    - EndSelection();
    - Menu anzeigen
    - SetPressed( sal_False );
    }


*/


