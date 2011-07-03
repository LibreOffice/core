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
    sal_Bool                isValid(const ::rtl::OString& option);
    const ::rtl::OString    getOption(const ::rtl::OString& option)
        throw( IllegalArgument );
    const OptionMap&        getOptions();

    const ::rtl::OString    getInputFile(sal_uInt16 index)
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
