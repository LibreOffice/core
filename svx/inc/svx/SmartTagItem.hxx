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
#ifndef _SVX_SMARTTAGITEM_HXX
#define _SVX_SMARTTAGITEM_HXX

#include <svl/poolitem.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/smarttags/XSmartTagAction.hpp>
#include "svx/svxdllapi.h"

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
    const rtl::OUString maApplicationName;
    const rtl::OUString maRangeText;

public:
    TYPEINFO();

    SvxSmartTagItem( const sal_uInt16 nId,
                     const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > > >& rActionComponentsSequence,
                     const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< sal_Int32 > >& rActionIndicesSequence,
                     const com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > >& rStringKeyMaps,
                     const com::sun::star::uno::Reference<com::sun::star::text::XTextRange> rRange,
                     const com::sun::star::uno::Reference<com::sun::star::frame::XController> rController,
                     const com::sun::star::lang::Locale rLocale,
                     const rtl::OUString& rApplicationName,
                     const rtl::OUString& rRangeText );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const; // leer
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion) const; // leer
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > > >& GetActionComponentsSequence() const { return maActionComponentsSequence; }
    const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< sal_Int32 > >& GetActionIndicesSequence() const { return maActionIndicesSequence; }
    const com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > >& GetStringKeyMaps() const { return maStringKeyMaps; }
    const com::sun::star::uno::Reference<com::sun::star::text::XTextRange>& GetTextRange() const { return mxRange; }
    const com::sun::star::uno::Reference<com::sun::star::frame::XController>& GetController() const { return mxController; }
    const com::sun::star::lang::Locale& GetLocale() const { return maLocale; }
    const rtl::OUString GetApplicationName() const { return maApplicationName; }
    const rtl::OUString GetRangeText() const { return maRangeText; }
};

#endif // #ifndef _SVX_SMARTTAGITEM_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
