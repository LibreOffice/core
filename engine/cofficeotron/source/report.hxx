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

#ifndef OFFICEOTRON_REPORT_HXX
#define OFFICEOTRON_REPORT_HXX

#include <string>

// The validation narrative. Comments carry a class ("INFO", "WARN",
// "ERROR", "FATAL") and are indented to reflect the processing depth.
// The error count only grows through incErrs; adding an "ERROR" comment
// does not change it by itself.
class ValidationReport
{
public:
    virtual ~ValidationReport() = default;

    // Adds a comment with the "INFO" class.
    virtual void addComment(const std::string& text) = 0;
    virtual void addComment(const std::string& klass, const std::string& text) = 0;

    void incIndent() { ++indentLevel; }
    void decIndent() { --indentLevel; }
    void incErrs() { ++errorCount; }
    int getErrCount() const { return errorCount; }

protected:
    int indentLevel = 0;
    int errorCount = 0;
};

// Prints the narrative to standard output, one comment per line, in the
// form "KLASS<indent> - text" with five spaces of indent per level.
// When showInfos is false, comments with the "INFO" class are dropped.
class StdioValidationReport : public ValidationReport
{
public:
    explicit StdioValidationReport(bool showInfos);

    void addComment(const std::string& text) override;
    void addComment(const std::string& klass, const std::string& text) override;

private:
    bool showInfos;
};

// Discards every comment; only the error count is kept. The unit tests
// use this to exercise sessions without producing output.
class NullValidationReport : public ValidationReport
{
public:
    void addComment(const std::string&) override {}
    void addComment(const std::string&, const std::string&) override {}
};

// Collects validation errors for one document fragment into the report,
// printing at most `threshold` of them and counting the rest silently.
// Each printed error is prefixed with "(fragment:line col:column)".
class ErrorCapper
{
public:
    static const int threshold = 10;

    ErrorCapper(ValidationReport& commentary, const std::string& fragmentName);

    void error(int line, int column, const std::string& message);
    void fatalError(const std::string& message);
    void warning(const std::string& message);

    int getInstanceErrCount() const { return instanceErrCount; }

    // Adds the "(N error(s) omitted for the sake of brevity)" note when
    // more errors arrived than were printed.
    void addOmissionNote();

private:
    ValidationReport& commentary;
    std::string fragmentName;
    int instanceErrCount = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
