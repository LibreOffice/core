/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SVDDE_HXX
#define _SVDDE_HXX

#include "svl/svldllapi.h"
#include <sot/exchange.hxx>
#include <tools/string.hxx>
#include <tools/list.hxx>
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
class ConvList;
struct DdeDataImp;
struct DdeImp;
class DdeItemImp;

#ifndef _SVDDE_NOLISTS
DECLARE_LIST( DdeConnections, DdeConnection* )
DECLARE_LIST( DdeServices, DdeService* )
DECLARE_LIST( DdeTopics, DdeTopic* )
DECLARE_LIST( DdeItems, DdeItem* )
#else
typedef List DdeConnections;
typedef List DdeServices;
typedef List DdeTopics;
typedef List DdeItems;
#endif

DECLARE_LIST( DdeTransactions, DdeTransaction* )
DECLARE_LIST( DdeFormats, long )

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

    void            SetFormat( ULONG nFmt );

public:
                    DdeData();
                    DdeData( const void*, long, ULONG = FORMAT_STRING );
                    DdeData( const String& );
                    DdeData( const DdeData& );
                    ~DdeData();

    operator const  void*() const;
    operator        long() const;

    ULONG           GetFormat() const;

    DdeData&        operator = ( const DdeData& );

    static ULONG GetExternalFormat( ULONG nFmt );
    static ULONG GetInternalFormat( ULONG nFmt );
};

// ------------------
// - DdeTransaction -
// ------------------

class SVL_DLLPUBLIC DdeTransaction
{
public:
    virtual void    Data( const DdeData* );
    virtual void    Done( BOOL bDataValid );
protected:
    DdeConnection&  rDde;
    DdeData         aDdeData;
    DdeString*      pName;
    short           nType;
    long            nId;
    long            nTime;
    Link            aData;
    Link            aDone;
    BOOL            bBusy;

                    DdeTransaction( DdeConnection&, const String&, long = 0 );

public:
    virtual        ~DdeTransaction();

    BOOL            IsBusy() { return bBusy; }
    const String&   GetName() const;

    void            Execute();

    void            SetDataHdl( const Link& rLink ) { aData = rLink; }
    const Link&     GetDataHdl() const { return aData; }

    void            SetDoneHdl( const Link& rLink ) { aDone = rLink; }
    const Link&     GetDoneHdl() const { return aDone; }

    void            SetFormat( ULONG nFmt ) { aDdeData.SetFormat( nFmt );  }
    ULONG           GetFormat() const       { return aDdeData.GetFormat(); }

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
                     ULONG = FORMAT_STRING, long = 0 );
            DdePoke( DdeConnection&, const String&, const DdeData&, long = 0 );
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
    DdeTransactions aTransactions;
    DdeString*      pService;
    DdeString*      pTopic;
    DdeImp*         pImp;

public:
                    DdeConnection( const String&, const String& );
                    ~DdeConnection();

    long            GetError();
    long            GetConvId();

    static const DdeConnections& GetConnections();

    BOOL            IsConnected();

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

    void            IncMonitor( ULONG );
    void            DecMonitor( ULONG );

protected:
    BYTE            nType;

public:
                    DdeItem( const sal_Unicode* );
                    DdeItem( const String& );
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

    virtual DdeData* Get( ULONG );
    virtual BOOL    Put( const DdeData* );
    virtual void    AdviseLoop( BOOL );     // AdviseLoop starten/stoppen
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
    virtual DdeData* Get( ULONG );
    virtual BOOL    Put( const DdeData* );
    virtual BOOL    Execute( const String* );
        // evt. ein neues anlegen; return 0 -> es konnte nicht angelegt werden
    virtual BOOL MakeItem( const String& rItem );

        // es wird ein Warm-/Hot-Link eingerichtet. Return-Wert
        // besagt ob es geklappt hat
    virtual BOOL    StartAdviseLoop();
    virtual BOOL    StopAdviseLoop();

private:
    friend class    DdeInternal;
    friend class    DdeService;
    friend class    DdeItem;

private:
    DdeString*      pName;
    String          aItem;
    DdeItems        aItems;
    Link            aConnectLink;
    Link            aDisconnectLink;
    Link            aGetLink;
    Link            aPutLink;
    Link            aExecLink;

public:
                    DdeTopic( const String& );
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
    BOOL            IsSystemTopic();

    void            InsertItem( DdeItem* );     // fuer eigene Ableitungen!
    DdeItem*        AddItem( const DdeItem& );  // werden kopiert !
    void            RemoveItem( const DdeItem& );
    const String&   GetCurItem() { return aItem;  }
    const DdeItems& GetItems()   { return aItems; }

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
    virtual BOOL    IsBusy();
    virtual String  GetHelp();
        // evt. ein neues anlegen; return 0 -> es konnte nicht angelegt werden
    virtual BOOL    MakeTopic( const String& rItem );

protected:
    virtual String  Topics();
    virtual String  Formats();
    virtual String  SysItems();
    virtual String  Status();
    virtual String  SysTopicGet( const String& );
    virtual BOOL    SysTopicExecute( const String* );

    const DdeTopic* GetSysTopic() const { return pSysTopic; }
private:
    DdeTopics       aTopics;
    DdeFormats      aFormats;
    DdeTopic*       pSysTopic;
    DdeString*      pName;
    ConvList*       pConv;
    short           nStatus;

    SVL_DLLPRIVATE BOOL            HasCbFormat( USHORT );

public:
                    DdeService( const String& );
    virtual        ~DdeService();

    const String&   GetName() const;
    short           GetError()              { return nStatus; }

    static DdeServices& GetServices();
    DdeTopics&      GetTopics()             { return aTopics; }

    void            AddTopic( const DdeTopic& );
    void            RemoveTopic( const DdeTopic& );

    void            AddFormat( ULONG );
    void            RemoveFormat( ULONG );
    BOOL            HasFormat( ULONG );

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
