/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impprn.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 10:00:31 $
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

class Queue;
struct QueuePage;

// ----------------
// - ImplQPrinter -
// ----------------

class ImplQPrinter : public Printer, public vcl::DeletionNotifier
{
private:
    Printer*    mpParent;
    Queue*      mpQueue;
    AutoTimer   maTimer;
    BOOL        mbAborted;
    BOOL        mbUserCopy;
    BOOL        mbDestroyAllowed;
    BOOL        mbDestroyed;

                DECL_LINK( ImplPrintHdl, Timer* );

                ~ImplQPrinter();

    void        ImplPrintMtf( GDIMetaFile& rMtf, long nMaxBmpDPIX, long nMaxBmpDPIY );

                ImplQPrinter( const ImplQPrinter& rPrinter );
    Printer&    operator =( const ImplQPrinter& rPrinter );

public:

                ImplQPrinter( Printer* pParent );
    void        Destroy();

    void        StartQueuePrint();
    void        EndQueuePrint();
    void        AbortQueuePrint();
    void        AddQueuePage( GDIMetaFile* pPage, USHORT nPage, BOOL bNewJobSetup );

    BOOL        IsUserCopy() const { return mbUserCopy; }
    void        SetUserCopy( BOOL bSet ) { mbUserCopy = bSet; }
};

#endif  // _SV_IMPPRN_HXX
