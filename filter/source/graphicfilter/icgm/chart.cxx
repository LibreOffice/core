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


#include <main.hxx>
#include <chart.hxx>

// ---------------------------------------------------------------

CGMChart::CGMChart( CGM& rCGM ) :
    mpCGM       ( &rCGM )
{
    for ( sal_Int8 i = 0; i < 7; i++ )
    {
        mDataNode[ i ].nBoxX1 = mDataNode[ i ].nBoxY1 = 0 ;
        mDataNode[ i ].nBoxX2 = mDataNode[ i ].nBoxY2 = 0 ;

        mDataNode[ i ].nZoneEnum = i;
    }
};

// ---------------------------------------------------------------

CGMChart::~CGMChart()
{
    // delete the whole textentry structure

    while( !maTextEntryList.empty() )
    {
        DeleteTextEntry( maTextEntryList[ 0 ] );
    }
};

// ---------------------------------------------------------------

void CGMChart::DeleteTextEntry( TextEntry* pTextEntry )
{
    if ( pTextEntry )
    {
        delete pTextEntry->pText;
        for ( TextAttribute* pTAttr = pTextEntry->pAttribute; pTAttr != NULL ; )
        {
            TextAttribute* pTempTAttr = pTAttr;
            pTAttr = pTAttr->pNextAttribute;
            delete pTempTAttr;
        }
        ::std::vector< TextEntry* >::iterator it;
        for ( it = maTextEntryList.begin(); it != maTextEntryList.end(); ++it )
        {
            if ( *it == pTextEntry )
            {
                maTextEntryList.erase( it );
                break;
            }
        }
        delete pTextEntry;
    }
};

// ---------------------------------------------------------------

void CGMChart::InsertTextEntry( TextEntry* pTextEntry )
{
    maTextEntryList.push_back( pTextEntry );
};

// ---------------------------------------------------------------

// ---------------------------------------------------------------
void CGMChart::ResetAnnotation()
{
    mDataNode[ 0 ].nZoneEnum = 0;
}

// ---------------------------------------------------------------

sal_Bool CGMChart::IsAnnotation()
{
    return ( mDataNode[ 0 ].nZoneEnum == 0 );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
