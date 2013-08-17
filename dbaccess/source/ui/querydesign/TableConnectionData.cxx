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

#include "TableConnectionData.hxx"
#include <tools/debug.hxx>
#include <osl/diagnose.h>
#include <comphelper/stl_types.hxx>

using namespace dbaui;
using namespace comphelper;
// class OTableConnectionData
DBG_NAME(OTableConnectionData)
OTableConnectionData::OTableConnectionData()
{
    DBG_CTOR(OTableConnectionData,NULL);
    Init();
}

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

void OTableConnectionData::Init()
{
    // initialise linedatalist with defaults
    OSL_ENSURE(m_vConnLineData.empty(), "OTableConnectionData::Init() : nur mit leere Linienliste aufzurufen !");
    ResetConnLines();
        // this creates the defaults
}

OTableConnectionData::OTableConnectionData( const OTableConnectionData& rConnData )
{
    DBG_CTOR(OTableConnectionData,NULL);
    *this = rConnData;
}

void OTableConnectionData::CopyFrom(const OTableConnectionData& rSource)
{
    *this = rSource;
    // here I revert to the (non-virtual) operator =, which only copies my members
}

OTableConnectionData::~OTableConnectionData()
{
    DBG_DTOR(OTableConnectionData,NULL);
    // delete LineDataList
    OConnectionLineDataVec().swap(m_vConnLineData);
}

OTableConnectionData& OTableConnectionData::operator=( const OTableConnectionData& rConnData )
{
    if (&rConnData == this)
        return *this;

    m_pReferencingTable = rConnData.m_pReferencingTable;
    m_pReferencedTable = rConnData.m_pReferencedTable;
    m_aConnName = rConnData.GetConnName();

    // clear line list
    ResetConnLines();

    // and copy
    OConnectionLineDataVec* pLineData = const_cast<OTableConnectionData*>(&rConnData)->GetConnLineDataList();

    OConnectionLineDataVec::const_iterator aIter = pLineData->begin();
    OConnectionLineDataVec::const_iterator aEnd = pLineData->end();
    for(;aIter != aEnd;++aIter)
        m_vConnLineData.push_back(new OConnectionLineData(**aIter));

    return *this;
}

sal_Bool OTableConnectionData::SetConnLine( sal_uInt16 nIndex, const String& rSourceFieldName, const String& rDestFieldName )
{
    if (sal_uInt16(m_vConnLineData.size()) < nIndex)
        return sal_False;

        // == still allowed, this correponds to a Append

    if (m_vConnLineData.size() == nIndex)
        return AppendConnLine(rSourceFieldName, rDestFieldName);

    OConnectionLineDataRef pConnLineData = m_vConnLineData[nIndex];
    OSL_ENSURE(pConnLineData != NULL, "OTableConnectionData::SetConnLine : habe ungueltiges LineData-Objekt");

    pConnLineData->SetSourceFieldName( rSourceFieldName );
    pConnLineData->SetDestFieldName( rDestFieldName );

    return sal_True;
}

sal_Bool OTableConnectionData::AppendConnLine( const OUString& rSourceFieldName, const OUString& rDestFieldName )
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
        if (!pNew.is())
            return sal_False;

        m_vConnLineData.push_back(pNew);
    }
    return sal_True;
}

void OTableConnectionData::ResetConnLines()
{
    OConnectionLineDataVec().swap(m_vConnLineData);
}

OConnectionLineDataRef OTableConnectionData::CreateLineDataObj()
{
    return new OConnectionLineData();
}

OConnectionLineDataRef OTableConnectionData::CreateLineDataObj( const OConnectionLineData& rConnLineData )
{
    return new OConnectionLineData( rConnLineData );
}

OTableConnectionData* OTableConnectionData::NewInstance() const
{
    return new OTableConnectionData();
}

OConnectionLineDataVec::size_type OTableConnectionData::normalizeLines()
{
    // remove empty lines
    OConnectionLineDataVec::size_type nCount = m_vConnLineData.size();
    OConnectionLineDataVec::size_type nRet = nCount;
    for(OConnectionLineDataVec::size_type i = 0; i < nCount;)
    {
        if(m_vConnLineData[i]->GetSourceFieldName().isEmpty() && m_vConnLineData[i]->GetDestFieldName().isEmpty())
        {
            OConnectionLineDataRef pData = m_vConnLineData[i];
            m_vConnLineData.erase(m_vConnLineData.begin()+i);
            --nCount;
            if (i < nRet)
                nRet=i;
        }
        else
            ++i;
    }
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
