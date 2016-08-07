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
#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#include "ConnectionLineData.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


using namespace dbaui;
DBG_NAME(OConnectionLineData)
//==================================================================
//class OConnectionLineData
//==================================================================
//------------------------------------------------------------------------
OConnectionLineData::OConnectionLineData()
{
    DBG_CTOR(OConnectionLineData,NULL);
}

//------------------------------------------------------------------------
OConnectionLineData::OConnectionLineData( const ::rtl::OUString& rSourceFieldName, const ::rtl::OUString& rDestFieldName )
    :m_aSourceFieldName( rSourceFieldName )
    ,m_aDestFieldName( rDestFieldName )
{
    DBG_CTOR(OConnectionLineData,NULL);
}

//------------------------------------------------------------------------
OConnectionLineData::OConnectionLineData( const OConnectionLineData& rConnLineData )
    : ::vos::OReference()
{
    DBG_CTOR(OConnectionLineData,NULL);
    *this = rConnLineData;
}

//------------------------------------------------------------------------
OConnectionLineData::~OConnectionLineData()
{
    DBG_DTOR(OConnectionLineData,NULL);
}

//------------------------------------------------------------------------
void OConnectionLineData::CopyFrom(const OConnectionLineData& rSource)
{
    *this = rSource;
    // hier ziehe ich mich auf das (nicht-virtuelle) operator= zurueck, das nur meine Members kopiert
}

//------------------------------------------------------------------------
OConnectionLineData& OConnectionLineData::operator=( const OConnectionLineData& rConnLineData )
{
    if (&rConnLineData == this)
        return *this;

    m_aSourceFieldName = rConnLineData.GetSourceFieldName();
    m_aDestFieldName = rConnLineData.GetDestFieldName();

    return *this;
}

//------------------------------------------------------------------------
bool OConnectionLineData::Reset()
{
    m_aDestFieldName = m_aSourceFieldName = ::rtl::OUString();
    return true;
}
// -----------------------------------------------------------------------------
namespace dbaui
{
//-------------------------------------------------------------------------
bool operator==(const OConnectionLineData& lhs, const OConnectionLineData& rhs)
{
    return (lhs.m_aSourceFieldName == rhs.m_aSourceFieldName)
        && (lhs.m_aDestFieldName == rhs.m_aDestFieldName);
}
}

