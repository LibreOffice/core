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


#include <officecfg/Setup.hxx>
#include <officecfg/System.hxx>
#include <sal/config.h>
#include <sal/macros.h>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

#include <i18nutil/paper.hxx>

#include <utility>
#include <cstdlib>
#include <unotools/configmgr.hxx>

#ifdef UNX
#include <stdio.h>
#include <string.h>
#include <locale.h>
#if defined(LC_PAPER) && defined(_GNU_SOURCE)
#include <langinfo.h>
#endif
#endif

struct PageDesc
{
    long m_nWidth;
    long m_nHeight;
    const char *m_pPSName;
    const char *m_pAltPSName;
};

#define PT2MM100( v ) \
    long(((v) * 35.27777778) + 0.5)

#define IN2MM100( v ) \
    (long(((v) * 2540) + 0.5))

#define MM2MM100( v ) \
    (long((v) * 100))

//PostScript Printer Description File Format Specification
//http://partners.adobe.com/public/developer/en/ps/5003.PPD_Spec_v4.3.pdf
//https://web.archive.org/web/20040912070512/http://www.y-adagio.com/public/committees/docsii/doc_00-49/symp_ulaan/china_ppr.pdf (Kai)
//http://www.sls.psi.ch/controls/help/howto/Howto_Print_a_A0_Poster_at_WSLA_012_2.pdf (Dia)

//!! The order of these entries must correspond to enum Paper in <i18nutil/paper.hxx>

