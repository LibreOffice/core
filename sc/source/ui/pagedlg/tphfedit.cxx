/*************************************************************************
 *
 *  $RCSfile: tphfedit.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:04 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#define _TPHFEDIT_CXX
#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/chardlg.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#include <svx/editview.hxx>
#include <svx/flditem.hxx>
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


// STATIC DATA -----------------------------------------------------------

static USHORT pPageRightHeaderRanges[] = { SID_SCATTR_PAGE_HEADERRIGHT,
                                           SID_SCATTR_PAGE_HEADERRIGHT,
                                           0 };

static USHORT pPageRightFooterRanges[] = { SID_SCATTR_PAGE_FOOTERRIGHT,
                                           SID_SCATTR_PAGE_FOOTERRIGHT,
                                           0 };

static USHORT pPageLeftHeaderRanges[] =  { SID_SCATTR_PAGE_HEADERLEFT,
                                           SID_SCATTR_PAGE_HEADERLEFT,
                                           0 };

static USHORT pPageLeftFooterRanges[] =  { SID_SCATTR_PAGE_FOOTERLEFT,
                                           SID_SCATTR_PAGE_FOOTERLEFT,
                                           0 };

static ScEditWindow* pActiveEdWnd = NULL;



//========================================================================
// class ScHFEditPage
//

ScHFEditPage::ScHFEditPage( Window*             pParent,
                            USHORT              nResId,
                            const SfxItemSet&   rCoreAttrs,
                            USHORT              nWhichId )

    :   SfxTabPage      ( pParent, ScResId( nResId ), rCoreAttrs ),

        aWndLeft        ( this, ScResId( WND_LEFT ) ),
        aWndCenter      ( this, ScResId( WND_CENTER ) ),
        aWndRight       ( this, ScResId( WND_RIGHT ) ),
        aFtLeft         ( this, ScResId( FT_LEFT ) ),
        aFtCenter       ( this, ScResId( FT_CENTER ) ),
        aFtRight        ( this, ScResId( FT_RIGHT ) ),
        aFtInfo         ( this, ScResId( FT_INFO ) ),
        aGbInfo         ( this, ScResId( GB_INFO ) ),
        aBtnText        ( this, ScResId( BTN_TEXT ) ),
        aBtnPage        ( this, ScResId( BTN_PAGE ) ),
        aBtnLastPage    ( this, ScResId( BTN_PAGES ) ),
        aBtnDate        ( this, ScResId( BTN_DATE ) ),
        aBtnTime        ( this, ScResId( BTN_TIME ) ),
        aBtnFile        ( this, ScResId( BTN_FILE ) ),
        aBtnTable       ( this, ScResId( BTN_TABLE ) ),
        aPopUpFile      ( ScResId( RID_POPUP_FCOMMAND) ),
        nWhich          ( nWhichId )
{
    //! use default style from current document?
    //! if font color is used, header/footer background color must be set

    ScPatternAttr   aPatAttr( rCoreAttrs.GetPool() );
    Font            aFnt;

    aPatAttr.GetFont( aFnt );
    aBtnFile.SetPopupMenu(&aPopUpFile);

    aBtnFile.SetMenuHdl( LINK( this, ScHFEditPage, MenuHdl ) );
    aBtnText    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnPage    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnLastPage.SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnDate    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnTime    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnFile    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnTable   .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );

    aWndLeft.   SetFont( aFnt );
    aWndCenter. SetFont( aFnt );
    aWndRight.  SetFont( aFnt );

    FillCmdArr();

    aWndLeft.GrabFocus();

    FreeResource();
}

// -----------------------------------------------------------------------

__EXPORT ScHFEditPage::~ScHFEditPage()
{
}

void ScHFEditPage::SetNumType(SvxNumType eNumType)
{
    aWndLeft.SetNumType(eNumType);
    aWndCenter.SetNumType(eNumType);
    aWndRight.SetNumType(eNumType);
}

// -----------------------------------------------------------------------

#define IS_AVAILABLE(w)(rCoreSet.GetItemState( (w) ) >= SFX_ITEM_AVAILABLE)

void __EXPORT ScHFEditPage::Reset( const SfxItemSet& rCoreSet )
{
    if ( IS_AVAILABLE( nWhich ) )
    {
        const ScPageHFItem& rItem = (const ScPageHFItem&)(rCoreSet.Get( nWhich ));

        const EditTextObject* pLeft   = rItem.GetLeftArea();
        const EditTextObject* pCenter = rItem.GetCenterArea();
        const EditTextObject* pRight  = rItem.GetRightArea();

        if ( pLeft && pCenter && pRight )
        {
            aWndLeft    .SetText( *pLeft );
            aWndCenter  .SetText( *pCenter );
            aWndRight   .SetText( *pRight );
        }
    }
}

#undef IS_AVAILABLE

// -----------------------------------------------------------------------

BOOL __EXPORT ScHFEditPage::FillItemSet( SfxItemSet& rCoreSet )
{
    ScPageHFItem    aItem( nWhich );
    EditTextObject* pLeft   = aWndLeft  .CreateTextObject();
    EditTextObject* pCenter = aWndCenter.CreateTextObject();
    EditTextObject* pRight  = aWndRight .CreateTextObject();

    aItem.SetLeftArea  ( *pLeft );
    aItem.SetCenterArea( *pCenter );
    aItem.SetRightArea ( *pRight );
    delete pLeft;
    delete pCenter;
    delete pRight;

    rCoreSet.Put( aItem );

    return TRUE;
}

// -----------------------------------------------------------------------

#define SET_CMD(i,id) \
    aCmd  = aDel;                           \
    aCmd += ScGlobal::GetRscString( id );   \
    aCmd += aDel;                           \
    aCmdArr[i] = aCmd;

// -----------------------------------------------------------------------

void ScHFEditPage::FillCmdArr()
{
    String aDel( ScGlobal::GetRscString( STR_HFCMD_DELIMITER ) );
    String aCmd;

    SET_CMD( 0, STR_HFCMD_PAGE )
    SET_CMD( 1, STR_HFCMD_PAGES )
    SET_CMD( 2, STR_HFCMD_DATE )
    SET_CMD( 3, STR_HFCMD_TIME )
    SET_CMD( 4, STR_HFCMD_FILE )
    SET_CMD( 5, STR_HFCMD_TABLE )
}

#undef SET_CMD

//-----------------------------------------------------------------------
// Handler:
//-----------------------------------------------------------------------

IMPL_LINK( ScHFEditPage, ClickHdl, ImageButton*, pBtn )
{
    if ( !pActiveEdWnd )
        return NULL;

    if ( pBtn == &aBtnText )
    {
        pActiveEdWnd->SetCharAttriutes();
    }
    else
    {
        if ( pBtn == &aBtnPage )
            pActiveEdWnd->InsertField( SvxFieldItem(SvxPageField()) );
        else if ( pBtn == &aBtnLastPage )
            pActiveEdWnd->InsertField( SvxFieldItem(SvxPagesField()) );
        else if ( pBtn == &aBtnDate )
            pActiveEdWnd->InsertField( SvxFieldItem(SvxDateField(Date(),SVXDATETYPE_VAR)) );
        else if ( pBtn == &aBtnTime )
            pActiveEdWnd->InsertField( SvxFieldItem(SvxTimeField()) );
        else if ( pBtn == &aBtnFile )
        {
            pActiveEdWnd->InsertField( SvxFieldItem( SvxFileField() ) );
        }
        else if ( pBtn == &aBtnTable )
            pActiveEdWnd->InsertField( SvxFieldItem(SvxTableField()) );
    }
    pActiveEdWnd->GrabFocus();

    return NULL;
}

IMPL_LINK( ScHFEditPage, MenuHdl, ScExtIButton*, pBtn )
{
    if ( !pActiveEdWnd )
        return NULL;

    if(pBtn!=NULL)
    {
        switch(pBtn->GetSelected())
        {
            case FILE_COMMAND_TITEL:
                pActiveEdWnd->InsertField( SvxFieldItem( SvxFileField() ) );
                break;
            case FILE_COMMAND_FILENAME:
                pActiveEdWnd->InsertField( SvxFieldItem( SvxExtFileField(
                        EMPTY_STRING, SVXFILETYPE_VAR, SVXFILEFORMAT_NAME_EXT ) ) );
                break;
            case FILE_COMMAND_PATH:
                pActiveEdWnd->InsertField( SvxFieldItem( SvxExtFileField(
                        EMPTY_STRING, SVXFILETYPE_VAR, SVXFILEFORMAT_FULLPATH ) ) );
                break;
        }
    }
    return NULL;
}

//========================================================================
// class ScRightHeaderEditPage
//========================================================================

ScRightHeaderEditPage::ScRightHeaderEditPage( Window* pParent, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pParent, RID_SCPAGE_HFED_HR, rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_HEADERRIGHT) )
    {}

// -----------------------------------------------------------------------

USHORT* __EXPORT ScRightHeaderEditPage::GetRanges()
    { return pPageRightHeaderRanges; }

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT ScRightHeaderEditPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
    { return ( new ScRightHeaderEditPage( pParent, rCoreSet ) ); };


//========================================================================
// class ScLeftHeaderEditPage
//========================================================================

ScLeftHeaderEditPage::ScLeftHeaderEditPage( Window* pParent, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pParent, RID_SCPAGE_HFED_HL, rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_HEADERLEFT) )
    {}

// -----------------------------------------------------------------------

USHORT* __EXPORT ScLeftHeaderEditPage::GetRanges()
    { return pPageLeftHeaderRanges; }

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT ScLeftHeaderEditPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
    { return ( new ScLeftHeaderEditPage( pParent, rCoreSet ) ); };

//========================================================================
// class ScRightFooterEditPage
//========================================================================

ScRightFooterEditPage::ScRightFooterEditPage( Window* pParent, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pParent, RID_SCPAGE_HFED_FR, rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_FOOTERRIGHT) )
    {}

// -----------------------------------------------------------------------

USHORT* __EXPORT ScRightFooterEditPage::GetRanges()
    { return pPageRightFooterRanges; }

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT ScRightFooterEditPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
    { return ( new ScRightFooterEditPage( pParent, rCoreSet ) ); };

//========================================================================
// class ScLeftFooterEditPage
//========================================================================

ScLeftFooterEditPage::ScLeftFooterEditPage( Window* pParent, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pParent, RID_SCPAGE_HFED_FL, rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_FOOTERLEFT) )
    {}

// -----------------------------------------------------------------------

USHORT* __EXPORT ScLeftFooterEditPage::GetRanges()
    { return pPageLeftFooterRanges; }

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT ScLeftFooterEditPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
    { return ( new ScLeftFooterEditPage( pParent, rCoreSet ) ); };

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

ScEditWindow::ScEditWindow( Window* pParent, const ResId& rResId )
    :   Control( pParent, rResId )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    SetMapMode( MAP_TWIP );
    SetPointer( POINTER_TEXT );
    SetBackground( aBgColor );

    Size aSize( GetOutputSize() );
    aSize.Height() *= 4;

    pEdEngine = new ScHeaderEditEngine( EditEngine::CreatePool(), TRUE );
    pEdEngine->SetPaperSize( aSize );
    pEdEngine->SetRefDevice( this );

    ScHeaderFieldData aData;
    lcl_GetFieldData( aData );

        //  Feldbefehle:
    pEdEngine->SetData( aData );
    pEdEngine->SetControlWord( pEdEngine->GetControlWord() | EE_CNTRL_MARKFIELDS );

    pEdView = new EditView( pEdEngine, this );
    pEdView->SetOutputArea( Rectangle( Point(0,0), GetOutputSize() ) );

    pEdView->SetBackgroundColor( aBgColor );
    pEdEngine->InsertView( pEdView );
}

// -----------------------------------------------------------------------

__EXPORT ScEditWindow::~ScEditWindow()
{
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
    USHORT nParCnt = pEdEngine->GetParagraphCount();
    for (USHORT i=0; i<nParCnt; i++)
        pEdEngine->SetParaAttribs( i, rEmpty );

    return pEdEngine->CreateTextObject();
}

// -----------------------------------------------------------------------

void ScEditWindow::SetFont( const Font& rFont )
{
    SfxItemSet* pSet = new SfxItemSet( pEdEngine->GetEmptyItemSet() );
    Font        aTextFont( rFont );

    aTextFont.SetTransparent( TRUE );
    aTextFont.SetColor( COL_BLACK );
    aTextFont.SetFillColor( COL_WHITE );

    pEdEngine->SetFontInfoInItemSet( *pSet, aTextFont );
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
        if(pTabViewSh!=NULL) pTabViewSh->SetInFormatDialog(TRUE);

        SfxItemSet aSet( pEdView->GetAttribs() );

        const SfxPoolItem* pInfoItem
            = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );

        SfxSingleTabDialog* pDlg
            = new SfxSingleTabDialog( pViewSh->GetViewFrame(),
                                      GetParent(),
                                      aSet,
                                      RID_SVXPAGE_CHAR_STD,
                                      FALSE );
        SvxCharStdPage* pPage
            = (SvxCharStdPage*)SvxCharStdPage::Create( pDlg, aSet );

        pPage->SetFontList( (const SvxFontListItem&)*pInfoItem );
        pDlg->SetTabPage( pPage );
        pDlg->SetText( ScGlobal::GetRscString( STR_TEXTATTRS ) );

        if ( pDlg->Execute() == RET_OK )
        {
            aSet.Put( *pDlg->GetOutputItemSet() );
            pEdView->SetAttribs( aSet );
        }
        if(pTabViewSh!=NULL) pTabViewSh->SetInFormatDialog(FALSE);
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
    USHORT nKey =  rKEvt.GetKeyCode().GetModifier()
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

void __EXPORT ScEditWindow::GetFocus()
{
    pActiveEdWnd = this;
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

    USHORT          GetSelected();

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
    SetDropDown( TRUE);

//  DrawArrow();
}

void ScExtIButton::SetPopupMenu(ScPopupMenu* pPopUp)
{
    pPopupMenu=pPopUp;
}

USHORT ScExtIButton::GetSelected()
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
        SetPressed( TRUE );
        EndSelection();
        Point aPoint(0,0);
        aPoint.Y()=GetOutputSizePixel().Height();

        nSelected=pPopupMenu->Execute( this, aPoint );

        if(nSelected)
        {
            aMLink.Call(this);
        }
        SetPressed( FALSE);
    }
}

long ScExtIButton::PreNotify( NotifyEvent& rNEvt )
{
    USHORT nSwitch=rNEvt.GetType();
    if(nSwitch==EVENT_MOUSEBUTTONUP)
    {
        MouseButtonUp(*rNEvt.GetMouseEvent());
    }

    return ImageButton::PreNotify(rNEvt );
}

IMPL_LINK( ScExtIButton, TimerHdl, Timer*, pTi)
{
    StartPopup();
    return 0;
}

/*
static void ImplDrawToolArrow( ToolBox* pBox, long nX, long nY, BOOL bBlack,
                               BOOL bLeft = FALSE, BOOL bTop = FALSE )
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
    - SetPressed( TRUE );
    - EndSelection();
    - Menu anzeigen
    - SetPressed( FALSE );
    }


*/


