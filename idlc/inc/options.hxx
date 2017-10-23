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

#ifndef INCLUDED_IDLC_INC_OPTIONS_HXX
#define INCLUDED_IDLC_INC_OPTIONS_HXX

#include "idlctypes.hxx"

typedef std::unordered_map< OString, OString > OptionMap;

class IllegalArgument
{
public:
    IllegalArgument(const OString& msg)
        : m_message(msg) {}

    OString  m_message;
};


class Options
{
public:
    explicit Options(char const * progname);
    ~Options();

    static bool checkArgument(std::vector< std::string > & rArgs, char const * arg, size_t len);
    static bool checkCommandFile(std::vector< std::string > & rArgs, char const * filename);

    /// @throws IllegalArgument
    bool initOptions(std::vector< std::string > & rArgs);
    /// @throws IllegalArgument
    static bool badOption(char const * reason, std::string const & rArg);
    bool setOption(char const * option, std::string const & rArg);

    OString prepareHelp() const;
    OString prepareVersion() const;

    const OString&   getProgramName() const { return m_program;}
    bool                isValid(const OString& option) const;
    /// @throws IllegalArgument
    const OString&   getOption(const OString& option);

    const std::vector< OString >& getInputFiles() const { return m_inputFiles; }
    bool readStdin() const { return m_stdin; }
    bool verbose() const { return m_verbose; }
    bool quiet() const { return m_quiet; }

protected:
    OString  m_program;
    std::vector< OString >    m_inputFiles;
    bool            m_stdin;
    bool            m_verbose;
    bool            m_quiet;
    OptionMap       m_options;
};

#endif // INCLUDED_IDLC_INC_OPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
