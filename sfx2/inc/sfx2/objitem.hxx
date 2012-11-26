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


#ifndef _SFX_OBJITEM_HXX
#define _SFX_OBJITEM_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <svl/poolitem.hxx>

#include <sfx2/shell.hxx>

//------------------------------------------------------------------------

class SFX2_DLLPUBLIC SfxObjectItem: public SfxPoolItem
{
    SfxShell*                _pSh;

public:
    POOLITEM_FACTORY()
                             SfxObjectItem( sal_uInt16 nWhich=0, SfxShell *pSh=0 );

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

    SfxShell*                GetShell() const
                             { return _pSh; }
};

#endif

