/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cmdline.cxx,v $
 * $Revision: 1.5 $
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

//################################
// Creation
//################################


CommandLine::CommandLine(size_t argc, char* argv[], const std::string& ArgPrefix) :
    m_argc(argc),
    m_argv(argv),
    m_argprefix(ArgPrefix)
{
}


//################################
// Query
//################################


/** Return the argument count
*/
size_t CommandLine::get_arg_count() const
{
    return m_argc;
}

/** Return an argument by index
    This method doesn't skip argument
    names if any, so if the second
    argument is an argument name the
    function nevertheless returns it.

    @precond    0 <= Index < GetArgumentCount

    @throws std::out_of_range exception
    if the given index is to high
*/
std::string CommandLine::get_arg(size_t Index) const
{
    OSL_PRECOND(Index < m_argc, "Index out of range");

    if (Index > (m_argc - 1))
        throw std::out_of_range("Invalid index");

    return m_argv[Index];
}


/** Returns all argument name found in the
    command line. An argument will be identified
    by a specified prefix. The standard prefix
    is '-'.
    If the are no argument names the returned
    container is empty.
*/
StringListPtr_t CommandLine::get_arg_names() const
{
    StringListPtr_t arg_cont(new StringList_t());

    for (size_t i = 0; i < m_argc; i++)
    {
        std::string argn = m_argv[i];

        if (is_arg_name(argn))
            arg_cont->push_back(argn);
    }

    return arg_cont;
}

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


//################################
// Command
//################################


/** Set the prefix used to identify arguments in
    the command line.

    @precond    prefix is not empty

    @throws std::invalid_argument exception if
    the prefix is empty
*/
void CommandLine::set_arg_prefix(const std::string& Prefix)
{
    OSL_PRECOND(Prefix.length(), "Empty argument prefix!");

    if (0 == Prefix.length())
        throw std::invalid_argument("Empty argument prefix not allowed");

    m_argprefix = Prefix;
}


/** Returns whether a given argument is an argument name
*/
bool CommandLine::is_arg_name(const std::string& Argument) const
{
    return (0 == Argument.compare(0, m_argprefix.length(), m_argprefix));
}
