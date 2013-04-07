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

#include <tools/shl.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <svx/svxids.hrc>   // SID_ATTR_...

#define _SVX_POSTDLG_CXX

#include <cuires.hrc>
#include <svx/postattr.hxx>
#include "postdlg.hxx"
#include <dialmgr.hxx>

#include "helpid.hrc"

// class SvxPostItDialog -------------------------------------------------

SvxPostItDialog::SvxPostItDialog(Window* pParent, const SfxItemSet& rCoreSet,
    bool bPrevNext)
    : SfxModalDialog(pParent, "CommentDialog", "cui/ui/comment.ui")
    , rSet(rCoreSet)
    , pOutSet(0)
{
    get(m_pLastEditFT, "lastedit");
    get(m_pInsertAuthor, "insertauthor");
    get(m_pAuthorBtn, "author");
    get(m_pOKBtn, "ok");
    get(m_pPrevBtn, "previous");
    get(m_pNextBtn, "next");
    get(m_pEditED, "edit");

    m_pPrevBtn->SetClickHdl( LINK( this, SvxPostItDialog, PrevHdl ) );
    m_pNextBtn->SetClickHdl( LINK( this, SvxPostItDialog, NextHdl ) );
    m_pAuthorBtn->SetClickHdl( LINK( this, SvxPostItDialog, Stamp ) );
    m_pOKBtn->SetClickHdl( LINK( this, SvxPostItDialog, OKHdl ) );

    Font aFont( m_pEditED->GetFont() );
    aFont.SetWeight( WEIGHT_LIGHT );
    m_pEditED->SetFont( aFont );

    sal_Bool bNew = sal_True;
    sal_uInt16 nWhich = 0;

    if ( !bPrevNext )
    {
        m_pPrevBtn->Hide();
        m_pNextBtn->Hide();
    }

    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_AUTHOR );
    String aAuthorStr, aDateStr;

    if ( rSet.GetItemState( nWhich, sal_True ) >= SFX_ITEM_AVAILABLE )
    {
        bNew = sal_False;
        const SvxPostItAuthorItem& rAuthor =
            (const SvxPostItAuthorItem&)rSet.Get( nWhich );
        aAuthorStr = rAuthor.GetValue();
    }
    else
        aAuthorStr = SvtUserOptions().GetID();

    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_DATE );

    if ( rSet.GetItemState( nWhich, sal_True ) >= SFX_ITEM_AVAILABLE )
    {
        const SvxPostItDateItem& rDate =
            (const SvxPostItDateItem&)rSet.Get( nWhich );
        aDateStr = rDate.GetValue();
    }
    else
    {
        const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
        aDateStr = rLocaleWrapper.getDate( Date( Date::SYSTEM ) );
    }

    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_TEXT );

    OUString aTextStr;
    if ( rSet.GetItemState( nWhich, sal_True ) >= SFX_ITEM_AVAILABLE )
    {
        const SvxPostItTextItem& rText =
            (const SvxPostItTextItem&)rSet.Get( nWhich );
        aTextStr = rText.GetValue();
    }

    ShowLastAuthor(aAuthorStr, aDateStr);

    //lock to initial .ui placeholder size before replacing contents
    Size aSize(m_pEditED->get_preferred_size());
    m_pEditED->set_width_request(aSize.Width());
    m_pEditED->set_height_request(aSize.Height());

    m_pEditED->SetText(convertLineEnd(aTextStr, GetSystemLineEnd()));

    if (!bNew)
        SetText( get<FixedText>("alttitle")->GetText() );
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
    OUString sTxt( rAuthor );
    sTxt += ", ";
    sTxt += rDate;
    m_pLastEditFT->SetText( sTxt );
}

// -----------------------------------------------------------------------

sal_uInt16* SvxPostItDialog::GetRanges()
{
    static sal_uInt16 pRanges[] =
    {
        SID_ATTR_POSTIT_AUTHOR,
        SID_ATTR_POSTIT_TEXT,
        0
    };
    return pRanges;
}

// -----------------------------------------------------------------------

void SvxPostItDialog::EnableTravel(sal_Bool bNext, sal_Bool bPrev)
{
    m_pPrevBtn->Enable(bPrev);
    m_pNextBtn->Enable(bNext);
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG_INLINE_START(SvxPostItDialog, PrevHdl)
{
    aPrevHdlLink.Call( this );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvxPostItDialog, PrevHdl)

// -----------------------------------------------------------------------

IMPL_LINK_NOARG_INLINE_START(SvxPostItDialog, NextHdl)
{
    aNextHdlLink.Call( this );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvxPostItDialog, NextHdl)

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxPostItDialog, Stamp)
{
    Date aDate( Date::SYSTEM );
    Time aTime( Time::SYSTEM );
    String aTmp( SvtUserOptions().GetID() );
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    OUString aStr( m_pEditED->GetText() );
    aStr += "\n---- ";

    if ( aTmp.Len() > 0 )
    {
        aStr += aTmp;
        aStr += ", ";
    }
    aStr += rLocaleWrapper.getDate(aDate);
    aStr += ", ";
    aStr += rLocaleWrapper.getTime(aTime, sal_False, sal_False);
    aStr += " ----\n";

    aStr = convertLineEnd(aStr, GetSystemLineEnd());

    m_pEditED->SetText(aStr);
    xub_StrLen nLen = aStr.getLength();
    m_pEditED->GrabFocus();
    m_pEditED->SetSelection( Selection( nLen, nLen ) );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxPostItDialog, OKHdl)
{
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    pOutSet = new SfxItemSet( rSet );
    pOutSet->Put( SvxPostItAuthorItem( SvtUserOptions().GetID(),
                                         rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_AUTHOR ) ) );
    pOutSet->Put( SvxPostItDateItem( rLocaleWrapper.getDate( Date( Date::SYSTEM ) ),
                                     rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_DATE ) ) );
    pOutSet->Put( SvxPostItTextItem( m_pEditED->GetText(),
                                     rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_TEXT ) ) );
    EndDialog( RET_OK );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
