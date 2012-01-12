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
#include "precompiled_sc.hxx"

#include "dpglobal.hxx"
#include "dpobject.hxx"
#include "document.hxx"

#include <stdio.h>

namespace ScDPGlobal
{
    Rectangle operator *( const Rectangle &rLeft, const std::pair<double,double> & rRight )
    {
        Rectangle rcResult( rLeft );
        rcResult.Bottom() = rcResult.Top() + static_cast<long>( rcResult.GetHeight() * rRight.second );
        rcResult.Right() = rcResult.Left() + static_cast<long>( rcResult.GetWidth() * rRight.first);
        return rcResult;
    }

    String GetFuncString( const String &rString, const sal_uInt16 nIndex )
    {
        if ( nIndex <= 1 ) return rString;
        sal_uLong uch = rString.Len() ? rString.GetChar( rString.Len()-1 ) : (L'9'+1);
        bool bEndWithDigital = ( L'0'<=uch && uch<=L'9');
        char szTemp[__MAX_NUM_LEN+1];
        int nLen = sprintf( szTemp, bEndWithDigital ? DATA_RENAME_SEPARATOR"%hu" : "%hu", nIndex );
        String strRet = rString;
        strRet.Append( String::CreateFromAscii( szTemp, static_cast<sal_uInt16>(nLen) ));
        return strRet;
    }

   bool ChkDPTableOverlap( ScDocument *pDestDoc, std::list<ScDPObject> & rClipboard, SCCOL nClipStartCol, SCROW nClipStartRow, SCCOL nStartCol, SCROW nStartRow, SCTAB nStartTab, sal_uInt16 nEndTab, sal_Bool bExcludeClip /*= sal_False*/ )
    {
        if ( ScDPCollection* pDPCollection = pDestDoc->GetDPCollection() )
        {
            sal_uInt16 nCount = pDPCollection->GetCount();
            SCsCOL nOffsetX = nStartCol - nClipStartCol;
            SCsROW nOffsetY = nStartRow - nClipStartRow;

            for( std::list<ScDPObject>::iterator iter = rClipboard.begin(); iter!=rClipboard.end(); iter++ )
            {
                ScRange aRange = iter->GetOutRange();

                for( sal_uInt16 nCurrTab = nStartTab; nCurrTab<=nEndTab; nCurrTab++ )
                {
                    SCsTAB nOffsetZ = nCurrTab - aRange.aStart.Tab();
                    aRange.Move( nOffsetX, nOffsetY, nOffsetZ );

                    for ( sal_uInt16 i = 0; i<nCount; i++)
                    {
                        if ( (*pDPCollection)[i] && aRange.Intersects( (*pDPCollection)[i]->GetOutRange()))
                        {
                            if ( bExcludeClip && iter->GetOutRange() == (*pDPCollection)[i]->GetOutRange() )
                            {
                                continue;
                            }
                            return false;
                        }
                    }
                }
            }
        }
    return true;
}
//end

}
// --------------------------------------------------------------------
// ScDPItemDataPool
// Construct
ScDPItemDataPool::ScDPItemDataPool(void)
{
}
//
ScDPItemDataPool::ScDPItemDataPool(const ScDPItemDataPool& r):
    maItems(r.maItems),
    maItemIds(r.maItemIds)
{
}

ScDPItemDataPool::~ScDPItemDataPool(void)
{
}


const ScDPItemData* ScDPItemDataPool::getData( sal_Int32 nId )
{
    if ( nId >= static_cast<sal_Int32>(maItems.size()) )
        return NULL;
    else
        return &(maItems[nId]);
}

sal_Int32 ScDPItemDataPool::getDataId( const ScDPItemData& aData )
{
    DataHash::const_iterator itr = maItemIds.find( aData),
            itrEnd = maItemIds.end();
    if ( itr == itrEnd )
         // not exist
        return -1;

    else //exist
        return itr->second;

}

sal_Int32 ScDPItemDataPool::insertData( const ScDPItemData& aData )
{
    sal_Int32 nResult = getDataId( aData );

    if( nResult < 0 )
    {
        maItemIds.insert( DataHash::value_type( aData, nResult = maItems.size() ) );
        maItems.push_back( aData );
    }

    return nResult;
}


