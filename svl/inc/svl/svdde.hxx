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

#ifndef _SVDDE_HXX
#define _SVDDE_HXX

#include "svl/svldllapi.h"
#include <sot/exchange.hxx>
#include <tools/string.hxx>
#include <tools/link.hxx>
#include <vector>

class DdeString;
class DdeData;
class DdeConnection;
class DdeTransaction;
class DdeLink;
class DdeRequest;
class DdeWarmLink;
class DdeHotLink;
class DdePoke;
class DdeExecute;
class DdeItem;
class DdeTopic;
class DdeService;
struct DdeDataImp;
struct DdeImp;
class DdeItemImp;
struct Conversation;

class DdeServices : public ::std::vector< DdeService* > {};
typedef ::std::vector< long > DdeFormats;
typedef ::std::vector< Conversation* > ConvList;

// -----------
// - DdeData -
// -----------

class SVL_DLLPUBLIC DdeData
{
    friend class    DdeInternal;
    friend class    DdeService;
    friend class    DdeConnection;
    friend class    DdeTransaction;
    DdeDataImp*     pImp;

    SVL_DLLPRIVATE void            Lock();

    void            SetFormat( sal_uLong nFmt );

public:
                    DdeData();
                    DdeData( SAL_UNUSED_PARAMETER const void*, SAL_UNUSED_PARAMETER long, SAL_UNUSED_PARAMETER sal_uLong = FORMAT_STRING );
                    DdeData( SAL_UNUSED_PARAMETER const String& );
                    DdeData( const DdeData& );
                    ~DdeData();

    operator const  void*() const;
    operator        long() const;

    sal_uLong           GetFormat() const;

    DdeData&        operator = ( const DdeData& );

    static sal_uLong GetExternalFormat( sal_uLong nFmt );
    static sal_uLong GetInternalFormat( sal_uLong nFmt );
};

// ------------------
// - DdeTransaction -
// ------------------

class SVL_DLLPUBLIC DdeTransaction
{
public:
    virtual void    Data( const DdeData* );
    virtual void    Done( sal_Bool bDataValid );
protected:
    DdeConnection&  rDde;
    DdeData         aDdeData;
    DdeString*      pName;
    short           nType;
    long            nId;
    long            nTime;
    Link            aData;
    Link            aDone;
    sal_Bool            bBusy;

                    DdeTransaction( DdeConnection&, SAL_UNUSED_PARAMETER const String&, SAL_UNUSED_PARAMETER long = 0 );

public:
    virtual        ~DdeTransaction();

    sal_Bool            IsBusy() { return bBusy; }
    const String&   GetName() const;

    void            Execute();

    void            SetDataHdl( const Link& rLink ) { aData = rLink; }
    const Link&     GetDataHdl() const { return aData; }

    void            SetDoneHdl( const Link& rLink ) { aDone = rLink; }
    const Link&     GetDoneHdl() const { return aDone; }

    void            SetFormat( sal_uLong nFmt ) { aDdeData.SetFormat( nFmt );  }
    sal_uLong           GetFormat() const       { return aDdeData.GetFormat(); }

    long            GetError();

private:
    friend class    DdeInternal;
    friend class    DdeConnection;

                            DdeTransaction( const DdeTransaction& );
    const DdeTransaction&   operator= ( const DdeTransaction& );

};

// -----------
// - DdeLink -
// -----------

class SVL_DLLPUBLIC DdeLink : public DdeTransaction
{
    Link            aNotify;

public:
                    DdeLink( DdeConnection&, const String&, long = 0 );
    virtual        ~DdeLink();

    void            SetNotifyHdl( const Link& rLink ) { aNotify = rLink; }
    const Link&     GetNotifyHdl() const { return aNotify; }
    virtual void    Notify();
};

// ---------------
// - DdeWarmLink -
// ---------------

class SVL_DLLPUBLIC DdeWarmLink : public DdeLink
{
public:
            DdeWarmLink( DdeConnection&, const String&, long = 0 );
};

// --------------
// - DdeHotLink -
// --------------

class SVL_DLLPUBLIC DdeHotLink : public DdeLink
{
public:
            DdeHotLink( DdeConnection&, const String&, long = 0 );
};

// --------------
// - DdeRequest -
// --------------

class SVL_DLLPUBLIC DdeRequest : public DdeTransaction
{
public:
            DdeRequest( DdeConnection&, const String&, long = 0 );
};

// -----------
// - DdePoke -
// -----------

class SVL_DLLPUBLIC DdePoke : public DdeTransaction
{
public:
            DdePoke( DdeConnection&, const String&, const char*, long,
                     sal_uLong = FORMAT_STRING, long = 0 );
            DdePoke( DdeConnection&, const String&, SAL_UNUSED_PARAMETER const DdeData&, long = 0 );
            DdePoke( DdeConnection&, const String&, const String&, long = 0 );
};

// --------------
// - DdeExecute -
// --------------

class SVL_DLLPUBLIC DdeExecute : public DdeTransaction
{
public:
            DdeExecute( DdeConnection&, const String&, long = 0 );
};

// -----------------
// - DdeConnection -
// -----------------

class SVL_DLLPUBLIC DdeConnection
{
    friend class    DdeInternal;
    friend class    DdeTransaction;
    std::vector<DdeTransaction*> aTransactions;
    DdeString*      pService;
    DdeString*      pTopic;
    DdeImp*         pImp;

public:
                    DdeConnection( SAL_UNUSED_PARAMETER const String&, SAL_UNUSED_PARAMETER const String& );
                    ~DdeConnection();

    long            GetError();
    long            GetConvId();

    static const std::vector<DdeConnection*>& GetConnections();

    sal_Bool            IsConnected();

