/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Caolán McNamara <caolanm@redhat.com>
 *
 * Contributor(s):
 *   Caolán McNamara <caolanm@redhat.com>
 *   Dózsa Bálint <dozsa@linux-dugf.site>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

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
