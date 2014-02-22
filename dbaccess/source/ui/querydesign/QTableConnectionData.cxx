/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "QTableConnectionData.hxx"
#include <tools/debug.hxx>
#include "QTableWindow.hxx"

using namespace dbaui;


OQueryTableConnectionData::OQueryTableConnectionData()
    : OTableConnectionData()
    , m_nFromEntryIndex(0)
    , m_nDestEntryIndex(0)
    , m_eJoinType (INNER_JOIN)
    , m_bNatural(false)
    , m_eFromType(TAB_NORMAL_FIELD)
    , m_eDestType(TAB_NORMAL_FIELD)
{
}

OQueryTableConnectionData::OQueryTableConnectionData( const OQueryTableConnectionData& rConnData )
    : OTableConnectionData( rConnData )
    , m_nFromEntryIndex(rConnData.m_nFromEntryIndex)
    , m_nDestEntryIndex(rConnData.m_nDestEntryIndex)
    , m_eJoinType(rConnData.m_eJoinType)
    , m_bNatural(rConnData.m_bNatural)
    , m_eFromType(rConnData.m_eFromType)
    , m_eDestType(rConnData.m_eDestType)
{

}

OQueryTableConnectionData::OQueryTableConnectionData(const TTableWindowData::value_type& _pReferencingTable,
                                                     const TTableWindowData::value_type& _pReferencedTable,
                                                     const OUString& rConnName)
    : OTableConnectionData( _pReferencingTable,_pReferencedTable, rConnName )
    , m_nFromEntryIndex(0)
    , m_nDestEntryIndex(0)
    , m_eJoinType (INNER_JOIN)
    , m_bNatural(false)
    , m_eFromType(TAB_NORMAL_FIELD)
    , m_eDestType(TAB_NORMAL_FIELD)
{
}

OQueryTableConnectionData::~OQueryTableConnectionData()
{
}

OConnectionLineDataRef OQueryTableConnectionData::CreateLineDataObj()
{
    
    return new OConnectionLineData();
}

OConnectionLineDataRef OQueryTableConnectionData::CreateLineDataObj( const OConnectionLineData& rConnLineData )
{
    return new OConnectionLineData( rConnLineData );
}

void OQueryTableConnectionData::CopyFrom(const OTableConnectionData& rSource)
{
    
    *this = (const OQueryTableConnectionData&)rSource;
}

OQueryTableConnectionData& OQueryTableConnectionData::operator=(const OQueryTableConnectionData& rConnData)
{
    if (&rConnData == this)
        return *this;

    OTableConnectionData::operator=(rConnData);

    m_nFromEntryIndex = rConnData.m_nFromEntryIndex;
    m_nDestEntryIndex = rConnData.m_nDestEntryIndex;

    m_eFromType = rConnData.m_eFromType;
    m_eDestType = rConnData.m_eDestType;
    m_eJoinType = rConnData.m_eJoinType;
    m_bNatural  = rConnData.m_bNatural;

    return *this;
}

OUString OQueryTableConnectionData::GetAliasName(EConnectionSide nWhich) const
{
    return nWhich == JTCS_FROM ? m_pReferencingTable->GetWinName() : m_pReferencedTable->GetWinName();
}

void OQueryTableConnectionData::InitFromDrag(const OTableFieldDescRef& rDragLeft, const OTableFieldDescRef& rDragRight)
{
    
    OQueryTableWindow* pSourceWin = static_cast<OQueryTableWindow*>(rDragLeft->GetTabWindow());
    OQueryTableWindow* pDestWin = static_cast<OQueryTableWindow*>(rDragRight->GetTabWindow());
    OSL_ENSURE(pSourceWin,"NO Source window found!");
    OSL_ENSURE(pDestWin,"NO Dest window found!");
    m_pReferencingTable = pSourceWin->GetData();
    m_pReferencedTable  = pDestWin->GetData();

    
    SetFieldIndex(JTCS_FROM, rDragLeft->GetFieldIndex());
    SetFieldIndex(JTCS_TO, rDragRight->GetFieldIndex());

    SetFieldType(JTCS_FROM, rDragLeft->GetFieldType());
    SetFieldType(JTCS_TO, rDragRight->GetFieldType());

    AppendConnLine((OUString)rDragLeft->GetField(),(OUString)rDragRight->GetField());
}

OTableConnectionData* OQueryTableConnectionData::NewInstance() const
{
    return new OQueryTableConnectionData();
}

sal_Bool OQueryTableConnectionData::Update()
{
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
