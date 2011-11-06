/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"
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

DdeData::~DdeData( void )
{
}

void DdeData::SetFormat( sal_uLong )
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

DdeConnection::~DdeConnection( void )
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

void DdeTransaction::Execute(void)
{
}

void DdeTransaction::Done( sal_Bool )
{
}

void DdeTransaction::Data( const DdeData* )
{
}

DdeTransaction::~DdeTransaction(void)
{
}

DdeRequest::DdeRequest(DdeConnection& rConnection, const String& rString, long lLong ) :
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

void DdeTopic::Connect (long )
{
}

void DdeTopic::Disconnect( long )
{
}

void DdeTopic::InsertItem( DdeItem* )
{
}

DdeItem* DdeTopic::AddItem( const DdeItem& rDdeItem )
{
  return (DdeItem*) &rDdeItem;
}

void DdeTopic::RemoveItem( const DdeItem& )
{
}

DdeData* DdeTopic::Get( sal_uLong )
{
  return NULL;
}

sal_Bool DdeTopic::MakeItem( const String& )
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

sal_Bool DdeTopic::Execute( const String* )
{
  return sal_False;
}

sal_Bool DdeTopic::Put( const DdeData* )
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

sal_Bool DdeService::SysTopicExecute(const String*) {
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

void DdeService::AddFormat( sal_uLong )
{
}

void DdeService::AddTopic( const DdeTopic& )
{
}

void DdeService::RemoveTopic( const DdeTopic& )
{
}

sal_Bool DdeService::MakeTopic( const String& )
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

DdeData* DdeGetPutItem::Get( sal_uLong )
{
  return NULL;
}

sal_Bool DdeGetPutItem::Put( const DdeData* )
{
  return sal_False;
}

void DdeGetPutItem::AdviseLoop( sal_Bool )
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
