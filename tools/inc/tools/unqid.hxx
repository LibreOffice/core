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


#ifndef _UNQID_HXX
#define _UNQID_HXX

#include "tools/toolsdllapi.h"
#include <tools/unqidx.hxx>

// ---------------
// - ImpUniqueId -
// ---------------

struct ImpUniqueId
{
    sal_uIntPtr    nId;
    sal_uInt16   nRefCount;
    void     Release()
             {
                nRefCount--;
                if( 0 == nRefCount )
                    delete this;
             }
};

// ------------
// - UniqueId -
// ------------

class UniqueIdContainer;
class UniqueItemId
{
    friend class UniqueIdContainer;
    ImpUniqueId*    pId;

                    UniqueItemId( ImpUniqueId * pIdP )
                        { pId = pIdP; pId->nRefCount++; }
public:
                    UniqueItemId() { pId = NULL; }
                    UniqueItemId( const UniqueItemId & rId )
                        { pId = rId.pId; if( pId ) pId->nRefCount++; }
                    ~UniqueItemId()
                        { if( pId ) pId->Release(); }
    UniqueItemId&   operator = ( const UniqueItemId & rId )
                        {
                            if( rId.pId ) rId.pId->nRefCount++;
                            if( pId ) pId->Release();
                            pId = rId.pId;
                            return *this;
                        }
    sal_uIntPtr           GetId() const { return pId ? pId->nId : 0; }
};

// ---------------------
// - UniqueIdContainer -
// ---------------------

class TOOLS_DLLPUBLIC UniqueIdContainer : private UniqueIndex
{
    sal_uInt16              nCollectCount;

public: // Irgend etwas mit protected falsch
    void                Clear( sal_Bool bAll );
    UniqueItemId        CreateIdProt( sal_uIntPtr nId );

public:
                        UniqueIdContainer( sal_uIntPtr _nStartIndex,
                                           sal_uIntPtr _nInitSize = 16,
                                           sal_uIntPtr _nReSize = 16 )
                            : UniqueIndex( _nStartIndex, _nInitSize, _nReSize )
                            , nCollectCount( 0 )
                            {}
                        UniqueIdContainer( const UniqueIdContainer& );

                        ~UniqueIdContainer()
                            { Clear( sal_True ); }
    UniqueIdContainer&  operator = ( const UniqueIdContainer & );

    sal_Bool                IsIndexValid( sal_uIntPtr nIndex ) const
                            { return UniqueIndex::IsIndexValid( nIndex ); }

    UniqueItemId        CreateId();
    static UniqueItemId CreateFreeId( sal_uIntPtr nId ); // freies Id
};

#endif // _UNQID_HXX
