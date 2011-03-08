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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
