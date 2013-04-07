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
 *       [ Caolan McNamara <caolanm@redhat.com> ]
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Caolan McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef INCLUDED_SVTOOLS_SAMPLETEXT_HXX
#define INCLUDED_SVTOOLS_SAMPLETEXT_HXX

#include "svtools/svtdllapi.h"
#include <rtl/ustring.hxx>
#include <unicode/uscript.h>
#include <i18nlangtag/lang.h>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <vcl/fontcapabilities.hxx>

class OutputDevice;
class Font;

SVT_DLLPUBLIC UScriptCode otCoverageToScript(vcl::UnicodeCoverage::UnicodeCoverageEnum eOTCoverage);

SVT_DLLPUBLIC bool isSymbolFont(const Font &rFont);
SVT_DLLPUBLIC bool isOpenSymbolFont(const Font &rFont);

SVT_DLLPUBLIC bool canRenderNameOfSelectedFont(OutputDevice &rDevice);

//These ones are typically for use in the font dropdown box beside the
//fontname, so say things roughly like "Script/Alphabet/Name-Of-Major-Language"
SVT_DLLPUBLIC OUString makeShortRepresentativeSymbolTextForSelectedFont(OutputDevice &rDevice);
SVT_DLLPUBLIC OUString makeShortRepresentativeTextForSelectedFont(OutputDevice &rDevice);
SVT_DLLPUBLIC OUString makeShortRepresentativeTextForScript(UScriptCode eScript);
//For the cases where the font doesn't fully support a script, but has partial support
//for a useful subset
SVT_DLLPUBLIC OUString makeShortMinimalTextForScript(UScriptCode eScript);

//These ones are typically for use in the font preview window in format character
SVT_DLLPUBLIC OUString makeRepresentativeTextForFont(sal_Int16 nScriptType, const Font &rFont);
SVT_DLLPUBLIC OUString makeRepresentativeTextForLanguage(LanguageType eLang);
SVT_DLLPUBLIC OUString makeRepresentativeTextForScript(UScriptCode eScript);
SVT_DLLPUBLIC OUString makeMinimalTextForScript(UScriptCode eScript);


#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
