/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "wpftqahelperdllapi.h"

#include <cpo/uno/Reference.hxx>
#include <cpo/uno/Sequence.hxx>

#include <rtl/ustring.hxx>

namespace com::sun::star
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
class XDesktop;
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
}
namespace cpo::uno { class XComponentContext; }

namespace writerperfect
{
namespace test
{
class WPFTQAHELPER_DLLPUBLIC WpftLoader
{
public:
    WpftLoader(const OUString& rURL, const cpo::uno::Reference<css::document::XFilter>& rxFilter,
               const OUString& rFactoryURL,
               const cpo::uno::Reference<css::frame::XDesktop>& rxDesktop,
               const cpo::uno::Reference<css::container::XNameAccess>& rxTypeMap,
               const cpo::uno::Reference<cpo::uno::XComponentContext>& rxContext);
    WpftLoader(const cpo::uno::Reference<css::io::XInputStream>& rxInputStream,
               const cpo::uno::Reference<css::document::XFilter>& rxFilter,
               const OUString& rFactoryURL,
               const cpo::uno::Reference<css::frame::XDesktop>& rxDesktop,
               const cpo::uno::Reference<cpo::uno::XComponentContext>& rxContext);
    ~WpftLoader();

    const cpo::uno::Reference<css::lang::XComponent>& getDocument() const;

private:
    bool impl_load();
    void impl_dispose();

    void impl_detectFilterName(cpo::uno::Sequence<css::beans::PropertyValue>& rDescriptor,
                               const OUString& rTypeName);

private:
    const OUString m_aURL;
    const cpo::uno::Reference<css::io::XInputStream> m_xInputStream;
    const OUString m_aFactoryURL;
    const cpo::uno::Reference<css::document::XFilter> m_xFilter;
    const cpo::uno::Reference<css::frame::XDesktop> m_xDesktop;
    const cpo::uno::Reference<css::container::XNameAccess> m_xTypeMap;
    const cpo::uno::Reference<cpo::uno::XComponentContext> m_xContext;
    cpo::uno::Reference<css::lang::XComponent> m_xDoc;
    cpo::uno::Reference<css::frame::XFrame> m_xFrame;
};
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
