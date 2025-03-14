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

#ifndef INCLUDED_SW_INC_SWSCANNER_HXX
#define INCLUDED_SW_INC_SWSCANNER_HXX

#include <i18nlangtag/lang.h>
#include "modeltoviewhelper.hxx"
#include <unotools/charclass.hxx>

#include <functional>
#include <optional>

class SwTextNode;

// Helper class that provides consecutively the words of a selected area
// during spell check
class SwScanner
{
    std::function<LanguageType (sal_Int32, sal_Int32, bool)> m_pGetLangOfChar;
    OUString m_aWord;
    OUString m_aPrevWord;
    const OUString m_aPreDashReplacementText;
    OUString m_aText;
    const LanguageType* m_pLanguage;
    const ModelToViewHelper m_ModelToView;
    sal_Int32 m_nStartPos;
    sal_Int32 m_nEndPos;
    sal_Int32 m_nBegin;
    sal_Int32 m_nLength;
    sal_Int32 m_nOverriddenDashCount;
    // caches the CharClass for m_aCurrentLang, which can be expensive to repeatedly retrieve
    std::optional<CharClass> moCharClass;
    LanguageType m_aCurrentLang;
    sal_uInt16 m_nWordType;
    bool m_bClip;

public:
    SwScanner(std::function<LanguageType(sal_Int32, sal_Int32, bool)>  pGetLangOfChar,
              OUString aText, const LanguageType* pLang, ModelToViewHelper aConvMap,
              sal_uInt16 nWordType, sal_Int32 nStart, sal_Int32 nEnd, bool bClip = false);
    SwScanner( const SwTextNode& rNd, const OUString& rText,
               const LanguageType* pLang,
               const ModelToViewHelper& rConvMap,
               sal_uInt16 nWordType,
               sal_Int32 nStart, sal_Int32 nEnd, bool bClip = false );

    // This next word function tries to find the language for the next word
    // It should currently _not_ be used for spell checking, and works only for
    // ! bReverse
    bool NextWord();

    const OUString& GetWord() const    { return m_aWord; }
    const OUString& GetPrevWord() const { return m_aPrevWord; }

    sal_Int32 GetBegin() const         { return m_nBegin; }
    sal_Int32 GetEnd() const           { return m_nBegin + m_nLength; }
    sal_Int32 GetLen() const           { return m_nLength; }

    LanguageType GetCurrentLanguage() const {return m_aCurrentLang;}

    sal_Int32 getOverriddenDashCount() const {return m_nOverriddenDashCount; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
