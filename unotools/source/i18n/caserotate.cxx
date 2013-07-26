/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "unotools/caserotate.hxx"
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>

//TODO Use XCharacterClassification::getStringType to determine the current
//(possibly mixed) case type and rotate to the next one

sal_uInt32 RotateTransliteration::getNextMode()
{
    using namespace ::com::sun::star::i18n;

    sal_uInt32 nMode = 0;

    switch (nF3ShiftCounter)
    {
        case 0:
            nMode = TransliterationModulesExtra::TITLE_CASE;
            break;
        case 1:
            nMode = TransliterationModules_LOWERCASE_UPPERCASE;
            break;
        default:
        case 2:
            nMode = TransliterationModules_UPPERCASE_LOWERCASE;
            nF3ShiftCounter = -1;
            break;
    }

    nF3ShiftCounter++;

    return nMode;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
