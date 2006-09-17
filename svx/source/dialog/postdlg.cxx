/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: postdlg.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:35:19 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

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

    aPostItFL       ( this, ResId( FL_POSTIT ) ),
    aLastEditLabelFT( this, ResId( FT_LASTEDITLABEL ) ),
    aLastEditFT     ( this, ResId( FT_LASTEDIT ) ),
    aEditFT         ( this, ResId( FT_EDIT ) ),
    aEditED         ( this, ResId( ED_EDIT ) ),
    aAuthorFT       ( this, ResId( FT_AUTHOR) ),
    aAuthorBtn      ( this, ResId( BTN_AUTHOR ) ),
    aOKBtn          ( this, ResId( BTN_POST_OK ) ),
    aCancelBtn      ( this, ResId( BTN_POST_CANCEL ) ),
    aHelpBtn        ( this, ResId( BTN_POST_HELP ) ),
    aPrevBtn        ( this, ResId( BTN_PREV ) ),
    aNextBtn        ( this, ResId( BTN_NEXT ) ),

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
        aAuthorStr = SvtUserOptions().GetID();

    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_DATE );

    if ( rSet.GetItemState( nWhich, TRUE ) >= SFX_ITEM_AVAILABLE )
    {
        const SvxPostItDateItem& rDate =
            (const SvxPostItDateItem&)rSet.Get( nWhich );
        aDateStr = rDate.GetValue();
    }
    else
    {
        LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
        aDateStr = aLocaleWrapper.getDate( Date() );
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
    Date aDate;
    Time aTime;
    String aTmp( SvtUserOptions().GetID() );
    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    String aStr( aEditED.GetText() );
    aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "\n---- " ) );

    if ( aTmp.Len() > 0 )
    {
        aStr += aTmp;
        aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ", " ) );
    }
    aStr += aLocaleWrapper.getDate(aDate);
    aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ", " ) );
    aStr += aLocaleWrapper.getTime(aTime, FALSE, FALSE);
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
    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    pOutSet = new SfxItemSet( rSet );
    pOutSet->Put( SvxPostItAuthorItem( SvtUserOptions().GetID(),
                                         rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_AUTHOR ) ) );
    pOutSet->Put( SvxPostItDateItem( aLocaleWrapper.getDate( Date() ),
                                     rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_DATE ) ) );
    pOutSet->Put( SvxPostItTextItem( aEditED.GetText(),
                                     rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_TEXT ) ) );
    EndDialog( RET_OK );
    return 0;
}

