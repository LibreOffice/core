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

#include <sal/config.h>

#include <svl/svldllapi.h>
#include <sot/formats.hxx>
#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <tools/link.hxx>
#include <tools/long.hxx>
#include <memory>
#include <vector>

class DdeString;
class DdeConnection;
class DdeTopic;
class DdeService;
struct DdeDataImp;
struct DdeImp;
struct DdeItemImpData;
struct Conversation;

typedef ::std::vector< DdeService* > DdeServices;


class SVL_DLLPUBLIC DdeData
{
    friend class    DdeInternal;
    friend class    DdeService;
    friend class    DdeConnection;
    friend class    DdeTransaction;
    std::unique_ptr<DdeDataImp>    xImp;

    SVL_DLLPRIVATE void            Lock();

    void            SetFormat( SotClipboardFormatId nFmt );

public:
                    DdeData();
                    DdeData(SAL_UNUSED_PARAMETER const void*, SAL_UNUSED_PARAMETER tools::Long, SAL_UNUSED_PARAMETER SotClipboardFormatId = SotClipboardFormatId::STRING);
                    DdeData(SAL_UNUSED_PARAMETER const OUString&);
                    DdeData(const DdeData&);
                    DdeData(DdeData&&) noexcept;
                    ~DdeData();

    void const *    getData() const;
    tools::Long            getSize() const;

    SotClipboardFormatId GetFormat() const;

    DdeData&        operator=(const DdeData&);
    DdeData&        operator=(DdeData&&) noexcept;

    static sal_uInt32 GetExternalFormat(SotClipboardFormatId nFmt);
    static SotClipboardFormatId GetInternalFormat(sal_uInt32 nFmt);
};


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

                    DdeTransaction( DdeConnection&, SAL_UNUSED_PARAMETER const OUString&, SAL_UNUSED_PARAMETER tools::Long = 0 );

public:
    virtual        ~DdeTransaction();

    bool            IsBusy() const { return bBusy; }
    OUString GetName() const;

    void            Execute();

    void            SetDataHdl( const Link<const DdeData*,void>& rLink ) { aData = rLink; }
    const Link<const DdeData*,void>& GetDataHdl() const { return aData; }

    void            SetDoneHdl( const Link<bool,void>& rLink ) { aDone = rLink; }
    const Link<bool,void>& GetDoneHdl() const { return aDone; }

    void                 SetFormat( SotClipboardFormatId nFmt ) { aDdeData.SetFormat( nFmt );  }
    SotClipboardFormatId GetFormat() const       { return aDdeData.GetFormat(); }

    tools::Long            GetError() const;

private:
    friend class    DdeInternal;
    friend class    DdeConnection;

                            DdeTransaction( const DdeTransaction& ) = delete;
    const DdeTransaction&   operator= ( const DdeTransaction& ) = delete;

};


class SVL_DLLPUBLIC DdeLink : public DdeTransaction
{
    Link<void*,void> aNotify;

public:
                    DdeLink( DdeConnection&, const OUString&, tools::Long = 0 );
    virtual        ~DdeLink() override;

    void            SetNotifyHdl( const Link<void*,void>& rLink ) { aNotify = rLink; }
    const Link<void*,void>&   GetNotifyHdl() const { return aNotify; }
    void    Notify();
};


class SVL_DLLPUBLIC DdeHotLink : public DdeLink
{
public:
            DdeHotLink( DdeConnection&, const OUString& );
};


class SVL_DLLPUBLIC DdeRequest : public DdeTransaction
{
public:
            DdeRequest( DdeConnection&, const OUString&, tools::Long = 0 );
};


class SVL_DLLPUBLIC DdePoke : public DdeTransaction
{
public:
            DdePoke( DdeConnection&, const OUString&, SAL_UNUSED_PARAMETER const DdeData&, tools::Long = 0 );
};


class SVL_DLLPUBLIC DdeExecute : public DdeTransaction
{
public:
            DdeExecute( DdeConnection&, const OUString&, tools::Long = 0 );
};