static const PageDesc aDinTab[] =
{
    { MM2MM100( 841 ),   MM2MM100( 1189 ),   "A0",  nullptr },
    { MM2MM100( 594 ),   MM2MM100( 841 ),    "A1",  nullptr },
    { MM2MM100( 420 ),   MM2MM100( 594 ),    "A2",  nullptr },
    { MM2MM100( 297 ),   MM2MM100( 420 ),    "A3",  nullptr },
    { MM2MM100( 210 ),   MM2MM100( 297 ),    "A4",  nullptr },
    { MM2MM100( 148 ),   MM2MM100( 210 ),    "A5",  nullptr },
    { MM2MM100( 250 ),   MM2MM100( 353 ),    "ISOB4",  nullptr },
    { MM2MM100( 176 ),   MM2MM100( 250 ),    "ISOB5",  nullptr },
    { IN2MM100( 8.5 ),   IN2MM100( 11 ),     "Letter",  "Note" },
    { IN2MM100( 8.5 ),   IN2MM100( 14 ),     "Legal",  nullptr },
    { IN2MM100( 11 ),    IN2MM100( 17 ),     "Tabloid",  "11x17" },
    { 0,                 0,                  nullptr, nullptr }, //User
    { MM2MM100( 125 ),   MM2MM100( 176 ),    "ISOB6",  nullptr },
    { MM2MM100( 229 ),   MM2MM100( 324 ),    "EnvC4",  "C4" },
    { MM2MM100( 162 ),   MM2MM100( 229 ),    "EnvC5",  "C5" },
    { MM2MM100( 114 ),   MM2MM100( 162 ),    "EnvC6",  "C6" },
    { MM2MM100( 114 ),   MM2MM100( 229 ),    "EnvC65", nullptr },
    { MM2MM100( 110 ),   MM2MM100( 220 ),    "EnvDL",  "DL" },
    { MM2MM100( 180),    MM2MM100( 270 ),    nullptr,  nullptr }, //Dia
    { MM2MM100( 210),    MM2MM100( 280 ),    nullptr,  nullptr }, //Screen 4:3
    { IN2MM100( 17 ),    IN2MM100( 22 ),     "AnsiC",  "CSheet" },
    { IN2MM100( 22 ),    IN2MM100( 34 ),     "AnsiD",  "DSheet" },
    { IN2MM100( 34 ),    IN2MM100( 44 ),     "AnsiE",  "ESheet" },
    { IN2MM100( 7.25 ),  IN2MM100( 10.5 ),   "Executive",  nullptr },
    //"Folio" is a different size in the PPD documentation than 8.5x11
    //This "FanFoldGermanLegal" is known in the Philippines as
    //"Legal" paper or "Long Bond Paper".  The "Legal" name causing untold
    //misery, given the differently sized US "Legal" paper
    { IN2MM100( 8.5 ),   IN2MM100( 13 ),     "FanFoldGermanLegal",  nullptr },
    { IN2MM100( 3.875 ), IN2MM100( 7.5 ),    "EnvMonarch", "Monarch" },
    { IN2MM100( 3.625 ), IN2MM100( 6.5 ),    "EnvPersonal",  "Personal" },
    { IN2MM100( 3.875 ), IN2MM100( 8.875 ),  "Env9",  nullptr },
    { IN2MM100( 4.125 ), IN2MM100( 9.5 ),    "Env10",  "Comm10" },
    { IN2MM100( 4.5 ),   IN2MM100( 10.375 ), "Env11",  nullptr },
    { IN2MM100( 4.75 ),  IN2MM100( 11 ),     "Env12",  nullptr },
    { MM2MM100( 184 ),   MM2MM100( 260 ),    nullptr,  nullptr }, //Kai16 / 16k
    { MM2MM100( 130 ),   MM2MM100( 184 ),    nullptr,  nullptr }, //Kai32
    { MM2MM100( 140 ),   MM2MM100( 203 ),    nullptr,  nullptr }, //BigKai32
    { MM2MM100( 257 ),   MM2MM100( 364 ),    "B4",  nullptr }, //JIS
    { MM2MM100( 182 ),   MM2MM100( 257 ),    "B5",  nullptr }, //JIS
    { MM2MM100( 128 ),   MM2MM100( 182 ),    "B6",  nullptr }, //JIS
    { IN2MM100( 17 ),    IN2MM100( 11 ),     "Ledger",  nullptr },
    { IN2MM100( 5.5 ),   IN2MM100( 8.5 ),    "Statement",  nullptr },
    { PT2MM100( 610 ),   PT2MM100( 780 ),    "Quarto",  nullptr },
    { IN2MM100( 10 ),    IN2MM100( 14 ),     "10x14",  nullptr },
    { IN2MM100( 5.5 ),   IN2MM100( 11.5 ),   "Env14",  nullptr },
    { MM2MM100( 324 ),   MM2MM100( 458 ),    "EnvC3",  "C3" },
    { MM2MM100( 110 ),   MM2MM100( 230 ),    "EnvItalian",  nullptr },
    { IN2MM100( 14.875 ),IN2MM100( 11 ),     "FanFoldUS",  nullptr },
    { IN2MM100( 8.5 ),   IN2MM100( 13 ),     "FanFoldGerman",  nullptr },
    { MM2MM100( 100 ),   MM2MM100( 148 ),    "Postcard",  nullptr },
    { IN2MM100( 9 ),     IN2MM100( 11 ),     "9x11",  nullptr },
    { IN2MM100( 10 ),    IN2MM100( 11 ),     "10x11",  nullptr },
    { IN2MM100( 15 ),    IN2MM100( 11 ),     "15x11",  nullptr },
    { MM2MM100( 220 ),   MM2MM100( 220 ),    "EnvInvite",  nullptr },
    { MM2MM100( 227 ),   MM2MM100( 356 ),    "SuperA",  nullptr },
    { MM2MM100( 305 ),   MM2MM100( 487 ),    "SuperB",  nullptr },
    { IN2MM100( 8.5 ),   IN2MM100( 12.69 ),  "LetterPlus",  nullptr },
    { MM2MM100( 210 ),   MM2MM100( 330 ),    "A4Plus",  nullptr },
    { MM2MM100( 200 ),   MM2MM100( 148 ),    "DoublePostcard",  nullptr },
    { MM2MM100( 105 ),   MM2MM100( 148 ),    "A6",  nullptr },
    { IN2MM100( 12 ),    IN2MM100( 11 ),     "12x11",  nullptr },
    { MM2MM100( 74 ),    MM2MM100( 105 ),    "A7",  nullptr },
    { MM2MM100( 52 ),    MM2MM100( 74 ),     "A8",  nullptr },
    { MM2MM100( 37 ),    MM2MM100( 52 ),     "A9",  nullptr },
    { MM2MM100( 26 ),    MM2MM100( 37 ),     "A10",  nullptr },
    { MM2MM100( 1000 ),  MM2MM100( 1414 ),   "ISOB0",  nullptr },
    { MM2MM100( 707 ),   MM2MM100( 1000 ),   "ISOB1",  nullptr },
    { MM2MM100( 500 ),   MM2MM100( 707 ),    "ISOB2",  nullptr },
    { MM2MM100( 353 ),   MM2MM100( 500 ),    "ISOB3",  nullptr },
    { MM2MM100( 88 ),    MM2MM100( 125 ),    "ISOB7",  nullptr },
    { MM2MM100( 62 ),    MM2MM100( 88 ),     "ISOB8",  nullptr },
    { MM2MM100( 44 ),    MM2MM100( 62 ),     "ISOB9",  nullptr },
    { MM2MM100( 31 ),    MM2MM100( 44 ),     "ISOB10", nullptr },
    { MM2MM100( 458 ),   MM2MM100( 648 ),    "EnvC2",  "C2" },
    { MM2MM100( 81 ),    MM2MM100( 114 ),    "EnvC7",  "C7" },
    { MM2MM100( 57 ),    MM2MM100( 81 ),     "EnvC8",  "C8" },
    { IN2MM100( 9 ),     IN2MM100( 12 ),     "ARCHA",  nullptr },
    { IN2MM100( 12 ),    IN2MM100( 18 ),     "ARCHB",  nullptr },
    { IN2MM100( 18 ),    IN2MM100( 24 ),     "ARCHC",  nullptr },
    { IN2MM100( 24 ),    IN2MM100( 36 ),     "ARCHD",  nullptr },
    { IN2MM100( 36 ),    IN2MM100( 48 ),     "ARCHE",  nullptr },
    { MM2MM100( 157.5),  MM2MM100( 280 ),    nullptr,  nullptr }, //Screen 16:9
    { MM2MM100( 175 ),   MM2MM100( 280 ),    nullptr,  nullptr }, //Screen 16:10
    { MM2MM100( 195 ),   MM2MM100( 270 ),    nullptr,  nullptr }, // 16k
    { MM2MM100( 197 ),   MM2MM100( 273 ),    nullptr,  nullptr }  // 16k

};

