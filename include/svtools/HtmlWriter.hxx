/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SVTOOLS_HTMLWRITER_HXX
#define INCLUDED_SVTOOLS_HTMLWRITER_HXX

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <vector>
#include <svtools/svtdllapi.h>

class SvStream;

class SVT_DLLPUBLIC HtmlWriter
{
private:
    std::vector<OString> maElementStack;

    SvStream& mrStream;

    bool mbElementOpen;
    bool mbContentWritten;
    bool mbPrettyPrint;
    rtl_TextEncoding maEncoding;

public:
    HtmlWriter(SvStream& rStream);
    virtual ~HtmlWriter();

    void prettyPrint(bool bChoice);

    void start(const OString& aElement);

    void end();

    void flushStack();
    void flushStack(const OString& aElement);

    void write(const OString& aContent);

    void attribute(const OString& aAttribute, const char* aValue);
    void attribute(const OString& aAttribute, sal_Int32 aValue);
    void attribute(const OString& aAttribute, const OString& aValue);
    void attribute(const OString& aAttribute, const OUString& aValue);
    // boolean attribute e.g. <img ismap>
    void attribute(const OString& aAttribute);

    void single(const OString& aContent);
    void endAttribute();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
