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
#ifndef INCLUDED_SVX_SMARTTAGITEM_HXX
#define INCLUDED_SVX_SMARTTAGITEM_HXX

#include <svl/poolitem.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/smarttags/XSmartTagAction.hpp>
#include <svx/svxdllapi.h>

namespace com { namespace sun { namespace star { namespace container {
    class XStringKeyMap;
} } } }

namespace com { namespace sun { namespace star { namespace text {
    class XTextRange;
} } } }

namespace com { namespace sun { namespace star { namespace frame {
    class XController;
} } } }

// class SvxSmartTagItem -----------------------------------------------------

class SVX_DLLPUBLIC SvxSmartTagItem : public SfxPoolItem
{
    const css::uno::Sequence < css::uno::Sequence< css::uno::Reference< css::smarttags::XSmartTagAction > > > maActionComponentsSequence;
    const css::uno::Sequence < css::uno::Sequence< sal_Int32 > > maActionIndicesSequence;
    const css::uno::Sequence< css::uno::Reference< css::container::XStringKeyMap > > maStringKeyMaps;
    const css::uno::Reference<css::text::XTextRange> mxRange;
    const css::uno::Reference<css::frame::XController> mxController;
    const css::lang::Locale maLocale;
    const OUString maApplicationName;
    const OUString maRangeText;

public:
    static SfxPoolItem* CreateDefault();

    SvxSmartTagItem( const sal_uInt16 nId,
                     const css::uno::Sequence < css::uno::Sequence< css::uno::Reference< css::smarttags::XSmartTagAction > > >& rActionComponentsSequence,
                     const css::uno::Sequence < css::uno::Sequence< sal_Int32 > >& rActionIndicesSequence,
                     const css::uno::Sequence< css::uno::Reference< css::container::XStringKeyMap > >& rStringKeyMaps,
                     const css::uno::Reference<css::text::XTextRange>& rRange,
                     const css::uno::Reference<css::frame::XController>& rController,
                     const css::lang::Locale& rLocale,
                     const OUString& rApplicationName,
                     const OUString& rRangeText );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const override; // leer
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion) const override; // leer
    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const css::uno::Sequence < css::uno::Sequence< css::uno::Reference< css::smarttags::XSmartTagAction > > >& GetActionComponentsSequence() const { return maActionComponentsSequence; }
    const css::uno::Sequence < css::uno::Sequence< sal_Int32 > >& GetActionIndicesSequence() const { return maActionIndicesSequence; }
    const css::uno::Sequence< css::uno::Reference< css::container::XStringKeyMap > >& GetStringKeyMaps() const { return maStringKeyMaps; }
    const css::uno::Reference<css::text::XTextRange>& GetTextRange() const { return mxRange; }
    const css::uno::Reference<css::frame::XController>& GetController() const { return mxController; }
    const css::lang::Locale& GetLocale() const { return maLocale; }
    const OUString GetApplicationName() const { return maApplicationName; }
    const OUString GetRangeText() const { return maRangeText; }
};

#endif // INCLUDED_SVX_SMARTTAGITEM_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
