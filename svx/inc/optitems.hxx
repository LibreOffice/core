/*************************************************************************
 *
 *  $RCSfile: optitems.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:00:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVX_OPTITEMS_HXX
#define _SVX_OPTITEMS_HXX

// include ---------------------------------------------------------------

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

//#ifndef _COM_SUN_STAR_LINGUISTIC_XSPELLALTERNATIVES_HPP_
//#include <com/sun/star/linguistic/XSpellAlternatives.hpp>
//#endif
//#ifndef _COM_SUN_STAR_LINGUISTIC_SPELLFAILURE_HPP_
//#include <com/sun/star/linguistic/SpellFailure.hpp>
//#endif
//#ifndef _COM_SUN_STAR_LINGUISTIC_XSPELLCHECKER_HPP_
//#include <com/sun/star/linguistic/XSpellChecker.hpp>
//#endif
//#ifndef _COM_SUN_STAR_LINGUISTIC_XSPELLCHECKER1_HPP_
//#include <com/sun/star/linguistic/XSpellChecker1.hpp>
//#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

// forward ---------------------------------------------------------------
namespace com{namespace sun{namespace star{
namespace beans{
//  class XPropertySet;
}
namespace linguistic{
//  class XDictionary;
    class XSpellChecker1;
//  class XOtherLingu;
    }}}}


// class SfxSpellCheckItem -----------------------------------------------

#ifdef ITEMID_SPELLCHECK

class SfxSpellCheckItem: public SfxPoolItem
{
public:
    TYPEINFO();

    SfxSpellCheckItem( ::com::sun::star::uno::Reference< ::com::sun::star::linguistic::XSpellChecker1 >  &xChecker,
                       sal_uInt16 nWhich = ITEMID_SPELLCHECK );
    SfxSpellCheckItem( const SfxSpellCheckItem& rItem );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const International * = 0 ) const;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic::XSpellChecker1 >        GetXSpellChecker() const { return xSpellCheck; }

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic::XSpellChecker1 >        xSpellCheck;
};
#endif

// class SfxHyphenRegionItem ---------------------------------------------

#ifdef ITEMID_HYPHENREGION

class SfxHyphenRegionItem: public SfxPoolItem
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
                                    String &rText, const International * = 0 ) const;

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

