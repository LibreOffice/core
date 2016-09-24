/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_BASIC_INC_GLOBAL_HXX
#define INCLUDED_BASIC_INC_GLOBAL_HXX

namespace utl {
    class TransliterationWrapper;
}

class SbGlobal
{
public:
    static utl::TransliterationWrapper& GetTransliteration();
};

#endif // INCLUDED_BASIC_INC_GLOBAL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
