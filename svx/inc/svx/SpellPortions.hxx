/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SpellPortions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:37:22 $
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

#ifndef SVX_SPELL_PORTIONS_HXX
#define SVX_SPELL_PORTIONS_HXX

#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#include <vector>

namespace com{ namespace sun{ namespace star{ namespace linguistic2{
    class XSpellAlternatives;
}}}}

namespace svx{
/** contains a portion of text that has the same language attributes applied
    and belongs to the same script type.
 */
struct SpellPortion
{
    /** contains the text of the portion.
     */
    rtl::OUString   sText;
    /** Marks the portion as field, footnote symbol or any other special content that
     should be protected against unintentional deletion.
     */
    bool bIsField;
    /** Marks the portion hidden content that should not be touched by spell checking
        and not be removed like redlines. The creator of the portions has to take care
        for them.
     */
    bool bIsHidden;
    /** contains the language applied to the text. It has to match the script type.
     */
    LanguageType    eLanguage;
    /** for wrong words this reference is filled with the error informations otherwise
        it's an empty reference
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellAlternatives> xAlternatives;

    SpellPortion() :
        bIsField(false),
        bIsHidden(false),
        eLanguage(LANGUAGE_DONTKNOW)
        {}
};
typedef std::vector<SpellPortion> SpellPortions;
}//namespace svx
#endif
