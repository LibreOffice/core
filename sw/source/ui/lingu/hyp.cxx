/*************************************************************************
 *
 *  $RCSfile: hyp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:44 $
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
            uno::Reference< linguistic::XHyphenator >  &rxHyph,
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
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.95  2000/09/18 16:05:54  willem.vandorp
    OpenOffice header added.

    Revision 1.94  2000/07/04 15:20:20  tl
    XHyphenator1 => XHyphenator

    Revision 1.93  2000/04/18 15:35:07  os
    UNICODE

    Revision 1.92  2000/03/23 07:42:45  os
    UNO III

    Revision 1.91  2000/03/08 17:23:12  os
    GetAppWindow() - misuse as parent window eliminated

    Revision 1.90  2000/02/24 12:14:03  tl
    #71915# SvxSpellWrapper::SpellEnd called in SpellEnd now

    Revision 1.89  2000/02/16 20:58:34  tl
    #72219# Locale Umstellung

    Revision 1.88  2000/02/11 14:55:51  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.87  1999/11/24 18:28:37  tl
    check for Service availability

    Revision 1.86  1999/11/19 16:40:23  os
    modules renamed

    Revision 1.85  1999/11/10 11:01:24  tl
     XInterface SpellContinue() -> sal_Bool SpellContinue()
    Ongoing ONE_LINGU implementation

    Revision 1.84  1999/10/25 19:13:24  tl
    ongoing ONE_LINGU implementation

    Revision 1.83  1999/08/31 08:37:30  TL
    #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)


      Rev 1.82   31 Aug 1999 10:37:30   TL
   #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)

      Rev 1.81   08 Jul 1999 16:16:14   MA
   Use internal object to toggle wait cursor

      Rev 1.80   08 Jul 1999 10:31:44   MA
   limit wait cursor to document window

      Rev 1.79   12 Aug 1998 18:16:00   HR
   #54781#: GCC braucht Temporary

      Rev 1.78   03 Dec 1997 17:10:10   AMA
   Fix: Paintprobleme durch Actionklammerung bei automatischer Trennung

      Rev 1.77   28 Nov 1997 15:30:32   MA
   includes

      Rev 1.76   28 Nov 1997 13:35:34   AMA
   Fix #45841#: Auch Fehlermeldungen duerfen verschoben werden.

      Rev 1.75   03 Nov 1997 13:21:46   MA
   precomp entfernt

      Rev 1.74   24 Oct 1997 10:30:54   AMA
   Fix #44994#: InfoBox nach Abschluss der Silbentrennung anzeigen

      Rev 1.73   05 Sep 1997 11:46:26   MH
   chg: header

      Rev 1.72   14 Feb 1997 15:19:30   AMA
   Fix #36204#: Falsche Parentbeziehung des Dialogs.

      Rev 1.71   08 Jan 1997 12:55:14   AMA
   New: Rueckwaerts kann unterbunden werden, wird genutzt von der Silbentrennung

      Rev 1.70   04 Dec 1996 23:04:40   MH
   chg: header

      Rev 1.69   09 Oct 1996 18:00:26   MA
   Progress

      Rev 1.68   28 Aug 1996 12:52:34   OS
   includes

      Rev 1.67   14 Mar 1996 11:37:24   AMA
   New: Progress-Anzeige bei Rechtschr. und Silbentrennung

      Rev 1.66   24 Nov 1995 16:57:40   OM
   PCH->PRECOMPILED

      Rev 1.65   31 Oct 1995 18:39:42   OM
   GetFrameWindow entfernt

      Rev 1.64   09 Aug 1995 17:14:04   AMA
   SEG_GLOBALS wieder da

      Rev 1.63   06 Aug 1995 14:36:54   AMA
   Benutzt den SvxSpellWrapper als HyphWrapper

      Rev 1.62   25 Feb 1995 13:33:08   MS
   IBM-Linguistik

      Rev 1.61   18 Nov 1994 16:10:40   MA
   min -> Min, max -> Max

      Rev 1.60   09 Nov 1994 07:59:42   MS
   Includes geaendert

      Rev 1.59   08 Nov 1994 16:47:46   MS
   pSpell ist global

      Rev 1.58   25 Oct 1994 18:21:54   ER
   add: PCH

      Rev 1.57   07 Oct 1994 14:30:30   MS
   Anpassung Sfx

      Rev 1.56   04 Aug 1994 09:59:56   SWG
   swg32: SED awt::Size to SSize, LSize to awt::Size etc.

      Rev 1.55   02 Aug 1994 16:00:18   MS
   Umstellung Linguistik

      Rev 1.54   22 Jul 1994 09:49:36   MS
   Bug 8784 richtuge Anzeige des zu trennenden Wortes

      Rev 1.53   21 Jun 1994 13:26:44   MS
   Bug 8526 Abrrechen des Trennvorganges und Defbutton auf dem MAC

      Rev 1.52   07 Apr 1994 16:58:08   MS
   neues Pragma

      Rev 1.51   07 Apr 1994 16:26:36   MS
   neu LangError

      Rev 1.50   26 Feb 1994 01:58:24   ER
   virt. dtor mit export

      Rev 1.49   26 Feb 1994 01:10:18   ER
   virtuellen dtor wegen export ueberladen

      Rev 1.48   25 Feb 1994 20:44:40   ER
   virtuellen dtor wegen export ueberladen

      Rev 1.47   23 Feb 1994 10:58:24   ER
   SEXPORT

      Rev 1.46   22 Feb 1994 14:40:04   ER
   EXPORT --> SEXPORT

      Rev 1.45   21 Feb 1994 21:35:46   ER
   LINK EXPORT

      Rev 1.44   17 Feb 1994 08:47:02   MI
   SEG_FUNCDEFS ausgefuellt

      Rev 1.43   16 Feb 1994 20:07:34   MI
   SEG_FUNCDEFS ausgefuellt

      Rev 1.42   16 Feb 1994 16:50:56   MI
   Pragmas zurechtgerueckt

      Rev 1.41   01 Feb 1994 18:18:58   MI
   TCOV() entfernt, SW_... nach SEG_... umbenannt

      Rev 1.40   01 Feb 1994 13:44:18   MS
   Umstellung Linguistik v2.0

      Rev 1.39   21 Jan 1994 09:53:10   MS
   Ablauf interaktive Trennung geaendert

      Rev 1.38   20 Jan 1994 18:41:18   MS
   Ablauf geaendert

      Rev 1.37   13 Jan 1994 08:44:48   MI
   Segmentierung per #define ermoeglicht

      Rev 1.36   09 Jan 1994 21:12:54   MI
   Provisorische Segmentierung

      Rev 1.35   06 Dec 1993 13:09:58   MS
   Anpassung der Selektion

      Rev 1.34   06 Dec 1993 10:31:56   MS
   Trennstellen selektieren

      Rev 1.33   27 Sep 1993 01:23:40   MI
   Segmentierung

      Rev 1.32   20 Sep 1993 14:58:52   MS
   Umstellung Lingu

      Rev 1.31   26 Aug 1993 09:46:08   MS
   Nach Weiter am Dokumentanfang Controlls initialisieren

      Rev 1.30   08 Aug 1993 11:56:24   MS
   fuer MSC

      Rev 1.29   08 Aug 1993 09:31:42   MS
   Ablauf interaktive Silbentrennung geaendert

      Rev 1.28   06 Aug 1993 08:13:38   ER
   String Array-Indexe mit sal_uInt16 statt short

      Rev 1.28   06 Aug 1993 08:11:36   ER
   String Array-Indexe mit sal_uInt16 statt short

      Rev 1.27   05 Aug 1993 10:32:30   MS
   Drunken Hyphenator beseitigt

      Rev 1.26   18 Jun 1993 09:21:28   BP
   bug: InitControls(), Initialisierung von nHyphPos

      Rev 1.25   28 May 1993 11:51:42   VB
   Umstellung Precompiled Headers

      Rev 1.24   14 May 1993 11:31:36   MS
   Traveling innerhalb des zu trennenden Wortes korrigiert

      Rev 1.23   27 Apr 1993 08:39:08   OK
   NEU: #pragma hdrstop

      Rev 1.22   20 Apr 1993 13:13:38   MS
   Entflechtung Linguistik

      Rev 1.21   23 Mar 1993 18:18:20   MS

      Rev 1.20   23 Mar 1993 18:10:30   MS
   richtigen Trennindex bestimmen

      Rev 1.19   22 Mar 1993 10:08:26   OK
   Anpassung fuer NT

      Rev 1.18   16 Mar 1993 08:32:42   MS
   Fix fuer kk entfernt

      Rev 1.17   12 Mar 1993 14:49:06   MS
   Uebergeben des SoftHyph geaendert

      Rev 1.16   12 Mar 1993 14:02:34   MS
   Navigation geaendert

      Rev 1.15   11 Mar 1993 14:00:04   MS
   keine alternativwoerter fuer Formatierung

      Rev 1.14   11 Mar 1993 13:54:54   MS
   Ablauf geaendert

      Rev 1.13   09 Mar 1993 10:48:24   MS
   Trennung mit Start und Continue - Mimik

      Rev 1.12   10 Feb 1993 19:59:46   TB
   Umstellung SV21

      Rev 1.11   22 Jan 1993 14:30:32   MS
   String im Header geaendert

      Rev 1.10   05 Jan 1993 10:01:56   MS
   neu mit Sprachanzeige

      Rev 1.9   22 Dec 1992 20:19:00   VB
   Aenderung der FN_-Ids

      Rev 1.8   22 Dec 1992 10:12:26   OK
   Anpassung an M70

      Rev 1.7   21 Dec 1992 16:19:34   MS
   Enable Disable Buttons

      Rev 1.6   18 Dec 1992 11:54:24   MS
   mit SL_SOFT_HYPHEN

      Rev 1.5   11 Dec 1992 13:42:48   MS
   IgnoreButton

      Rev 1.4   17 Nov 1992 16:55:44   MS
   Trennhilfe-Dialog geaendert

------------------------------------------------------------------------*/

