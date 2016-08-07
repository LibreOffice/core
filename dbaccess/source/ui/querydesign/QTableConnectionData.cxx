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
#include "precompiled_dbui.hxx"
#ifndef DBAUI_QTABLECONNECTIONDATA_HXX
#include "QTableConnectionData.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_QTABLECONNECTIONDATA_HXX
#include "QTableConnectionData.hxx"
#endif
#ifndef DBAUI_QUERY_TABLEWINDOWDATA_HXX
#include "QTableWindow.hxx"
#endif

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




