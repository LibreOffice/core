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

#ifndef INCLUDED_CODEMAKER_OPTIONS_HXX
#define INCLUDED_CODEMAKER_OPTIONS_HXX

#include <boost/unordered_map.hpp>

#include <codemaker/global.hxx>

typedef ::boost::unordered_map
<
    ::rtl::OString,
    ::rtl::OString,
    HashString,
    EqualString
> OptionMap;

class IllegalArgument
{
public:
    IllegalArgument(const ::rtl::OString& msg)
        : m_message(msg) {}

    ::rtl::OString  m_message;
};

class Options
{
public:
    Options();
    virtual ~Options();

    virtual sal_Bool initOptions(int ac, char* av[], sal_Bool bCmdFile=sal_False)
        throw( IllegalArgument ) = 0;

    virtual ::rtl::OString  prepareHelp() = 0;

    const ::rtl::OString&   getProgramName() const;
    sal_Bool                isValid(const ::rtl::OString& option) const;
    const ::rtl::OString    getOption(const ::rtl::OString& option) const
        throw( IllegalArgument );

    const StringVector& getInputFiles();

    inline sal_uInt16 getNumberOfExtraInputFiles() const
        { return (sal_uInt16)m_extra_input_files.size(); }
    inline const StringVector& getExtraInputFiles() const
        { return m_extra_input_files; }
protected:
    ::rtl::OString  m_program;
    StringVector    m_inputFiles;
    StringVector    m_extra_input_files;
    OptionMap       m_options;
};

#endif // INCLUDED_CODEMAKER_OPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
