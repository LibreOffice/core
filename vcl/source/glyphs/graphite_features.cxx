/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// Description:
// Parse a string of features specified as & separated pairs.
// e.g.
// 1001=1&2002=2&fav1=0

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <sal/types.h>

#ifdef WNT
#include <tools/svwin.h>
#include <svsys.h>
#endif

#include <vcl/graphite_features.hxx>

using namespace grutils;
// These mustn't conflict with font name lists which use ; and ,
const char GrFeatureParser::FEAT_PREFIX = ':';
const char GrFeatureParser::FEAT_SEPARATOR = '&';
const char GrFeatureParser::FEAT_ID_VALUE_SEPARATOR = '=';

GrFeatureParser::GrFeatureParser(gr::Font & font, const std::string lang)
    : mnNumSettings(0), mbErrors(false)
{
    maLang.rgch[0] = maLang.rgch[1] = maLang.rgch[2] = maLang.rgch[3] = '\0';
    setLang(font, lang);
}

GrFeatureParser::GrFeatureParser(gr::Font & font, const std::string features, const std::string lang)
    : mnNumSettings(0), mbErrors(false)
{
    size_t nEquals = 0;
    size_t nFeatEnd = 0;
    size_t pos = 0;
    maLang.rgch[0] = maLang.rgch[1] = maLang.rgch[2] = maLang.rgch[3] = '\0';
    setLang(font, lang);
    while (pos < features.length() && mnNumSettings < MAX_FEATURES)
    {
        nEquals = features.find(FEAT_ID_VALUE_SEPARATOR,pos);
        if (nEquals == std::string::npos)
        {
            mbErrors = true;
            break;
        }
        // check for a lang=xxx specification
        if (features.compare(pos, nEquals - pos, "lang") == 0)
        {
            pos = nEquals + 1;
            nFeatEnd = features.find(FEAT_SEPARATOR, pos);
            if (nFeatEnd == std::string::npos)
            {
                nFeatEnd = features.length();
            }
            if (nFeatEnd - pos > 3)
                mbErrors = true;
            else
            {
                gr::isocode aLang = maLang;
                for (size_t i = pos; i < nFeatEnd; i++)
                    aLang.rgch[i-pos] = features[i];
                std::pair<gr::LanguageIterator,gr::LanguageIterator> aSupported
                    = font.getSupportedLanguages();
                gr::LanguageIterator iL = aSupported.first;
                while (iL != aSupported.second)
                {
                    gr::isocode aSupportedLang = *iL;
                    // here we only expect full 3 letter codes
                    if (aLang.rgch[0] == aSupportedLang.rgch[0] &&
                        aLang.rgch[1] == aSupportedLang.rgch[1] &&
                        aLang.rgch[2] == aSupportedLang.rgch[2] &&
                        aLang.rgch[3] == aSupportedLang.rgch[3]) break;
                    ++iL;
                }
                if (iL == aSupported.second) mbErrors = true;
                else maLang = aLang;
            }
        }
        else
        {
            if (isCharId(features, pos, nEquals - pos))
                maSettings[mnNumSettings].id = getCharId(features, pos, nEquals - pos);
            else maSettings[mnNumSettings].id = getIntValue(features, pos, nEquals - pos);
            pos = nEquals + 1;
            nFeatEnd = features.find(FEAT_SEPARATOR, pos);
            if (nFeatEnd == std::string::npos)
            {
                nFeatEnd = features.length();
            }
            if (isCharId(features, pos, nFeatEnd - pos))
                maSettings[mnNumSettings].value = getCharId(features, pos, nFeatEnd - pos);
            else
                maSettings[mnNumSettings].value= getIntValue(features, pos, nFeatEnd - pos);
            if (isValid(font, maSettings[mnNumSettings]))
                mnNumSettings++;
            else
                mbErrors = true;
        }
        pos = nFeatEnd + 1;
    }
}

