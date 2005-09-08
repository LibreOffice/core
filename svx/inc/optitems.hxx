/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optitems.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:13:04 $
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
#ifndef _SVX_OPTITEMS_HXX
#define _SVX_OPTITEMS_HXX

// include ---------------------------------------------------------------

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// forward ---------------------------------------------------------------
namespace com{namespace sun{namespace star{
namespace beans{
//  class XPropertySet;
}
namespace linguistic2{
    class XSpellChecker1;
}}}}


// class SfxSpellCheckItem -----------------------------------------------

#ifdef ITEMID_SPELLCHECK

class SVX_DLLPUBLIC SfxSpellCheckItem: public SfxPoolItem
{
public:
    TYPEINFO();

    SfxSpellCheckItem( ::com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XSpellChecker1 >  &xChecker,
                       sal_uInt16 nWhich = ITEMID_SPELLCHECK );
    SfxSpellCheckItem( const SfxSpellCheckItem& rItem );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >
            GetXSpellChecker() const { return xSpellCheck; }

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >         xSpellCheck;
};
#endif

// class SfxHyphenRegionItem ---------------------------------------------

#ifdef ITEMID_HYPHENREGION

class SVX_DLLPUBLIC SfxHyphenRegionItem: public SfxPoolItem
{
    sal_uInt8 nMinLead;
    sal_uInt8 nMinTrail;

public:
    TYPEINFO();

    SfxHyphenRegionItem( const sal_uInt16 nId = ITEMID_HYPHENREGION );
    SfxHyphenRegionItem( const SfxHyphenRegionItem& rItem );

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create( SvStream& rStrm, sal_uInt16 nVer ) const;
    virtual SvStream&        Store( SvStream& rStrm, sal_uInt16 ) const;

    inline sal_uInt8 &GetMinLead() { return nMinLead; }
    inline sal_uInt8 GetMinLead() const { return nMinLead; }

    inline sal_uInt8 &GetMinTrail() { return nMinTrail; }
    inline sal_uInt8 GetMinTrail() const { return nMinTrail; }

    inline SfxHyphenRegionItem& operator=( const SfxHyphenRegionItem& rNew )
    {
        nMinLead = rNew.GetMinLead();
        nMinTrail = rNew.GetMinTrail();
        return *this;
    }
};
#endif


#endif

