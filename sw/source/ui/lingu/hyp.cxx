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

#include <boost/scoped_ptr.hpp>

#define PSH         (&pView->GetWrtShell())

using namespace ::com::sun::star;

/*--------------------------------------------------------------------
     Description: interactive separation
 --------------------------------------------------------------------*/

SwHyphWrapper::SwHyphWrapper( SwView* pVw,
            uno::Reference< linguistic2::XHyphenator >  &rxHyph,
            sal_Bool bStart, sal_Bool bOther, sal_Bool bSelect ) :
    SvxSpellWrapper( &pVw->GetEditWin(), rxHyph, bStart, bOther ),
    pView( pVw ),
    xHyph( rxHyph ),
    nPageCount( 0 ),
    nPageStart( 0 ),
    bInSelection( bSelect ),
    bInfoBox( sal_False )
{
    uno::Reference< linguistic2::XLinguProperties >  xProp( GetLinguPropertySet() );
    bAutomatic = xProp.is() ? xProp->getIsHyphAuto() : sal_False;
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



sal_Bool SwHyphWrapper::SpellContinue()
{
    // for automatic separation, make actions visible only at the end
    boost::scoped_ptr<SwWait> pWait;
    if( bAutomatic )
    {
        PSH->StartAllAction();
        pWait.reset(new SwWait( *pView->GetDocShell(), true ));
    }

        uno::Reference< uno::XInterface >  xHyphWord = bInSelection ?
                PSH->HyphContinue( NULL, NULL ) :
                PSH->HyphContinue( &nPageCount, &nPageStart );
        SetLast( xHyphWord );

    // for automatic separation, make actions visible only at the end
    if( bAutomatic )
    {
        PSH->EndAllAction();
        pWait.reset();
    }

    return GetLast().is();
}


void SwHyphWrapper::SpellEnd()
{
    PSH->HyphEnd();
    SvxSpellWrapper::SpellEnd();
}


sal_Bool SwHyphWrapper::SpellMore()
{
    PSH->Push();
    bInfoBox = sal_True;
    PSH->Combine();
    return sal_False;
}



void SwHyphWrapper::InsertHyphen( const sal_uInt16 nPos )
{
    if( nPos)
        PSH->InsertSoftHyph( nPos + 1); // does nPos == 1 really mean
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
        InfoBox( &pView->GetEditWin(), SW_RESSTR(STR_HYP_OK) ).Execute();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
