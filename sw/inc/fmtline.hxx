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


#ifndef SW_FMTLINE_HXX
#define SW_FMTLINE_HXX


#include <svl/poolitem.hxx>
#include <hintids.hxx>
#include <format.hxx>
#include "swdllapi.h"

class IntlWrapper;

class SW_DLLPUBLIC SwFmtLineNumber: public SfxPoolItem
{
    sal_uLong nStartValue   :24; //Startwert fuer den Absatz, 0 == kein Startwert
    sal_uLong bCountLines   :1;  //Zeilen des Absatzes sollen mitgezaehlt werden.

public:
    POOLITEM_FACTORY()
    SwFmtLineNumber();
    ~SwFmtLineNumber();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    sal_uLong GetStartValue() const { return nStartValue; }
    sal_Bool  IsCount()       const { return bCountLines != 0; }

    void SetStartValue( sal_uLong nNew ) { nStartValue = nNew; }
    void SetCountLines( sal_Bool b )     { bCountLines = b;    }
};

inline const SwFmtLineNumber &SwAttrSet::GetLineNumber(sal_Bool bInP) const
    { return (const SwFmtLineNumber&)Get( RES_LINENUMBER,bInP); }

#endif

