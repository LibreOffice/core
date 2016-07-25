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

#include <tools/date.hxx>
#include <tools/time.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <svx/svxids.hrc>

#include <cuires.hrc>
#include <svx/postattr.hxx>
#include "postdlg.hxx"
#include <dialmgr.hxx>

#include "helpid.hrc"

// class SvxPostItDialog -------------------------------------------------

SvxPostItDialog::SvxPostItDialog(vcl::Window* pParent, const SfxItemSet& rCoreSet,
    bool bPrevNext)
    : SfxModalDialog(pParent, "CommentDialog", "cui/ui/comment.ui")
    , rSet(rCoreSet)
    , pOutSet(nullptr)
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

    vcl::Font aFont( m_pEditED->GetFont() );
    aFont.SetWeight( WEIGHT_LIGHT );
    m_pEditED->SetFont( aFont );

    bool bNew = true;
    sal_uInt16 nWhich = 0;

    if ( !bPrevNext )
    {
        m_pPrevBtn->Hide();
        m_pNextBtn->Hide();
    }

    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_AUTHOR );
    OUString aAuthorStr, aDateStr;

    if ( rSet.GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        bNew = false;
        const SvxPostItAuthorItem& rAuthor =
            static_cast<const SvxPostItAuthorItem&>(rSet.Get( nWhich ));
        aAuthorStr = rAuthor.GetValue();
    }
    else
        aAuthorStr = SvtUserOptions().GetID();

    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_DATE );

    if ( rSet.GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        const SvxPostItDateItem& rDate =
            static_cast<const SvxPostItDateItem&>(rSet.Get( nWhich ));
        aDateStr = rDate.GetValue();
    }
    else
    {
        const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
        aDateStr = rLocaleWrapper.getDate( Date( Date::SYSTEM ) );
    }

    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_POSTIT_TEXT );

    OUString aTextStr;
    if ( rSet.GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        const SvxPostItTextItem& rText =
            static_cast<const SvxPostItTextItem&>(rSet.Get( nWhich ));
        aTextStr = rText.GetValue();
    }

    ShowLastAuthor(aAuthorStr, aDateStr);

    //lock to an initial size before replacing contents
    m_pEditED->set_width_request(m_pEditED->approximate_char_width() * 40);
    m_pEditED->set_height_request(m_pEditED->GetTextHeight() * 10);

    m_pEditED->SetText(convertLineEnd(aTextStr, GetSystemLineEnd()));

    if (!bNew)
        SetText( get<FixedText>("alttitle")->GetText() );
}


SvxPostItDialog::~SvxPostItDialog()
{
    disposeOnce();
}

void SvxPostItDialog::dispose()
{
    delete pOutSet;
    pOutSet = nullptr;
    m_pLastEditFT.clear();
    m_pEditED.clear();
    m_pInsertAuthor.clear();
    m_pAuthorBtn.clear();
    m_pOKBtn.clear();
    m_pPrevBtn.clear();
    m_pNextBtn.clear();
    SfxModalDialog::dispose();
}


void SvxPostItDialog::ShowLastAuthor(const OUString& rAuthor, const OUString& rDate)
{
    OUString sTxt = rAuthor  + ", " + rDate;
    m_pLastEditFT->SetText( sTxt );
}


const sal_uInt16* SvxPostItDialog::GetRanges()
{
    static const sal_uInt16 pRanges[] =
    {
        SID_ATTR_POSTIT_AUTHOR,
        SID_ATTR_POSTIT_TEXT,
        0
    };
    return pRanges;
}


void SvxPostItDialog::EnableTravel(bool bNext, bool bPrev)
{
    m_pPrevBtn->Enable(bPrev);
    m_pNextBtn->Enable(bNext);
}


IMPL_LINK_NOARG_TYPED(SvxPostItDialog, PrevHdl, Button*, void)
{
    aPrevHdlLink.Call( *this );
}

IMPL_LINK_NOARG_TYPED(SvxPostItDialog, NextHdl, Button*, void)
{
    aNextHdlLink.Call( *this );
}

IMPL_LINK_NOARG_TYPED(SvxPostItDialog, Stamp, Button*, void)
{
    Date aDate( Date::SYSTEM );
    tools::Time aTime( tools::Time::SYSTEM );
    OUString aTmp( SvtUserOptions().GetID() );
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    OUString aStr( m_pEditED->GetText() );
    aStr += "\n---- ";

    if ( !aTmp.isEmpty() )
    {
        aStr += aTmp + ", ";
    }
    aStr += rLocaleWrapper.getDate(aDate) + ", " + rLocaleWrapper.getTime(aTime, false) + " ----\n";

    aStr = convertLineEnd(aStr, GetSystemLineEnd());

    m_pEditED->SetText(aStr);
    sal_Int32 nLen = aStr.getLength();
    m_pEditED->GrabFocus();
    m_pEditED->SetSelection( Selection( nLen, nLen ) );
}


IMPL_LINK_NOARG_TYPED(SvxPostItDialog, OKHdl, Button*, void)
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
