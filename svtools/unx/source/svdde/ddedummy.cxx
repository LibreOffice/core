/*************************************************************************
 *
 *  $RCSfile: ddedummy.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVDDE_HXX
#include <svdde.hxx>
#endif

static String aString;

DdeData::DdeData()
{
}

DdeData::DdeData( const String& )
{
}

DdeData::DdeData( const DdeData& )
{
}

DdeData::DdeData( const void*, long, ULONG)
{
}

DdeData::~DdeData( void )
{
}

void DdeData::SetFormat( ULONG )
{
}

ULONG DdeData::GetFormat() const
{
  return 0L;
}

DdeData& DdeData::operator = ( const DdeData& rDdeData )
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
  return aString;
}

const String& DdeConnection::GetTopicName()
{
  return aString;
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

void DdeTransaction::Done( BOOL bDataValid )
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

DdeData* DdeTopic::Get( ULONG )
{
  return NULL;
}

BOOL DdeTopic::MakeItem( const String& )
{
  return FALSE;
}

BOOL DdeTopic::StartAdviseLoop()
{
  return FALSE;
}

BOOL DdeTopic::StopAdviseLoop()
{
  return FALSE;
}

BOOL DdeTopic::Execute( const String* )
{
  return FALSE;
}

BOOL DdeTopic::Put( const DdeData* )
{
  return FALSE;
}

const String& DdeTopic::GetName() const
{
  return aString;
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

BOOL DdeService::SysTopicExecute(const String*) {
    return FALSE;
}

DdeService::~DdeService()
{
}

BOOL DdeService::IsBusy()
{
  return FALSE;
}

String DdeService::GetHelp()
{
  return aString;
}

void DdeService::AddFormat( ULONG )
{
}

void DdeService::AddTopic( const DdeTopic& )
{
}

void DdeService::RemoveTopic( const DdeTopic& )
{
}

BOOL DdeService::MakeTopic( const String& )
{
  return FALSE;
}

const String& DdeService::GetName() const
{
  return aString;
}

static DdeServices aDdeServices;
DdeServices& DdeService::GetServices()
{
  return aDdeServices;
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

DdeData* DdeGetPutItem::Get( ULONG )
{
  return NULL;
}

BOOL DdeGetPutItem::Put( const DdeData* )
{
  return FALSE;
}

void DdeGetPutItem::AdviseLoop( BOOL )
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
