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
#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#include "TableConnectionData.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

using namespace dbaui;
using namespace comphelper;
//==================================================================
// class OTableConnectionData
//==================================================================
DBG_NAME(OTableConnectionData)
//------------------------------------------------------------------------
OTableConnectionData::OTableConnectionData()
{
    DBG_CTOR(OTableConnectionData,NULL);
    Init();
}
// -----------------------------------------------------------------------------
OTableConnectionData::OTableConnectionData(const TTableWindowData::value_type& _pReferencingTable
                                          ,const TTableWindowData::value_type& _pReferencedTable
                                          ,const String& rConnName )
 :m_pReferencingTable(_pReferencingTable)
 ,m_pReferencedTable(_pReferencedTable)
 ,m_aConnName( rConnName )
{
    DBG_CTOR(OTableConnectionData,NULL);
    Init();
}
//------------------------------------------------------------------------
void OTableConnectionData::Init()
{
    //////////////////////////////////////////////////////////////////////
    // LineDataList mit Defaults initialisieren
    DBG_ASSERT(m_vConnLineData.size() == 0, "OTableConnectionData::Init() : nur mit leere Linienliste aufzurufen !");
    ResetConnLines(sal_True);
        // das legt Defaults an
}
//------------------------------------------------------------------------
OTableConnectionData::OTableConnectionData( const OTableConnectionData& rConnData )
{
    DBG_CTOR(OTableConnectionData,NULL);
    *this = rConnData;
}
//------------------------------------------------------------------------
void OTableConnectionData::CopyFrom(const OTableConnectionData& rSource)
{
    *this = rSource;
    // hier ziehe ich mich auf das (nicht-virtuelle) operator= zurueck, das nur meine Members kopiert
}

//------------------------------------------------------------------------
OTableConnectionData::~OTableConnectionData()
{
    DBG_DTOR(OTableConnectionData,NULL);
    // LineDataList loeschen
    OConnectionLineDataVec().swap(m_vConnLineData);
    //ResetConnLines(sal_False);
}

//------------------------------------------------------------------------
OTableConnectionData& OTableConnectionData::operator=( const OTableConnectionData& rConnData )
{
    if (&rConnData == this)
        return *this;

    m_pReferencingTable = rConnData.m_pReferencingTable;
    m_pReferencedTable = rConnData.m_pReferencedTable;
    m_aConnName = rConnData.GetConnName();

    // clear line list
    ResetConnLines(sal_False);

    // und kopieren
    OConnectionLineDataVec* pLineData = const_cast<OTableConnectionData*>(&rConnData)->GetConnLineDataList();

    OConnectionLineDataVec::const_iterator aIter = pLineData->begin();
    OConnectionLineDataVec::const_iterator aEnd = pLineData->end();
    for(;aIter != aEnd;++aIter)
        m_vConnLineData.push_back(new OConnectionLineData(**aIter));

    return *this;
}

//------------------------------------------------------------------------
sal_Bool OTableConnectionData::SetConnLine( sal_uInt16 nIndex, const String& rSourceFieldName, const String& rDestFieldName )
{
    if (sal_uInt16(m_vConnLineData.size()) < nIndex)
        return sal_False;
        // == ist noch erlaubt, das entspricht einem Append

    if (m_vConnLineData.size() == nIndex)
        return AppendConnLine(rSourceFieldName, rDestFieldName);

    OConnectionLineDataRef pConnLineData = m_vConnLineData[nIndex];
    DBG_ASSERT(pConnLineData != NULL, "OTableConnectionData::SetConnLine : habe ungueltiges LineData-Objekt");

    pConnLineData->SetSourceFieldName( rSourceFieldName );
    pConnLineData->SetDestFieldName( rDestFieldName );

    return sal_True;
}

//------------------------------------------------------------------------
sal_Bool OTableConnectionData::AppendConnLine( const ::rtl::OUString& rSourceFieldName, const ::rtl::OUString& rDestFieldName )
{
    OConnectionLineDataVec::iterator aIter = m_vConnLineData.begin();
    OConnectionLineDataVec::iterator aEnd = m_vConnLineData.end();
    for(;aIter != aEnd;++aIter)
    {
        if((*aIter)->GetDestFieldName() == rDestFieldName && (*aIter)->GetSourceFieldName() == rSourceFieldName)
            break;
    }
    if(aIter == aEnd)
    {
        OConnectionLineDataRef pNew = new OConnectionLineData(rSourceFieldName, rDestFieldName);
        if (!pNew.isValid())
            return sal_False;

        m_vConnLineData.push_back(pNew);
    }
    return sal_True;
}

//------------------------------------------------------------------------
void OTableConnectionData::ResetConnLines( sal_Bool /*bUseDefaults*/ )
{
    OConnectionLineDataVec().swap(m_vConnLineData);
}

//------------------------------------------------------------------------
OConnectionLineDataRef OTableConnectionData::CreateLineDataObj()
{
    return new OConnectionLineData();
}

//------------------------------------------------------------------------
OConnectionLineDataRef OTableConnectionData::CreateLineDataObj( const OConnectionLineData& rConnLineData )
{
    return new OConnectionLineData( rConnLineData );
}
// -----------------------------------------------------------------------------
OTableConnectionData* OTableConnectionData::NewInstance() const
{
    return new OTableConnectionData();
}
// -----------------------------------------------------------------------------
void OTableConnectionData::normalizeLines()
{
    // noch ein wenig Normalisierung auf den LineDatas : leere Lines vom Anfang an das Ende verschieben
    sal_Int32 nCount = m_vConnLineData.size();
    for(sal_Int32 i=0;i<nCount;)
    {
        if(!m_vConnLineData[i]->GetSourceFieldName().getLength() || !m_vConnLineData[i]->GetDestFieldName().getLength())
        {
            OConnectionLineDataRef pData = m_vConnLineData[i];
            m_vConnLineData.erase(m_vConnLineData.begin()+i);
            m_vConnLineData.push_back(pData);
            --nCount;
        }
        else
            ++i;
    }
}
// -----------------------------------------------------------------------------




