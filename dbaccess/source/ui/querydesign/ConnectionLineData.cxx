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
#include "ConnectionLineData.hxx"
#include <tools/debug.hxx>


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
    : ::salhelper::SimpleReferenceObject()
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
