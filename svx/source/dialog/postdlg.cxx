/*************************************************************************
 *
 *  $RCSfile: postdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:11 $
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

// include ---------------------------------------------------------------

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#ifndef _SFX_INIMGR_HXX //autogen
#include <sfx2/inimgr.hxx>
#endif
#pragma hdrstop

#define _SVX_POSTDLG_CXX

#include "dialogs.hrc"
#include "postdlg.hrc"

#define ITEMID_AUTHOR   SID_ATTR_POSTIT_AUTHOR
#define ITEMID_DATE     SID_ATTR_POSTIT_DATE
#define ITEMID_TEXT     SID_ATTR_POSTIT_TEXT

#include "postattr.hxx"
#include "postdlg.hxx"
#include "dialmgr.hxx"

#include "helpid.hrc"
// static ----------------------------------------------------------------

static USHORT pRanges[] =
{
    SID_ATTR_POSTIT_AUTHOR,
    SID_ATTR_POSTIT_TEXT,
    0
};

// class SvxPostItDialog -------------------------------------------------

SvxPostItDialog::SvxPostItDialog( Window* pParent,
                                  const SfxItemSet& rCoreSet,
                                  BOOL bPrevNext,
                                  BOOL bRedline ) :

    SfxModalDialog( pParent, SVX_RES( RID_SVXDLG_POSTIT ) ),

    aLastEditLabelFT( this, ResId( FT_LASTEDITLABEL ) ),
    aLastEditFT     ( this, ResId( FT_LASTEDIT ) ),
    aEditFT         ( this, ResId( FT_EDIT ) ),
    aEditED         ( this, ResId( ED_EDIT ) ),
    aPostItGB       ( this, ResId( GB_POSTIT ) ),
    aOKBtn          ( this, ResId( BTN_POST_OK ) ),
    aCancelBtn      ( this, ResId( BTN_POST_CANCEL ) ),
    aPrevBtn        ( this, ResId( BTN_PREV ) ),
    aNextBtn        ( this, ResId( BTN_NEXT ) ),
    aAuthorBtn      ( this, ResId( BTN_AUTHOR ) ),
    aHelpBtn        ( this, ResId( BTN_POST_HELP ) ),

    rSet        ( rCoreSet ),
    pOutSet     ( 0 )

{
    if (bRedline)   // HelpIDs fuer Redlining
    {
        SetHelpId(HID_REDLINING_DLG);
        aEditED.SetHelpId(HID_REDLINING_EDIT);
        aPrevBtn.SetHelpId(HID_REDLINING_PREV);
        aNextBtn.SetHelpId(HID_REDLINING_NEXT);
    }

    aPrevBtn.SetClickHdl( LINK( this, SvxPostItDialog, PrevHdl ) );
    aNextBtn.SetClickHdl( LINK( this, SvxPostItDialog, NextHdl ) );
    aAuthorBtn.SetClickHdl( LINK( this, SvxPostItDialog, Stamp ) );
    aOKBtn.SetClickHdl( LINK( this, SvxPostItDialog, OKHdl ) );

    Font aFont( aEditED.GetFont() );
    aFont.SetWeight( WEIGHT_LIGHT );
    aEditED.SetFont( aFont );

    BOOL bNew = TRUE;
    const SfxPoolItem* pItem = 0;
    USHORT nWhich            = 0;

    if ( !bPrevNext )
    {
        aPrevBtn.Hide();
        aNextBtn.Hide();
    }

    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_AUTHOR );
    String aAuthorStr, aDateStr, aTextStr;

    if ( rSet.GetItemState( nWhich, TRUE ) >= SFX_ITEM_AVAILABLE )
    {
        bNew = FALSE;
        const SvxPostItAuthorItem& rAuthor =
            (const SvxPostItAuthorItem&)rSet.Get( nWhich );
        aAuthorStr = rAuthor.GetValue();
    }
    else
        aAuthorStr = SFX_INIMANAGER()->Get(SFX_KEY_USER_ID);

    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_DATE );

    if ( rSet.GetItemState( nWhich, TRUE ) >= SFX_ITEM_AVAILABLE )
    {
        const SvxPostItDateItem& rDate =
            (const SvxPostItDateItem&)rSet.Get( nWhich );
        aDateStr = rDate.GetValue();
    }
    else
    {
        International aInter( GetpApp()->GetAppInternational() );
        aDateStr = aInter.GetDate( Date() );
    }

    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_TEXT );

    if ( rSet.GetItemState( nWhich, TRUE ) >= SFX_ITEM_AVAILABLE )
    {
        const SvxPostItTextItem& rText =
            (const SvxPostItTextItem&)rSet.Get( nWhich );
        aTextStr = rText.GetValue();
    }

    ShowLastAuthor(aAuthorStr, aDateStr);
    aEditED.SetText( aTextStr.ConvertLineEnd() );

    if ( !bNew )
        SetText( SVX_RESSTR( STR_NOTIZ_EDIT ) );
    else
        // neu anlegen
        SetText( SVX_RESSTR( STR_NOTIZ_INSERT ) );

    FreeResource();
}

// -----------------------------------------------------------------------

SvxPostItDialog::~SvxPostItDialog()
{
    delete pOutSet;
    pOutSet = 0;
}

// -----------------------------------------------------------------------

void SvxPostItDialog::ShowLastAuthor(const String& rAuthor, const String& rDate)
{
    String sTxt( rAuthor );
    sTxt.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ", " ) );
    sTxt += rDate;
    aLastEditFT.SetText( sTxt );
}

// -----------------------------------------------------------------------

USHORT* SvxPostItDialog::GetRanges()
{
    return pRanges;
}

// -----------------------------------------------------------------------

void SvxPostItDialog::EnableTravel(BOOL bNext, BOOL bPrev)
{
    aPrevBtn.Enable(bPrev);
    aNextBtn.Enable(bNext);
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxPostItDialog, PrevHdl, Button *, EMPTYARG )
{
    aPrevHdlLink.Call( this );
    return 0;
}
IMPL_LINK_INLINE_END( SvxPostItDialog, PrevHdl, Button *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxPostItDialog, NextHdl, Button *, EMPTYARG )
{
    aNextHdlLink.Call( this );
    return 0;
}
IMPL_LINK_INLINE_END( SvxPostItDialog, NextHdl, Button *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK( SvxPostItDialog, Stamp, Button *, EMPTYARG )
{
    Date    aDate;
    Time    aTime;
    String  aTmp( SFX_INIMANAGER()->Get(SFX_KEY_USER_ID) );
    International aInter( GetpApp()->GetAppInternational() );
    String aStr( aEditED.GetText() );

    aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "\n---- " ) );


    if ( aTmp.Len() > 0 )
    {
        aStr += aTmp;
        aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ", " ) );
    }
    aStr += aInter.GetDate(aDate);
    aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ", " ) );
    aStr += aInter.GetTime(aTime, FALSE, FALSE);
    aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " ----\n" ) );


    aEditED.SetText( aStr.ConvertLineEnd() );
    xub_StrLen nLen = aStr.Len();
    aEditED.GrabFocus();
    aEditED.SetSelection( Selection( nLen, nLen ) );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPostItDialog, OKHdl, Button *, EMPTYARG )
{
    International aInter( GetpApp()->GetAppInternational() );
    pOutSet = new SfxItemSet( rSet );
    pOutSet->Put( SvxPostItAuthorItem( SFX_INIMANAGER()->Get(SFX_KEY_USER_ID),
        rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_AUTHOR ) ) );
    pOutSet->Put( SvxPostItDateItem( aInter.GetDate( Date() ),
        rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_DATE ) ) );
    pOutSet->Put( SvxPostItTextItem( aEditED.GetText(),
        rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_TEXT ) ) );
    EndDialog( RET_OK );
    return 0;
}


