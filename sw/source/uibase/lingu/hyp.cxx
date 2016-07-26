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

#include "initui.hxx"
#include "view.hxx"
#include "edtwin.hxx"
#include "wrtsh.hxx"
#include "globals.hrc"
#include <vcl/msgbox.hxx>
#include <vcl/wrkwin.hxx>
#include <linguistic/lngprops.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <swwait.hxx>

#include "hyp.hxx"
#include "mdiexp.hxx"
#include "olmenu.hrc"

#include <unomid.h>

#include <memory>

#define PSH         (&pView->GetWrtShell())

using namespace ::com::sun::star;

// interactive separation
SwHyphWrapper::SwHyphWrapper( SwView* pVw,
            uno::Reference< linguistic2::XHyphenator >  &rxHyph,
            bool bStart, bool bOther, bool bSelect ) :
    SvxSpellWrapper( &pVw->GetEditWin(), rxHyph, bStart, bOther ),
    pView( pVw ),
    nPageCount( 0 ),
    nPageStart( 0 ),
    bInSelection( bSelect ),
    bInfoBox( false )
{
    uno::Reference< linguistic2::XLinguProperties >  xProp( GetLinguPropertySet() );
    bAutomatic = xProp.is() && xProp->getIsHyphAuto();
    SetHyphen();
}

void SwHyphWrapper::SpellStart( SvxSpellArea eSpell )
{
    if( SVX_SPELL_OTHER == eSpell && nPageCount )
    {
        ::EndProgress( pView->GetDocShell() );
        nPageCount = 0;
        nPageStart = 0;
    }
    pView->HyphStart( eSpell );
}

void SwHyphWrapper::SpellContinue()
{
    // for automatic separation, make actions visible only at the end
    std::unique_ptr<SwWait> pWait;
    if( bAutomatic )
    {
        PSH->StartAllAction();
        pWait.reset(new SwWait( *pView->GetDocShell(), true ));
    }

        uno::Reference< uno::XInterface >  xHyphWord = bInSelection ?
                PSH->HyphContinue( nullptr, nullptr ) :
                PSH->HyphContinue( &nPageCount, &nPageStart );
        SetLast( xHyphWord );

    // for automatic separation, make actions visible only at the end
    if( bAutomatic )
    {
        PSH->EndAllAction();
        pWait.reset();
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
    bInfoBox = true;
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
    if( nPageCount )
        ::EndProgress( pView->GetDocShell() );
    if( bInfoBox )
        ScopedVclPtrInstance<InfoBox>(&pView->GetEditWin(), SW_RESSTR(STR_HYP_OK))->Execute();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
