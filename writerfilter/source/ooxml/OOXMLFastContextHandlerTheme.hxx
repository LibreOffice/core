/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <docmodel/theme/Theme.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XFastContextHandler.hpp>
#include <oox/mathml/importutils.hxx>
#include <oox/drawingml/themefragmenthandler.hxx>
#include <rtl/ref.hxx>
#include "OOXMLParserState.hxx"
#include "OOXMLPropertySet.hxx"
#include "OOXMLFastContextHandler.hxx"

namespace writerfilter::ooxml
{
class OOXMLFastContextHandlerTheme : public OOXMLFastContextHandler
{
private:
    rtl::Reference<oox::drawingml::ThemeFragmentHandler> mpThemeFragmentHandler;

public:
    explicit OOXMLFastContextHandlerTheme(OOXMLFastContextHandler* pContext);
    std::string getType() const override { return "Theme"; }

protected:
    void lcl_startFastElement(
        Token_t Element,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& Attribs) override;
    void lcl_endFastElement(Token_t Element) override;
    css::uno::Reference<css::xml::sax::XFastContextHandler> lcl_createFastChildContext(
        Token_t Element,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& Attribs) override;
    virtual void lcl_characters(const OUString& aChars) override;
};

} // end namespace writerfilter::ooxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
