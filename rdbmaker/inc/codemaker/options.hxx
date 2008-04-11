/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: options.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _CODEMAKER_OPTIONS_HXX_
#define _CODEMAKER_OPTIONS_HXX_

#include    <hash_map>
#include    <codemaker/global.hxx>

#if defined( _MSC_VER ) && ( _MSC_VER < 1200 )
typedef ::std::__hash_map__
<
    ::rtl::OString,
    ::rtl::OString,
    HashString,
    EqualString,
    NewAlloc
> OptionMap;
#else
typedef ::std::hash_map
<
    ::rtl::OString,
    ::rtl::OString,
    HashString,
    EqualString
> OptionMap;
#endif

class CannotDumpException
{
public:
    CannotDumpException(const ::rtl::OString& msg)
        : m_message(msg) {}

    ::rtl::OString  m_message;
};


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

protected:
    ::rtl::OString  m_program;
    StringVector    m_inputFiles;
    OptionMap       m_options;
};

#endif // _CODEMAKER_OPTIONS_HXX_

