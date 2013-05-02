/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* DocumentElement: The items we are collecting to be put into the Writer
 * document: paragraph and spans of text, as well as section breaks.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information visit http://libwpd.sourceforge.net
 */

#ifndef _DOCUMENTELEMENT_H
#define _DOCUMENTELEMENT_H
#include <libwpd/libwpd.h>
#include <libwpd/WPXProperty.h>
#include <libwpd/WPXString.h>
#include <vector>

#include "OdfDocumentHandler.hxx"

class DocumentElement
{
public:
    virtual ~DocumentElement() {}
    virtual void write(OdfDocumentHandler *pHandler) const = 0;
    virtual void print() const {}
};

class TagElement : public DocumentElement
{
public:
    virtual ~TagElement() {}
    TagElement(const WPXString &szTagName) : msTagName(szTagName) {}
    const WPXString &getTagName() const
    {
        return msTagName;
    }
    virtual void print() const;
private:
    WPXString msTagName;
};

class TagOpenElement : public TagElement
{
public:
    TagOpenElement(const WPXString &szTagName) : TagElement(szTagName), maAttrList() {}
    virtual ~TagOpenElement() {}
    void addAttribute(const WPXString &szAttributeName, const WPXString &sAttributeValue);
    virtual void write(OdfDocumentHandler *pHandler) const;
    virtual void print () const;
private:
    WPXPropertyList maAttrList;
};

class TagCloseElement : public TagElement
{
public:
    TagCloseElement(const WPXString &szTagName) : TagElement(szTagName) {}
    virtual ~TagCloseElement() {}
    virtual void write(OdfDocumentHandler *pHandler) const;
};

class CharDataElement : public DocumentElement
{
public:
    CharDataElement(const WPXString &sData) : DocumentElement(), msData(sData) {}
    virtual ~CharDataElement() {}
    virtual void write(OdfDocumentHandler *pHandler) const;
private:
    WPXString msData;
};

class TextElement : public DocumentElement
{
public:
    TextElement(const WPXString &sTextBuf) : DocumentElement(), msTextBuf(sTextBuf, false) {}
    virtual ~TextElement() {}
    virtual void write(OdfDocumentHandler *pHandler) const;

private:
    WPXString msTextBuf;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