static const size_t nTabSize = SAL_N_ELEMENTS(aDinTab);

#define MAXSLOPPY 21

void PaperInfo::doSloppyFit()
{
    if (m_eType != PAPER_USER)
        return;

    for ( size_t i = 0; i < nTabSize; ++i )
    {
        if (i == PAPER_USER) continue;

        long lDiffW = labs(aDinTab[i].m_nWidth - m_nPaperWidth);
        long lDiffH = labs(aDinTab[i].m_nHeight - m_nPaperHeight);
        long lFlipDiffW = labs(aDinTab[i].m_nHeight - m_nPaperWidth);
        long lFlipDiffH = labs(aDinTab[i].m_nWidth - m_nPaperHeight);

        if ( (lDiffW < MAXSLOPPY && lDiffH < MAXSLOPPY) ||
            (lFlipDiffW < MAXSLOPPY && lFlipDiffH < MAXSLOPPY) )
        {
            m_nPaperWidth = aDinTab[i].m_nWidth;
            m_nPaperHeight = aDinTab[i].m_nHeight;
            m_eType = static_cast<Paper>(i);
            return;
        }
    }
}

bool PaperInfo::sloppyEqual(const PaperInfo &rOther) const
{
    return
    (
      (labs(m_nPaperWidth - rOther.m_nPaperWidth) < MAXSLOPPY) &&
      (labs(m_nPaperHeight - rOther.m_nPaperHeight) < MAXSLOPPY)
    );
}

long PaperInfo::sloppyFitPageDimension(long nDimension)
{
    for ( size_t i = 0; i < nTabSize; ++i )
    {
        if (i == PAPER_USER) continue;
        long lDiff;

        lDiff = labs(aDinTab[i].m_nWidth - nDimension);
        if ( lDiff < MAXSLOPPY )
            return aDinTab[i].m_nWidth;

        lDiff = labs(aDinTab[i].m_nHeight - nDimension);
        if ( lDiff < MAXSLOPPY )
            return aDinTab[i].m_nHeight;
    }
    return nDimension;
}

