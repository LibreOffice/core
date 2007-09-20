/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ddeimp.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-20 16:30:19 $
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
    HDDEDATA        hData;
    LPBYTE          pData;
    long            nData;
    ULONG           nFmt;
};

class DdeConnections;
class DdeServices;

struct DdeInstData
{
    USHORT          nRefCount;
    DdeConnections* pConnections;
    // Server
    long            hCurConvSvr;
    ULONG           hDdeInstSvr;
    short           nInstanceSvr;
    DdeServices*    pServicesSvr;
    // Client
    ULONG           hDdeInstCli;
    short           nInstanceCli;
};

#ifndef SHL_SVDDE
#define SHL_SVDDE   SHL_SHL2
#endif

inline DdeInstData* ImpGetInstData()
{
    return (DdeInstData*)(*GetAppData( SHL_SVDDE ));
}
DdeInstData* ImpInitInstData();
void ImpDeinitInstData();

#endif // _DDEIMP_HXX
