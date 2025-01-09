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

#include <sal/config.h>

#include <svl/setitem.hxx>
#include <svx/svxdllapi.h>
#include <editeng/svxenum.hxx>

/*--------------------------------------------------------------------
  usage of the page
 --------------------------------------------------------------------*/

enum class SvxPageUsage
{
    NONE           = 0,
    Left           = 1,
    Right          = 2,
    All            = 3,
    Mirror         = 7
};

/*--------------------------------------------------------------------
  parts of the page description
 --------------------------------------------------------------------*/

/*
This item describes a page attribute (name of the template, enumeration,
portrait or landscape, layout)
*/

class SVX_DLLPUBLIC SvxPageItem final : public SfxPoolItem
{
private:
    OUString            aDescName;          // name of the template
    SvxNumType          eNumType;
    bool                bLandscape;         // Portrait / Landscape
    SvxPageUsage        eUse;               // Layout

public:

    static SfxPoolItem* CreateDefault();
    DECLARE_ITEM_TYPE_FUNCTION(SvxPageItem)
    SvxPageItem( const TypedWhichId<SvxPageItem> nId );
    SvxPageItem( const SvxPageItem& rItem );

    virtual ~SvxPageItem() override;

    virtual SvxPageItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool             operator==( const SfxPoolItem& ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    // orientation
    SvxPageUsage    GetPageUsage() const                { return eUse;       }
    void            SetPageUsage(SvxPageUsage eU)       { eUse= eU;          }

    bool            IsLandscape() const                 { return bLandscape; }
    void            SetLandscape(bool bL)               { bLandscape = bL;   }

    // enumeration
    SvxNumType      GetNumType() const                  { return eNumType;   }
    void            SetNumType(SvxNumType eNum)         { eNumType = eNum;   }

    // name of the descriptor
    void            SetDescName(const OUString& rStr)   { aDescName = rStr;  }
};


/*--------------------------------------------------------------------
  container for header/footer attributes
 --------------------------------------------------------------------*/

class SVX_DLLPUBLIC SvxSetItem final : public SfxSetItem
{
public:
    DECLARE_ITEM_TYPE_FUNCTION(SvxSetItem)
    SvxSetItem( const TypedWhichId<SvxSetItem> nId, const SfxItemSet& rSet );
    SvxSetItem( const SvxSetItem& rItem, SfxItemPool* pPool = nullptr );
    SvxSetItem( const TypedWhichId<SvxSetItem> nId, SfxItemSet&& pSet );

    virtual SvxSetItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
