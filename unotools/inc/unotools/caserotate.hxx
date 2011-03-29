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
