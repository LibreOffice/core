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



#ifndef _SOT_STORINFO_HXX
#define _SOT_STORINFO_HXX

#include <tools/pstm.hxx>
#include <tools/globname.hxx>
#include <tools/ownlist.hxx>
#include "sot/sotdllapi.h"

class StgDirEntry;
class SvStorageInfo
{
friend class SvStorage;
    String          aName;
    SvGlobalName    aClassName;
    sal_uLong           nSize;
    sal_Bool            bStream:1,
                    bStorage:1;

                            SvStorageInfo(){}; // Fuer SvStorage
public:
                            SvStorageInfo( const StgDirEntry& );
                            SvStorageInfo( const String& rName, sal_uLong nSz, sal_Bool bIsStorage )
                                : aName( rName )
                                , nSize( nSz )
                                , bStream( !bIsStorage )
                                , bStorage( bIsStorage )
                            {}

    const SvGlobalName &    GetClassName() const { return aClassName; }
    const String &          GetName() const { return aName; }
    sal_Bool                    IsStream() const { return bStream; }
    sal_Bool                    IsStorage() const { return bStorage; }
    sal_uLong                   GetSize() const { return nSize;      }
};

class SOT_DLLPUBLIC SvStorageInfoList
{
    PRV_SV_DECL_OWNER_LIST(SvStorageInfoList,SvStorageInfo)
    const SvStorageInfo * Get( const String & rName );
};

SOT_DLLPUBLIC sal_uLong ReadClipboardFormat( SvStream & rStm );
SOT_DLLPUBLIC void WriteClipboardFormat( SvStream & rStm, sal_uLong nFormat );

#endif // _STORINFO_HXX
