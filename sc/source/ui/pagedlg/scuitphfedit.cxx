/*************************************************************************
 *
 *  $RCSfile: scuitphfedit.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:05:13 $
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

//CHINA001 #include <svx/chardlg.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#include <svx/editview.hxx>
#include <svx/flditem.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

//CHINA001 #include "tphfedit.hxx"
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
#include "textdlgs.hxx"

#ifndef _SC_ACCESSIBLEEDITOBJECT_HXX
#include "AccessibleEditObject.hxx"
#endif

#include "scuitphfedit.hxx" //CHINA001


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

        aWndLeft        ( this, ScResId( WND_LEFT ), Left ),
        aWndCenter      ( this, ScResId( WND_CENTER ), Center ),
        aWndRight       ( this, ScResId( WND_RIGHT ), Right ),
        aFtLeft         ( this, ScResId( FT_LEFT ) ),
        aFtCenter       ( this, ScResId( FT_CENTER ) ),
        aFtRight        ( this, ScResId( FT_RIGHT ) ),
        aFlInfo         ( this, ScResId( FL_INFO ) ),
        aFtInfo         ( this, ScResId( FT_INFO ) ),
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

    aBtnFile.SetPopupMenu(&aPopUpFile);

    aBtnFile.SetMenuHdl( LINK( this, ScHFEditPage, MenuHdl ) );
    aBtnText    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnPage    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnLastPage.SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnDate    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnTime    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnFile    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnTable   .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );

    aBtnText    .SetModeImage( Image( ScResId( IMG_TEXT_H  ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnFile    .SetModeImage( Image( ScResId( IMG_FILE_H  ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnTable   .SetModeImage( Image( ScResId( IMG_TABLE_H ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnPage    .SetModeImage( Image( ScResId( IMG_PAGE_H  ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnLastPage.SetModeImage( Image( ScResId( IMG_PAGES_H ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnDate    .SetModeImage( Image( ScResId( IMG_DATE_H  ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnTime    .SetModeImage( Image( ScResId( IMG_TIME_H  ) ), BMP_COLOR_HIGHCONTRAST );

    aWndLeft.   SetFont( aPatAttr );
    aWndCenter. SetFont( aPatAttr );
    aWndRight.  SetFont( aPatAttr );

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
    pActiveEdWnd = ::GetScEditWindow(); //CHINA001
    if ( !pActiveEdWnd )
        return 0;

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

    return 0;
}

IMPL_LINK( ScHFEditPage, MenuHdl, ScExtIButton*, pBtn )
{
    pActiveEdWnd = ::GetScEditWindow(); //CHINA001
    if ( !pActiveEdWnd )
        return 0;

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
    return 0;
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
