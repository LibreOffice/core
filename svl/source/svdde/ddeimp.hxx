/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVL_SOURCE_SVDDE_DDEIMP_HXX
#define INCLUDED_SVL_SOURCE_SVDDE_DDEIMP_HXX

#include <windows.h>
#include <ddeml.h>

#include <boost/noncopyable.hpp>
#include <rtl/ustring.hxx>
#include <svl/svdde.hxx>
#include <tools/shl.hxx>
#include <vector>


// - Conversation -


struct Conversation
{
    HCONV       hConv;
    DdeTopic*   pTopic;
};

typedef ::std::vector< Conversation* > ConvList;


// - DdeInternal -


class DdeInternal
{
public:
    static HDDEDATA CALLBACK CliCallback
           ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
    static HDDEDATA CALLBACK SvrCallback
           ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
    static HDDEDATA CALLBACK InfCallback
           ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
    static DdeService*      FindService( HSZ );
    static DdeTopic*        FindTopic( DdeService&, HSZ );
    static DdeItem*         FindItem( DdeTopic&, HSZ );
};


// - DdeString -


class DdeString
{
private:
    OUString m_aString;
protected:
    HSZ         hString;
    DWORD       hInst;

public:
                DdeString( DWORD, const sal_Unicode* );
                DdeString( DWORD, const OUString& );
                ~DdeString();

    int         operator==( HSZ );
                operator HSZ();
    OUString toOUString() const { return m_aString; }
};


// - DdeDataImp -


struct DdeDataImp
{
    HDDEDATA        hData;
    LPBYTE          pData;
    long            nData;
    sal_uLong           nFmt;
};

class DdeConnection;

class DdeInstData : private boost::noncopyable
{
public:
    sal_uInt16          nRefCount;
    std::vector<DdeConnection*> aConnections;
    // Server
    sal_IntPtr      hCurConvSvr;
    DWORD           hDdeInstSvr;
    short           nInstanceSvr;
    DdeServices*    pServicesSvr;
    // Client
    DWORD           hDdeInstCli;
    short           nInstanceCli;

    DdeInstData()
        : nRefCount(0)
        , hCurConvSvr(0)
        , hDdeInstSvr(0)
        , nInstanceSvr(0)
        , pServicesSvr(NULL)
        , hDdeInstCli(0)
        , nInstanceCli(0)
    {
    }
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

#endif // INCLUDED_SVL_SOURCE_SVDDE_DDEIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
