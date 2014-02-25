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

#ifndef INCLUDED_SVL_SVDDE_HXX
#define INCLUDED_SVL_SVDDE_HXX

#include <svl/svldllapi.h>
#include <sot/exchange.hxx>
#include <tools/solar.h>
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

typedef ::std::vector< DdeService* > DdeServices;
typedef ::std::vector< long > DdeFormats;
typedef ::std::vector< Conversation* > ConvList;


// - DdeData -


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
                    DdeData( SAL_UNUSED_PARAMETER const OUString& );
                    DdeData( const DdeData& );
                    ~DdeData();

    operator const  void*() const;
    operator        long() const;

    sal_uLong           GetFormat() const;

    DdeData&        operator = ( const DdeData& );

    static sal_uLong GetExternalFormat( sal_uLong nFmt );
    static sal_uLong GetInternalFormat( sal_uLong nFmt );
};


// - DdeTransaction -


class SVL_DLLPUBLIC DdeTransaction
{
public:
    virtual void    Data( const DdeData* );
    virtual void    Done( bool bDataValid );
protected:
    DdeConnection&  rDde;
    DdeData         aDdeData;
    DdeString*      pName;
    short           nType;
    sal_IntPtr      nId;
    sal_IntPtr      nTime;
    Link            aData;
    Link            aDone;
    bool            bBusy;

                    DdeTransaction( DdeConnection&, SAL_UNUSED_PARAMETER const OUString&, SAL_UNUSED_PARAMETER long = 0 );

public:
    virtual        ~DdeTransaction();

    bool            IsBusy() { return bBusy; }
    const OUString GetName() const;

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


// - DdeLink -


class SVL_DLLPUBLIC DdeLink : public DdeTransaction
{
    Link            aNotify;

public:
                    DdeLink( DdeConnection&, const OUString&, long = 0 );
    virtual        ~DdeLink();

    void            SetNotifyHdl( const Link& rLink ) { aNotify = rLink; }
    const Link&     GetNotifyHdl() const { return aNotify; }
    virtual void    Notify();
};


// - DdeWarmLink -


class SVL_DLLPUBLIC DdeWarmLink : public DdeLink
{
public:
            DdeWarmLink( DdeConnection&, const OUString&, long = 0 );
};


// - DdeHotLink -


class SVL_DLLPUBLIC DdeHotLink : public DdeLink
{
public:
            DdeHotLink( DdeConnection&, const OUString&, long = 0 );
};


// - DdeRequest -


class SVL_DLLPUBLIC DdeRequest : public DdeTransaction
{
public:
            DdeRequest( DdeConnection&, const OUString&, long = 0 );
};


// - DdePoke -


class SVL_DLLPUBLIC DdePoke : public DdeTransaction
{
public:
            DdePoke( DdeConnection&, const OUString&, const char*, long,
                     sal_uLong = FORMAT_STRING, long = 0 );
            DdePoke( DdeConnection&, const OUString&, SAL_UNUSED_PARAMETER const DdeData&, long = 0 );
            DdePoke( DdeConnection&, const OUString&, const OUString&, long = 0 );
};


// - DdeExecute -


class SVL_DLLPUBLIC DdeExecute : public DdeTransaction
{
public:
            DdeExecute( DdeConnection&, const OUString&, long = 0 );
};


// - DdeConnection -


class SVL_DLLPUBLIC DdeConnection
{
    friend class    DdeInternal;
    friend class    DdeTransaction;
    std::vector<DdeTransaction*> aTransactions;
    DdeString*      pService;
    DdeString*      pTopic;
    DdeImp*         pImp;

public:
                    DdeConnection( SAL_UNUSED_PARAMETER const OUString&, SAL_UNUSED_PARAMETER const OUString& );
                    ~DdeConnection();

    long            GetError();
    sal_IntPtr      GetConvId();

    static const std::vector<DdeConnection*>& GetConnections();

    bool            IsConnected();

    const OUString  GetServiceName();
    const OUString  GetTopicName();

private:
                            DdeConnection( const DdeConnection& );
    const DdeConnection&    operator= ( const DdeConnection& );
};


// - DdeItem -


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
                    DdeItem( SAL_UNUSED_PARAMETER const OUString& );
                    DdeItem( const DdeItem& );
                    virtual ~DdeItem();

