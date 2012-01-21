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

#include <svl/svdde.hxx>
#include <rtl/instance.hxx>

DdeData::DdeData()
{
}

DdeData::DdeData( const String& )
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

DdeConnection::DdeConnection( const String&, const String& )
{
}

DdeConnection::~DdeConnection()
{
}

const String& DdeConnection::GetServiceName()
{
  return String::EmptyString();
}

const String& DdeConnection::GetTopicName()
{
  return String::EmptyString();
}

DdeTransaction::DdeTransaction( DdeConnection& rConnection, const String&, long ) :
     rDde( rConnection )
{
}

DdeTransaction::DdeTransaction( const DdeTransaction& rTransaction ) :
     rDde( rTransaction.rDde )
{
}

void DdeTransaction::Execute()
{
}

void DdeTransaction::Done( SAL_UNUSED_PARAMETER sal_Bool )
{
}

void DdeTransaction::Data( SAL_UNUSED_PARAMETER const DdeData* )
{
}

DdeTransaction::~DdeTransaction()
{
}

DdeRequest::DdeRequest( DdeConnection& rConnection, const String& rString, long lLong ) :
     DdeTransaction( rConnection, rString, lLong )
{
}

DdeExecute::DdeExecute( DdeConnection& rConnection, const String& rString, long lLong ) :
     DdeTransaction( rConnection, rString, lLong )
{
}

DdePoke::DdePoke( DdeConnection& rConnection, const String& rString, const DdeData&, long lLong ) :
     DdeTransaction( rConnection, rString, lLong )
{
}


DdeTopic::DdeTopic( const String& )
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

sal_Bool DdeTopic::MakeItem( SAL_UNUSED_PARAMETER const String& )
{
  return sal_False;
}

sal_Bool DdeTopic::StartAdviseLoop()
{
  return sal_False;
}

sal_Bool DdeTopic::StopAdviseLoop()
{
  return sal_False;
}

sal_Bool DdeTopic::Execute( SAL_UNUSED_PARAMETER const String* )
{
  return sal_False;
}

sal_Bool DdeTopic::Put( SAL_UNUSED_PARAMETER const DdeData* )
{
  return sal_False;
}

const String& DdeTopic::GetName() const
{
  return String::EmptyString();
}

DdeService::DdeService( const String& )
{
  nStatus = 0;
}

String DdeService::Topics() {
    return String();
}

String DdeService::Formats() {
    return String();
}

String DdeService::SysItems() {
    return String();
}

String DdeService::Status() {
    return String();
}

String DdeService::SysTopicGet(const String& rString) {
    return rString;
}

sal_Bool DdeService::SysTopicExecute(SAL_UNUSED_PARAMETER const String*) {
    return sal_False;
}

DdeService::~DdeService()
{
}

sal_Bool DdeService::IsBusy()
{
  return sal_False;
}

String DdeService::GetHelp()
{
  return String::EmptyString();
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

sal_Bool DdeService::MakeTopic( SAL_UNUSED_PARAMETER const String& )
{
  return sal_False;
}

const String& DdeService::GetName() const
{
  return String::EmptyString();
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

DdeItem::DdeItem( const String& )
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

DdeGetPutItem::DdeGetPutItem( const String& rStr ) :
DdeItem( rStr )
{
}

DdeGetPutItem::DdeGetPutItem( const DdeItem& rItem ) :
DdeItem( rItem )
{
}

DdeData* DdeGetPutItem::Get( SAL_UNUSED_PARAMETER sal_uLong )
{
  return NULL;
}

sal_Bool DdeGetPutItem::Put( SAL_UNUSED_PARAMETER const DdeData* )
{
  return sal_False;
}

void DdeGetPutItem::AdviseLoop( SAL_UNUSED_PARAMETER sal_Bool )
{
}

DdeLink::DdeLink( DdeConnection& rConnection, const String& rString, long l ) :
DdeTransaction( rConnection, rString, l )
{
}

DdeLink::~DdeLink()
{
}

void DdeLink::Notify()
{
}

DdeHotLink::DdeHotLink( DdeConnection& rConnection, const String& rString, long l ) :
DdeLink( rConnection, rString, l )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
