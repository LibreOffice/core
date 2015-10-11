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
#include <tools/stream.hxx>

HtmlWriter::HtmlWriter(SvStream& rStream) :
    mrStream(rStream),
    mbElementOpen(false),
    mbContentWritten(false),
    mbPrettyPrint(true),
    maEncoding(RTL_TEXTENCODING_UTF8)
{}

HtmlWriter::~HtmlWriter()
{}

void HtmlWriter::prettyPrint(bool bChoice)
{
    mbPrettyPrint = bChoice;
}

void HtmlWriter::start(const OString& aElement)
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

void HtmlWriter::single(const OString &aContent)
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

void HtmlWriter::flushStack()
{
    while (!maElementStack.empty())
    {
        end();
    }
}

void HtmlWriter::flushStack(const OString& aElement)
{
    OString sCurrentElement;
    do
    {
        sCurrentElement = maElementStack.back();
        end();
    } while (!maElementStack.empty() && aElement != sCurrentElement);
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

void HtmlWriter::write(const OString &aContent)
{
    if (mbElementOpen)
    {
        mrStream.WriteChar('>');
        mbElementOpen = false;
    }
    mbContentWritten = true;
    mrStream.WriteOString(aContent);
}

void HtmlWriter::attribute(const OString &aAttribute, const OString& aValue)
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

void HtmlWriter::attribute(const OString& aAttribute, const sal_Int32 aValue)
{
    attribute(aAttribute, OString::number(aValue));
}

void HtmlWriter::attribute(const OString& aAttribute, const char* pValue)
{
    attribute(aAttribute, OString(pValue));
}

void HtmlWriter::attribute(const OString& aAttribute, const OUString& aValue)
{
    attribute(aAttribute, OUStringToOString(aValue, maEncoding));
}

void HtmlWriter::attribute(const OString& aAttribute)
{
    if (mbElementOpen && !aAttribute.isEmpty())
    {
        mrStream.WriteChar(' ');
        mrStream.WriteOString(aAttribute);
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
