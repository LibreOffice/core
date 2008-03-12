/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 13:10:05 $
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

#ifndef _SHL_HXX
#define _SHL_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

// ----------------
// - GetAppData() -
// ----------------

#define SHL_SHL1        0
#define SHL_SHL2        1
#define SHL_SHL3        2
#define SHL_APP1        3
#define SHL_APP2        4
#define SHL_APP3        5
#define SHL_TOOLS       6
#define SHL_SV          7
#define SHL_SVT         8
#define SHL_SVDDE       9
#define SHL_ERR         10
#define SHL_IPC         11
#define SHL_SVX         12
#define SHL_ITEM        13
#define SHL_SVD         14
#define SHL_SI          15
#define SHL_SFC         16
#define SHL_SFX         17
//#define SHL_SO2       18
#define SHL_IDL         19
#define SHL_IDE         20
#define SHL_EDIT        21
#define SHL_VCED        22
#define SHL_BASIC       23
#define SHL_HELP        24
#define SHL_LNKCCH      25
#define SHL_CHANNEL     26
#define SHL_SBX         27
#define SHL_SBC         28
#define SHL_SDB         29
#define SHL_SBA         30
#define SHL_SBA2        31
#define SHL_SCH         32
#define SHL_SIM         33
#define SHL_SM          34
#define SHL_SGA         35
#define SHL_DOCMGR      36
// 37 removed
#define SHL_MAIL        38
#define SHL_NEWS        39
#define SHL_OFFAPP      40
#define SHL_INET        41
#define SHL_CHAOS       42
#define SHL_HISTORY     43
#define SHL_SJ          44
#define SHL_CALC        45
#define SHL_DRAW        46
#define SHL_WRITER      47
#define SHL_MONEY       48
#define SHL_HBCI        49
#define SHL_BASE3D      50
#define SHL_BRUSHITEM   51
#define SHL_SFONTITEM   52

// #110743#
// #define SHL_COUNT        53
// the following added for binary filter project
// Sice an array is created for that values, it is necessary to
// put them directly behind the defined ones. Else, some space is
// wasted.
#define BF_SHL_SVX      53
#define BF_SHL_ITEM     54
#define BF_SHL_SVD      55
#define BF_SHL_EDIT     56
#define BF_SHL_SCH      57
#define BF_SHL_SM       58
#define BF_SHL_CALC     59
#define BF_SHL_DRAW     60
#define BF_SHL_WRITER   61
#define BF_SHL_SVT      62
#define BF_SHL_SVDDE    63

#define SHL_COUNT       64

TOOLS_DLLPUBLIC void** GetAppData( USHORT nSharedLib );

#endif // _SHL_HXX
