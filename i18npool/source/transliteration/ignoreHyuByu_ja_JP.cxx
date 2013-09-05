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

#define TRANSLITERATION_HyuByu_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace com { namespace sun { namespace star { namespace i18n {

static const Mapping HyuByu[] = {
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
