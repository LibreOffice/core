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

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <ddeml.h>

#include <rtl/ustring.hxx>
#include <svl/svdde.hxx>
#include <vector>


struct Conversation
{
    HCONV       hConv;
    DdeTopic*   pTopic;
};


class DdeInternal
{
public:
    static HDDEDATA CALLBACK CliCallback
           ( UINT, UINT, HCONV, HSZ, HSZ, HDDEDATA, ULONG_PTR, ULONG_PTR );
    static HDDEDATA CALLBACK SvrCallback
           ( UINT, UINT, HCONV, HSZ, HSZ, HDDEDATA, ULONG_PTR, ULONG_PTR );
    static DdeService*      FindService( HSZ );
    static DdeTopic*        FindTopic( DdeService&, HSZ );
    static DdeItem*         FindItem( DdeTopic&, HSZ );
    static void DisconnectTopic(DdeTopic &, HCONV);
    static void IncMonitor(DdeItem *pItem, HCONV);
    static void DecMonitor(DdeItem *pItem, HCONV);
};


class DdeString
{
private:
    OUString m_aString;
protected:
    HSZ         hString;
    DWORD       hInst;

public:
                DdeString( DWORD, const OUString& );
                ~DdeString();

    bool        operator==( HSZ );
    HSZ getHSZ();
    OUString toOUString() const { return m_aString; }
};


struct DdeDataImp
{
    HDDEDATA        hData;
    void const *    pData;
    DWORD           nData;
    SotClipboardFormatId nFmt;
};

class DdeConnection;

class DdeInstData
{
public:
    sal_uInt16          nRefCount;
    std::vector<DdeConnection*> aConnections;
    // Server
    DWORD           hDdeInstSvr;
    short           nInstanceSvr;
    DdeServices*    pServicesSvr;
    // Client
    DWORD           hDdeInstCli;
    short           nInstanceCli;

    DdeInstData()
        : nRefCount(0)
        , hDdeInstSvr(0)
        , nInstanceSvr(0)
        , pServicesSvr(nullptr)
        , hDdeInstCli(0)
        , nInstanceCli(0)
    {
    }
    DdeInstData(const DdeInstData&) = delete;
    DdeInstData& operator=(const DdeInstData&) = delete;
};

DdeInstData* ImpGetInstData();
DdeInstData* ImpInitInstData();
void ImpDeinitInstData();

#endif // INCLUDED_SVL_SOURCE_SVDDE_DDEIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
