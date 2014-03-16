/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <svtools/HtmlWriter.hxx>

HtmlWriter::HtmlWriter(SvStream& rStream) :
    mrStream(rStream),
    mbElementOpen(false),
    mbContentWritten(false),
    mbPrettyPrint(true)
{}

HtmlWriter::~HtmlWriter()
{}

void HtmlWriter::prettyPrint(bool bChoice)
{
    mbPrettyPrint = bChoice;
}

void HtmlWriter::start(OString aElement)
{
    if (mbElementOpen)
    {
        mrStream.WriteChar('>');
        if (!mbContentWritten && mbPrettyPrint)
            mrStream.WriteChar('\n');
        mbContentWritten = false;
    }
    maElementStack.push_back(aElement);

    if (mbPrettyPrint)
    {
        for(size_t i = 0; i < maElementStack.size() - 1; i++)
        {
            mrStream.WriteCharPtr("  ");
        }
    }

    mrStream.WriteChar('<');
    mrStream.WriteOString(aElement);
    mbElementOpen = true;
}

void HtmlWriter::single(OString aContent)
{
    start(aContent);
    end();
}

void HtmlWriter::endAttribute()
{
    if (mbElementOpen)
    {
        mrStream.WriteCharPtr("/>");
        if (mbPrettyPrint)
            mrStream.WriteCharPtr("\n");
        mbElementOpen = false;
    }
}

void HtmlWriter::end()
{
    if (mbElementOpen)
    {
        mrStream.WriteCharPtr("/>");
        if (mbPrettyPrint)
            mrStream.WriteCharPtr("\n");
    }
    else
    {
        if (!mbContentWritten && mbPrettyPrint)
        {
            for(size_t i = 0; i < maElementStack.size() - 1; i++)
            {
                mrStream.WriteCharPtr("  ");
            }
        }
        mrStream.WriteCharPtr("</");
        mrStream.WriteOString(maElementStack.back());
        mrStream.WriteCharPtr(">");
        if (mbPrettyPrint)
            mrStream.WriteCharPtr("\n");
    }
    maElementStack.pop_back();
    mbElementOpen = false;
    mbContentWritten = false;
}

void HtmlWriter::write(OString aContent)
{
    if (mbElementOpen)
    {
        mrStream.WriteChar('>');
        mbElementOpen = false;
    }
    mbContentWritten = true;
    mrStream.WriteOString(aContent);
}

void HtmlWriter::attribute(OString aAttribute, OString aValue)
{
    if (mbElementOpen && !aAttribute.isEmpty() && !aValue.isEmpty())
    {
        mrStream.WriteChar(' ');
        mrStream.WriteOString(aAttribute);
        mrStream.WriteChar('=');
        mrStream.WriteChar('"');
        mrStream.WriteOString(aValue);
        mrStream.WriteChar('"');
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
