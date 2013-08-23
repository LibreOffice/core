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
#ifndef _TXATRITR_HXX
#define _TXATRITR_HXX

#include <tools/solar.h>
#include <sal/types.h>
#include <editeng/langitem.hxx>
#include <hintids.hxx>
#include <deque>
#include <tools/string.hxx>

class SwTxtNode;
class SwTxtAttr;
class SfxPoolItem;


class SwScriptIterator
{
    const OUString& m_rText;
    sal_Int32 m_nChgPos;
    sal_uInt16 nCurScript;
    bool bForward;

public:
    SwScriptIterator( const OUString& rStr, xub_StrLen nStart = 0,
                      bool bFrwrd = true );

    bool Next();

    sal_uInt16 GetCurrScript() const        { return nCurScript; }
    xub_StrLen GetScriptChgPos() const      { return (m_nChgPos == -1) ? STRING_LEN : m_nChgPos; }
    const OUString& GetText() const         { return m_rText; }
};


class SwTxtAttrIterator
{
    SwScriptIterator aSIter;
    std::deque<const SwTxtAttr*> aStack;
    const SwTxtNode& rTxtNd;
    const SfxPoolItem *pParaItem, *pCurItem;
    xub_StrLen nChgPos;
    sal_uInt16 nAttrPos, nWhichId;
    bool bIsUseGetWhichOfScript;

    void AddToStack( const SwTxtAttr& rAttr );
    void SearchNextChg();

public:
    SwTxtAttrIterator( const SwTxtNode& rTxtNd, sal_uInt16 nWhichId,
                        xub_StrLen nStart = 0, bool bUseGetWhichOfScript = true );

    bool Next();

    const SfxPoolItem& GetAttr() const  { return *pCurItem; }
    xub_StrLen GetChgPos() const        { return nChgPos; }
};


class SwLanguageIterator : public SwTxtAttrIterator
{
public:
    SwLanguageIterator( const SwTxtNode& rTxtNode, xub_StrLen nStart = 0,
                        sal_uInt16 nWhich = RES_CHRATR_LANGUAGE,
                        bool bUseGetWhichOfScript = true )
        : SwTxtAttrIterator( rTxtNode, nWhich, nStart, bUseGetWhichOfScript )
    {}

    sal_uInt16 GetLanguage() const
        { return ((SvxLanguageItem&)GetAttr()).GetValue(); }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
