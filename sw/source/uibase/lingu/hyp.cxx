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

#include <view.hxx>
#include <edtwin.hxx>
#include <wrtsh.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <swwait.hxx>

#include <hyp.hxx>
#include <mdiexp.hxx>
#include <strings.hrc>

#include <memory>

#define PSH         (&m_rView.GetWrtShell())

using namespace ::com::sun::star;

// interactive separation
SwHyphWrapper::SwHyphWrapper( SwView& rView,
            uno::Reference< linguistic2::XHyphenator > const &rxHyph,
            bool bStart, bool bOther, bool bSelect ) :
    SvxSpellWrapper( rView.GetEditWin().GetFrameWeld(), rxHyph, bStart, bOther ),
    m_rView( rView ),
    m_nPageCount( 0 ),
    m_nPageStart( 0 ),
    m_bInSelection( bSelect ),
    m_bInfoBox( false )
{
    uno::Reference< linguistic2::XLinguProperties >  xProp( GetLinguPropertySet() );
    m_bAutomatic = xProp.is() && xProp->getIsHyphAuto();
}

void SwHyphWrapper::SpellStart( SvxSpellArea eSpell )
{
    if( SvxSpellArea::Other == eSpell && m_nPageCount )
    {
        ::EndProgress( m_rView.GetDocShell() );
        m_nPageCount = 0;
        m_nPageStart = 0;
    }
    m_rView.HyphStart( eSpell );
}

void SwHyphWrapper::SpellContinue()
{
    // for automatic separation, make actions visible only at the end
    std::optional<SwWait> oWait;
    if( m_bAutomatic )
    {
        PSH->StartAllAction();
        oWait.emplace( *m_rView.GetDocShell(), true );
    }

    uno::Reference< uno::XInterface >  xHyphWord = m_bInSelection ?
                PSH->HyphContinue( nullptr, nullptr ) :
                PSH->HyphContinue( &m_nPageCount, &m_nPageStart );
    SetLast( xHyphWord );

    // for automatic separation, make actions visible only at the end
    if( m_bAutomatic )
    {
        PSH->EndAllAction();
        oWait.reset();
    }
}

void SwHyphWrapper::SpellEnd()
{
    PSH->HyphEnd();
    SvxSpellWrapper::SpellEnd();
}

bool SwHyphWrapper::SpellMore()
{
    PSH->Push();
    m_bInfoBox = true;
    PSH->Combine();
    return false;
}

void SwHyphWrapper::InsertHyphen( const sal_Int32 nPos )
{
    if( nPos)
        SwEditShell::InsertSoftHyph(nPos + 1); // does nPos == 1 really mean
                                        // insert hyphen after first char?
                                        // (instead of nPos == 0)
    else
        PSH->HyphIgnore();
}

SwHyphWrapper::~SwHyphWrapper()
{
    if( m_nPageCount )
        ::EndProgress( m_rView.GetDocShell() );
    if( m_bInfoBox && !Application::IsHeadlessModeEnabled() )
    {
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_rView.GetEditWin().GetFrameWeld(),
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      SwResId(STR_HYP_OK)));
        xInfoBox->run();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
