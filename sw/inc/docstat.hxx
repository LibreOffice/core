/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docstat.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:42:01 $
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
#ifndef _DOCSTAT_HXX
#define _DOCSTAT_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

///////////////////////////////////////////////////////////////////////////
// PRODUCT: struct SwInternStat ist nur fuer CORE Interne Statistics und
//          hat nichts in einer ProductVersion zu suchen.
///////////////////////////////////////////////////////////////////////////

struct SwInternStat;

struct SW_DLLPUBLIC SwDocStat
{
    USHORT          nTbl;
    USHORT          nGrf;
    USHORT          nOLE;
    ULONG           nPage;
    ULONG           nPara;
    ULONG           nWord;
    ULONG           nChar;
    BOOL            bModified;
    SwInternStat*   pInternStat;
    SwDocStat();
    void Reset();
};

#endif
