/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/log.hxx>
#include "OOXMLFastContextHandlerTheme.hxx"
#include <oox/drawingml/theme.hxx>
#include <oox/token/namespaces.hxx>

using namespace ::com::sun::star;

namespace writerfilter::ooxml
{
OOXMLFastContextHandlerTheme::OOXMLFastContextHandlerTheme(OOXMLFastContextHandler* pContext)
    : OOXMLFastContextHandler(pContext)
{
}

uno::Reference<xml::sax::XFastContextHandler>
OOXMLFastContextHandlerTheme::lcl_createFastChildContext(
    Token_t Element, const uno::Reference<xml::sax::XFastAttributeList>& Attribs)
{
    if (mpThemeFragmentHandler.is())
        return mpThemeFragmentHandler->createFastChildContext(Element, Attribs);

    return this;
}

void OOXMLFastContextHandlerTheme::lcl_startFastElement(
    Token_t Element, const uno::Reference<xml::sax::XFastAttributeList>& Attribs)
{
    if (!mpThemeFragmentHandler.is())
    {
        auto xThemeFilterBase = getDocument()->getThemeFilterBase();
        OUString aThemeFragmentPath
            = xThemeFilterBase->getFragmentPathFromFirstTypeFromOfficeDoc(u"theme");
        auto pThemePtr = getDocument()->getTheme();
        if (!pThemePtr)
        {
            pThemePtr = std::make_shared<oox::drawingml::Theme>();
            auto pTheme = std::make_shared<model::Theme>();
            pThemePtr->setTheme(pTheme);
            getDocument()->setTheme(pThemePtr);
        }
        mpThemeFragmentHandler = new oox::drawingml::ThemeFragmentHandler(
            *xThemeFilterBase, aThemeFragmentPath, *pThemePtr, *pThemePtr->getTheme());
    }

    if (mpThemeFragmentHandler.is())
        mpThemeFragmentHandler->startFastElement(Element, Attribs);
}

void OOXMLFastContextHandlerTheme::lcl_endFastElement(Token_t Element)
{
    if (mpThemeFragmentHandler.is())
        mpThemeFragmentHandler->endFastElement(Element);
}

void OOXMLFastContextHandlerTheme::lcl_characters(const OUString& aChars)
{
    if (mpThemeFragmentHandler.is())
        mpThemeFragmentHandler->characters(aChars);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
