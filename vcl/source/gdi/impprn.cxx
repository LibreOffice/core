/*************************************************************************
 *
 *  $RCSfile: impprn.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:38 $
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

#ifndef REMOTE_APPSERVER

#define _SV_IMPPRN_CXX
#define _SPOOLPRINTER_EXT

#ifndef _QUEUE_HXX
#include <tools/queue.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif
#ifndef _SV_IMPPRN_HXX
#include <impprn.hxx>
#endif

// =======================================================================

struct QueuePage
{
    GDIMetaFile*    mpMtf;
    JobSetup*       mpSetup;
    USHORT          mnPage;
    BOOL            mbEndJob;

                    QueuePage() { mpMtf = NULL; mpSetup = NULL; }
                    ~QueuePage() { delete mpMtf; if ( mpSetup ) delete mpSetup; }
};

// =======================================================================

ImplQPrinter::ImplQPrinter( Printer* pParent ) :
    Printer( pParent->GetName() )
{
    SetSelfAsQueuePrinter( TRUE );
    SetPrinterProps( pParent );
    SetPageQueueSize( 0 );
    mpParent        = pParent;
    mnCopyCount     = pParent->mnCopyCount;
    mbCollateCopy   = pParent->mbCollateCopy;
    mpQueue         = new Queue( mpParent->GetPageQueueSize() );
    mbAborted       = FALSE;
    mbUserCopy      = FALSE;
    mbDestroyAllowed= TRUE;
    mbDestroyed     = FALSE;
}

// -----------------------------------------------------------------------

ImplQPrinter::~ImplQPrinter()
{
    QueuePage* pQueuePage = (QueuePage*)mpQueue->Get();
    while ( pQueuePage )
    {
        delete pQueuePage;
        pQueuePage = (QueuePage*)mpQueue->Get();
    }

    delete mpQueue;
}

// -----------------------------------------------------------------------------

void ImplQPrinter::Destroy()
{
    if( mbDestroyAllowed )
        delete this;
    else
        mbDestroyed = TRUE;
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplQPrinter, ImplPrintHdl, Timer*, EMPTYARG )
{
    // Ist Drucken abgebrochen wurden?
    if ( !IsPrinting() )
        return 0;

    // Nur drucken, wenn genuegend Seiten im Cache stehen
    if ( mpParent->IsJobActive() && (mpQueue->Count() < (ULONG)mpParent->GetPageQueueSize()) )
        return 0;

    // Druck-Job zuende?
    QueuePage* pActPage = (QueuePage*) mpQueue->Get();

    if ( pActPage->mbEndJob )
    {
        maTimer.Stop();
        delete pActPage;
        EndJob();
        mpParent->ImplEndPrint();
    }
    else
    {
        USHORT          nCopyCount = 1;
        GDIMetaFile     aMtf;

        mbDestroyAllowed = FALSE;
        GetPreparedMetaFile( *pActPage->mpMtf, aMtf );

        if ( mbUserCopy && !mbCollateCopy )
            nCopyCount = mnCopyCount;

        for ( USHORT i = 0; i < nCopyCount; i++ )
        {
            ULONG nActionPos = 0UL;

            if ( pActPage->mpSetup )
            {
                SetJobSetup( *pActPage->mpSetup );
                if ( mbAborted )
                     break;
            }

            StartPage();

            if ( mbAborted )
                break;

            for( MetaAction* pAct = aMtf.FirstAction(); pAct; pAct = aMtf.NextAction() )
            {
                pAct->Execute( this );
                Application::Reschedule();

                if( mbAborted )
                    break;
            }

            if( !mbAborted )
                EndPage();
            else
                break;
        }

        delete pActPage;
        mbDestroyAllowed = TRUE;

        if( mbDestroyed )
            Destroy();
    }

    return 0;
}

// -----------------------------------------------------------------------

void ImplQPrinter::StartQueuePrint()
{
    maTimer.SetTimeout( 50 );
    maTimer.SetTimeoutHdl( LINK( this, ImplQPrinter, ImplPrintHdl ) );
    maTimer.Start();
}

// -----------------------------------------------------------------------

void ImplQPrinter::EndQueuePrint()
{
    QueuePage* pQueuePage   = new QueuePage;
    pQueuePage->mbEndJob    = TRUE;
    mpQueue->Put( pQueuePage );
}

// -----------------------------------------------------------------------

void ImplQPrinter::AbortQueuePrint()
{
    maTimer.Stop();
    mbAborted = TRUE;
    AbortJob();
}

// -----------------------------------------------------------------------

void ImplQPrinter::AddQueuePage( GDIMetaFile* pPage, USHORT nPage, BOOL bNewJobSetup )
{
    QueuePage* pQueuePage   = new QueuePage;
    pQueuePage->mpMtf       = pPage;
    pQueuePage->mnPage      = nPage;
    pQueuePage->mbEndJob    = FALSE;
    if ( bNewJobSetup )
        pQueuePage->mpSetup = new JobSetup( mpParent->GetJobSetup() );
    mpQueue->Put( pQueuePage );
}

#endif
