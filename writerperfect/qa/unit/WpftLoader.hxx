/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "wpftqahelperdllapi.h"

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/ustring.hxx>

namespace com
{
namespace sun
{
namespace star
{
namespace beans
{
struct PropertyValue;
}
namespace container
{
class XNameAccess;
}
namespace document
{
class XFilter;
}
namespace frame
{
class XDesktop2;
class XFrame;
}
namespace io
{
class XInputStream;
}
namespace lang
{
class XComponent;
}
namespace uno
{
class XComponentContext;
}
}
}
}

namespace writerperfect
{
namespace test
{
class WPFTQAHELPER_DLLPUBLIC WpftLoader
{
public:
    WpftLoader(const OUString& rURL, const css::uno::Reference<css::document::XFilter>& rxFilter,
               const OUString& rFactoryURL,
               const css::uno::Reference<css::frame::XDesktop2>& rxDesktop,
               const css::uno::Reference<css::container::XNameAccess>& rxTypeMap,
               const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    WpftLoader(const css::uno::Reference<css::io::XInputStream>& rxInputStream,
               const css::uno::Reference<css::document::XFilter>& rxFilter,
               const OUString& rFactoryURL,
               const css::uno::Reference<css::frame::XDesktop2>& rxDesktop,
               const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    ~WpftLoader();

    const css::uno::Reference<css::lang::XComponent>& getDocument() const;

private:
    bool impl_load();
    void impl_dispose();

    void impl_detectFilterName(css::uno::Sequence<css::beans::PropertyValue>& rDescriptor,
                               const OUString& rTypeName);

private:
    const OUString m_aURL;
    const css::uno::Reference<css::io::XInputStream> m_xInputStream;
    const OUString m_aFactoryURL;
    const css::uno::Reference<css::document::XFilter> m_xFilter;
    const css::uno::Reference<css::frame::XDesktop2> m_xDesktop;
    const css::uno::Reference<css::container::XNameAccess> m_xTypeMap;
    const css::uno::Reference<css::uno::XComponentContext> m_xContext;
    css::uno::Reference<css::lang::XComponent> m_xDoc;
    css::uno::Reference<css::frame::XFrame> m_xFrame;
};
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
