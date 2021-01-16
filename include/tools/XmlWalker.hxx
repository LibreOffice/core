/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TOOLS_XMLWALKER_HXX
#define INCLUDED_TOOLS_XMLWALKER_HXX

#include <tools/toolsdllapi.h>
#include <rtl/string.hxx>
#include <memory>

class SvStream;

namespace tools
{
struct XmlWalkerImpl;

/**
 * XmlWalker main purpose is to make it easier for walking the
 * parsed XML DOM tree.
 *
 * It hides all the libxml2 and C -isms and makes the usage more
 * comfortable from LO developer point of view.
 *
 */
class TOOLS_DLLPUBLIC XmlWalker final
{
private:
    std::unique_ptr<XmlWalkerImpl> mpImpl;

public:
    XmlWalker();

    ~XmlWalker();

    bool open(SvStream* pStream);

    OString name();
    OString namespaceHref();
    OString namespacePrefix();

    OString content();
    void children();
    void parent();
    void next();
    bool isValid() const;
    OString attribute(const OString& sName);
};

} // end tools namespace

#endif // INCLUDED_TOOLS_XMLWRITER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
