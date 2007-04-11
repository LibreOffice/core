/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: jobset.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:58:33 $
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

#ifndef _SV_JOBSET_H
#define _SV_JOBSET_H

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _SV_PRNTYPES_HXX
#include <vcl/prntypes.hxx>
#endif
#include <hash_map>
#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

// ------------------
// - JobSetup-Types -
// ------------------

// see com.sun.star.portal.client.JobSetupSystem.idl:
#define JOBSETUP_SYSTEM_DONTKNOW    0
#define JOBSETUP_SYSTEM_WINDOWS     1
#define JOBSETUP_SYSTEM_OS2         2
#define JOBSETUP_SYSTEM_UNIX        3
#define JOBSETUP_SYSTEM_MAC         4
#define JOBSETUP_SYSTEM_JAVA        5

// ----------------
// - ImplJobSetup -
// ----------------

struct ImplJobSetup
{
    USHORT          mnRefCount;         // RefCount (only independ data)
    USHORT          mnSystem;           // Sytem - JOBSETUP_SYSTEM_xxxx
    String          maPrinterName;      // Printer-Name
    String          maDriver;           // Driver-Name
    Orientation     meOrientation;      // Orientation
    USHORT          mnPaperBin;         // Papierschacht
    Paper           mePaperFormat;      // Papierformat
    long            mnPaperWidth;       // Papierbreite in 100tel mm
    long            mnPaperHeight;      // Papierhoehe in 100tel mm
    ULONG           mnDriverDataLen;    // Laenge der systemabhaengigen Daten
    BYTE*           mpDriverData;       // Systemabhaengige Daten die als Byte-Block rausgeschrieben werden
    ::std::hash_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash >            maValueMap;

                    ImplJobSetup();
                    ImplJobSetup( const ImplJobSetup& rJobSetup );
                    ~ImplJobSetup();
};

// Papierformat wird wenn PAPER_USER im unabhaengigen Teil automatisch aus
// Papierbreite/hoehe berechnet
// Papierbreite/hoehe wird wenn 0 im unabhaengigen Teil automatisch aus
// Papierformat berechnet, wenn dieses ungleich PAPER_USER ist

#endif  // _SV_JOBSET_H
