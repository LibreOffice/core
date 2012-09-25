/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "initui.hxx"
#include "view.hxx"
#include "edtwin.hxx"
#include "wrtsh.hxx"
#include "globals.hrc"
#include <vcl/msgbox.hxx>
#include <vcl/wrkwin.hxx>
#include <linguistic/lngprops.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <swwait.hxx>

#include "hyp.hxx"
#include "mdiexp.hxx"
#include "olmenu.hrc"

#include <unomid.h>

#define PSH         (&pView->GetWrtShell())

using namespace ::com::sun::star;
using ::rtl::OUString;

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
    uno::Reference< beans::XPropertySet >  xProp( GetLinguPropertySet() );
    bAutomatic = xProp.is() ?
            *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_HYPH_AUTO) ).getValue() : sal_False;
    SetHyphen();
}

// -----------------------------------------------------------------------
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

// -----------------------------------------------------------------------

sal_Bool SwHyphWrapper::SpellContinue()
{
    // for automatic separation, make actions visible only at the end
    SwWait *pWait = 0;
    if( bAutomatic )
    {
        PSH->StartAllAction();
        pWait = new SwWait( *pView->GetDocShell(), sal_True );
    }

        uno::Reference< uno::XInterface >  xHyphWord = bInSelection ?
                PSH->HyphContinue( NULL, NULL ) :
                PSH->HyphContinue( &nPageCount, &nPageStart );
        SetLast( xHyphWord );

    // for automatic separation, make actions visible only at the end
    if( bAutomatic )
    {
        PSH->EndAllAction();
        delete pWait;
    }

    return GetLast().is();
}

// -----------------------------------------------------------------------
void SwHyphWrapper::SpellEnd()
{
    PSH->HyphEnd();
    SvxSpellWrapper::SpellEnd();
}

// -----------------------------------------------------------------------
sal_Bool SwHyphWrapper::SpellMore()
{
    PSH->Push();
    bInfoBox = sal_True;
    PSH->Combine();
    return sal_False;
}

// -----------------------------------------------------------------------

void SwHyphWrapper::InsertHyphen( const sal_uInt16 nPos )
{
    if( nPos)
        PSH->InsertSoftHyph( nPos + 1); // does nPos == 1 really mean
                                        // insert hyphen after first char?
                                        // (instead of nPos == 0)
    else
        PSH->HyphIgnore();
}

// -----------------------------------------------------------------------
SwHyphWrapper::~SwHyphWrapper()
{
    if( nPageCount )
        ::EndProgress( pView->GetDocShell() );
    if( bInfoBox )
        InfoBox( &pView->GetEditWin(), SW_RESSTR(STR_HYP_OK) ).Execute();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
