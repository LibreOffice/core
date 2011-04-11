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

#ifndef _IDLC_OPTIONS_HXX_
#define _IDLC_OPTIONS_HXX_

#include <idlc/idlctypes.hxx>

typedef ::boost::unordered_map< ::rtl::OString,
                         ::rtl::OString,
                         HashString,
                         EqualString > OptionMap;

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
    explicit Options(char const * progname);
    ~Options();

    static bool checkArgument(std::vector< std::string > & rArgs, char const * arg, size_t len);
    static bool checkCommandFile(std::vector< std::string > & rArgs, char const * filename);

    bool initOptions(std::vector< std::string > & rArgs)
        throw(IllegalArgument);
    bool badOption(char const * reason, std::string const & rArg)
        throw(IllegalArgument);
    bool setOption(char const * option, std::string const & rArg);

#if 0  /* @@@ */
    sal_Bool initOptions(int ac, char* av[], sal_Bool bCmdFile=sal_False)
        throw( IllegalArgument );
#endif /* @@@ */

    ::rtl::OString prepareHelp();
    ::rtl::OString prepareVersion();

    const ::rtl::OString&   getProgramName() const;
    bool                isValid(const ::rtl::OString& option);
    const ::rtl::OString&   getOption(const ::rtl::OString& option)
        throw( IllegalArgument );

    const StringVector& getInputFiles() const { return m_inputFiles; }
    bool readStdin() const { return m_stdin; }
    bool verbose() const { return m_verbose; }
    bool quiet() const { return m_quiet; }

protected:
    ::rtl::OString  m_program;
    StringVector    m_inputFiles;
    bool            m_stdin;
    bool            m_verbose;
    bool            m_quiet;
    OptionMap       m_options;
};

#endif // _IDLC_OPTIONS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
