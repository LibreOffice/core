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



#ifndef _SDGINITM_HXX
#define _SDGINITM_HXX

#include <svx/sdooitm.hxx>
#include <svx/svddef.hxx>
#include "svx/svxdllapi.h"

//--------------------
// SdrGrafInvertItem -
//--------------------

class SVX_DLLPUBLIC SdrGrafInvertItem : public SdrOnOffItem
{
public:

                            SdrGrafInvertItem( sal_Bool bInvert = sal_False ) : SdrOnOffItem( SDRATTR_GRAFINVERT, bInvert ) {}
                            SdrGrafInvertItem( SvStream& rIn ) : SdrOnOffItem( SDRATTR_GRAFINVERT, rIn ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const;
};

#endif // _SDGINITM_HXX
