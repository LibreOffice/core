/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SmartTagItem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 13:37:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_SMARTTAGITEM_HXX
#define _SVX_SMARTTAGITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_SMARTTAGS_XSMARTTAGACTION_HPP_
#include <com/sun/star/smarttags/XSmartTagAction.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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

    SvxSmartTagItem( const USHORT nId,
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
    virtual SfxPoolItem*     Create(SvStream &, USHORT) const; // leer
    virtual SvStream&        Store(SvStream &, USHORT nItemVersion) const; // leer
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const; // leer
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ); // leer

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


