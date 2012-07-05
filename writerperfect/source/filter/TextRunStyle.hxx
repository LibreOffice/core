/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* TextRunStyle: Stores (and writes) paragraph/span-style-based information
 * (e.g.: a paragraph might be bold) that is needed at the head of an OO
 * document.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information visit http://libwpd.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#ifndef _TEXTRUNSTYLE_H
#define _TEXTRUNSTYLE_H

#include <map>

#include <libwpd/libwpd.h>

#include "FilterInternal.hxx"

#include "Style.hxx"

class OdfDocumentHandler;

class ParagraphStyle
{
public:
    ParagraphStyle(WPXPropertyList const &propList, const WPXPropertyListVector &tabStops, const WPXString &sName);
    virtual ~ParagraphStyle();
    virtual void write(OdfDocumentHandler *pHandler) const;
    WPXString getName() const
    {
        return msName;
    }
private:
    WPXPropertyList mpPropList;
    WPXPropertyListVector mxTabStops;
    WPXString msName;
};


class SpanStyle : public Style
{
public:
    SpanStyle(const char *psName, const WPXPropertyList &xPropList);
    virtual void write(OdfDocumentHandler *pHandler) const;

private:
    WPXPropertyList mPropList;
};

class ParagraphStyleManager : public StyleManager
{
public:
    ParagraphStyleManager() : mNameHash(), mStyleHash() {}
    virtual ~ParagraphStyleManager()
    {
        clean();
    }

    /* create a new style if it does not exists. In all case, returns the name of the style

    Note: using S%i as new name*/
    WPXString findOrAdd(const WPXPropertyList &xPropList, const WPXPropertyListVector &tabStops);

    /* returns the style corresponding to a given name ( if it exists ) */
    shared_ptr<ParagraphStyle> const get(const WPXString &name) const;

    virtual void clean();
    virtual void write(OdfDocumentHandler *) const;


protected:
    // return a unique key
    WPXString getKey(const WPXPropertyList &xPropList, const WPXPropertyListVector &tabStops) const;

    // hash key -> name
    std::map<WPXString, WPXString, ltstr> mNameHash;
    // style name -> paragraph style
    std::map<WPXString, shared_ptr<ParagraphStyle>, ltstr> mStyleHash;
};

class SpanStyleManager : public StyleManager
{
public:
    SpanStyleManager() : mNameHash(), mStyleHash() {}
    virtual ~SpanStyleManager()
    {
        clean();
    }

    /* create a new style if it does not exists. In all case, returns the name of the style

    Note: using Span%i as new name*/
    WPXString findOrAdd(const WPXPropertyList &xPropList);

    /* returns the style corresponding to a given name ( if it exists ) */
    shared_ptr<SpanStyle> const get(const WPXString &name) const;

    virtual void clean();
    virtual void write(OdfDocumentHandler *) const;

protected:
    // hash key -> style name
    std::map<WPXString, WPXString, ltstr> mNameHash;
    // style name -> SpanStyle
    std::map<WPXString, shared_ptr<SpanStyle>, ltstr> mStyleHash;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
