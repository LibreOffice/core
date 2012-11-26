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



#ifndef _SVX_XEXCH_HXX
#define _SVX_XEXCH_HXX


#include <tools/stream.hxx>
#include "svx/svxdllapi.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                Clipboard-Format fuer XOutDev-Fuellattribute
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class XFillAttrSetItem;
class SfxItemPool;



class SVX_DLLPUBLIC XFillExchangeData
{
private:
    XFillAttrSetItem*   pXFillAttrSetItem;
    SfxItemPool*        pPool;

public:
                        XFillExchangeData();
                        XFillExchangeData(const XFillAttrSetItem rXFillAttrSetItem);
                        virtual ~XFillExchangeData();

                        // Zuweisungsoperator
                        XFillExchangeData& operator=( const XFillExchangeData& rXFillExchangeData );

    SVX_DLLPUBLIC friend SvStream&    operator<<( SvStream& rOStm, const XFillExchangeData& rXFillExchangeData );
    SVX_DLLPUBLIC friend SvStream&    operator>>( SvStream& rIStm, XFillExchangeData& rXFillExchangeData );

    static sal_uLong        RegisterClipboardFormatName();
    XFillAttrSetItem*   GetXFillAttrSetItem() { return pXFillAttrSetItem; }
};

#endif                  // _SVX_XEXCH_HXX