void GrFeatureParser::setLang(gr::Font & font, const std::string & lang)
{
    gr::isocode aLang = {{0,0,0,0}};
    if (lang.length() > 2)
    {
        for (size_t i = 0; i < lang.length() && i < 3; i++)
        {
            if (lang[i] == '-') break;
            aLang.rgch[i] = lang[i];
        }
        std::pair<gr::LanguageIterator,gr::LanguageIterator> aSupported
                    = font.getSupportedLanguages();
        gr::LanguageIterator iL = aSupported.first;
        while (iL != aSupported.second)
        {
            gr::isocode aSupportedLang = *iL;
            if (aLang.rgch[0] == aSupportedLang.rgch[0] &&
                aLang.rgch[1] == aSupportedLang.rgch[1] &&
                aLang.rgch[2] == aSupportedLang.rgch[2] &&
                aLang.rgch[3] == aSupportedLang.rgch[3]) break;
            ++iL;
        }
        if (iL != aSupported.second)
            maLang = aLang;
#ifdef DEBUG
        else
            printf("%s has no features\n", aLang.rgch);
#endif
    }
}

GrFeatureParser::GrFeatureParser(const GrFeatureParser & aCopy)
 : maLang(aCopy.maLang), mbErrors(aCopy.mbErrors)
{
    mnNumSettings = aCopy.getFontFeatures(maSettings);
}

GrFeatureParser::~GrFeatureParser()
{
}

size_t GrFeatureParser::getFontFeatures(gr::FeatureSetting settings[64]) const
{
    if (settings)
    {
        std::copy(maSettings, maSettings + mnNumSettings, settings);
    }
    return mnNumSettings;
}

bool GrFeatureParser::isValid(gr::Font & font, gr::FeatureSetting & setting)
{
    gr::FeatureIterator i = font.featureWithID(setting.id);
    if (font.getFeatures().second == i)
    {
        return false;
    }
    std::pair< gr::FeatureSettingIterator, gr::FeatureSettingIterator >
        validValues = font.getFeatureSettings(i);
    gr::FeatureSettingIterator j = validValues.first;
    while (j != validValues.second)
    {
        if (*j == setting.value) return true;
        ++j;
    }
    return false;
}

bool GrFeatureParser::isCharId(const std::string & id, size_t offset, size_t length)
{
    if (length > 4) return false;
    for (size_t i = 0; i < length; i++)
    {
        if (i > 0 && id[offset+i] == '\0') continue;
        if ((id[offset+i]) < 0x20 || (id[offset+i]) < 0)
            return false;
        if (i==0 && id[offset+i] < 0x41)
            return false;
    }
    return true;
}

int GrFeatureParser::getCharId(const std::string & id, size_t offset, size_t length)
{
    FeatId charId;
    charId.num = 0;
#ifdef WORDS_BIGENDIAN
    for (size_t i = 0; i < length; i++)
    {
        charId.label[i] = id[offset+i];
    }
#else
    for (size_t i = 0; i < length; i++)
    {
        charId.label[3-i] = id[offset+i];
    }
#endif
    return charId.num;
}

int GrFeatureParser::getIntValue(const std::string & id, size_t offset, size_t length)
{
    int value = 0;
    int sign = 1;
    for (size_t i = 0; i < length; i++)
    {
        switch (id[offset + i])
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            value *= 10;
            if (sign < 0)
            {
                value = -(id[offset + i] - '0');
                sign = 1;
            }
            value += (id[offset + i] - '0');
            break;
        case '-':
            if (i == 0)
                sign = -1;
            else
            {
                mbErrors = true;
                break;
            }
        default:
            mbErrors = true;
            break;
        }
    }
    return value;
}


sal_Int32 GrFeatureParser::hashCode() const
{
    union IsoHash { sal_Int32 mInt; gr::isocode mCode; };
    IsoHash isoHash;
    isoHash.mCode = maLang;
    sal_Int32 hash = isoHash.mInt;
    for (size_t i = 0; i < mnNumSettings; i++)
    {
        hash = (hash << 16) ^ ((maSettings[i].id << 8) | maSettings[i].value);
    }
    return hash;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
