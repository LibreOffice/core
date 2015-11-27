/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVX_PAGEITEM_HXX
#define INCLUDED_SVX_PAGEITEM_HXX

#include <svl/poolitem.hxx>
#include <svx/svxdllapi.h>


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
  usage of the page
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
  parts of the page description
 --------------------------------------------------------------------*/

/*
This item describes a page attribute (name of the template, enumeration,
portrait or landscape, layout)
*/

class SVX_DLLPUBLIC SvxPageItem: public SfxPoolItem
{
private:
    OUString            aDescName;          // name of the template
    SvxNumType          eNumType;           // enumeration
    bool                bLandscape;         // Portrait / Landscape
    sal_uInt16          eUse;               // Layout

public:

    static SfxPoolItem* CreateDefault();
    SvxPageItem( const sal_uInt16 nId );
    SvxPageItem( const SvxPageItem& rItem );

    virtual ~SvxPageItem();

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool             operator==( const SfxPoolItem& ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = nullptr ) const override;

    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    virtual SfxPoolItem*     Create( SvStream&, sal_uInt16 ) const override;
    virtual SvStream&        Store( SvStream& , sal_uInt16 nItemVersion ) const override;

    // orientation
    sal_uInt16          GetPageUsage() const                { return eUse;       }
    void            SetPageUsage(sal_uInt16 eU)             { eUse= eU;          }

    bool            IsLandscape() const                 { return bLandscape; }
    void            SetLandscape(bool bL)               { bLandscape = bL;   }

    // enumeration
    SvxNumType      GetNumType() const                  { return eNumType;   }
    void            SetNumType(SvxNumType eNum)         { eNumType = eNum;   }

    // name of the descriptor
    void            SetDescName(const OUString& rStr)     { aDescName = rStr;  }
};



/*--------------------------------------------------------------------
  container for header/footer attributes
 --------------------------------------------------------------------*/

class SVX_DLLPUBLIC SvxSetItem: public SfxSetItem
{
public:
    SvxSetItem( const sal_uInt16 nId, const SfxItemSet& rSet );
    SvxSetItem( const SvxSetItem& rItem );
    SvxSetItem( const sal_uInt16 nId, SfxItemSet* pSet );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = nullptr ) const override;

    virtual SfxPoolItem*    Create( SvStream&, sal_uInt16 nVersion ) const override;
    virtual SvStream&       Store( SvStream&, sal_uInt16 nItemVersion ) const override;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
