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


// prevent internal compiler error with MSVC6SP3
#include <utility>

#define TRANSLITERATION_SeZe_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace com { namespace sun { namespace star { namespace i18n {

static const Mapping SeZe[] = {
        // SI + E --> SE
        { 0x30B7, 0x30A7, 0x30BB, sal_True },
        // SI + E --> SE
        { 0x3057, 0x3047, 0x305B, sal_True },
        // ZI + E --> ZE
        { 0x30B8, 0x30A7, 0x30BC, sal_True },
        // ZI + E --> ZE
        { 0x3058, 0x3047, 0x305C, sal_True },

        { 0, 0, 0, sal_True }
};

ignoreSeZe_ja_JP::ignoreSeZe_ja_JP()
{
        func = (TransFunc) 0;
        table = 0;
        map = SeZe;
        transliterationName = "ignoreSeZe_ja_JP";
        implementationName = "com.sun.star.i18n.Transliteration.ignoreSeZe_ja_JP";
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
