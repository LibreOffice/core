/*************************************************************************
 *
 *  $RCSfile: saldata.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: bmahbod $ $Date: 2001-03-12 23:15:31 $
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

#ifndef _SV_SALDATA_HXX
#define _SV_SALDATA_HXX

#ifndef _SV_SV_H
    #include <sv.h>
#endif

#ifdef __cplusplus

    #ifndef _SV_SVDATA_HXX
        #include <svdata.hxx>
    #endif

#endif // __cplusplus

#ifndef _SV_SALWTYPE_HXX
    #include <salwtype.hxx>
#endif

#ifndef _SV_VCLWINDOW_H
    #include <VCLWindow.h>
#endif

#if PRAGMA_ONCE    #pragma once#endif#ifdef __cplusplus

    class SalInstance;
    class SalObject;
    class SalFrame;
    class SalVirtualDevice;
    class SalPrinter;

    class FontList;

#else // __cplusplus

    #define SalInstance void
    #define SalVirtualDevice void
    #define SalPrinter void
    #define FontList void

#endif // __cplusplus

// -----------
// - SalData -
// -----------

struct SalData
{
    SALTIMERPROC       mpTimerProc;     // timer callback proc
    SalInstance       *mpFirstInstance; // pointer of first instance
    SalFrame          *mpFirstFrame;    // pointer of first frame
    SalObject         *mpFirstObject;   // pointer of first object window
    SalVirtualDevice  *mpFirstVD;       // first VirDev
    SalPrinter        *mpFirstPrinter;  // first printing printer
    FontList          *mpFontList;          // Mac OS font list
};

#ifdef __cplusplus

    inline void SetSalData( SalData* pData ) { ImplGetSVData()->mpSalData = (void*)pData; }

    inline SalData *GetSalData() { return (SalData*)ImplGetSVData()->mpSalData; }
    inline SalData *GetAppSalData() { return (SalData*)ImplGetAppSVData()->mpSalData; }

#else // __cplusplus

    // C wrapper functions around SetSalData, GetSalData, and GetAppSalData

    void SalSetSalData( struct SalData* pData );

    struct SalData *SalGetSalData();
    struct SalData *SalGetAppSalData();

    BOOL SalGetAppQuit();

#endif // __cplusplus

#endif  // _SV_SALDATA_HXX
