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


#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "xmlscript/xml_helper.hxx"
#include "ucbhelper/content.hxx"
#include <list>

#include "dp_ucb.h"
#include "rtl/ustrbuf.hxx"
#include "dp_properties.hxx"

namespace lang  = com::sun::star::lang;
namespace ucb = com::sun::star::ucb;
namespace uno = com::sun::star::uno;

#define OUSTR(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

using ::com::sun::star::uno::Reference;
using ::rtl::OUString;

#define PROP_SUPPRESS_LICENSE "SUPPRESS_LICENSE"
#define PROP_EXTENSION_UPDATE "EXTENSION_UPDATE"

namespace dp_manager {

//Reading the file
ExtensionProperties::ExtensionProperties(
    OUString const & urlExtension,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv,
    Reference<uno::XComponentContext> const & xContext) :
    m_xCmdEnv(xCmdEnv), m_xContext(xContext)
{
    m_propFileUrl = urlExtension + OUSTR("properties");

    ::std::list< ::std::pair< OUString, OUString> > props;
    if (! dp_misc::create_ucb_content(NULL, m_propFileUrl, 0, false))
        return;

    ::ucbhelper::Content contentProps(m_propFileUrl, m_xCmdEnv, m_xContext);
    dp_misc::readProperties(props, contentProps);

    typedef ::std::list< ::std::pair< OUString, OUString> >::const_iterator CI;
    for (CI i = props.begin(); i != props.end(); ++i)
    {
        if (i->first.equals(OUSTR(PROP_SUPPRESS_LICENSE)))
            m_prop_suppress_license = i->second;
    }
}

//Writing the file
ExtensionProperties::ExtensionProperties(
    OUString const & urlExtension,
    uno::Sequence<css::beans::NamedValue> const & properties,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv,
    Reference<uno::XComponentContext> const & xContext) :
    m_xCmdEnv(xCmdEnv), m_xContext(xContext)
{
    m_propFileUrl = urlExtension + OUSTR("properties");

    for (sal_Int32 i = 0; i < properties.getLength(); i++)
    {
        css::beans::NamedValue const & v = properties[i];
        if (v.Name.equals(OUSTR(PROP_SUPPRESS_LICENSE)))
        {
            m_prop_suppress_license = getPropertyValue(v);
        }
        else if (v.Name.equals(OUSTR(PROP_EXTENSION_UPDATE)))
        {
            m_prop_extension_update = getPropertyValue(v);
        }
        else
        {
            throw lang::IllegalArgumentException(
                OUSTR("Extension Manager: unknown property"), 0, -1);
        }
    }
}

OUString ExtensionProperties::getPropertyValue(css::beans::NamedValue const & v)
{
    OUString value(OUSTR("0"));
    if (v.Value >>= value)
    {
        if (value.equals(OUSTR("1")))
            value = OUSTR("1");
    }
    else
    {
        throw lang::IllegalArgumentException(
            OUSTR("Extension Manager: wrong property value"), 0, -1);
    }
    return value;
}
void ExtensionProperties::write()
{
    ::ucbhelper::Content contentProps(m_propFileUrl, m_xCmdEnv, m_xContext);
    ::rtl::OUStringBuffer buf;

    if (m_prop_suppress_license)
    {
        buf.append(OUSTR(PROP_SUPPRESS_LICENSE));
        buf.append(OUSTR("="));
        buf.append(*m_prop_suppress_license);
    }

    ::rtl::OString stamp = ::rtl::OUStringToOString(
        buf.makeStringAndClear(), RTL_TEXTENCODING_UTF8);
    Reference<css::io::XInputStream> xData(
        ::xmlscript::createInputStream(
            ::rtl::ByteSequence(
                reinterpret_cast<sal_Int8 const *>(stamp.getStr()),
                stamp.getLength() ) ) );
    contentProps.writeStream( xData, true /* replace existing */ );
}

bool ExtensionProperties::isSuppressedLicense()
{
    bool ret = false;
    if (m_prop_suppress_license)
    {
        if (m_prop_suppress_license->equals(OUSTR("1")))
            ret = true;
    }
    return ret;
}

bool ExtensionProperties::isExtensionUpdate()
{
    bool ret = false;
    if (m_prop_extension_update)
    {
        if (m_prop_extension_update->equals(OUSTR("1")))
            ret = true;
    }
    return ret;
}

} // namespace dp_manager


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
