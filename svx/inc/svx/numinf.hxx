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


#ifndef _SVX_NUMINF_HXX
#define _SVX_NUMINF_HXX

// include ---------------------------------------------------------------

#include <svl/itempool.hxx>
#include <svx/numfmtsh.hxx>
#include "svx/svxdllapi.h"

// class SvxNumberInfoItem -----------------------------------------------



/*
[Beschreibung]
Dieses Item dient als Transport-Medium fuer einen Number-Formatter.
*/

class SVX_DLLPUBLIC SvxNumberInfoItem : public SfxPoolItem
{
public:
    SvxNumberInfoItem( const sal_uInt16 nId  );
    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                       const sal_uInt16 nId  );
    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter, const String& rVal,
                       const sal_uInt16 nId  );
    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter, const double& rVal,
                       const sal_uInt16 nId  );
    // if both double and String are supplied, String is used for text formats
    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter, const double& rVal,
                       const String& rValueStr, const sal_uInt16 nId );
    SvxNumberInfoItem( const SvxNumberInfoItem& );
    ~SvxNumberInfoItem();

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create( SvStream& rStream, sal_uInt16 nVer ) const;
    virtual SvStream&        Store( SvStream& , sal_uInt16 nItemVersion ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    SvNumberFormatter*      GetNumberFormatter() const { return pFormatter; }
    void                    SetNumberFormatter( SvNumberFormatter* pNumFrmt );
    const String&           GetValueString() const { return aStringVal; }
    void                    SetStringValue( const String& rNewVal );
    double                  GetValueDouble() const  { return nDoubleVal; }
    void                    SetDoubleValue( const double& rNewVal );

    const sal_uInt32*       GetDelArray() const { return pDelFormatArr; }
    void                    SetDelFormatArray( const sal_uInt32* pData,
                                               const sal_uInt32  nCount );

    SvxNumberValueType      GetValueType() const { return eValueType; }
    sal_uInt32              GetDelCount() const  { return nDelCount; }

private:
    SvNumberFormatter*  pFormatter;
    SvxNumberValueType  eValueType;
    String              aStringVal;
    double              nDoubleVal;

    sal_uInt32*         pDelFormatArr;
    sal_uInt32          nDelCount;
};



#endif

