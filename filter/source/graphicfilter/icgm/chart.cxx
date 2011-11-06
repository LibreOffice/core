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
#include "precompiled_filter.hxx"

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

    TextEntry* pTextEntry;
    while( ( pTextEntry = (TextEntry*)maTextEntryList.First() ) != NULL )
    {
        DeleteTextEntry( pTextEntry );
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
        delete pTextEntry;
        maTextEntryList.Remove( pTextEntry );
    }
};

// ---------------------------------------------------------------

void CGMChart::InsertTextEntry( TextEntry* pTextEntry )
{
    maTextEntryList.Insert( pTextEntry );
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

