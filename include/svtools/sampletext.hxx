/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SVTOOLS_SAMPLETEXT_HXX
#define INCLUDED_SVTOOLS_SAMPLETEXT_HXX

#include <svtools/svtdllapi.h>
#include <rtl/ustring.hxx>
#include <unicode/uscript.h>
#include <i18nlangtag/lang.h>
#include <vcl/fontcapabilities.hxx>

class OutputDevice;
namespace vcl { class Font; }

SVT_DLLPUBLIC UScriptCode otCoverageToScript(vcl::UnicodeCoverage::UnicodeCoverageEnum eOTCoverage);

SVT_DLLPUBLIC bool isSymbolFont(const vcl::Font &rFont);

SVT_DLLPUBLIC bool canRenderNameOfSelectedFont(OutputDevice const &rDevice);

//These ones are typically for use in the font dropdown box beside the
//fontname, so say things roughly like "Script/Alphabet/Name-Of-Major-Language"
SVT_DLLPUBLIC OUString makeShortRepresentativeSymbolTextForSelectedFont(OutputDevice const &rDevice);
SVT_DLLPUBLIC OUString makeShortRepresentativeTextForSelectedFont(OutputDevice const &rDevice);
SVT_DLLPUBLIC OUString makeShortRepresentativeTextForScript(UScriptCode eScript);
//For the cases where the font doesn't fully support a script, but has partial support
//for a useful subset
SVT_DLLPUBLIC OUString makeShortMinimalTextForScript(UScriptCode eScript);

//These ones are typically for use in the font preview window in format character
SVT_DLLPUBLIC OUString makeRepresentativeTextForFont(sal_Int16 nScriptType, const vcl::Font &rFont);
SVT_DLLPUBLIC OUString makeRepresentativeTextForLanguage(LanguageType eLang);
SVT_DLLPUBLIC OUString makeRepresentativeTextForScript(UScriptCode eScript);
SVT_DLLPUBLIC OUString makeMinimalTextForScript(UScriptCode eScript);


#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
