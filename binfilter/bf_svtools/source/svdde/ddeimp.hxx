/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DDEIMP_HXX
#define _DDEIMP_HXX

#ifdef OS2

#include "ddemlos2.h"

#define WORD USHORT
#define DWORD ULONG
#define LPBYTE BYTE*
#define LPWORD USHORT*
#define LPDWORD ULONG*
#define LPCTSTR PCSZ

#else

#include <tools/prewin.h>
#include <ddeml.h>
#include <tools/postwin.h>
#include "ddewrap.hxx"

/*
extern "C"
{
#define BOOL WIN_BOOL
#define BYTE WIN_BYTE
#undef BOOL
#undef BYTE
};
*/

#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif

namespace binfilter
{

class DdeService;
class DdeTopic;
class DdeItem;
class DdeTopics;
class DdeItems;

// ----------------
// - Conversation -
// ----------------

struct Conversation
{
    HCONV       hConv;
    DdeTopic*   pTopic;
};

DECLARE_LIST( ConvList, Conversation* );

// ---------------
// - DdeInternal -
// ---------------

class DdeInternal
{
public:
#ifdef WNT
    static HDDEDATA CALLBACK CliCallback
           ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
    static HDDEDATA CALLBACK SvrCallback
           ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
    static HDDEDATA CALLBACK InfCallback
           ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
#else
#if defined ( MTW ) || ( defined ( GCC ) && defined ( OS2 )) || defined( ICC )
    static HDDEDATA CALLBACK __EXPORT CliCallback
           ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
    static HDDEDATA CALLBACK __EXPORT SvrCallback
           ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
    static HDDEDATA CALLBACK __EXPORT InfCallback
           ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
#else
    static HDDEDATA CALLBACK _export CliCallback
           ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
    static HDDEDATA CALLBACK _export SvrCallback
           ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
    static HDDEDATA CALLBACK _export InfCallback
           ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
#endif
#endif
    static DdeService*      FindService( HSZ );
    static DdeTopic*        FindTopic( DdeService&, HSZ );
    static DdeItem*         FindItem( DdeTopic&, HSZ );
};

// -------------
// - DdeString -
// -------------

class DdeString : public String
{
protected:
    HSZ         hString;
    DWORD       hInst;

public:
                DdeString( DWORD, const sal_Unicode* );
                DdeString( DWORD, const String& );
                ~DdeString();

    int         operator==( HSZ );
                operator HSZ();
};

// --------------
// - DdeDataImp -
// --------------

struct DdeDataImp
{
    HDDEDATA		hData;
    LPBYTE			pData;
    long			nData;
    ULONG 			nFmt;
};

class DdeConnections;
class DdeServices;

struct DdeInstData
{
    USHORT			nRefCount;
    DdeConnections*	pConnections;
    // Server
    long 			hCurConvSvr;
    ULONG			hDdeInstSvr;
    short			nInstanceSvr;
    DdeServices*	pServicesSvr;
    // Client
    ULONG			hDdeInstCli;
    short			nInstanceCli;
};

#ifndef SHL_SVDDE
#define SHL_SVDDE	SHL_SHL2
#endif

inline DdeInstData* ImpGetInstData()
{
    return (DdeInstData*)(*GetAppData( SHL_SVDDE ));
}
DdeInstData* ImpInitInstData();
void ImpDeinitInstData();

}

#endif // _DDEIMP_HXX
