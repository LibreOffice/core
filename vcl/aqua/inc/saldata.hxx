
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: saldata.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 16:30:39 $
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

#ifndef _SV_SALDATA_HXX
#define _SV_SALDATA_HXX

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>


#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _SV_SVDATA_HXX
#include <vcl/svdata.hxx>
#endif

#ifndef _SV_SALWTYPE_HXX
#include <vcl/salwtype.hxx>
#endif

#include <list>
#include <hash_set>

#include <cstdio>
#include <cstdarg>


class SalInstance;
class SalObject;
class SalFrame;
class SalVirtualDevice;
class SalPrinter;
class SystemFontList;

// ------------------
// - Some constants -
// ------------------

#define SAL_CLIPRECT_COUNT 16

// -----------
// - SalData -
// -----------

class AquaSalFrame;
struct FrameHash : public std::hash<sal_IntPtr>
{
    size_t operator()(const AquaSalFrame* frame) const
    { return std::hash<sal_IntPtr>::operator()( reinterpret_cast<const sal_IntPtr>(frame) ); }
};

struct SalData
{

    SALTIMERPROC                                  mpTimerProc;      // timer callback proc
    SalInstance                                  *mpFirstInstance;  // pointer of first instance
    std::list<AquaSalFrame*>                      maFrames;         // pointer of first frame
    std::hash_set<const AquaSalFrame*,FrameHash>  maFrameCheck;     // for fast check of frame existance
    SalObject                                    *mpFirstObject;    // pointer of first object window
    SalVirtualDevice                             *mpFirstVD;        // first VirDev
    SalPrinter                                   *mpFirstPrinter;   // first printing printer
    SystemFontList                               *mpFontList;

    CGColorSpaceRef                               mxRGBSpace;
    CGColorSpaceRef                               mxGraySpace;

    /*
     * SalTimer related members
     */
    BOOL              mbInTimerProc;     // timer event is currently being dispatched
    BOOL              mbTimerInstalled;  // timer is in the event loop
    ULONG             mnTimerMS;         // Current Time (in MS) of the Timer
    ULONG             mnTimerOrgMS;      // Current Original Time (in MS)

    EventLoopTimerRef mrTimerRef;
    EventLoopTimerUPP mrTimerUPP;
    static FILE                                  *s_pLog;

    bool              mbIsScrollbarDoubleMax;   // TODO: support DoubleMin and DoubleBoth too

    SalData() :
        mpTimerProc( NULL ),
        mpFirstInstance( NULL ),
        mpFirstObject( NULL ),
        mpFirstVD( NULL ),
        mpFirstPrinter( NULL ),
        mpFontList( NULL ),
        mxRGBSpace( CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB) ),
        mxGraySpace( CGColorSpaceCreateWithName(kCGColorSpaceGenericGray) ),
        mbInTimerProc( FALSE ),
        mbTimerInstalled( FALSE ),
        mnTimerMS( 0 ),
        mnTimerOrgMS( 0 ),
        mrTimerRef( 0 ),
        mrTimerUPP( 0 )
    {}

    ~SalData()
    {
        CFRelease( mxRGBSpace );
        CFRelease( mxGraySpace );
    }
};

void AquaLog( const char* pFormat, ... );

inline void SetSalData( SalData* pData ) { ImplGetSVData()->mpSalData = (void*)pData; }
inline SalData *GetSalData() { return (SalData*)ImplGetSVData()->mpSalData; }
inline SalData *GetAppSalData() { return (SalData*)ImplGetAppSVData()->mpSalData; }

// --- Prototypes ---

BOOL ImplSalYieldMutexTryToAcquire();
void ImplSalYieldMutexAcquire();
void ImplSalYieldMutexRelease();

#endif  // _SV_SALDATA_HXX
