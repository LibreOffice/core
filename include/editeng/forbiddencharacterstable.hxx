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

#ifndef INCLUDED_EDITENG_FORBIDDENCHARACTERSTABLE_HXX
#define INCLUDED_EDITENG_FORBIDDENCHARACTERSTABLE_HXX

#include <salhelper/simplereferenceobject.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/i18n/ForbiddenCharacters.hpp>
#include <editeng/editengdllapi.h>
#include <i18nlangtag/lang.h>
#include <map>
#include <memory>

namespace com {
namespace sun {
namespace star {
namespace uno {
    class XComponentContext;
}}}}

class EDITENG_DLLPUBLIC SvxForbiddenCharactersTable
{
public:
    typedef std::map<LanguageType, css::i18n::ForbiddenCharacters> Map;
private:
    Map                                                maMap;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    SvxForbiddenCharactersTable(const css::uno::Reference< css::uno::XComponentContext >& rxContext);

public:
    static std::shared_ptr<SvxForbiddenCharactersTable> makeForbiddenCharactersTable(const css::uno::Reference<css::uno::XComponentContext>& rxContext);

    Map&    GetMap() { return maMap; }
    const css::i18n::ForbiddenCharacters* GetForbiddenCharacters( LanguageType nLanguage, bool bGetDefault );
    void    SetForbiddenCharacters(  LanguageType nLanguage , const css::i18n::ForbiddenCharacters& );
    void    ClearForbiddenCharacters( LanguageType nLanguage );
};

#endif // INCLUDED_EDITENG_FORBIDDENCHARACTERSTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