PaperInfo PaperInfo::getSystemDefaultPaper()
{
    if (utl::ConfigManager::IsFuzzing())
        return PaperInfo(PAPER_A4);

    OUString aLocaleStr = officecfg::Setup::L10N::ooSetupSystemLocale::get();

#ifdef UNX
    // if set to "use system", get papersize from system
    if (aLocaleStr.isEmpty())
    {
        static bool bInitialized = false;
        static PaperInfo aInstance(PAPER_A4);

        if (bInitialized)
            return aInstance;

#ifndef MACOSX
        // try libpaper
        // #i78617# workaround missing paperconf command
        FILE* pPipe = popen( "paperconf 2>/dev/null", "r" );
        if( pPipe )
        {
            Paper ePaper = PAPER_USER;

            char aBuffer[ 1024 ];
            aBuffer[0] = 0;
            char *pBuffer = fgets( aBuffer, sizeof(aBuffer), pPipe );
            bool bOk = pclose(pPipe) == 0;

            if (bOk && pBuffer && *pBuffer != 0)
            {
                OString aPaper(pBuffer);
                aPaper = aPaper.trim();
                static const struct { const char *pName; Paper ePaper; } aCustoms [] =
                {
                    { "B0",   PAPER_B0_ISO },
                    { "B1",   PAPER_B1_ISO },
                    { "B2",   PAPER_B2_ISO },
                    { "B3",   PAPER_B3_ISO },
                    { "B4",   PAPER_B4_ISO },
                    { "B5",   PAPER_B5_ISO },
                    { "B6",   PAPER_B6_ISO },
                    { "B7",   PAPER_B7_ISO },
                    { "B8",   PAPER_B8_ISO },
                    { "B9",   PAPER_B9_ISO },
                    { "B10",  PAPER_B10_ISO },
                    { "folio", PAPER_FANFOLD_LEGAL_DE },
                    { "flsa",  PAPER_FANFOLD_LEGAL_DE },
                    { "flse",  PAPER_FANFOLD_LEGAL_DE }
                };

                bool bHalve = false;

                size_t const nExtraTabSize = SAL_N_ELEMENTS(aCustoms);
                for (size_t i = 0; i < nExtraTabSize; ++i)
                {
                    if (rtl_str_compareIgnoreAsciiCase(aCustoms[i].pName, aPaper.getStr()) == 0)
                    {
                        ePaper = aCustoms[i].ePaper;
                        break;
                    }
                }

                if (ePaper == PAPER_USER)
                {
                    bHalve = aPaper.startsWith("half", &aPaper);
                    ePaper = PaperInfo::fromPSName(aPaper);
                }

                if (ePaper != PAPER_USER)
                {
                    aInstance = PaperInfo(ePaper);
                    if (bHalve)
                        aInstance = PaperInfo(aInstance.getHeight()/2, aInstance.getWidth());
                    bInitialized = true;
                    return aInstance;
                }
            }
        }
#endif

// _NL_PAPER_WIDTH / HEIGHT not available with android unified headers
#if defined(LC_PAPER) && defined(_GNU_SOURCE) && !defined(ANDROID)
        // try LC_PAPER
        locale_t loc = newlocale(LC_PAPER_MASK, "", static_cast<locale_t>(0));
        if (loc != static_cast<locale_t>(0))
        {
            union paperword { char *string; int word; };
            paperword w, h;
            w.string = nl_langinfo_l(_NL_PAPER_WIDTH, loc);
            h.string = nl_langinfo_l(_NL_PAPER_HEIGHT, loc);

            freelocale(loc);

            //glibc stores sizes as integer mm units
            w.word *= 100;
            h.word *= 100;

            for ( size_t i = 0; i < nTabSize; ++i )
            {
                if (i == PAPER_USER) continue;

                //glibc stores sizes as integer mm units, and so is inaccurate.
                //To find a standard paper size we calculate the standard paper
                //sizes into equally inaccurate mm and compare
                long width = (aDinTab[i].m_nWidth + 50) / 100;
                long height = (aDinTab[i].m_nHeight + 50) / 100;

                if (width == w.word/100 && height == h.word/100)
                {
                    w.word = aDinTab[i].m_nWidth;
                    h.word = aDinTab[i].m_nHeight;
                    break;
                }
            }

            aInstance = PaperInfo(w.word, h.word);
            bInitialized = true;
            return aInstance;
        }
#endif
    }
#endif

    // if set to "use system", try to get locale from system
    if (aLocaleStr.isEmpty())
        aLocaleStr = officecfg::System::L10N::Locale::get();

    if (aLocaleStr.isEmpty())
        aLocaleStr = OUString::intern(RTL_CONSTASCII_USTRINGPARAM("en-US"));

    // convert locale string to locale struct
    css::lang::Locale aSysLocale;
    sal_Int32 nDashPos = aLocaleStr.indexOf( '-' );
    if( nDashPos < 0 ) nDashPos = aLocaleStr.getLength();
    aSysLocale.Language = aLocaleStr.copy( 0, nDashPos );
    if( nDashPos + 1 < aLocaleStr.getLength() )
        aSysLocale.Country = aLocaleStr.copy( nDashPos + 1 );

    return PaperInfo::getDefaultPaperForLocale(aSysLocale);
}

PaperInfo::PaperInfo(Paper eType) : m_eType(eType)
{
    static_assert( SAL_N_ELEMENTS(aDinTab) == NUM_PAPER_ENTRIES,
            "mismatch between array entries and enum values" );

    m_nPaperWidth = aDinTab[m_eType].m_nWidth;
    m_nPaperHeight = aDinTab[m_eType].m_nHeight;
}

