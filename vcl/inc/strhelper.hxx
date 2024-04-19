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

#include <rtl/math.hxx>
#include <rtl/ustring.hxx>

namespace psp
{
    OUString GetCommandLineToken( int, const OUString& );
    OString GetCommandLineToken(int, const OString&);
    // gets one token of a unix command line style string
    // doublequote, singlequote and singleleftquote protect their respective
    // contents

    int GetCommandLineTokenCount(const OUString&);
    // returns number of tokens (zero if empty or whitespace only)

    OUString WhitespaceToSpace(std::u16string_view);
    OString WhitespaceToSpace(std::string_view);
    // returns a string with multiple adjacent occurrences of whitespace
    // converted to a single space. if bProtect is sal_True (nonzero), then
    // doublequote, singlequote and singleleftquote protect their respective
    // contents


    // parses the first double in the string; decimal is '.' only
    inline double StringToDouble( std::u16string_view rStr )
    {
        return rtl::math::stringToDouble(rStr, u'.', u'\0');
    }

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
