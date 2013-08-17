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

#include "ConnectionLineData.hxx"
#include <tools/debug.hxx>

using namespace dbaui;
DBG_NAME(OConnectionLineData)
//class OConnectionLineData
OConnectionLineData::OConnectionLineData()
{
    DBG_CTOR(OConnectionLineData,NULL);
}

OConnectionLineData::OConnectionLineData( const OUString& rSourceFieldName, const OUString& rDestFieldName )
    :m_aSourceFieldName( rSourceFieldName )
    ,m_aDestFieldName( rDestFieldName )
{
    DBG_CTOR(OConnectionLineData,NULL);
}

OConnectionLineData::OConnectionLineData( const OConnectionLineData& rConnLineData )
    : ::salhelper::SimpleReferenceObject()
{
    DBG_CTOR(OConnectionLineData,NULL);
    *this = rConnLineData;
}

OConnectionLineData::~OConnectionLineData()
{
    DBG_DTOR(OConnectionLineData,NULL);
}

void OConnectionLineData::CopyFrom(const OConnectionLineData& rSource)
{
    *this = rSource;
    // Here I rely on the (non-virtual) operator=, which only copies my members
}

OConnectionLineData& OConnectionLineData::operator=( const OConnectionLineData& rConnLineData )
{
    if (&rConnLineData == this)
        return *this;

    m_aSourceFieldName = rConnLineData.GetSourceFieldName();
    m_aDestFieldName = rConnLineData.GetDestFieldName();

    return *this;
}

bool OConnectionLineData::Reset()
{
    m_aDestFieldName = m_aSourceFieldName = OUString();
    return true;
}

namespace dbaui
{
bool operator==(const OConnectionLineData& lhs, const OConnectionLineData& rhs)
{
    return (lhs.m_aSourceFieldName == rhs.m_aSourceFieldName)
        && (lhs.m_aDestFieldName == rhs.m_aDestFieldName);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
