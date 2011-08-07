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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"
#include <stdexcept>
#include <osl/diagnose.h>
#include "cmdline.hxx"

//---------------------------------
/** Simple command line abstraction
*/

// Creation


CommandLine::CommandLine(size_t argc, char* argv[]) :
    m_argc(argc),
    m_argv(argv)
{
}


// Query


/** Returns an argument by name. If there are
    duplicate argument names in the command line,
    the first one wins.
    Argument name an the argument value must be separated
    by spaces. If the argument value starts with an
    argument prefix use quotes else the return value is
    an empty string because the value will be interpreted
    as the next argument name.
    If an argument value contains spaces use quotes.

    @precond    GetArgumentNames() -> has element ArgumentName

    @throws std::invalid_argument exception
    if the specified argument could not be
    found
*/
std::string CommandLine::get_arg(const std::string& ArgumentName) const
{
    std::string arg_value;
    size_t i;
    for ( i = 0; i < m_argc; i++)
    {
        std::string arg = m_argv[i];

        if (ArgumentName == arg && ((i+1) < m_argc) && !is_arg_name(m_argv[i+1]))
        {
            arg_value = m_argv[i+1];
            break;
        }
    }

    if (i == m_argc)
        throw std::invalid_argument("Invalid argument name");

    return arg_value;
}


// Command


/** Returns whether a given argument is an argument name
*/
bool CommandLine::is_arg_name(const std::string& Argument) const
{
    return (Argument.length() > 0 && Argument[0] == '-');
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
