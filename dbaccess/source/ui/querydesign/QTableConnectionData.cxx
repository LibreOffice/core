/*************************************************************************
 *
 *  $RCSfile: QTableConnectionData.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-07-19 09:27:16 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
TYPEINIT1(OQueryTableConnectionData, OTableConnectionData);

//========================================================================
// class OQueryTableConnectionData
//========================================================================
DBG_NAME(OQueryTableConnectionData);
//------------------------------------------------------------------------
OQueryTableConnectionData::OQueryTableConnectionData()
    :OTableConnectionData()
    ,m_eJoinType (INNER_JOIN)
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
}

//------------------------------------------------------------------------
OQueryTableConnectionData::OQueryTableConnectionData(const ::rtl::OUString& strSourceTable, const ::rtl::OUString& strDestTable,
        const ::rtl::OUString& strSourceAlias, const ::rtl::OUString& strDestAlias, const ::rtl::OUString& rConnName)
    :OTableConnectionData( strSourceAlias, strDestAlias, rConnName )
    ,m_strSourceTableName(strSourceTable)
    ,m_strDestTableName(strDestTable)
    ,m_nFromEntryIndex(0)
    ,m_nDestEntryIndex(0)
    ,m_eFromType(TAB_NORMAL_FIELD)
    ,m_eDestType(TAB_NORMAL_FIELD)
    ,m_eJoinType (INNER_JOIN)
{
    DBG_CTOR(OQueryTableConnectionData,NULL);
}

//------------------------------------------------------------------------
OQueryTableConnectionData::~OQueryTableConnectionData()
{
    DBG_DTOR(OQueryTableConnectionData,NULL);
}

//------------------------------------------------------------------------
OConnectionLineData* OQueryTableConnectionData::CreateLineDataObj()
{
    DBG_CHKTHIS(OQueryTableConnectionData,NULL);
    // keine Spezialisierung bei den LineDatas, also eine Instanz der Standard-Klasse
    return new OConnectionLineData();
}

//------------------------------------------------------------------------
OConnectionLineData* OQueryTableConnectionData::CreateLineDataObj( const OConnectionLineData& rConnLineData )
{
    DBG_CHKTHIS(OQueryTableConnectionData,NULL);
    return new OConnectionLineData( rConnLineData );
}

//------------------------------------------------------------------------
void OQueryTableConnectionData::CopyFrom(const OTableConnectionData& rSource)
{
    DBG_CHKTHIS(OQueryTableConnectionData,NULL);
    DBG_ASSERT(rSource.ISA(OQueryTableConnectionData), "QueryTabConn::CopyFrom : ungueltiger Parameter !");
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

    return *this;
}

//------------------------------------------------------------------------------
::rtl::OUString OQueryTableConnectionData::GetTableName(EConnectionSide nWhich) const
{
    DBG_CHKTHIS(OQueryTableConnectionData,NULL);
    return nWhich == JTCS_FROM ? m_strSourceTableName : m_strDestTableName;
}

//------------------------------------------------------------------------------
::rtl::OUString OQueryTableConnectionData::GetAliasName(EConnectionSide nWhich) const
{
    DBG_CHKTHIS(OQueryTableConnectionData,NULL);
    return nWhich == JTCS_FROM ? GetSourceWinName() : GetDestWinName();
}

//------------------------------------------------------------------------------
void OQueryTableConnectionData::InitFromDrag(const OTableFieldDesc& rDragLeft, const OTableFieldDesc& rDragRight)
{
    DBG_CHKTHIS(OQueryTableConnectionData,NULL);
    // die Infos in rDrag in Parameter fuer das Basisklassen-Init umsetzen ...
    OQueryTableWindow* pSourceWin = static_cast<OQueryTableWindow*>(rDragLeft.GetTabWindow());
    OQueryTableWindow* pDestWin = static_cast<OQueryTableWindow*>(rDragRight.GetTabWindow());
    OSL_ENSURE(pSourceWin,"NO Source window found!");
    OSL_ENSURE(pDestWin,"NO Dest window found!");

    Init(pSourceWin->GetWinName(), pDestWin->GetWinName());

    // und dann meine Members setzen
    SetFieldIndex(JTCS_FROM, rDragLeft.GetFieldIndex());
    SetFieldIndex(JTCS_TO, rDragRight.GetFieldIndex());

    SetFieldType(JTCS_FROM, rDragLeft.GetFieldType());
    SetFieldType(JTCS_TO, rDragRight.GetFieldType());

    m_strSourceTableName = pSourceWin->GetTableName();
    m_aSourceWinName = pSourceWin->GetWinName();

    m_strDestTableName = pDestWin->GetTableName();
    m_aDestWinName = pDestWin->GetWinName();

    AppendConnLine((::rtl::OUString)rDragLeft.GetField(),(::rtl::OUString)rDragRight.GetField());
}
// -----------------------------------------------------------------------------
OTableConnectionData* OQueryTableConnectionData::NewInstance() const
{
    return new OQueryTableConnectionData();
}
// -----------------------------------------------------------------------------




