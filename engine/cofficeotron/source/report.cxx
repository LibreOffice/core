/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * cofficeotron - a C++ port of the Java(tm) Office-o-tron
 *
 * Copyright (c) 2009-2010 Griffin Brown Digital Publishing Ltd.
 * Copyright (c) 2010 Novell Inc.
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "report.hxx"

#include <stdio.h>

StdioValidationReport::StdioValidationReport(bool showInfos)
    : showInfos(showInfos)
{
}

void StdioValidationReport::addComment(const std::string& text)
{
    if (showInfos)
        addComment("INFO", text);
}

void StdioValidationReport::addComment(const std::string& klass, const std::string& text)
{
    std::string indent(static_cast<size_t>(indentLevel < 0 ? 0 : indentLevel) * 5, ' ');
    printf("%s%s - %s\n", klass.c_str(), indent.c_str(), text.c_str());
}

ErrorCapper::ErrorCapper(ValidationReport& commentary, const std::string& fragmentName)
    : commentary(commentary)
    , fragmentName(fragmentName)
{
}

void ErrorCapper::error(int line, int column, const std::string& message)
{
    if (instanceErrCount < threshold)
    {
        std::string location = "(" + fragmentName + ":" + std::to_string(line) + " col:"
                               + std::to_string(column) + ") ";
        commentary.addComment("ERROR", location + message);
    }
    commentary.incErrs();
    ++instanceErrCount;
}

void ErrorCapper::fatalError(const std::string& message)
{
    commentary.addComment("ERROR", message);
}

void ErrorCapper::warning(const std::string& message)
{
    commentary.addComment("WARN", message);
}

void ErrorCapper::addOmissionNote()
{
    if (instanceErrCount > threshold)
    {
        commentary.addComment("(<i>" + std::to_string(instanceErrCount - threshold)
                              + " error(s) omitted for the sake of brevity</i>)");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
