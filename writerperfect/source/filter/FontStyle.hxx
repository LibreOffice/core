/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* FontStyle: Stores (and writes) font-based information that is needed at
 * the head of an OO document.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information visit http://libwpd.sourceforge.net
 */

#ifndef _FONTSTYLE_H
#define _FONTSTYLE_H
#include <map>

#include <libwpd/libwpd.h>

#include "FilterInternal.hxx"

#include "Style.hxx"

class FontStyle : public Style
{
public:
    FontStyle(const char *psName, const char *psFontFamily);
    ~FontStyle();
    virtual void write(OdfDocumentHandler *pHandler) const;
    const WPXString &getFontFamily() const
    {
        return msFontFamily;
    }

private:
    WPXString msFontFamily;
    WPXString msFontPitch;
};

class FontStyleManager : public StyleManager
{
public:
    FontStyleManager() : mStyleHash() {}
    virtual ~FontStyleManager()
    {
        clean();
    }

    /* create a new font if the font does not exists and returns a font name

    Note: the returned font name is actually equalled to psFontFamily
    */
    WPXString findOrAdd(const char *psFontFamily);

    virtual void clean();
    virtual void write(OdfDocumentHandler *) const {}
    virtual void writeFontsDeclaration(OdfDocumentHandler *) const;


protected:
    // style name -> SpanStyle
    std::map<WPXString, shared_ptr<FontStyle>, ltstr> mStyleHash;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
