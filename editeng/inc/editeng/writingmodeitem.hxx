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


#ifndef _SVX_WRITINGMODEITEM_HXX
#define _SVX_WRITINGMODEITEM_HXX

// include ---------------------------------------------------------------

#include <com/sun/star/text/WritingMode.hpp>
#include <svl/intitem.hxx>
#include "editeng/editengdllapi.h"

// class SvxWritingModeItem ----------------------------------------------

class EDITENG_DLLPUBLIC SvxWritingModeItem : public SfxUInt16Item
{
public:
    SvxWritingModeItem( ::com::sun::star::text::WritingMode eValue /*= com::sun::star::text::WritingMode_LR_TB*/,
                sal_uInt16 nWhich /*= SDRATTR_TEXTDIRECTION*/ );
    virtual ~SvxWritingModeItem();

    SvxWritingModeItem& operator=( const SvxWritingModeItem& rItem );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual SvStream&       Store(SvStream & rStrm, sal_uInt16 nIVer) const;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileVersion ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper * = 0 ) const;

    virtual sal_Bool PutValue( const com::sun::star::uno::Any& rVal,
                                    sal_uInt8 nMemberId );
    virtual sal_Bool QueryValue( com::sun::star::uno::Any& rVal,
                                sal_uInt8 nMemberId ) const;
};

#endif // #ifndef _SVX_WRITINGMODEITEM_HXX

