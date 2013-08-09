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

#include <tools/debug.hxx>
#include "impastpl.hxx"

// Class XMLFamilyData_Impl
// ctor/dtor class XMLFamilyData_Impl

XMLFamilyData_Impl::XMLFamilyData_Impl(
        sal_Int32 nFamily,
        const OUString& rStrName,
        const UniReference < SvXMLExportPropertyMapper > &rMapper,
        const OUString& rStrPrefix,
        sal_Bool bAsFam )
    : pCache( 0 ), mnFamily( nFamily ), maStrFamilyName( rStrName), mxMapper( rMapper ),
      mnCount( 0 ), mnName( 0 ), maStrPrefix( rStrPrefix ), bAsFamily( bAsFam )

{
    mpParentList = new SvXMLAutoStylePoolParentsP_Impl;
    mpNameList   = new SvXMLAutoStylePoolNamesP_Impl;
}

XMLFamilyData_Impl::~XMLFamilyData_Impl()
{
    delete mpParentList;
    delete mpNameList;
    DBG_ASSERT( !pCache || !pCache->size(), "auto style pool cache is not empty!" );
    if( pCache )
    {
        for ( size_t i = 0, n = pCache->size(); i < n; ++i )
            delete (*pCache)[ i ];
        pCache->clear();
        delete pCache;
    }
}

void XMLFamilyData_Impl::ClearEntries()
{
    delete mpParentList;
    mpParentList = new SvXMLAutoStylePoolParentsP_Impl;
    DBG_ASSERT( !pCache || !pCache->size(), "auto style pool cache is not empty!" );
    if( pCache )
    {
        for ( size_t i = 0, n = pCache->size(); i < n; ++i )
            delete (*pCache)[ i ];
        pCache->clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
