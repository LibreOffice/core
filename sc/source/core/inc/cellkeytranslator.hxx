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

#pragma once

#include <formula/opcode.hxx>
#include <rtl/ustring.hxx>

namespace com::sun::star::lang
{
struct Locale;
}

/** Translate cell function keywords.

    Provides a convenient way to translate a string keyword used as
    a cell function argument.  Since Calc's built-in cell functions don't
    localize string keywords, this is used mainly to deal with an Excel
    document where string names may be localized.

    To use, simply call the

       ScCellKeywordTranslator::transKeyword(...)

    function.

    Note that the function only finds a string with matching locale and opcode.
    Locale is matched as close as possible */
namespace ScCellKeywordTranslator
{
void transKeyword(OUString& rName, const css::lang::Locale& rLocale, OpCode eOpCode);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
