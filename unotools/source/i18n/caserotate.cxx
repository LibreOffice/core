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
//#include <vcl/help.hxx>

//TODO Use XCharacterClassification::getStringType to determine the current
//(possibly mixed) case type and rotate to the next one

TransliterationFlags RotateTransliteration::getNextMode()
{
    // tdf#119495 start cycling from zero after 3 seconds
    sal_uInt64 tCurrentTime;
    tCurrentTime = tools::Time::GetMonotonicTicks();
    if ( tCurrentTime-tLastEvent > 3000000) {
        nF3ShiftCounter = 0;
    }
    tLastEvent = tCurrentTime;
    // show
//    Help::ShowQuickHelp( this, tools::Rectangle(), "rHelpText" );


    TransliterationFlags nMode = TransliterationFlags::NONE;

    switch (nF3ShiftCounter)
    {
        case 0:
            nMode = TransliterationFlags::TITLE_CASE;
            break;
        case 1: //tdf#116315
            nMode = TransliterationFlags::SENTENCE_CASE;
            break;
        case 2:
            nMode = TransliterationFlags::LOWERCASE_UPPERCASE;
            break;
        default:
        case 3:
            nMode = TransliterationFlags::UPPERCASE_LOWERCASE;
            nF3ShiftCounter = -1;
            break;
    }
/*
    m_aKeyInputTimer.SetInvokeHandler(LINK(this, RotateTransliteration, KeyInputTimerHandler));
    m_aKeyInputTimer.SetTimeout( 3000 );
    m_aKeyInputTimer.Start();
*/
    nF3ShiftCounter++;

    return nMode;
}
/*
IMPL_LINK_NOARG(RotateTransliteration, KeyInputTimerHandler, Timer *, void)
{
    nF3ShiftCounter = 0;
    m_aKeyInputTimer.Stop();
}
*/
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
