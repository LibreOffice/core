/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_UNOTOOLS_CASEROTATE_HXX
#define INCLUDED_UNOTOOLS_CASEROTATE_HXX

#include <sal/config.h>

#include "unotools/unotoolsdllapi.h"

//TODO Use XCharacterClassification::getStringType to determine the current
//(possibly mixed) case type and rotate to the next one

class UNOTOOLS_DLLPUBLIC RotateTransliteration
{
private:
    int nF3ShiftCounter;
public:
    RotateTransliteration() : nF3ShiftCounter(0)
    {
    }
    sal_uInt32 getNextMode();
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
