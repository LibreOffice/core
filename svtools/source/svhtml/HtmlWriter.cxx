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
#include <sal/log.hxx>
#include <svtools/htmlout.hxx>

HtmlWriter::HtmlWriter(SvStream& rStream, std::string_view rNamespace) :
    mrStream(rStream),
    mbPrettyPrint(true)
{
    if (!rNamespace.empty())
    {
        // Convert namespace alias to a prefix.
        maNamespace = OString::Concat(rNamespace) + ":";
    }
}

HtmlWriter::~HtmlWriter()
{}

void HtmlWriter::prettyPrint(bool b)
{
    mbPrettyPrint = b;
}

void HtmlWriter::start(const OString& aElement)
{
    if (mbOpeningTagOpen)
    {
        mrStream.WriteChar('>');
        if (mbPrettyPrint)
            mrStream.WriteChar('\n');
    }
    maElementStack.push_back(aElement);

    if (mbPrettyPrint)
    {
        for(size_t i = 0; i < maElementStack.size() - 1; i++)
        {
            mrStream.WriteOString("  ");
        }
    }

    mrStream.WriteChar('<');
    mrStream.WriteOString(Concat2View(maNamespace + aElement));
    mbOpeningTagOpen = true;
}

void HtmlWriter::single(const OString &aContent)
{
    start(aContent);
    end();
}

void HtmlWriter::endAttribute()
{
    if (mbOpeningTagOpen)
    {
        mrStream.WriteOString("/>");
        if (mbPrettyPrint)
            mrStream.WriteOString("\n");
        mbOpeningTagOpen = false;
    }
}

void HtmlWriter::flushStack()
{
    while (!maElementStack.empty())
    {
        end();
    }
}

bool HtmlWriter::end(const OString& aElement)
{
    bool bExpected = maElementStack.back() == aElement;
    SAL_WARN_IF(!bExpected, "svtools", "HtmlWriter: end element mismatch - '" << aElement << "' expected '" << maElementStack.back() << "'");
    end();
    return bExpected;
}

void HtmlWriter::end()
{
    if (mbOpeningTagOpen)
    {
        mrStream.WriteOString("/>");
        if (mbPrettyPrint)
            mrStream.WriteOString("\n");
        mbOpeningTagOpen = false;
    }
    else
    {
        if (mbPrettyPrint)
        {
            for(size_t i = 0; i < maElementStack.size() - 1; i++)
            {
                mrStream.WriteOString("  ");
            }
        }
        mrStream.WriteOString("</");
        mrStream.WriteOString(Concat2View(maNamespace + maElementStack.back()));
        mrStream.WriteOString(">");
        if (mbPrettyPrint)
            mrStream.WriteOString("\n");
    }
    maElementStack.pop_back();
}

void HtmlWriter::writeAttribute(SvStream& rStream, std::string_view aAttribute, sal_Int32 aValue)
{
    writeAttribute(rStream, aAttribute, OString::number(aValue));
}

void HtmlWriter::writeAttribute(SvStream& rStream, std::string_view aAttribute, std::string_view aValue)
{
    rStream.WriteOString(aAttribute);
    rStream.WriteChar('=');
    rStream.WriteChar('"');
    HTMLOutFuncs::Out_String(rStream, OStringToOUString(aValue, RTL_TEXTENCODING_UTF8));
    rStream.WriteChar('"');
}

void HtmlWriter::attribute(std::string_view aAttribute, std::string_view aValue)
{
    if (mbOpeningTagOpen && !aAttribute.empty() && !aValue.empty())
    {
        mrStream.WriteChar(' ');
        writeAttribute(mrStream, aAttribute, aValue);
    }
}

void HtmlWriter::attribute(std::string_view aAttribute, const sal_Int32 aValue)
{
    attribute(aAttribute, OString::number(aValue));
}

void HtmlWriter::attribute(std::string_view aAttribute, const char* pValue)
{
    attribute(aAttribute, std::string_view(pValue));
}

void HtmlWriter::attribute(std::string_view aAttribute, std::u16string_view aValue)
{
    attribute(aAttribute, OUStringToOString(aValue, RTL_TEXTENCODING_UTF8));
}

void HtmlWriter::attribute(std::string_view aAttribute)
{
    if (mbOpeningTagOpen && !aAttribute.empty())
    {
        mrStream.WriteChar(' ');
        mrStream.WriteOString(aAttribute);
    }
}

void HtmlWriter::characters(std::string_view rChars)
{
    if (mbOpeningTagOpen)
    {
        mrStream.WriteOString(">");
        mbOpeningTagOpen = false;
    }
    mrStream.WriteOString(rChars);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
