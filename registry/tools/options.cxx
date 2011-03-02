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

#include "options.hxx"

#include "osl/diagnose.h"

#include <stdio.h>
#include <string.h>

namespace registry
{
namespace tools
{

Options::Options (char const * program)
    : m_program (program)
{}

Options::~Options()
{}

// static
bool Options::checkArgument(std::vector< std::string> & rArgs, char const * arg, size_t len)
{
    bool result = ((arg != 0) && (len > 0));
    OSL_PRECOND(result, "registry::tools::Options::checkArgument(): invalid arguments");
    if (result)
    {
        OSL_TRACE("registry::tools:Options::checkArgument(): \"%s\"", arg);
        switch (arg[0])
        {
        case '@':
            if ((result = (len > 1)) == true)
            {
                // "@<cmdfile>"
                result = Options::checkCommandFile(rArgs, &(arg[1]));
            }
            break;
        case '-':
            if ((result = (len > 1)) == true)
            {
                // "-<option>"
                std::string option (&(arg[0]), 2);
                rArgs.push_back(option);
                if (len > 2)
                {
                    // "-<option><param>"
                    std::string param(&(arg[2]), len - 2);
                    rArgs.push_back(param);
                }
            }
            break;
        default:
            rArgs.push_back(std::string(arg, len));
            break;
        }
    }
    return (result);
}

// static
bool Options::checkCommandFile(std::vector< std::string > & rArgs, char const * filename)
{
    FILE * fp = fopen(filename, "r");
    if (fp == 0)
    {
        fprintf(stderr, "ERROR: Can't open command file \"%s\"\n", filename);
        return (false);
    }

    std::string buffer;
    buffer.reserve(256);

    bool quoted = false;
    int  c = EOF;
    while ((c = fgetc(fp)) != EOF)
    {
        switch(c)
        {
        case '\"':
            quoted = !quoted;
            break;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            if (!quoted)
            {
                if (!buffer.empty())
                {
                    if (!checkArgument(rArgs, buffer.c_str(), buffer.size()))
                    {
                        // failure.
                        (void) fclose(fp);
                        return false;
                    }
                    buffer.clear();
                }
                break;
            }
        default:
            // quoted white-space fall through
            buffer.push_back(sal::static_int_cast<char>(c));
            break;
        }
    }
    return (fclose(fp) == 0);
}

bool Options::initOptions (std::vector< std::string > & rArgs)
{
    return initOptions_Impl (rArgs);
}

bool Options::badOption (char const * reason, char const * option) const
{
    (void) fprintf(stderr, "%s: %s option '%s'\n", m_program.c_str(), reason, option);
    return printUsage();
}

bool Options::printUsage() const
{
    printUsage_Impl();
    return false;
}

} // namespace tools
} // namespace registry
