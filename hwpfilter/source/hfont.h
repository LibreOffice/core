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

#include <stdlib.h>
#include <string.h>
#include <memory>

    256
 40

class HWPFile;

/**
 * The HWPFont class has the font list for the document when it's saved.
 * @short Font information
 */
class DLLEXPORT HWPFont final
{
/**
 * System font count for each language
 * NLanguage is 7 in common case.
 */
        int   nFonts[NLanguage];
/**
 * list of the font family name
 */
        std::unique_ptr<char[]> fontnames[NLanguage];

    public:
        HWPFont(void);
        ~HWPFont(void);

/**
 * Adds a font into font list of a language
 * @param lang Language index
 * @param font Name of font family
 */
        void       AddFont( int lang, const char *font );
/**
 * @param lang Language index
 * @param id Index of font
 * @returns Font name
 */
        const char    *GetFontName( int lang, int id );

        void Read( HWPFile &hwpf );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
