/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: halfwidthToFullwidth.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-28 15:37:42 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

// prevent internal compiler error with MSVC6SP3
#include <utility>

#include <i18nutil/widthfolding.hxx>
#define TRANSLITERATION_halfwidthToFullwidth
#define TRANSLITERATION_halfwidthKatakanaToFullwidthKatakana
#define TRANSLITERATION_halfwidthToFullwidthLikeJIS
#include <transliteration_OneToOne.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

halfwidthToFullwidth::halfwidthToFullwidth()
{
    func = (TransFunc) 0;
    table = &widthfolding::gethalf2fullTable();
    transliterationName = "halfwidthToFullwidth";
    implementationName = "com.sun.star.i18n.Transliteration.HALFWIDTH_FULLWIDTH";
}

OUString SAL_CALL
halfwidthToFullwidth::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    sal_Bool _useOffset = useOffset;
    // One to One mapping
    useOffset = sal_False;
    const OUString& newStr = transliteration_OneToOne::transliterate( inStr, startPos, nCount, offset);
    useOffset = _useOffset;

    // Composition: KA + voice-mark --> GA
    return widthfolding::compose_ja_voiced_sound_marks ( newStr, 0, newStr.getLength(), offset, _useOffset );
}

sal_Unicode SAL_CALL
halfwidthToFullwidth::transliterateChar2Char( sal_Unicode inChar)
  throw(RuntimeException, MultipleCharsOutputException)
{
    return transliteration_OneToOne::transliterateChar2Char(inChar);
}


halfwidthKatakanaToFullwidthKatakana::halfwidthKatakanaToFullwidthKatakana()
{
    func = (TransFunc) 0;
    table = &widthfolding::gethalfKana2fullKanaTable();
    transliterationName = "halfwidthKatakanaToFullwidthKatakana";
    implementationName = "com.sun.star.i18n.Transliteration.HALFWIDTHKATAKANA_FULLWIDTHKATAKANA";
}

OUString SAL_CALL
halfwidthKatakanaToFullwidthKatakana::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    sal_Bool _useOffset = useOffset;
    // One to One mapping
    useOffset = sal_False;
    const OUString& newStr = transliteration_OneToOne::transliterate( inStr, startPos, nCount, offset);
    useOffset = _useOffset;

    // Composition: KA + voice-mark --> GA
    return widthfolding::compose_ja_voiced_sound_marks ( newStr, 0, newStr.getLength(), offset, _useOffset );
}

sal_Unicode SAL_CALL
halfwidthKatakanaToFullwidthKatakana::transliterateChar2Char( sal_Unicode inChar)
  throw(RuntimeException, MultipleCharsOutputException)
{
    return transliteration_OneToOne::transliterateChar2Char(inChar);
}


halfwidthToFullwidthLikeJIS::halfwidthToFullwidthLikeJIS()
{
    func = (TransFunc) 0;
    table = &widthfolding::gethalf2fullTableForJIS();
    transliterationName = "halfwidthToFullwidthLikeJIS";
    implementationName = "com.sun.star.i18n.Transliteration.HALFWIDTH_FULLWIDTH_LIKE_JIS";
}

OUString SAL_CALL
halfwidthToFullwidthLikeJIS::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    sal_Bool _useOffset = useOffset;
    // One to One mapping
    useOffset = sal_False;
    const OUString& newStr = transliteration_OneToOne::transliterate( inStr, startPos, nCount, offset);
    useOffset = _useOffset;

    // Composition: KA + voice-mark --> GA
    return widthfolding::compose_ja_voiced_sound_marks ( newStr, 0, newStr.getLength(), offset, _useOffset, WIDTHFOLDNIG_DONT_USE_COMBINED_VU );
}

sal_Unicode SAL_CALL
halfwidthToFullwidthLikeJIS::transliterateChar2Char( sal_Unicode inChar)
  throw(RuntimeException, MultipleCharsOutputException)
{
    return transliteration_OneToOne::transliterateChar2Char(inChar);
}

} } } }