PaperInfo::PaperInfo(long nPaperWidth, long nPaperHeight)
    : m_eType(PAPER_USER),
      m_nPaperWidth(nPaperWidth),
      m_nPaperHeight(nPaperHeight)
{
    for ( size_t i = 0; i < nTabSize; ++i )
    {
        if (
             (nPaperWidth == aDinTab[i].m_nWidth) &&
             (nPaperHeight == aDinTab[i].m_nHeight)
           )
        {
            m_eType = static_cast<Paper>(i);
            break;
        }
    }
}

OString PaperInfo::toPSName(Paper ePaper)
{
    return static_cast<size_t>(ePaper) < nTabSize ?
        OString(aDinTab[ePaper].m_pPSName) : OString();
}

Paper PaperInfo::fromPSName(const OString &rName)
{
    if (rName.isEmpty())
        return PAPER_USER;

    for ( size_t i = 0; i < nTabSize; ++i )
    {
        if (aDinTab[i].m_pPSName &&
          !rtl_str_compareIgnoreAsciiCase(aDinTab[i].m_pPSName, rName.getStr()))
        {
            return static_cast<Paper>(i);
        }
        else if (aDinTab[i].m_pAltPSName &&
          !rtl_str_compareIgnoreAsciiCase(aDinTab[i].m_pAltPSName, rName.getStr()))
        {
            return static_cast<Paper>(i);
        }
    }

    return PAPER_USER;
}

//http://wiki.openoffice.org/wiki/DefaultPaperSize
//http://www.unicode.org/cldr/data/charts/supplemental/territory_language_information.html
//http://sourceware.org/git/?p=glibc.git;a=tree;f=localedata/locales
//http://en.wikipedia.org/wiki/Paper_size
//http://msdn.microsoft.com/en-us/library/cc195164.aspx
PaperInfo PaperInfo::getDefaultPaperForLocale( const css::lang::Locale & rLocale )
{
    Paper eType = PAPER_A4;

    if (
        //United States, Letter
        rLocale.Country == "US" ||
        //Puerto Rico:
        //    http://unicode.org/cldr/trac/ticket/1710
        //    http://sources.redhat.com/ml/libc-hacker/2001-07/msg00046.html
        rLocale.Country == "PR" ||
        //Canada:
        //    http://sources.redhat.com/ml/libc-hacker/2001-07/msg00053.html
        rLocale.Country == "CA" ||
        //Venuzuela:
        //    http://unicode.org/cldr/trac/ticket/1710
        //    https://www.redhat.com/archives/fedora-devel-list/2008-August/msg00019.html
        rLocale.Country == "VE" ||
        //Chile:
        //    http://unicode.org/cldr/trac/ticket/1710
        //    https://www.redhat.com/archives/fedora-devel-list/2008-August/msg00240.html
        rLocale.Country == "CL" ||
        //Mexico:
        //    http://unicode.org/cldr/trac/ticket/1710
        //    http://qa.openoffice.org/issues/show_bug.cgi?id=49739
        rLocale.Country == "MX" ||
        //Colombia:
        //    http://unicode.org/cldr/trac/ticket/1710
        //    http://qa.openoffice.org/issues/show_bug.cgi?id=69703
        rLocale.Country == "CO" ||
        //Philippines:
        //    http://unicode.org/cldr/trac/ticket/1710
        //    http://ubuntuliving.blogspot.com/2008/07/default-paper-size-in-evince.html
        //    http://www.gov.ph/faqs/driverslicense.asp
        rLocale.Country == "PH" ||
        //Belize:
        //    http://unicode.org/cldr/trac/ticket/2585
        //    http://www.belize.gov.bz/ct.asp?xItem=1666&ctNode=486&mp=27
        rLocale.Country == "BZ" ||
        //Costa Rica:
        //    http://unicode.org/cldr/trac/ticket/2585
        //    http://sources.redhat.com/bugzilla/show_bug.cgi?id=11258
        rLocale.Country == "CR" ||
        //Guatemala:
        //    http://unicode.org/cldr/trac/ticket/2585
        //    http://sources.redhat.com/bugzilla/show_bug.cgi?id=10936
        rLocale.Country == "GT" ||
        //Nicaragua:
        //    http://unicode.org/cldr/trac/ticket/2585
        rLocale.Country == "NI" ||
        //Panama:
        //    http://unicode.org/cldr/trac/ticket/2585
        //    http://www.minsa.gob.pa/minsa/tl_files/documents/baner_informativo/INSTRUMENTO%20DE%20INVESTIGACION%20DE%20RAAV%202009.pdf
        rLocale.Country == "PA" ||
        //El Salvador:
        //    http://unicode.org/cldr/trac/ticket/2585
        //    http://www.tse.gob.sv
        rLocale.Country == "SV"
       )
    {
        eType = PAPER_LETTER;
    }

    return eType;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
