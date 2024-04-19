/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <sal/config.h>

#include <o3tl/sorted_vector.hxx>
#include <tools/fontenum.hxx>
#include <vcl/dllapi.h>
#include <vcl/timer.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <unx/fc_fontoptions.hxx>

#include <font/PhysicalFontFace.hxx>

#include <set>
#include <memory>
#include <string_view>
#include <vector>
#include <unordered_map>

/*
 *  some words on metrics: every length returned by PrintFontManager and
 *  friends are PostScript afm style, that is they are 1/1000 font height
 */

class FontAttributes;
class FontConfigFontOptions;
namespace vcl::font
{
class FontSelectPattern;
}
namespace vcl { struct NameRecord; }
class GenericUnixSalData;

namespace psp {
class PPDParser;

typedef int fontID;

// a class to manage printable fonts

class VCL_PLUGIN_PUBLIC PrintFontManager
{
    struct PrintFont;
    friend struct PrintFont;

    struct SAL_DLLPRIVATE PrintFont
    {
        FontAttributes    m_aFontAttributes;

        int               m_nDirectory;       // atom containing system dependent path
        OString           m_aFontFile;        // relative to directory
        int               m_nCollectionEntry; // 0 for regular fonts, 0 to ... for fonts stemming from collections
        int               m_nVariationEntry;  // 0 for regular fonts, 0 to ... for fonts stemming from font variations

        explicit PrintFont();
    };

    fontID                                      m_nNextFontID;
    std::unordered_map< fontID, PrintFont >     m_aFonts;
    // for speeding up findFontFileID
    std::unordered_map< OString, o3tl::sorted_vector< fontID > >
                                                m_aFontFileToFontID;

    std::unordered_map< OString, int >
    m_aDirToAtom;
    std::unordered_map< int, OString >          m_aAtomToDir;
    int                                         m_nNextDirAtom;

    OString getFontFile(const PrintFont& rFont) const;

    std::vector<PrintFont> analyzeFontFile(int nDirID, const OString& rFileName) const;
    bool analyzeSfntFile(PrintFont& rFont) const;
    // finds the font id for the nFaceIndex face in this font file
    // There may be multiple font ids for font collections
    fontID findFontFileID(int nDirID, const OString& rFile, int nFaceIndex, int nVariationIndex) const;

    // There may be multiple font ids for font collections
    std::vector<fontID> findFontFileIDs( int nDirID, const OString& rFile ) const;

    static FontFamily matchFamilyName( std::u16string_view rFamily );

    OString getDirectory( int nAtom ) const;
    int getDirectoryAtom( const OString& rDirectory );

    /* try to initialize fonts from libfontconfig

    called from <code>initialize()</code>
    */
    static void initFontconfig();
    void countFontconfigFonts();
    /* deinitialize fontconfig
     */
    static void deinitFontconfig();

    /* register an application specific font directory for libfontconfig

    since fontconfig is asked for font substitutes before OOo will check for font availability
    and fontconfig will happily substitute fonts it doesn't know (e.g. "Arial Narrow" -> "DejaVu Sans Book"!)
    it becomes necessary to tell the library about all the hidden font treasures
    */
    static void addFontconfigDir(const OString& rDirectory);

    /* register an application specific font file for libfontconfig */
    static void addFontconfigFile(const OString& rFile);

    std::set<OString> m_aPreviousLangSupportRequests;
    std::vector<OUString> m_aCurrentRequests;
    Timer m_aFontInstallerTimer;

    DECL_DLLPRIVATE_LINK( autoInstallFontLangSupport, Timer*, void );
    PrintFontManager();
public:
    ~PrintFontManager();
    friend class ::GenericUnixSalData;
    static PrintFontManager& get(); // one instance only

    // There may be multiple font ids for font collections
    std::vector<fontID> addFontFile( std::u16string_view rFileUrl );

    void initialize();

    const PrintFont* getFont( fontID nID ) const
    {
        auto it = m_aFonts.find( nID );
        return it == m_aFonts.end() ? nullptr : &it->second;
    }

    // returns the ids of all managed fonts.
    void getFontList( std::vector< fontID >& rFontIDs );

    // routines to get font info in small pieces

    // get a specific fonts system dependent filename
    OString getFontFileSysPath( fontID nFontID ) const
    {
        return getFontFile( *getFont( nFontID ) );
    }

    // get the ttc face number
    int getFontFaceNumber( fontID nFontID ) const;

    // get the ttc face variation
    int getFontFaceVariation( fontID nFontID ) const;

    // font administration functions

    /*  system dependent font matching

    <p>
    <code>matchFont</code> matches a pattern of font characteristics
    and returns the closest match if possible. If a match was found
    it will update rDFA to the found matching font.
    </p>
    <p>
    implementation note: currently the function is only implemented
    for fontconfig.
    </p>

    @param rDFA
    out of the FontAttributes structure the following
    fields will be used for the match:
    <ul>
    <li>family name</li>
    <li>italic</li>
    <li>width</li>
    <li>weight</li>
    <li>pitch</li>
    </ul>

    @param rLocale
    if <code>rLocal</code> contains non empty strings the corresponding
    locale will be used for font matching also; e.g. "Sans" can result
    in different fonts in e.g. english and japanese
     */
    bool matchFont(FontAttributes& rDFA, const css::lang::Locale& rLocale);

    static std::unique_ptr<FontConfigFontOptions> getFontOptions(const FontAttributes& rFontAttributes, int nSize);

    void Substitute(vcl::font::FontSelectPattern &rPattern, OUString& rMissingCodes);

};

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
