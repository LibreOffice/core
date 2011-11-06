/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
