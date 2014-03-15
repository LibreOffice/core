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
    mStream(rStream),
    mElementOpen(false),
    mContentWritten(false)
{}

HtmlWriter::~HtmlWriter()
{}

void HtmlWriter::start(OString aElement)
{
    if (mElementOpen)
    {
        mStream.WriteChar('>');
        if (!mContentWritten)
            mStream.WriteChar('\n');
        mContentWritten = false;
    }
    mElementStack.push_back(aElement);

    for(size_t i = 0; i < mElementStack.size() - 1; i++)
        mStream.WriteCharPtr("  ");

    mStream.WriteChar('<');
    mStream.WriteOString(aElement);
    mElementOpen = true;
}

void HtmlWriter::endAttribute()
{
    if (mElementOpen)
    {
        mStream.WriteCharPtr(" />");
        mStream.WriteCharPtr("\n");
        mElementOpen = false;
    }
}

void HtmlWriter::end()
{
    if (mElementOpen)
    {
        mStream.WriteCharPtr(" />");
        mStream.WriteCharPtr("\n");
    }
    else
    {
        if (!mContentWritten)
        {
            for(size_t i = 0; i < mElementStack.size() - 1; i++)
            {
                mStream.WriteCharPtr("  ");
            }
        }
        mStream.WriteCharPtr("</");
        mStream.WriteOString(mElementStack.back());
        mStream.WriteCharPtr(">\n");
    }
    mElementStack.pop_back();
    mElementOpen = false;
    mContentWritten = false;
}

void HtmlWriter::write(OString aContent)
{
    if (mElementOpen)
    {
        mStream.WriteChar('>');
        mElementOpen = false;
    }
    mContentWritten = true;
    mStream.WriteOString(aContent);
}

void HtmlWriter::attribute(OString aAttribute, OString aValue)
{
    if (mElementOpen && !aAttribute.isEmpty() && !aValue.isEmpty())
    {
        mStream.WriteChar(' ');
        mStream.WriteOString(aAttribute);
        mStream.WriteChar('=');
        mStream.WriteChar('"');
        mStream.WriteOString(aValue);
        mStream.WriteChar('"');
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
