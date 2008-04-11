/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: options.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _IDLC_IDLCTYPEs_HXX_
#include <idlc/idlctypes.hxx>
#endif

typedef ::std::hash_map< ::rtl::OString,
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
    Options();
    ~Options();

    sal_Bool initOptions(int ac, char* av[], sal_Bool bCmdFile=sal_False)
        throw( IllegalArgument );

    ::rtl::OString prepareHelp();
    ::rtl::OString prepareVersion();

    const ::rtl::OString&   getProgramName() const;
    sal_uInt16              getNumberOfOptions() const;
    sal_Bool                isValid(const ::rtl::OString& option);
    const ::rtl::OString    getOption(const ::rtl::OString& option)
        throw( IllegalArgument );
    const OptionMap&        getOptions();

    const StringVector& getInputFiles() const { return m_inputFiles; }
    bool readStdin() const { return m_stdin; }

protected:
    ::rtl::OString  m_program;
    StringVector    m_inputFiles;
    bool            m_stdin;
    OptionMap       m_options;
};

#endif // _IDLC_OPTIONS_HXX_

