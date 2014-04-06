/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_HTML_HXX
#define INCLUDED_HTML_HXX

#include <rtl/string.hxx>
#include <tools/stream.hxx>
#include <vector>
#include <svtools/svtdllapi.h>

class SVT_DLLPUBLIC HtmlWriter
{
private:
    std::vector<OString> maElementStack;
    SvStream&            mrStream;

    bool                 mbElementOpen;
    bool                 mbContentWritten;
    bool                 mbPrettyPrint;

public:
    HtmlWriter(SvStream& rStream);
    virtual ~HtmlWriter();

    void prettyPrint(bool bChoice);

    void start(const OString &aElement);
    void end();
    void write(const OString &aContent);
    void attribute(const OString &aAttribute, const OString &aValue);
    void single(const OString &aContent);
    void endAttribute();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
