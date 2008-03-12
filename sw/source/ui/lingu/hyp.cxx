/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hyp.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:52:00 $
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
#include "precompiled_sw.hxx"



#include "initui.hxx"
#include "view.hxx"
#include "edtwin.hxx"
#include "wrtsh.hxx"
#include "globals.hrc"

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
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

#include <unomid.h>

#define HYPHHERE    '-'
#define PSH         (&pView->GetWrtShell())

using namespace ::com::sun::star;
using ::rtl::OUString;

/*--------------------------------------------------------------------
     Beschreibung: Interaktive Trennung
 --------------------------------------------------------------------*/

SwHyphWrapper::SwHyphWrapper( SwView* pVw,
            uno::Reference< linguistic2::XHyphenator >  &rxHyph,
            sal_Bool bStart, sal_Bool bOther, sal_Bool bSelect ) :
    SvxSpellWrapper( &pVw->GetEditWin(), rxHyph, bStart, bOther ),
    pView( pVw ),
    xHyph( rxHyph ),
    nLangError( 0 ),
    nPageCount( 0 ),
    nPageStart( 0 ),
    bInSelection( bSelect ),
    bShowError( sal_False ),
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
    // Fuer autom. Trennung Aktionen erst am Ende sichtbar machen
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
        pView->SpellError( &nLangError );
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

IMPL_LINK( SwHyphWrapper, SpellError, LanguageType *, pLang )
{
    if (pLang &&  *pLang != nLangError )
    {
        nLangError = *pLang;
        bShowError = sal_True;
    }
    return 0;
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

