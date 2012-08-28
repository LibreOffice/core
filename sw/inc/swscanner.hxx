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
#ifndef _SWSCANNER_HXX
#define _SWSCANNER_HXX

#include <i18npool/lang.h>
#include <modeltoviewhelper.hxx>

class SwTxtNode;

/*************************************************************************
 *                class SwScanner
 * Helper class that provides consecutively the words of a selected area
 * during spell check
 *************************************************************************/

class SwScanner
{
    rtl::OUString aWord;
    const SwTxtNode& rNode;
    const rtl::OUString aPreDashReplacementText;
    rtl::OUString aText;
    const LanguageType* pLanguage;
    const ModelToViewHelper& rConversionMap;
    sal_Int32 nStartPos;
    sal_Int32 nEndPos;
    sal_Int32 nBegin;
    sal_Int32 nLen;
    sal_Int32 nOverriddenDashCount;
    LanguageType aCurrLang;
    sal_uInt16 nWordType;
    sal_Bool bClip;

public:
    SwScanner( const SwTxtNode& rNd, const rtl::OUString& rTxt,
               const LanguageType* pLang,
               const ModelToViewHelper& rConvMap,
               sal_uInt16 nWordType,
               sal_Int32 nStart, sal_Int32 nEnde, sal_Bool bClip = sal_False );


    // This next word function tries to find the language for the next word
    // It should currently _not_ be used for spell checking, and works only for
    // ! bReverse
    sal_Bool NextWord();

    const rtl::OUString& GetWord() const    { return aWord; }

    sal_Int32 GetBegin() const         { return nBegin; }
    sal_Int32 GetEnd() const           { return nBegin + nLen; }
    sal_Int32 GetLen() const           { return nLen; }

    LanguageType GetCurrentLanguage() const {return aCurrLang;}

    sal_Int32 getOverriddenDashCount() const {return nOverriddenDashCount; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