    const String&   GetServiceName();
    const String&   GetTopicName();

private:
                            DdeConnection( const DdeConnection& );
    const DdeConnection&    operator= ( const DdeConnection& );
};

// -----------
// - DdeItem -
// -----------

class SVL_DLLPUBLIC DdeItem
{
    friend class    DdeInternal;
    friend class    DdeTopic;
    DdeString*      pName;
    DdeTopic*       pMyTopic;
    DdeItemImp*     pImpData;

    void            IncMonitor( sal_uLong );
    void            DecMonitor( sal_uLong );

protected:
    sal_uInt8            nType;

public:
                    DdeItem( const sal_Unicode* );
                    DdeItem( SAL_UNUSED_PARAMETER const String& );
                    DdeItem( const DdeItem& );
                    virtual ~DdeItem();

    const String&   GetName() const;
    short           GetLinks();
    void            NotifyClient();
};

// -----------
// - DdeItem -
// -----------

class SVL_DLLPUBLIC DdeGetPutItem : public DdeItem
{
public:
                    DdeGetPutItem( const sal_Unicode* p );
                    DdeGetPutItem( const String& rStr );
                    DdeGetPutItem( const DdeItem& rItem );

    virtual DdeData* Get( sal_uLong );
    virtual sal_Bool    Put( const DdeData* );
    virtual void    AdviseLoop( sal_Bool );     // AdviseLoop starten/stoppen
};

// ------------
// - DdeTopic -
// ------------

class SVL_DLLPUBLIC DdeTopic
{
    SVL_DLLPRIVATE void _Disconnect( long );

public:
    virtual void    Connect( long );
    virtual void    Disconnect( long );
    virtual DdeData* Get( sal_uLong );
    virtual sal_Bool    Put( const DdeData* );
    virtual sal_Bool    Execute( const String* );
        // evt. ein neues anlegen; return 0 -> es konnte nicht angelegt werden
    virtual sal_Bool MakeItem( const String& rItem );

        // es wird ein Warm-/Hot-Link eingerichtet. Return-Wert
        // besagt ob es geklappt hat
    virtual sal_Bool    StartAdviseLoop();
    virtual sal_Bool    StopAdviseLoop();

private:
    friend class    DdeInternal;
    friend class    DdeService;
    friend class    DdeItem;

private:
    DdeString*      pName;
    String          aItem;
    std::vector<DdeItem*> aItems;
    Link            aConnectLink;
    Link            aDisconnectLink;
    Link            aGetLink;
    Link            aPutLink;
    Link            aExecLink;

public:
                    DdeTopic( SAL_UNUSED_PARAMETER const String& );
    virtual        ~DdeTopic();

    const String&   GetName() const;
    long            GetConvId();

    void            SetConnectHdl( const Link& rLink ) { aConnectLink = rLink; }
    const Link&     GetConnectHdl() const { return aConnectLink;  }
    void            SetDisconnectHdl( const Link& rLink ) { aDisconnectLink = rLink; }
    const Link&     GetDisconnectHdl() const { return aDisconnectLink;  }
    void            SetGetHdl( const Link& rLink ) { aGetLink = rLink; }
    const Link&     GetGetHdl() const { return aGetLink;  }
    void            SetPutHdl( const Link& rLink ) { aPutLink = rLink; }
    const Link&     GetPutHdl() const { return aPutLink;  }
    void            SetExecuteHdl( const Link& rLink ) { aExecLink = rLink; }
    const Link&     GetExecuteHdl() const { return aExecLink; }

    void            NotifyClient( const String& );
    sal_Bool            IsSystemTopic();

    void            InsertItem( DdeItem* );     // fuer eigene Ableitungen!
    DdeItem*        AddItem( const DdeItem& );  // werden kopiert !
    void            RemoveItem( const DdeItem& );
    const String&   GetCurItem() { return aItem;  }
    const std::vector<DdeItem*>& GetItems() const  { return aItems; }

private:
                    DdeTopic( const DdeTopic& );
    const DdeTopic& operator= ( const DdeTopic& );
};

// --------------
// - DdeService -
// --------------

class SVL_DLLPUBLIC DdeService
{
    friend class    DdeInternal;

public:
    virtual sal_Bool    IsBusy();
    virtual String  GetHelp();
        // evt. ein neues anlegen; return 0 -> es konnte nicht angelegt werden
    virtual sal_Bool    MakeTopic( const String& rItem );

protected:
    virtual String  Topics();
    virtual String  Formats();
    virtual String  SysItems();
    virtual String  Status();
    virtual String  SysTopicGet( const String& );
    virtual sal_Bool    SysTopicExecute( const String* );

    const DdeTopic* GetSysTopic() const { return pSysTopic; }
private:
    std::vector<DdeTopic*> aTopics;
    DdeFormats      aFormats;
    DdeTopic*       pSysTopic;
    DdeString*      pName;
    ConvList*       pConv;
    short           nStatus;

    SVL_DLLPRIVATE sal_Bool            HasCbFormat( sal_uInt16 );

public:
                    DdeService( SAL_UNUSED_PARAMETER const String& );
    virtual        ~DdeService();

    const String&   GetName() const;
    short           GetError()              { return nStatus; }

    static DdeServices& GetServices();
    std::vector<DdeTopic*>& GetTopics() { return aTopics; }

    void            AddTopic( const DdeTopic& );
    void            RemoveTopic( const DdeTopic& );

    void            AddFormat( sal_uLong );
    void            RemoveFormat( sal_uLong );
    sal_Bool            HasFormat( sal_uLong );

private:
      //              DdeService( const DdeService& );
    //int             operator= ( const DdeService& );
};

// ------------------
// - DdeTransaction -
// ------------------

inline long DdeTransaction::GetError()
{
    return rDde.GetError();
}
#endif // _SVDDE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