    const OUString GetName() const;
    short           GetLinks();
    void            NotifyClient();
};


// - DdeItem -


class SVL_DLLPUBLIC DdeGetPutItem : public DdeItem
{
public:
                    DdeGetPutItem( const sal_Unicode* p );
                    DdeGetPutItem( const OUString& rStr );
                    DdeGetPutItem( const DdeItem& rItem );

    virtual DdeData* Get( sal_uLong );
    virtual bool    Put( const DdeData* );
    virtual void    AdviseLoop( bool );     // AdviseLoop starten/stoppen
};


// - DdeTopic -


class SVL_DLLPUBLIC DdeTopic
{
    SVL_DLLPRIVATE void _Disconnect( sal_IntPtr );

public:
    virtual void    Connect( sal_IntPtr );
    virtual void    Disconnect( sal_IntPtr );
    virtual DdeData* Get( sal_uLong );
    virtual bool    Put( const DdeData* );
    virtual bool    Execute( const OUString* );
        // evt. ein neues anlegen; return 0 -> es konnte nicht angelegt werden
    virtual bool    MakeItem( const OUString& rItem );

        // es wird ein Warm-/Hot-Link eingerichtet. Return-Wert
        // besagt ob es geklappt hat
    virtual bool    StartAdviseLoop();
    virtual bool    StopAdviseLoop();

private:
    friend class    DdeInternal;
    friend class    DdeService;
    friend class    DdeItem;

private:
    DdeString*      pName;
    OUString   aItem;
    std::vector<DdeItem*> aItems;
    Link            aConnectLink;
    Link            aDisconnectLink;
    Link            aGetLink;
    Link            aPutLink;
    Link            aExecLink;

public:
                    DdeTopic( SAL_UNUSED_PARAMETER const OUString& );
    virtual        ~DdeTopic();

    const OUString GetName() const;
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

    void            NotifyClient( const OUString& );
    bool            IsSystemTopic();

    void            InsertItem( DdeItem* );     // fuer eigene Ableitungen!
    DdeItem*        AddItem( const DdeItem& );  // werden kopiert !
    void            RemoveItem( const DdeItem& );
    const OUString&   GetCurItem() { return aItem;  }
    const std::vector<DdeItem*>& GetItems() const  { return aItems; }

private:
                    DdeTopic( const DdeTopic& );
    const DdeTopic& operator= ( const DdeTopic& );
};


// - DdeService -


class SVL_DLLPUBLIC DdeService
{
    friend class    DdeInternal;

public:
    virtual bool    IsBusy();
    virtual OUString GetHelp();
        // evt. ein neues anlegen; return 0 -> es konnte nicht angelegt werden
    virtual bool    MakeTopic( const OUString& rItem );

protected:
    virtual OUString Topics();
    virtual OUString Formats();
    virtual OUString SysItems();
    virtual OUString Status();
    virtual OUString SysTopicGet( const OUString& );
    virtual bool    SysTopicExecute( const OUString* );

    const DdeTopic* GetSysTopic() const { return pSysTopic; }
private:
    std::vector<DdeTopic*> aTopics;
    DdeFormats      aFormats;
    DdeTopic*       pSysTopic;
    DdeString*      pName;
    ConvList*       pConv;
    short           nStatus;

    SVL_DLLPRIVATE bool HasCbFormat( sal_uInt16 );

public:
                    DdeService( SAL_UNUSED_PARAMETER const OUString& );
    virtual        ~DdeService();

    const OUString  GetName() const;
    short           GetError()              { return nStatus; }

    static DdeServices& GetServices();
    std::vector<DdeTopic*>& GetTopics() { return aTopics; }

    void            AddTopic( const DdeTopic& );
    void            RemoveTopic( const DdeTopic& );

    void            AddFormat( sal_uLong );
    void            RemoveFormat( sal_uLong );
    bool            HasFormat( sal_uLong );

private:
      //              DdeService( const DdeService& );
    //int             operator= ( const DdeService& );
};


// - DdeTransaction -


inline long DdeTransaction::GetError()
{
    return rDde.GetError();
}
#endif // INCLUDED_SVL_SVDDE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
