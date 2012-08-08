/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_PAGEITEM_HXX
#define _SVX_PAGEITEM_HXX

#include <svl/poolitem.hxx>
#include <tools/string.hxx>
#include "svx/svxdllapi.h"


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
portait or landscape, layout)
*/

class SVX_DLLPUBLIC SvxPageItem: public SfxPoolItem
{
private:
    String          aDescName;          // name of the template
    SvxNumType      eNumType;           // enumeration
    sal_Bool            bLandscape;         // Portrait / Landscape
    sal_uInt16          eUse;               // Layout

public:

    TYPEINFO();
    SvxPageItem( const sal_uInt16 nId );
    SvxPageItem( const SvxPageItem& rItem );

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    virtual SfxPoolItem*     Create( SvStream&, sal_uInt16 ) const;
    virtual SvStream&        Store( SvStream& , sal_uInt16 nItemVersion ) const;

    // orientation
    sal_uInt16          GetPageUsage() const                { return eUse;       }
    void            SetPageUsage(sal_uInt16 eU)             { eUse= eU;          }

    sal_Bool            IsLandscape() const                 { return bLandscape; }
    void            SetLandscape(sal_Bool bL)               { bLandscape = bL;   }

    // enumeration
    SvxNumType      GetNumType() const                  { return eNumType;   }
    void            SetNumType(SvxNumType eNum)         { eNumType = eNum;   }

    // name of the descriptor
    const String&   GetDescName() const                 { return aDescName;  }
    void            SetDescName(const String& rStr)     { aDescName = rStr;  }
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

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*    Create( SvStream&, sal_uInt16 nVersion ) const;
    virtual SvStream&       Store( SvStream&, sal_uInt16 nItemVersion ) const;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