class SVL_DLLPUBLIC DdeConnection
{
    friend class    DdeInternal;
    friend class    DdeTransaction;
    std::vector<DdeTransaction*> aTransactions;
    DdeString*      pService;
    DdeString*      pTopic;
    std::unique_ptr<DdeImp> pImp;

public:
                    DdeConnection( SAL_UNUSED_PARAMETER const OUString&, SAL_UNUSED_PARAMETER const OUString& );
                    ~DdeConnection();

    tools::Long            GetError() const;

    static const std::vector<DdeConnection*>& GetConnections();

    bool            IsConnected();

    OUString        GetServiceName() const;
    OUString        GetTopicName() const;

private:
                            DdeConnection( const DdeConnection& ) = delete;
    const DdeConnection&    operator= ( const DdeConnection& ) = delete;
};


class SVL_DLLPUBLIC DdeItem
{
    friend class    DdeInternal;
    friend class    DdeTopic;
    DdeString*      pName;
    DdeTopic*       pMyTopic;
    std::vector<DdeItemImpData>* pImpData;

protected:
    sal_uInt8            nType;

public:
                    DdeItem( const sal_Unicode* );
                    DdeItem( SAL_UNUSED_PARAMETER const OUString& );
                    DdeItem( const DdeItem& );
                    virtual ~DdeItem();

    OUString GetName() const;
    short           GetLinks();
    void            NotifyClient();
};


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


class SVL_DLLPUBLIC DdeTopic
{

public:
    virtual DdeData* Get(SotClipboardFormatId);
    virtual bool Put( const DdeData* );
    virtual bool Execute( const OUString* );
    // Eventually create a new item. return 0 -> Item creation failed
    virtual bool MakeItem( const OUString& rItem );

    // A Warm-/Hot-Link is created. Return true if successful
    virtual bool    StartAdviseLoop();

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

    OUString        GetName() const;

    void            NotifyClient( const OUString& );
    bool            IsSystemTopic();

    void            InsertItem( DdeItem* );     // For own superclasses
    DdeItem*        AddItem( const DdeItem& );  // Will be cloned
    void            RemoveItem( const DdeItem& );
    const OUString& GetCurItem() const { return aItem;  }
    const std::vector<DdeItem*>& GetItems() const  { return aItems; }

private:
                    DdeTopic( const DdeTopic& ) = delete;
    const DdeTopic& operator= ( const DdeTopic& ) = delete;
};


class SVL_DLLPUBLIC DdeService
{
    friend class    DdeInternal;

protected:
    OUString Topics();
    OUString Formats();
    OUString SysItems();
    OUString Status();

    const DdeTopic* GetSysTopic() const { return pSysTopic; }
private:
    std::vector<DdeTopic*> aTopics;
    std::vector< sal_uInt32 > aFormats;
    DdeTopic*       pSysTopic;
    DdeString*      pName;
    std::vector<std::unique_ptr<Conversation>>
                    m_vConv;
    short           nStatus;

    SVL_DLLPRIVATE bool HasCbFormat( sal_uInt32 );

public:
                    DdeService( SAL_UNUSED_PARAMETER const OUString& );
    virtual        ~DdeService();

                    DdeService( const DdeService& ) = delete;
    DdeService&     operator= ( const DdeService& ) = delete;

    OUString        GetName() const;
    short           GetError() const { return nStatus; }

    static DdeServices& GetServices();
    std::vector<DdeTopic*>& GetTopics() { return aTopics; }

    void            AddTopic( const DdeTopic& );
    void            RemoveTopic( const DdeTopic& );

    void            AddFormat(SotClipboardFormatId);
    void            RemoveFormat(SotClipboardFormatId);
    bool            HasFormat(SotClipboardFormatId);
};


inline tools::Long DdeTransaction::GetError() const
{
    return rDde.GetError();
}
#endif // INCLUDED_SVL_SVDDE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
