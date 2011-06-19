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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

// prevent internal compiler error with MSVC6SP3
#include <utility>

#define TRANSLITERATION_HyuByu_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace com { namespace sun { namespace star { namespace i18n {

static Mapping HyuByu[] = {
        { 0x30D5, 0x30E5, 0x30D2, sal_False },
        { 0x3075, 0x3085, 0x3072, sal_False },
        { 0x30F4, 0x30E5, 0x30D3, sal_False },
        { 0x3094, 0x3085, 0x3073, sal_False },
        { 0, 0, 0, sal_False }
};

ignoreHyuByu_ja_JP::ignoreHyuByu_ja_JP()
{
        func = (TransFunc) 0;
        table = 0;
        map = HyuByu;
        transliterationName = "ignoreHyuByu_ja_JP";
        implementationName = "com.sun.star.i18n.Transliteration.ignoreHyuByu_ja_JP";
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
