/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef EDITENG_SECTIONATTRIBUTE_HXX
#define EDITENG_SECTIONATTRIBUTE_HXX

#include "editeng/editengdllapi.h"

#include <vector>

class SfxPoolItem;

namespace editeng {

struct EDITENG_DLLPUBLIC Section
{
    size_t mnParagraph;
    size_t mnStart;
    size_t mnEnd;

    std::vector<const SfxPoolItem*> maAttributes;

    Section();
    Section(size_t nPara, size_t nStart, size_t nEnd);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
