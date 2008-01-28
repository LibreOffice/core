/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: widthfolding.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-28 15:31:47 $
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
#ifndef INCLUDED_I18NUTIL_WIDTHFOLDING_HXX
#define INCLUDED_I18NUTIL_WIDTHFOLDING_HXX

#include <sal/types.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <i18nutil/oneToOneMapping.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

#define WIDTHFOLDNIG_DONT_USE_COMBINED_VU 0x01

class widthfolding
{
public:
    static oneToOneMapping& getfull2halfTable();
    static oneToOneMapping& gethalf2fullTable();

    static oneToOneMapping& getfull2halfTableForASC();
    static oneToOneMapping& gethalf2fullTableForJIS();

    static oneToOneMapping& getfullKana2halfKanaTable();
    static oneToOneMapping& gethalfKana2fullKanaTable();

    static rtl::OUString decompose_ja_voiced_sound_marks(const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset, sal_Bool useOffset);
    static sal_Unicode decompose_ja_voiced_sound_marksChar2Char (sal_Unicode inChar);
    static rtl::OUString compose_ja_voiced_sound_marks(const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset, sal_Bool useOffset, sal_Int32 nFlags = 0 );
    static sal_Unicode getCompositionChar(sal_Unicode c1, sal_Unicode c2);
};


} } } }

#endif
