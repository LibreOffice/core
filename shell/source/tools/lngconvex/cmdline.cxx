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
