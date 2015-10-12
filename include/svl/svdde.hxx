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

    void            SetFormat( SotClipboardFormatId nFmt );

public:
                    DdeData();
                    DdeData( SAL_UNUSED_PARAMETER const void*, SAL_UNUSED_PARAMETER long, SAL_UNUSED_PARAMETER SotClipboardFormatId = SotClipboardFormatId::STRING );
                    DdeData( SAL_UNUSED_PARAMETER const OUString& );
                    DdeData( const DdeData& );
                    ~DdeData();

    operator const  void*() const;
    operator        long() const;

    SotClipboardFormatId GetFormat() const;

    DdeData&        operator = ( const DdeData& );

    static sal_uLong GetExternalFormat(SotClipboardFormatId nFmt);
    static SotClipboardFormatId GetInternalFormat(sal_uLong nFmt);
};


// - DdeTransaction -


class SVL_DLLPUBLIC DdeTransaction
{
public:
    void    Data( const DdeData* );
    void    Done( bool bDataValid );
protected:
    DdeConnection&  rDde;
    DdeData         aDdeData;
    DdeString*      pName;
    short           nType;
    sal_IntPtr      nId;
    sal_IntPtr      nTime;
    Link<const DdeData*,void> aData;
    Link<bool,void>           aDone;
    bool            bBusy;

                    DdeTransaction( DdeConnection&, SAL_UNUSED_PARAMETER const OUString&, SAL_UNUSED_PARAMETER long = 0 );

public:
    virtual        ~DdeTransaction();

    bool            IsBusy() { return bBusy; }
    const OUString GetName() const;

    void            Execute();

    void            SetDataHdl( const Link<const DdeData*,void>& rLink ) { aData = rLink; }
    const Link<const DdeData*,void>& GetDataHdl() const { return aData; }

    void            SetDoneHdl( const Link<bool,void>& rLink ) { aDone = rLink; }
    const Link<bool,void>& GetDoneHdl() const { return aDone; }

    void                 SetFormat( SotClipboardFormatId nFmt ) { aDdeData.SetFormat( nFmt );  }
    SotClipboardFormatId GetFormat() const       { return aDdeData.GetFormat(); }

    long            GetError();

private:
    friend class    DdeInternal;
    friend class    DdeConnection;

                            DdeTransaction( const DdeTransaction& ) = delete;
    const DdeTransaction&   operator= ( const DdeTransaction& ) = delete;

};


// - DdeLink -


class SVL_DLLPUBLIC DdeLink : public DdeTransaction
{
    Link<void*,void> aNotify;

public:
                    DdeLink( DdeConnection&, const OUString&, long = 0 );
    virtual        ~DdeLink();

    void            SetNotifyHdl( const Link<void*,void>& rLink ) { aNotify = rLink; }
    const Link<void*,void>&   GetNotifyHdl() const { return aNotify; }
    void    Notify();
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
                     SotClipboardFormatId = SotClipboardFormatId::STRING, long = 0 );
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
                            DdeConnection( const DdeConnection& ) = delete;
    const DdeConnection&    operator= ( const DdeConnection& ) = delete;
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

    virtual DdeData* Get( SotClipboardFormatId );
    virtual bool    Put( const DdeData* );
    virtual void    AdviseLoop( bool );     // Start / Stop AdviseLoop
};


// - DdeTopic -


class SVL_DLLPUBLIC DdeTopic
{
    SVL_DLLPRIVATE void Disconnect( sal_IntPtr );

public:
    virtual DdeData* Get(SotClipboardFormatId);
    virtual bool Put( const DdeData* );
    virtual bool Execute( const OUString* );
    // Eventually create a new item. return 0 -> Item creation failed
    virtual bool MakeItem( const OUString& rItem );

    // A Warm-/Hot-Link is created. Return true if successful
    virtual bool    StartAdviseLoop();
    bool    StopAdviseLoop();

private:
    friend class    DdeInternal;
    friend class    DdeService;
    friend class    DdeItem;

private:
    DdeString*            pName;
    OUString              aItem;
    std::vector<DdeItem*> aItems;

public:
                    DdeTopic( SAL_UNUSED_PARAMETER const OUString& );
    virtual        ~DdeTopic();

    const OUString  GetName() const;
    long            GetConvId();

    void            NotifyClient( const OUString& );
    bool            IsSystemTopic();

    void            InsertItem( DdeItem* );     // For own superclasses
    DdeItem*        AddItem( const DdeItem& );  // Will be cloned
    void            RemoveItem( const DdeItem& );
    const OUString& GetCurItem() { return aItem;  }
    const std::vector<DdeItem*>& GetItems() const  { return aItems; }

private:
                    DdeTopic( const DdeTopic& ) = delete;
    const DdeTopic& operator= ( const DdeTopic& ) = delete;
};


// - DdeService -


class SVL_DLLPUBLIC DdeService
{
    friend class    DdeInternal;

public:
    bool    IsBusy();
    OUString GetHelp();
    // Eventually creating a new item. return 0 -> Topic creation failed
    bool    MakeTopic( const OUString& rItem );

protected:
    OUString Topics();
    OUString Formats();
    OUString SysItems();
    OUString Status();
    OUString SysTopicGet( const OUString& );
    bool    SysTopicExecute( const OUString* );

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

    void            AddFormat(SotClipboardFormatId);
    void            RemoveFormat(SotClipboardFormatId);
    bool            HasFormat(SotClipboardFormatId);

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
