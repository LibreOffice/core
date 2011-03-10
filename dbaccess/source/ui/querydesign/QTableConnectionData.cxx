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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "QTableConnectionData.hxx"
#include <tools/debug.hxx>
#include "QTableConnectionData.hxx"
#include "QTableWindow.hxx"

using namespace dbaui;

//========================================================================
// class OQueryTableConnectionData
//========================================================================
DBG_NAME(OQueryTableConnectionData)
//------------------------------------------------------------------------
OQueryTableConnectionData::OQueryTableConnectionData()
    :OTableConnectionData()
    ,m_eJoinType (INNER_JOIN)
    ,m_bNatural(false)
{
    DBG_CTOR(OQueryTableConnectionData,NULL);
}

//------------------------------------------------------------------------
OQueryTableConnectionData::OQueryTableConnectionData( const OQueryTableConnectionData& rConnData )
    :OTableConnectionData( rConnData )
{
    DBG_CTOR(OQueryTableConnectionData,NULL);
    m_nFromEntryIndex = rConnData.m_nFromEntryIndex;
    m_nDestEntryIndex = rConnData.m_nDestEntryIndex;

    m_eFromType = rConnData.m_eFromType;
    m_eDestType = rConnData.m_eDestType;
    m_eJoinType = rConnData.m_eJoinType;
    m_bNatural  = rConnData.m_bNatural;
}

//------------------------------------------------------------------------
OQueryTableConnectionData::OQueryTableConnectionData(const TTableWindowData::value_type& _pReferencingTable
                                                    ,const TTableWindowData::value_type& _pReferencedTable
                                                    ,const ::rtl::OUString& rConnName)
    :OTableConnectionData( _pReferencingTable,_pReferencedTable, rConnName )
    ,m_nFromEntryIndex(0)
    ,m_nDestEntryIndex(0)
    ,m_eJoinType (INNER_JOIN)
    ,m_bNatural(false)
    ,m_eFromType(TAB_NORMAL_FIELD)
    ,m_eDestType(TAB_NORMAL_FIELD)
{
    DBG_CTOR(OQueryTableConnectionData,NULL);
}

//------------------------------------------------------------------------
OQueryTableConnectionData::~OQueryTableConnectionData()
{
    DBG_DTOR(OQueryTableConnectionData,NULL);
}

//------------------------------------------------------------------------
OConnectionLineDataRef OQueryTableConnectionData::CreateLineDataObj()
{
    DBG_CHKTHIS(OQueryTableConnectionData,NULL);
    // keine Spezialisierung bei den LineDatas, also eine Instanz der Standard-Klasse
    return new OConnectionLineData();
}

//------------------------------------------------------------------------
OConnectionLineDataRef OQueryTableConnectionData::CreateLineDataObj( const OConnectionLineData& rConnLineData )
{
    DBG_CHKTHIS(OQueryTableConnectionData,NULL);
    return new OConnectionLineData( rConnLineData );
}

//------------------------------------------------------------------------
void OQueryTableConnectionData::CopyFrom(const OTableConnectionData& rSource)
{
    DBG_CHKTHIS(OQueryTableConnectionData,NULL);
    // wie in der Basisklasse zurueckziehen auf das (nicht-virtuelle) operator=
    *this = (const OQueryTableConnectionData&)rSource;
}

//------------------------------------------------------------------------
OQueryTableConnectionData& OQueryTableConnectionData::operator=(const OQueryTableConnectionData& rConnData)
{
    DBG_CHKTHIS(OQueryTableConnectionData,NULL);
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

//------------------------------------------------------------------------------
::rtl::OUString OQueryTableConnectionData::GetAliasName(EConnectionSide nWhich) const
{
    DBG_CHKTHIS(OQueryTableConnectionData,NULL);
    return nWhich == JTCS_FROM ? m_pReferencingTable->GetWinName() : m_pReferencedTable->GetWinName();
}

//------------------------------------------------------------------------------
void OQueryTableConnectionData::InitFromDrag(const OTableFieldDescRef& rDragLeft, const OTableFieldDescRef& rDragRight)
{
    DBG_CHKTHIS(OQueryTableConnectionData,NULL);
    // die Infos in rDrag in Parameter fuer das Basisklassen-Init umsetzen ...
    OQueryTableWindow* pSourceWin = static_cast<OQueryTableWindow*>(rDragLeft->GetTabWindow());
    OQueryTableWindow* pDestWin = static_cast<OQueryTableWindow*>(rDragRight->GetTabWindow());
    OSL_ENSURE(pSourceWin,"NO Source window found!");
    OSL_ENSURE(pDestWin,"NO Dest window found!");
    m_pReferencingTable = pSourceWin->GetData();
    m_pReferencedTable  = pDestWin->GetData();

    // und dann meine Members setzen
    SetFieldIndex(JTCS_FROM, rDragLeft->GetFieldIndex());
    SetFieldIndex(JTCS_TO, rDragRight->GetFieldIndex());

    SetFieldType(JTCS_FROM, rDragLeft->GetFieldType());
    SetFieldType(JTCS_TO, rDragRight->GetFieldType());

    AppendConnLine((::rtl::OUString)rDragLeft->GetField(),(::rtl::OUString)rDragRight->GetField());
}
// -----------------------------------------------------------------------------
OTableConnectionData* OQueryTableConnectionData::NewInstance() const
{
    return new OQueryTableConnectionData();
}
// -----------------------------------------------------------------------------
sal_Bool OQueryTableConnectionData::Update()
{
    return sal_True;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
