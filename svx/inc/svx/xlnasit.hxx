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



#ifndef _SVX_XLNASIT_HXX
#define _SVX_XLNASIT_HXX


#include <svl/poolitem.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* SetItem mit dem ItemSet aller Linienattribute
|*
\************************************************************************/

class SVX_DLLPUBLIC XLineAttrSetItem : public SfxSetItem
{
public:
                            XLineAttrSetItem(SfxItemSet* pItemSet );
                            XLineAttrSetItem(SfxItemPool* pItemPool);
                            XLineAttrSetItem(const XLineAttrSetItem& rAttr);
                            XLineAttrSetItem(const XLineAttrSetItem& rAttr,
                                             SfxItemPool* pItemPool);
    virtual SfxPoolItem*    Clone( SfxItemPool* pToPool ) const;
    virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVersion ) const;
    virtual SvStream&       Store( SvStream& rStream, sal_uInt16 nItemVersion ) const;
};

#endif
