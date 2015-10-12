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
    const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > > > maActionComponentsSequence;
    const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< sal_Int32 > > maActionIndicesSequence;
    const com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > > maStringKeyMaps;
    const com::sun::star::uno::Reference<com::sun::star::text::XTextRange> mxRange;
    const com::sun::star::uno::Reference<com::sun::star::frame::XController> mxController;
    const com::sun::star::lang::Locale maLocale;
    const OUString maApplicationName;
    const OUString maRangeText;

public:
    TYPEINFO_OVERRIDE();

    SvxSmartTagItem( const sal_uInt16 nId,
                     const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > > >& rActionComponentsSequence,
                     const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< sal_Int32 > >& rActionIndicesSequence,
                     const com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > >& rStringKeyMaps,
                     const com::sun::star::uno::Reference<com::sun::star::text::XTextRange> rRange,
                     const com::sun::star::uno::Reference<com::sun::star::frame::XController> rController,
                     const com::sun::star::lang::Locale rLocale,
                     const OUString& rApplicationName,
                     const OUString& rRangeText );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const override;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const override; // leer
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion) const override; // leer
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > > >& GetActionComponentsSequence() const { return maActionComponentsSequence; }
    const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< sal_Int32 > >& GetActionIndicesSequence() const { return maActionIndicesSequence; }
    const com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > >& GetStringKeyMaps() const { return maStringKeyMaps; }
    const com::sun::star::uno::Reference<com::sun::star::text::XTextRange>& GetTextRange() const { return mxRange; }
    const com::sun::star::uno::Reference<com::sun::star::frame::XController>& GetController() const { return mxController; }
    const com::sun::star::lang::Locale& GetLocale() const { return maLocale; }
    const OUString GetApplicationName() const { return maApplicationName; }
    const OUString GetRangeText() const { return maRangeText; }
};

#endif // INCLUDED_SVX_SMARTTAGITEM_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
