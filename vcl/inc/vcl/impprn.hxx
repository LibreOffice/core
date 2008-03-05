/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impprn.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:00:07 $
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

#ifndef _SV_IMPPRN_HXX
#define _SV_IMPPRN_HXX

#ifndef _SV_PRINT_HXX
#include <vcl/print.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _VCL_IMPDEL_HXX
#include <vcl/impdel.hxx>
#endif

#include <vector>

struct QueuePage;

// ----------------
// - ImplQPrinter -
// ----------------

/*
    ImplQPrinter is on most systems a simple buffer that allows a potential
    lengthy print job to be printed in the background. For this it saves all
    normal drawing operations for each printed page to a metafile, then spooling
    the metafiles timer based to a normal printer. The application can act in the meantime
    including changing the original document without influencing the print job.

    On some systems (currently Mac/Aqua Cocoa) ImplQPrinter has the additional
    purpose of adapting to the print system: here theprint systems starts a
    job and will not return from that function until it has ended; to do so
    it queries for each consecutive page to be printed. Also the Cocoa print system
    needs to know the number of pages BEFORE starting a print job. Since our Printer
    does not know that, we need to do the completing spooling to ImplQPrinter before
    we can actually print to the real print system. Let's call this the pull model
    instead of the push model (because the systems pulls the pages).
*/

class ImplQPrinter : public Printer, public vcl::DeletionNotifier
{
private:
    Printer*                        mpParent;
    std::vector< QueuePage* >       maQueue;
    AutoTimer                       maTimer;
    bool                            mbAborted;
    bool                            mbUserCopy;
    bool                            mbDestroyAllowed;
    bool                            mbDestroyed;

    GDIMetaFile                     maCurPageMetaFile;
    long                            mnMaxBmpDPIX;
    long                            mnMaxBmpDPIY;
    ULONG                           mnRestoreDrawMode;
    int                             mnCurCopyCount;

                DECL_LINK( ImplPrintHdl, Timer* );

                ~ImplQPrinter();

    void        ImplPrintMtf( GDIMetaFile& rMtf, long nMaxBmpDPIX, long nMaxBmpDPIY );

                ImplQPrinter( const ImplQPrinter& rPrinter );
    Printer&    operator =( const ImplQPrinter& rPrinter );

    void        PrePrintPage( QueuePage* );
    void        PostPrintPage();

public:

                ImplQPrinter( Printer* pParent );
    void        Destroy();

    void        StartQueuePrint();
    void        EndQueuePrint();
    void        AbortQueuePrint();
    void        AddQueuePage( GDIMetaFile* pPage, USHORT nPage, BOOL bNewJobSetup );

    bool        IsUserCopy() const { return mbUserCopy; }
    void        SetUserCopy( bool bSet ) { mbUserCopy = bSet; }

    /**
    used by pull implementation to emit the next page
    */
    using Printer::PrintPage;
    void        PrintPage( unsigned int nPage );
    /**
    used by pull implementation to get the number of physical pages
    (that is how often PrintNextPage should be called)
    */
    ULONG       GetPrintPageCount();
};

#endif  // _SV_IMPPRN_HXX
