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
#include <modeltoviewhelper.hxx>

class SwTxtNode;

/*************************************************************************
 *                class SwScanner
 * Helper class that provides consecutively the words of a selected area
 * during spell check
 *************************************************************************/

class SwScanner
{
    OUString aWord;
    const SwTxtNode& rNode;
    const OUString aPreDashReplacementText;
    OUString aText;
    const LanguageType* pLanguage;
    const ModelToViewHelper rConversionMap;
    sal_Int32 nStartPos;
    sal_Int32 nEndPos;
    sal_Int32 nBegin;
    sal_Int32 nLen;
    sal_Int32 nOverriddenDashCount;
    LanguageType aCurrLang;
    sal_uInt16 nWordType;
    sal_Bool bClip;

public:
    SwScanner( const SwTxtNode& rNd, const OUString& rTxt,
               const LanguageType* pLang,
               const ModelToViewHelper& rConvMap,
               sal_uInt16 nWordType,
               sal_Int32 nStart, sal_Int32 nEnde, sal_Bool bClip = sal_False );

    // This next word function tries to find the language for the next word
    // It should currently _not_ be used for spell checking, and works only for
    // ! bReverse
    sal_Bool NextWord();

    const OUString& GetWord() const    { return aWord; }

    sal_Int32 GetBegin() const         { return nBegin; }
    sal_Int32 GetEnd() const           { return nBegin + nLen; }
    sal_Int32 GetLen() const           { return nLen; }

    LanguageType GetCurrentLanguage() const {return aCurrLang;}

    sal_Int32 getOverriddenDashCount() const {return nOverriddenDashCount; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
