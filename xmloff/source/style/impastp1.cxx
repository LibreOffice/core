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

#include <tools/debug.hxx>
#include "impastpl.hxx"

using ::rtl::OUString;

//#############################################################################
//
// Class XMLFamilyData_Impl
//

///////////////////////////////////////////////////////////////////////////////
//
// ctor/dtor class XMLFamilyData_Impl
//

XMLFamilyData_Impl::XMLFamilyData_Impl(
        sal_Int32 nFamily,
        const ::rtl::OUString& rStrName,
        const UniReference < SvXMLExportPropertyMapper > &rMapper,
        const OUString& rStrPrefix,
        sal_Bool bAsFam )
    : pCache( 0 ), mnFamily( nFamily ), maStrFamilyName( rStrName), mxMapper( rMapper ),
      mnCount( 0 ), mnName( 0 ), maStrPrefix( rStrPrefix ), bAsFamily( bAsFam )

{
    mpParentList = new SvXMLAutoStylePoolParentsP_Impl( 5, 5 );
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
    mpParentList = new SvXMLAutoStylePoolParentsP_Impl( 5, 5 );
    DBG_ASSERT( !pCache || !pCache->size(), "auto style pool cache is not empty!" );
    if( pCache )
    {
        for ( size_t i = 0, n = pCache->size(); i < n; ++i )
            delete (*pCache)[ i ];
        pCache->clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
