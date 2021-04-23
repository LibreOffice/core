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

#include <sal/config.h>

#include <com/sun/star/logging/XLogFormatter.hpp>
#include <com/sun/star/logging/LogLevel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <rtl/ustrbuf.hxx>

namespace logging
{
using css::logging::LogRecord;
using namespace css::uno;

namespace
{
class SimpleTextFormatter
    : public cppu::WeakImplHelper<css::logging::XLogFormatter, css::lang::XServiceInfo>
{
public:
    SimpleTextFormatter();

private:
    // XLogFormatter
    virtual OUString SAL_CALL getHead() override;
    virtual OUString SAL_CALL format(const LogRecord& Record) override;
    virtual OUString SAL_CALL getTail() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& _rServiceName) override;
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};
}

SimpleTextFormatter::SimpleTextFormatter() {}

OUString SAL_CALL SimpleTextFormatter::getHead() { return OUString(); }

OUString SAL_CALL SimpleTextFormatter::format(const LogRecord& _rRecord)
{
    OUString aLogEntry;
    // Highlight warnings
    if (_rRecord.Level == css::logging::LogLevel::SEVERE)
        aLogEntry = "ERROR: ";
    else if (_rRecord.Level == css::logging::LogLevel::WARNING)
        aLogEntry = "WARNING: ";

    return aLogEntry + _rRecord.Message + "\n";
}

OUString SAL_CALL SimpleTextFormatter::getTail() { return OUString(); }

sal_Bool SAL_CALL SimpleTextFormatter::supportsService(const OUString& _rServiceName)
{
    return cppu::supportsService(this, _rServiceName);
}

OUString SAL_CALL SimpleTextFormatter::getImplementationName()
{
    return "com.sun.star.comp.extensions.SimpleTextFormatter";
}

Sequence<OUString> SAL_CALL SimpleTextFormatter::getSupportedServiceNames()
{
    return { "com.sun.star.logging.SimpleTextFormatter" };
}

} // namespace logging

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_extensions_SimpleTextFormatter(css::uno::XComponentContext*,
                                                 css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new logging::SimpleTextFormatter());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
