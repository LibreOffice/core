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
#include <vcl/fontcapabilities.hxx>

class OutputDevice;
namespace vcl { class Font; }

UScriptCode otCoverageToScript(vcl::UnicodeCoverage::UnicodeCoverageEnum eOTCoverage);

bool isSymbolFont(const vcl::Font &rFont);

bool canRenderNameOfSelectedFont(OutputDevice const &rDevice);

//These ones are typically for use in the font dropdown box beside the
//fontname, so say things roughly like "Script/Alphabet/Name-Of-Major-Language"
OUString makeShortRepresentativeSymbolTextForSelectedFont(OutputDevice const &rDevice);
OUString makeShortRepresentativeTextForSelectedFont(OutputDevice const &rDevice);
OUString makeShortRepresentativeTextForScript(UScriptCode eScript);
//For the cases where the font doesn't fully support a script, but has partial support
//for a useful subset
OUString makeShortMinimalTextForScript(UScriptCode eScript);

//These ones are typically for use in the font preview window in format character
SVT_DLLPUBLIC OUString makeRepresentativeTextForFont(sal_Int16 nScriptType, const vcl::Font &rFont);


#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
