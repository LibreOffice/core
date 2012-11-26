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


#ifndef _SVX_LCOLITEM_HXX
#define _SVX_LCOLITEM_HXX

// include ---------------------------------------------------------------

#include <editeng/colritem.hxx>
#include <editeng/editengdllapi.h>

/*
 * dummy-Item fuer ToolBox-Controller:
 *
 */

class EDITENG_DLLPUBLIC SvxLineColorItem : public SvxColorItem
{
public:
    SvxLineColorItem( const sal_uInt16 nId  );
    SvxLineColorItem( const Color& aColor, const sal_uInt16 nId  );
    SvxLineColorItem( SvStream& rStrm, const sal_uInt16 nId  );
    SvxLineColorItem( const SvxLineColorItem& rCopy );
    ~SvxLineColorItem();

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper * pIntlWrapper = 0 )
        const;

    inline SvxLineColorItem& operator=(const SvxLineColorItem& rColor)
        { return (SvxLineColorItem&)SvxColorItem::
            operator=((const SvxColorItem&)rColor); }

};

#endif

