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


#ifndef _SVX_PBINITEM_HXX
#define _SVX_PBINITEM_HXX

// include ---------------------------------------------------------------

#include <svl/intitem.hxx>
#include <editeng/editengdllapi.h>

// define ----------------------------------------------------------------

#define PAPERBIN_PRINTER_SETTINGS   ((sal_uInt8)0xFF)

// class SvxPaperBinItem -------------------------------------------------

/*  [Beschreibung]

    Dieses Item beschreibt die Auswahl eines Papierschachts des Drucker.
*/

class EDITENG_DLLPUBLIC SvxPaperBinItem : public SfxByteItem
{
public:
    POOLITEM_FACTORY()
    inline SvxPaperBinItem( const sal_uInt16 nId = 0,
                            const sal_uInt8 nTray = PAPERBIN_PRINTER_SETTINGS );
    inline SvxPaperBinItem &operator=( const SvxPaperBinItem &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream &, sal_uInt16 ) const;
    virtual SvStream&       Store( SvStream &, sal_uInt16 nItemVersion ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                             SfxMapUnit eCoreMetric,
                                             SfxMapUnit ePresMetric,
                                             String &rText, const IntlWrapper * = 0 ) const;
};

inline SvxPaperBinItem::SvxPaperBinItem( const sal_uInt16 nId, const sal_uInt8 nT )
    : SfxByteItem( nId, nT )
{}

inline SvxPaperBinItem &SvxPaperBinItem::operator=(
    const SvxPaperBinItem &rCpy )
{
    SetValue( rCpy.GetValue() );
    return *this;
}

#endif

