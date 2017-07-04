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

#include "QTableConnectionData.hxx"
#include "QTableWindow.hxx"

using namespace dbaui;

// class OQueryTableConnectionData
OQueryTableConnectionData::OQueryTableConnectionData()
    : OTableConnectionData()
    , m_nFromEntryIndex(0)
    , m_nDestEntryIndex(0)
    , m_eJoinType (INNER_JOIN)
    , m_bNatural(false)
{
}

OQueryTableConnectionData::OQueryTableConnectionData( const OQueryTableConnectionData& rConnData )
    : OTableConnectionData( rConnData )
    , m_nFromEntryIndex(rConnData.m_nFromEntryIndex)
    , m_nDestEntryIndex(rConnData.m_nDestEntryIndex)
    , m_eJoinType(rConnData.m_eJoinType)
    , m_bNatural(rConnData.m_bNatural)
{
}

OQueryTableConnectionData::OQueryTableConnectionData(const TTableWindowData::value_type& _pReferencingTable,
                                                     const TTableWindowData::value_type& _pReferencedTable)
    : OTableConnectionData( _pReferencingTable,_pReferencedTable )
    , m_nFromEntryIndex(0)
    , m_nDestEntryIndex(0)
    , m_eJoinType (INNER_JOIN)
    , m_bNatural(false)
{
}

OQueryTableConnectionData::~OQueryTableConnectionData()
{
}

OConnectionLineDataRef OQueryTableConnectionData::CreateLineDataObj()
{
    // no specializing of LineDatas, so it is an instance of standard class
    return new OConnectionLineData();
}

void OQueryTableConnectionData::CopyFrom(const OTableConnectionData& rSource)
{
    // same as in base class, use of (non-virtual) operator=
    *this = static_cast<const OQueryTableConnectionData&>(rSource);
}

OQueryTableConnectionData& OQueryTableConnectionData::operator=(const OQueryTableConnectionData& rConnData)
{
    if (&rConnData == this)
        return *this;

    OTableConnectionData::operator=(rConnData);

    m_nFromEntryIndex = rConnData.m_nFromEntryIndex;
    m_nDestEntryIndex = rConnData.m_nDestEntryIndex;

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
    // convert Information in rDrag into parameters for the base class init
    OQueryTableWindow* pSourceWin = static_cast<OQueryTableWindow*>(rDragLeft->GetTabWindow());
    OQueryTableWindow* pDestWin = static_cast<OQueryTableWindow*>(rDragRight->GetTabWindow());
    OSL_ENSURE(pSourceWin,"NO Source window found!");
    OSL_ENSURE(pDestWin,"NO Dest window found!");
    m_pReferencingTable = pSourceWin->GetData();
    m_pReferencedTable  = pDestWin->GetData();

    // set members
    SetFieldIndex(JTCS_FROM, rDragLeft->GetFieldIndex());
    SetFieldIndex(JTCS_TO, rDragRight->GetFieldIndex());

    AppendConnLine(rDragLeft->GetField(), rDragRight->GetField());
}

OTableConnectionData* OQueryTableConnectionData::NewInstance() const
{
    return new OQueryTableConnectionData();
}

bool OQueryTableConnectionData::Update()
{
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
