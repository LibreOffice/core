/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <string_view>
#include <vector>
#include <svtools/svtdllapi.h>

class SvStream;

class SVT_DLLPUBLIC HtmlWriter final
{
private:
    std::vector<OString> maElementStack;

    SvStream& mrStream;

    bool mbElementOpen;
    bool mbCharactersWritten;
    bool mbPrettyPrint;
    /// XML namespace, in case of XHTML.
    OString maNamespace;

public:
    HtmlWriter(SvStream& rStream, std::string_view rNamespace = std::string_view());
    ~HtmlWriter();

    void prettyPrint(bool b);

    void start(const OString& aElement);

    void end();

    void flushStack();

    void attribute(const OString& aAttribute, const char* aValue);
    void attribute(const OString& aAttribute, sal_Int32 aValue);
    void attribute(const OString& aAttribute, const OString& aValue);
    void attribute(const OString& aAttribute, std::u16string_view aValue);
    // boolean attribute e.g. <img ismap>
    void attribute(const OString& aAttribute);

    void single(const OString& aContent);
    void endAttribute();

    /// Writes character data.
    void characters(const OString& rChars);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
