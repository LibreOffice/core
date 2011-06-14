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
#include <windows.h>
#endif

#include <vcl/graphite_features.hxx>

using namespace grutils;
// These mustn't conflict with font name lists which use ; and ,
const char GrFeatureParser::FEAT_PREFIX = ':';
const char GrFeatureParser::FEAT_SEPARATOR = '&';
const char GrFeatureParser::FEAT_ID_VALUE_SEPARATOR = '=';

GrFeatureParser::GrFeatureParser(const gr_face * pFace, const ::rtl::OString lang)
    : mnNumSettings(0), mbErrors(false), mpSettings(NULL)
{
    maLang.label[0] = maLang.label[1] = maLang.label[2] = maLang.label[3] = '\0';
    setLang(pFace, lang);
}

GrFeatureParser::GrFeatureParser(const gr_face * pFace, const ::rtl::OString features, const ::rtl::OString lang)
    : mnNumSettings(0), mbErrors(false), mpSettings(NULL)
{
    sal_Int32 nEquals = 0;
    sal_Int32 nFeatEnd = 0;
    sal_Int32 pos = 0;
    maLang.num = 0u;
    setLang(pFace, lang);
    while ((pos < features.getLength()) && (mnNumSettings < MAX_FEATURES))
    {
        nEquals = features.indexOf(FEAT_ID_VALUE_SEPARATOR, pos);
        if (nEquals == -1)
        {
            mbErrors = true;
            break;
        }
        // check for a lang=xxx specification
        const ::rtl::OString aLangPrefix("lang");
        if (features.match(aLangPrefix, pos ))
        {
            pos = nEquals + 1;
            nFeatEnd = features.indexOf(FEAT_SEPARATOR, pos);
            if (nFeatEnd == -1)
            {
                nFeatEnd = features.getLength();
            }
            if (nFeatEnd - pos > 3)
                mbErrors = true;
            else
            {
                FeatId aLang = maLang;
                aLang.num = 0;
                for (sal_Int32 i = pos; i < nFeatEnd; i++)
                    aLang.label[i-pos] = features[i];

                //ext_std::pair<gr::LanguageIterator,gr::LanguageIterator> aSupported
                //    = font.getSupportedLanguages();
                //gr::LanguageIterator iL = aSupported.first;
                unsigned short i = 0;
                for (; i < gr_face_n_languages(pFace); i++)
                {
                    gr_uint32 nFaceLang = gr_face_lang_by_index(pFace, i);
                    FeatId aSupportedLang;
                    aSupportedLang.num = nFaceLang;
#ifdef __BIG_ENDIAN__
                    // here we only expect full 3 letter codes
                    if (aLang.label[0] == aSupportedLang.label[0] &&
                        aLang.label[1] == aSupportedLang.label[1] &&
                        aLang.label[2] == aSupportedLang.label[2] &&
                        aLang.label[3] == aSupportedLang.label[3])
#else
                    if (aLang.label[0] == aSupportedLang.label[3] &&
                        aLang.label[1] == aSupportedLang.label[2] &&
                        aLang.label[2] == aSupportedLang.label[1] &&
                        aLang.label[3] == aSupportedLang.label[0])
#endif
                    {
                        maLang = aSupportedLang;
                        break;
                    }
                }
                if (i == gr_face_n_languages(pFace)) mbErrors = true;
                else
                {
                    mnHash = maLang.num;
                    mpSettings = gr_face_featureval_for_lang(pFace, maLang.num);
                }
            }
        }
        else
        {
            sal_uInt32 featId = 0;
            if (isCharId(features, pos, nEquals - pos))
            {
                featId = getCharId(features, pos, nEquals - pos);
            }
            else
            {
                featId = getIntValue(features, pos, nEquals - pos);
            }
            const gr_feature_ref * pFref = gr_face_find_fref(pFace, featId);
            pos = nEquals + 1;
            nFeatEnd = features.indexOf(FEAT_SEPARATOR, pos);
            if (nFeatEnd == -1)
            {
                nFeatEnd = features.getLength();
            }
            sal_Int16 featValue = 0;
            featValue = getIntValue(features, pos, nFeatEnd - pos);
            if (pFref && gr_fref_set_feature_value(pFref, featValue, mpSettings))
            {
                mnHash = (mnHash << 16) ^ ((featId << 8) | featValue);
                mnNumSettings++;
            }
            else
                mbErrors = true;
        }
        pos = nFeatEnd + 1;
    }
}

void GrFeatureParser::setLang(const gr_face * pFace, const rtl::OString & lang)
{
    FeatId aLang;
    aLang.num = 0;
    if (lang.getLength() >= 2)
    {
        for (sal_Int32 i = 0; i < lang.getLength() && i < 3; i++)
        {
            if (lang[i] == '-') break;
            aLang.label[i] = lang[i];
        }
        unsigned short i = 0;
        for (; i < gr_face_n_languages(pFace); i++)
        {
            gr_uint32 nFaceLang = gr_face_lang_by_index(pFace, i);
            FeatId aSupportedLang;
            aSupportedLang.num = nFaceLang;
            // here we only expect full 2 & 3 letter codes
#ifdef __BIG_ENDIAN__
            if (aLang.label[0] == aSupportedLang.label[0] &&
                aLang.label[1] == aSupportedLang.label[1] &&
                aLang.label[2] == aSupportedLang.label[2] &&
                aLang.label[3] == aSupportedLang.label[3])
#else
            if (aLang.label[0] == aSupportedLang.label[3] &&
                aLang.label[1] == aSupportedLang.label[2] &&
                aLang.label[2] == aSupportedLang.label[1] &&
                aLang.label[3] == aSupportedLang.label[0])
#endif
            {
                maLang = aSupportedLang;
                break;
            }
        }
        if (i != gr_face_n_languages(pFace))
        {
            if (mpSettings)
                gr_featureval_destroy(mpSettings);
            mpSettings = gr_face_featureval_for_lang(pFace, maLang.num);
            mnHash = maLang.num;
        }
    }
    if (!mpSettings)
        mpSettings = gr_face_featureval_for_lang(pFace, 0);
}

GrFeatureParser::~GrFeatureParser()
{
    if (mpSettings)
    {
        gr_featureval_destroy(mpSettings);
        mpSettings = NULL;
    }
}

bool GrFeatureParser::isCharId(const rtl::OString & id, size_t offset, size_t length)
{
    if (length > 4) return false;
    for (size_t i = 0; i < length; i++)
    {
        if (i > 0 && id[offset+i] == '\0') continue;
        if ((id[offset+i] < 0x20) || (id[offset+i] < 0))
            return false;
        if (i==0 && (id[offset+i] < 0x41))
            return false;
    }
    return true;
}

gr_uint32 GrFeatureParser::getCharId(const rtl::OString & id, size_t offset, size_t length)
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

short GrFeatureParser::getIntValue(const rtl::OString & id, size_t offset, size_t length)
{
    short value = 0;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
