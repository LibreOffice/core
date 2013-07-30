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

#include "xedbdata.hxx"
#include "document.hxx"

XclExpXmlDBDataStyleInfo::XclExpXmlDBDataStyleInfo( const XclExpRoot& rRoot, ScDBDataFormatting& rDBDataFormatting )
    : XclExpRoot( rRoot )
{
}

void XclExpXmlDBDataStyleInfo::SaveXml( XclExpXmlStream& rStrm )
{
}

// ============================================================================
XclExpXmlDBDataColumn::XclExpXmlDBDataColumn( const XclExpRoot& rRoot, int iID, OUString& rName )
    : XclExpRoot( rRoot )
{
}

void XclExpXmlDBDataColumn::SaveXml( XclExpXmlStream& rStrm )
{
}

// ============================================================================
XclExpXmlDBDataColumns::XclExpXmlDBDataColumns( const XclExpRoot& rRoot, ScDBData& rDBData )
    : XclExpRoot( rRoot )
{
}

void XclExpXmlDBDataColumns::SaveXml( XclExpXmlStream& rStrm )
{
}

// ============================================================================
XclExpXmlDBDataTable::XclExpXmlDBDataTable(const XclExpRoot& rRoot, ScDBData& rDBData )
    : XclExpRoot( rRoot )
{
    maTableColumns.reset( new XclExpXmlDBDataColumns( rRoot, rDBData ) );
    ScDBDataFormatting aDBFormatting;
    rDBData.GetTableFormatting( aDBFormatting );
    maStyleInfo.reset( new XclExpXmlDBDataStyleInfo( rRoot, aDBFormatting) );
}
void XclExpXmlDBDataTable::SaveXml( XclExpXmlStream& rStrm )
{
}

// =============================================================================
XclExpXmlDBDataTables::XclExpXmlDBDataTables( const XclExpRoot& rRoot )
    : XclExpRoot( rRoot )
{
    ScDBCollection* pDBCollection = rRoot.GetDoc().GetDBCollection();
    if( pDBCollection )
    {
        ScDBCollection::NamedDBs& aNamedDBs = pDBCollection->getNamedDBs();
        ScDBCollection::NamedDBs::iterator itr = aNamedDBs.begin();
        ScDBCollection::NamedDBs::iterator itrEnd = aNamedDBs.end();
        for(; itr!= itrEnd; ++itr)
        {
            maDBDataTableContainer.push_back( new XclExpXmlDBDataTable( rRoot, *itr ) );
            ++miCount;
        }
    }
}

void XclExpXmlDBDataTables::SaveXml( XclExpXmlStream& rStrm )
{
}
// =============================================================================
