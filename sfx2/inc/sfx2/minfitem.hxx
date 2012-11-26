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


#ifndef _SFX_MINFITEM_HXX
#define _SFX_MINFITEM_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <tools/string.hxx>
#include <svl/poolitem.hxx>
class BasicManager;

class SFX2_DLLPUBLIC SfxMacroInfoItem: public SfxPoolItem
{
    const BasicManager*     pBasicManager;
    String                  aLibName;
    String                  aModuleName;
    String                  aMethodName;
    String                  aCommentText;

public:
    POOLITEM_FACTORY()

    SfxMacroInfoItem() : SfxPoolItem( 0 ), pBasicManager(0) {}
    SfxMacroInfoItem( sal_uInt16 nWhich,
                    const BasicManager* pMgr,
                    const String &rLibName,
                    const String &rModuleName,
                    const String &rMethodName,
                    const String &rComment);

    SfxMacroInfoItem( const SfxMacroInfoItem& );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    String                  GetComment() const
                                { return aCommentText; }
    void                    SetComment( const String& r )
                                { aCommentText = r; }
    String                  GetMethod() const
                                { return aMethodName; }
    void                    SetMethod( const String& r )
                                { aMethodName = r; }
    String                  GetModule() const
                                { return aModuleName; }
    void                    SetModule( const String& r )
                                { aModuleName = r; }
    String                  GetLib() const
                                { return aLibName; }
    void                    SetLib( const String& r )
                                { aLibName = r; }
    const BasicManager*     GetBasicManager() const
                            { return pBasicManager; }
    String                  GetQualifiedName() const;
};

#endif
