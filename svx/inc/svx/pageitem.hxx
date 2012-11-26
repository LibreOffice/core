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


#ifndef _SVX_PAGEITEM_HXX
#define _SVX_PAGEITEM_HXX

// include ---------------------------------------------------------------

#include <svl/poolitem.hxx>
#include <tools/string.hxx>
#include "svx/svxdllapi.h"


/*--------------------------------------------------------------------
    Beschreibung:   SvxNumType
 --------------------------------------------------------------------*/

enum SvxNumType
{
    SVX_CHARS_UPPER_LETTER,
    SVX_CHARS_LOWER_LETTER,
    SVX_ROMAN_UPPER,
    SVX_ROMAN_LOWER,
    SVX_ARABIC,
    SVX_NUMBER_NONE,
    SVX_CHAR_SPECIAL,
    SVX_PAGEDESC
};

/*--------------------------------------------------------------------
    Beschreibung:   Benutzung der Seite
 --------------------------------------------------------------------*/

enum SvxPageUsage
{
    SVX_PAGE_LEFT           = 0x0001,
    SVX_PAGE_RIGHT          = 0x0002,
    SVX_PAGE_ALL            = 0x0003,
    SVX_PAGE_MIRROR         = 0x0007,
    SVX_PAGE_HEADERSHARE    = 0x0040,
    SVX_PAGE_FOOTERSHARE    = 0x0080
};

/*--------------------------------------------------------------------
    Beschreibung:   Teile der Seitenbeschreibung
 --------------------------------------------------------------------*/



/*
[Beschreibung]
Dieses Item beschreibt ein Seiten-Attribut (Name der Vorlage, Numerierung,
Portrait oder Landscape, Layout).
*/

class SVX_DLLPUBLIC SvxPageItem: public SfxPoolItem
{
private:
    String          aDescName;          // Name der Vorlage
    SvxNumType      eNumType;           // Numerierung
    sal_Bool            bLandscape;         // Portrait / Landscape
    sal_uInt16          eUse;               // Layout

public:
    POOLITEM_FACTORY()
    SvxPageItem();
    SvxPageItem( sal_uInt16 nId );
    SvxPageItem( const SvxPageItem& rItem );

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    virtual SfxPoolItem*     Create( SvStream&, sal_uInt16 ) const;
    virtual SvStream&        Store( SvStream& , sal_uInt16 nItemVersion ) const;

    // Ausrichtung
    sal_uInt16          GetPageUsage() const                { return eUse;       }
    void            SetPageUsage(sal_uInt16 eU)             { eUse= eU;          }

    sal_Bool            IsLandscape() const                 { return bLandscape; }
    void            SetLandscape(sal_Bool bL)               { bLandscape = bL;   }

    // Numerierung
    SvxNumType      GetNumType() const                  { return eNumType;   }
    void            SetNumType(SvxNumType eNum)         { eNumType = eNum;   }

    // Name des Descriptors
    const String&   GetDescName() const                 { return aDescName;  }
    void            SetDescName(const String& rStr)     { aDescName = rStr;  }
};



/*--------------------------------------------------------------------
    Beschreibung:   Container fuer Header/Footer-Attribute
 --------------------------------------------------------------------*/



/*
[Beschreibung]
Dieses Item dient als Container fuer Header- und Footer-Attribute.
*/

class SVX_DLLPUBLIC SvxSetItem: public SfxSetItem
{
public:
    SvxSetItem( const sal_uInt16 nId, const SfxItemSet& rSet );
    SvxSetItem( const SvxSetItem& rItem );
    SvxSetItem( const sal_uInt16 nId, SfxItemSet* pSet );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*    Create( SvStream&, sal_uInt16 nVersion ) const;
    virtual SvStream&       Store( SvStream&, sal_uInt16 nItemVersion ) const;
};




#endif

