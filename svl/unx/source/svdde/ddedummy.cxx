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

#include <svl/svdde.hxx>
#include <rtl/instance.hxx>

DdeData::DdeData()
{
}

DdeData::DdeData( const OUString& )
{
}

DdeData::DdeData( const DdeData& )
{
}

DdeData::DdeData( const void*, long, sal_uLong)
{
}

DdeData::~DdeData()
{
}

void DdeData::SetFormat( SAL_UNUSED_PARAMETER sal_uLong )
{
}

sal_uLong DdeData::GetFormat() const
{
    return 0L;
}

DdeData& DdeData::operator = ( const DdeData& )
{
    return *this;
}

DdeData::operator long() const
{
    return 0L;
}

DdeData::operator const void*() const
{
  return NULL;
}

long DdeConnection::GetError()
{
    return 0L;
}

DdeConnection::DdeConnection( const OUString&, const OUString& )
{
}

DdeConnection::~DdeConnection()
{
}

const OUString DdeConnection::GetServiceName()
{
    return OUString();
}

const OUString DdeConnection::GetTopicName()
{
    return OUString();;
}

DdeTransaction::DdeTransaction( DdeConnection& rConnection, const OUString&, long )
    : rDde( rConnection )
{
}

DdeTransaction::DdeTransaction( const DdeTransaction& rTransaction )
    : rDde( rTransaction.rDde )
{
}

void DdeTransaction::Execute()
{
}

void DdeTransaction::Done( SAL_UNUSED_PARAMETER bool )
{
}

void DdeTransaction::Data( SAL_UNUSED_PARAMETER const DdeData* )
{
}

DdeTransaction::~DdeTransaction()
{
}

DdeRequest::DdeRequest( DdeConnection& rConnection, const OUString& rString, long lLong )
    : DdeTransaction( rConnection, rString, lLong )
{
}

DdeExecute::DdeExecute( DdeConnection& rConnection, const OUString& rString, long lLong )
    : DdeTransaction( rConnection, rString, lLong )
{
}

DdePoke::DdePoke( DdeConnection& rConnection, const OUString& rString, const DdeData&, long lLong )
    : DdeTransaction( rConnection, rString, lLong )
{
}


DdeTopic::DdeTopic( const OUString& )
{
}

DdeTopic::~DdeTopic()
{
}

void DdeTopic::Connect( SAL_UNUSED_PARAMETER long )
{
}

void DdeTopic::Disconnect( SAL_UNUSED_PARAMETER long )
{
}

void DdeTopic::InsertItem( SAL_UNUSED_PARAMETER DdeItem* )
{
}

DdeItem* DdeTopic::AddItem( const DdeItem& rDdeItem )
{
    return (DdeItem*) &rDdeItem;
}

void DdeTopic::RemoveItem( SAL_UNUSED_PARAMETER const DdeItem& )
{
}

DdeData* DdeTopic::Get( SAL_UNUSED_PARAMETER sal_uLong )
{
    return NULL;
}

bool DdeTopic::MakeItem( SAL_UNUSED_PARAMETER const OUString& )
{
    return false;
}

bool DdeTopic::StartAdviseLoop()
{
    return false;
}

bool DdeTopic::StopAdviseLoop()
{
    return false;
}

bool DdeTopic::Execute( SAL_UNUSED_PARAMETER const OUString* )
{
    return false;
}

bool DdeTopic::Put( SAL_UNUSED_PARAMETER const DdeData* )
{
    return false;
}

const OUString DdeTopic::GetName() const
{
    return OUString();
}

DdeService::DdeService( const OUString& )
{
    nStatus = 0;
}

OUString DdeService::Topics()
{
    return OUString();
}

OUString DdeService::Formats() {
    return OUString();
}

OUString DdeService::SysItems()
{
    return OUString();
}

OUString DdeService::Status()
{
    return OUString();
}

OUString DdeService::SysTopicGet(const OUString& rString)
{
    return rString;
}

bool DdeService::SysTopicExecute(SAL_UNUSED_PARAMETER const OUString*)
{
    return false;
}

DdeService::~DdeService()
{
}

bool DdeService::IsBusy()
{
  return false;
}

OUString DdeService::GetHelp()
{
    return OUString();
}

void DdeService::AddFormat( SAL_UNUSED_PARAMETER sal_uLong )
{
}

void DdeService::AddTopic( SAL_UNUSED_PARAMETER const DdeTopic& )
{
}

void DdeService::RemoveTopic( SAL_UNUSED_PARAMETER const DdeTopic& )
{
}

bool DdeService::MakeTopic( SAL_UNUSED_PARAMETER const OUString& )
{
    return false;
}

const OUString DdeService::GetName() const
{
    return OUString();
}

namespace
{
    struct theDdeServices
        : public rtl::Static< DdeServices, theDdeServices > {};
}

DdeServices& DdeService::GetServices()
{
  return theDdeServices::get();
}

DdeItem::DdeItem( const OUString& )
{
}

DdeItem::DdeItem( const DdeItem& )
{
}

DdeItem::~DdeItem()
{
}

void DdeItem::NotifyClient()
{
}

DdeGetPutItem::DdeGetPutItem( const OUString& rStr )
    : DdeItem( rStr )
{
}

DdeGetPutItem::DdeGetPutItem( const DdeItem& rItem )
    : DdeItem( rItem )
{
}

DdeData* DdeGetPutItem::Get( SAL_UNUSED_PARAMETER sal_uLong )
{
    return NULL;
}

bool DdeGetPutItem::Put( SAL_UNUSED_PARAMETER const DdeData* )
{
    return false;
}

void DdeGetPutItem::AdviseLoop( SAL_UNUSED_PARAMETER bool )
{
}

DdeLink::DdeLink( DdeConnection& rConnection, const OUString& rString, long l )
    : DdeTransaction( rConnection, rString, l )
{
}

DdeLink::~DdeLink()
{
}

void DdeLink::Notify()
{
}

DdeHotLink::DdeHotLink( DdeConnection& rConnection, const OUString& rString, long l )
    : DdeLink( rConnection, rString, l )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
