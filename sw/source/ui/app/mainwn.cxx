/*************************************************************************
 *
 *  $RCSfile: mainwn.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:31 $
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


#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SVARRAY_HXX
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX //autogen
#include <sfx2/progress.hxx>
#endif
#ifndef _RESID_HXX //autogen
#include <vcl/resid.hxx>
#endif

#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _SWMODULE_HXX //autogen
#include <swmodule.hxx>
#endif
#include "swtypes.hxx"

class SwDocShell;

struct SwProgress
{
    long nStartValue,
         nStartCount;
    SwDocShell  *pDocShell;
    SfxProgress *pProgress;
};

static SvPtrarr *pProgressContainer = 0;

SwProgress *lcl_SwFindProgress( SwDocShell *pDocShell )
{
    for ( USHORT i = 0; i < pProgressContainer->Count(); ++i )
    {
        SwProgress *pTmp = (SwProgress*)(*pProgressContainer)[i];
        if ( pTmp->pDocShell == pDocShell )
            return pTmp;
    }
    return 0;
}


void StartProgress( USHORT nMessResId, long nStartValue, long nEndValue,
                    SwDocShell *pDocShell )
{
    if( !SW_MOD()->IsEmbeddedLoadSave() )
    {
        SwProgress *pProgress = 0;

        if ( !pProgressContainer )
            pProgressContainer = new SvPtrarr( 2, 2 );
        else
        {
            if ( 0 != (pProgress = lcl_SwFindProgress( pDocShell )) )
                ++pProgress->nStartCount;
        }
        if ( !pProgress )
        {
            pProgress = new SwProgress;
            pProgress->pProgress = new SfxProgress( pDocShell,
                                                    SW_RESSTR(nMessResId),
                                                    nEndValue - nStartValue,
                                                    FALSE,
                                                    TRUE );
            pProgress->nStartCount = 1;
            pProgress->pDocShell = pDocShell;
            pProgressContainer->Insert( (void*)pProgress, 0 );
        }
        pProgress->nStartValue = nStartValue;
    }
}


void SetProgressState( long nPosition, SwDocShell *pDocShell )
{
    if( pProgressContainer && !SW_MOD()->IsEmbeddedLoadSave() )
    {
        SwProgress *pProgress = lcl_SwFindProgress( pDocShell );
        if ( pProgress )
            pProgress->pProgress->SetState(nPosition - pProgress->nStartValue);
    }
}


void EndProgress( SwDocShell *pDocShell )
{
    if( pProgressContainer && !SW_MOD()->IsEmbeddedLoadSave() )
    {
        SwProgress *pProgress = 0;
        USHORT i;
        for ( i = 0; i < pProgressContainer->Count(); ++i )
        {
            SwProgress *pTmp = (SwProgress*)(*pProgressContainer)[i];
            if ( pTmp->pDocShell == pDocShell )
            {
                pProgress = pTmp;
                break;
            }
        }

        if ( pProgress && 0 == --pProgress->nStartCount )
        {
            pProgress->pProgress->Stop();
            pProgressContainer->Remove( i );
            delete pProgress->pProgress;
            delete pProgress;
            if ( !pProgressContainer->Count() )
                delete pProgressContainer, pProgressContainer = 0;
        }
    }
}


void SetProgressText( USHORT nId, SwDocShell *pDocShell )
{
    if( pProgressContainer && !SW_MOD()->IsEmbeddedLoadSave() )
    {
        SwProgress *pProgress = lcl_SwFindProgress( pDocShell );
        if ( pProgress )
            pProgress->pProgress->SetStateText( 0, SW_RESSTR(nId) );
    }
}


void RescheduleProgress( SwDocShell *pDocShell )
{
    if( pProgressContainer && !SW_MOD()->IsEmbeddedLoadSave() )
    {
        SwProgress *pProgress = lcl_SwFindProgress( pDocShell );
        if ( pProgress )
            pProgress->pProgress->Reschedule();
    }
}

/************************************************************************
    $Log: not supported by cvs2svn $
    Revision 1.44  2000/09/18 16:05:11  willem.vandorp
    OpenOffice header added.

    Revision 1.43  2000/02/11 14:42:46  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.42  2000/01/17 14:06:30  jp
    Bug #71812#: SetProgress call operator==() and not operator=()

    Revision 1.41  2000/01/05 18:21:57  jp
    Bug #71524#: EndProgress call operator==() and not operator=()

    Revision 1.40  1999/06/29 07:43:54  MA
    enable wait cursor


      Rev 1.39   29 Jun 1999 09:43:54   MA
   enable wait cursor

      Rev 1.38   28 Jun 1999 14:15:00   MA
   Blocking of Progress limited to one Document

      Rev 1.37   29 Jun 1998 17:16:14   MA
   #46809# Nachdem der SFX seinen Service noch immer nicht liefert der
   Hack jetzt auch in der aktuellen Version.

      Rev 1.36   28 Nov 1997 11:02:30   TJ
   include

      Rev 1.35   24 Nov 1997 14:22:44   MA
   includes

      Rev 1.34   09 Sep 1997 11:35:58   OS
   SwMainWin ueberfluessig

      Rev 1.33   01 Sep 1997 13:08:08   OS
   DLL-Umstellung

      Rev 1.32   08 Aug 1997 17:26:58   OM
   Headerfile-Umstellung

      Rev 1.31   06 Aug 1997 11:02:24   TRI
   GetpApp statt pApp

      Rev 1.30   13 Feb 1997 10:27:38   TRI
   includes

      Rev 1.29   10 Jan 1997 18:37:58   MA
   fix: Progress nicht zerstoren fuer Embedded

      Rev 1.28   14 Nov 1996 15:15:26   TRI
   includes

      Rev 1.27   08 Nov 1996 19:39:32   MA
   ResMgr

      Rev 1.26   09 Oct 1996 17:55:28   MA
   Progress

      Rev 1.25   29 Jul 1996 19:37:40   MA
   includes

      Rev 1.24   29 Feb 1996 12:55:52   OS
   im Close wird jetzt auch der ChartAutoPilot geschlossen

      Rev 1.23   14 Dec 1995 19:55:54   MIB
   neue Funktion Reschedule

      Rev 1.22   24 Nov 1995 16:56:50   OM
   PCH->PRECOMPILED

      Rev 1.21   22 Aug 1995 20:22:14   JP
   ueberfluessige includes entfernt

      Rev 1.20   15 Aug 1995 19:20:54   JP
   neu: IsEmbeddedFlag an SwWriterApp - keinen Progressbar anzeigen

      Rev 1.19   14 Aug 1995 18:26:44   MD
   SfxProgress-ctor erhaelt keinen ViewFrame mehr

      Rev 1.18   09 Aug 1995 21:41:34   ER
   static vor seg_eofglobals

      Rev 1.17   01 Aug 1995 19:35:22   OS
   StartWert fuer Progress auf 0 normalisieren

      Rev 1.16   20 Jul 1995 12:01:02   OS
   RechenleistenChildWindow im Close abschalten

      Rev 1.15   19 Jul 1995 17:31:12   OS
   Rechenleiste nicht mehr am Doc

      Rev 1.14   11 Jul 1995 15:43:58   OS
   Close ueberladen, um rechenleiste zu schliessen

      Rev 1.13   09 May 1995 11:29:44   OS
   -Disable/EnableCmdInterface

      Rev 1.12   08 May 1995 11:59:44   JP
   StatLineSetPercent: auf 0 - Pointer abfragen

      Rev 1.11   21 Apr 1995 19:04:10   JP
   BugId 12042: doppeltes Loeschen vom Pointer verhindern!

      Rev 1.10   11 Apr 1995 12:44:00   OS
   noch ein paar Kleinigkeiten

      Rev 1.9   07 Apr 1995 18:10:12   OS
   Rest auskommentiert

      Rev 1.8   07 Apr 1995 14:30:22   OS
   +SfxProgress

      Rev 1.7   26 Feb 1995 18:49:44   MA
   Laufbalken reanimiert.

      Rev 1.6   23 Feb 1995 22:18:00   ER
   sexport

      Rev 1.5   09 Jan 1995 16:06:34   ER
    del: globals hrc, app hrc

      Rev 1.4   25 Oct 1994 14:38:28   ER
   add: PCH

      Rev 1.3   20 Sep 1994 11:34:06   VB
   GetStarWriter inline, Flags nach swapp

      Rev 1.2   19 Sep 1994 15:25:00   MS
   Fix fuers Update
************************************************************************/


