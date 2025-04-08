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
#ifndef INCLUDED_SW_INC_TXATRITR_HXX
#define INCLUDED_SW_INC_TXATRITR_HXX

#include <sal/types.h>
#include <editeng/langitem.hxx>
#include "swtypes.hxx"
#include <deque>

class SwTextNode;
class SwTextAttr;

class SwScriptIterator
{
    const OUString& m_rText;
    sal_Int32 m_nChgPos;
    sal_uInt16 m_nCurScript;
    bool m_bForward;

public:
    SwScriptIterator( const OUString& rStr, sal_Int32 nStart,
                      bool bFrwrd = true );

    void Next();

    sal_uInt16 GetCurrScript() const { return m_nCurScript; }
    sal_Int32 GetScriptChgPos() const { return (m_nChgPos == -1) ? COMPLETE_STRING : m_nChgPos; }
    const OUString& GetText() const { return m_rText; }
};

class SwLanguageIterator
{
    SwScriptIterator    m_aScriptIter;
    std::deque<const SwTextAttr*>
                        m_aStack;
    const SwTextNode&   m_rTextNode;
    const SvxLanguageItem* m_pParaItem;
    const SvxLanguageItem* m_pCurrentItem;
    size_t              m_nAttrPos;
    sal_Int32           m_nChgPos;

    void AddToStack( const SwTextAttr& rAttr );
    void SearchNextChg();

public:
    SwLanguageIterator( const SwTextNode& rTextNd, sal_Int32 nStart );

    bool               Next();
    sal_Int32          GetChgPos() const        { return m_nChgPos; }
    LanguageType       GetLanguage() const { return m_pCurrentItem->GetValue(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
