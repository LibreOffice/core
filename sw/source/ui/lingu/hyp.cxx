/*************************************************************************
 *
 *  $RCSfile: hyp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tl $ $Date: 2000-10-27 12:20:11 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "initui.hxx"
#include "view.hxx"
#include "edtwin.hxx"
#include "wrtsh.hxx"
#include "globals.hrc"

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _LINGU_LNGPROPS_HHX_
#include <lingu/lngprops.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif


#include "hyp.hxx"
#include "mdiexp.hxx"
#include "olmenu.hrc"

#define HYPHHERE    '-'
#define PSH         (&pView->GetWrtShell())

using namespace ::com::sun::star;
using namespace ::rtl;
#define C2U(cChar) OUString::createFromAscii(cChar)
/*--------------------------------------------------------------------
     Beschreibung: Interaktive Trennung
 --------------------------------------------------------------------*/

SwHyphWrapper::SwHyphWrapper( SwView* pVw,
            uno::Reference< linguistic2::XHyphenator >  &rxHyph,
            sal_Bool bStart, sal_Bool bOther, sal_Bool bSelect ) :
    SvxSpellWrapper( &pVw->GetEditWin(), rxHyph, bStart, bOther ),
    xHyph( rxHyph ),
    nLangError( 0 ),
    nPageCount( 0 ),
    nPageStart( 0 ),
    bInSelection( bSelect ),
    bShowError( sal_False ),
    pView( pVw )
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
    // Fuer autom. Trennung Aktionen erst am Ende sichtbar machen
    SwWait *pWait;
    if( bAutomatic )
    {
        PSH->StartAllAction();
        pWait = new SwWait( *pView->GetDocShell(), sal_True );
    }

        uno::Reference< uno::XInterface >  xHyphWord = bInSelection ?
                PSH->HyphContinue( NULL, NULL ) :
                PSH->HyphContinue( &nPageCount, &nPageStart );
        SetLast( xHyphWord );

    // Fuer autom. Trennung Aktionen erst am Ende sichtbar machen
    if( bAutomatic )
    {
        PSH->EndAllAction();
        delete pWait;
    }

    if( bShowError )
    {
        bShowError = sal_False;
        PSH->Push();
        PSH->ClearMark();
        pView->SpellError( (void*)nLangError );
        PSH->Combine();
    }
    return GetLast().is();
}

// -----------------------------------------------------------------------
void SwHyphWrapper::SpellEnd()
{
    PSH->HyphEnd();
    SvxSpellWrapper::SpellEnd();
}

IMPL_LINK( SwHyphWrapper, SpellError, void *, nLang )
{
    sal_uInt32 nNew = (sal_uInt32)nLang;
    if( nNew != nLangError )
    {
        nLangError = nNew;
        bShowError = sal_True;
    }
    return 0;
}


// -----------------------------------------------------------------------
sal_Bool SwHyphWrapper::SpellMore()
{
    PSH->Push();
    InfoBox( &pView->GetEditWin(), SW_RESSTR(STR_HYP_OK) ).Execute();
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
}

