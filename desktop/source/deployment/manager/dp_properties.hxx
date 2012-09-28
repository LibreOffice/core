/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_DP_PROPERTIES_HXX
#define INCLUDED_DP_PROPERTIES_HXX



#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "boost/optional.hpp"


namespace css = ::com::sun::star;

namespace dp_manager {



/**

 */
class ExtensionProperties
{
protected:
    ::rtl::OUString m_propFileUrl;
    const css::uno::Reference<css::ucb::XCommandEnvironment> m_xCmdEnv;
    const css::uno::Reference<css::uno::XComponentContext> m_xContext;
    ::boost::optional< ::rtl::OUString> m_prop_suppress_license;
    ::boost::optional< ::rtl::OUString> m_prop_extension_update;

    ::rtl::OUString getPropertyValue(css::beans::NamedValue const & v);
public:

    virtual ~ExtensionProperties() {};
    ExtensionProperties(::rtl::OUString const & urlExtension,
                        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
                        css::uno::Reference<css::uno::XComponentContext> const & xContext);

    ExtensionProperties(::rtl::OUString const & urlExtension,
                        css::uno::Sequence<css::beans::NamedValue> const & properties,
                        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
                        css::uno::Reference<css::uno::XComponentContext> const & xContext);

    void write();

    bool isSuppressedLicense();

    bool isExtensionUpdate();
};
}




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
