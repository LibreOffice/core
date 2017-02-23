/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotools/caserotate.hxx>
#include <i18nutil/transliteration.hxx>

//TODO Use XCharacterClassification::getStringType to determine the current
//(possibly mixed) case type and rotate to the next one

TransliterationFlags RotateTransliteration::getNextMode()
{
    TransliterationFlags nMode = TransliterationFlags::NONE;

    switch (nF3ShiftCounter)
    {
        case 0:
            nMode = TransliterationFlags::TITLE_CASE;
            break;
        case 1:
            nMode = TransliterationFlags::LOWERCASE_UPPERCASE;
            break;
        default:
        case 2:
            nMode = TransliterationFlags::UPPERCASE_LOWERCASE;
            nF3ShiftCounter = -1;
            break;
    }

    nF3ShiftCounter++;

    return nMode;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
