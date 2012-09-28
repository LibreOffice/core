/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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
namespace css = com::sun::star;

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
