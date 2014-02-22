/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/i18n/CTLScriptType.hpp>
#include <com/sun/star/i18n/ScriptDirection.hpp>
#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <i18nutil/scripttypedetector.hxx>
#include <i18nutil/unicode.hxx>

using namespace com::sun::star::i18n;

static const sal_Int16 scriptDirection[] = {
    ScriptDirection::LEFT_TO_RIGHT,     
    ScriptDirection::RIGHT_TO_LEFT,     
    ScriptDirection::LEFT_TO_RIGHT,     
    ScriptDirection::LEFT_TO_RIGHT,     
    ScriptDirection::LEFT_TO_RIGHT,     
    ScriptDirection::RIGHT_TO_LEFT,     
    ScriptDirection::NEUTRAL,           
    ScriptDirection::NEUTRAL,           
    ScriptDirection::NEUTRAL,           
    ScriptDirection::NEUTRAL,           
    ScriptDirection::NEUTRAL,           
    ScriptDirection::LEFT_TO_RIGHT,     
    ScriptDirection::LEFT_TO_RIGHT,     
    ScriptDirection::RIGHT_TO_LEFT,     
    ScriptDirection::RIGHT_TO_LEFT,     
    ScriptDirection::RIGHT_TO_LEFT,     
    ScriptDirection::NEUTRAL,           
    ScriptDirection::NEUTRAL,           
    ScriptDirection::NEUTRAL,           
};

sal_Int16 ScriptTypeDetector::getScriptDirection( const OUString& Text, sal_Int32 nPos, sal_Int16 defaultScriptDirection )
{
    sal_Int16 dir = scriptDirection[unicode::getUnicodeDirection(Text[nPos])];
    return (dir == ScriptDirection::NEUTRAL) ? defaultScriptDirection : dir;
}


sal_Int32 ScriptTypeDetector::beginOfScriptDirection( const OUString& Text, sal_Int32 nPos, sal_Int16 direction )
{
        sal_Int32 cPos = nPos;

        if (cPos < Text.getLength()) {
            for (; cPos >= 0; cPos--) {
                if (direction != getScriptDirection(Text, cPos, direction))
                    break;
            }
        }
        return cPos == nPos ? -1 : cPos + 1;
}

sal_Int32 ScriptTypeDetector::endOfScriptDirection( const OUString& Text, sal_Int32 nPos, sal_Int16 direction )
{
        sal_Int32 cPos = nPos;
        sal_Int32 len = Text.getLength();

        if (cPos >=0) {
            for (; cPos < len; cPos++) {
                if (direction != getScriptDirection(Text, cPos, direction))
                    break;
            }
        }
        return cPos == nPos ? -1 : cPos;
}

sal_Int16 ScriptTypeDetector::getCTLScriptType( const OUString& Text, sal_Int32 nPos )
{
    static const ScriptTypeList typeList[] = {
        { UnicodeScript_kHebrew, UnicodeScript_kHebrew, CTLScriptType::CTL_HEBREW },    
        { UnicodeScript_kArabic, UnicodeScript_kArabic, CTLScriptType::CTL_ARABIC },    
        { UnicodeScript_kDevanagari, UnicodeScript_kDevanagari, CTLScriptType::CTL_INDIC },     
        { UnicodeScript_kThai, UnicodeScript_kThai, CTLScriptType::CTL_THAI },      
        { UnicodeScript_kScriptCount, UnicodeScript_kScriptCount,   CTLScriptType::CTL_UNKNOWN }    
    };

    return unicode::getUnicodeScriptType(Text[nPos], typeList, CTLScriptType::CTL_UNKNOWN);
}


sal_Int32 ScriptTypeDetector::beginOfCTLScriptType( const OUString& Text, sal_Int32 nPos )
{
    if (nPos < 0)
        return 0;
    else if (nPos >= Text.getLength())
        return Text.getLength();
    else {
        sal_Int16 cType = getCTLScriptType(Text, nPos);
        for (nPos--; nPos >= 0; nPos--) {
            if (cType != getCTLScriptType(Text, nPos))
                break;
        }
        return nPos + 1;
    }
}


sal_Int32 ScriptTypeDetector::endOfCTLScriptType( const OUString& Text, sal_Int32 nPos )
{
    if (nPos < 0)
        return 0;
    else if (nPos >= Text.getLength())
        return Text.getLength();
    else {
        sal_Int16 cType = getCTLScriptType(Text, nPos);
        sal_Int32 len = Text.getLength();
        for (nPos++; nPos < len; nPos++) {
            if (cType != getCTLScriptType(Text, nPos))
                break;
        }
        return nPos;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
