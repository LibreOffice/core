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
#include "precompiled_idlc.hxx"

#include "idlc/options.hxx"

#include "osl/diagnose.h"
#include "rtl/string.hxx"
#include "rtl/strbuf.hxx"

#include <stdio.h>
#include <string.h>

using rtl::OString;
using rtl::OStringBuffer;

#ifdef SAL_UNX
#define SEPARATOR '/'
#else
#define SEPARATOR '\\'
#endif

Options::Options(char const * progname)
  : m_program(progname), m_stdin(false), m_verbose(false), m_quiet(false)
{
}

Options::~Options()
{
}

// static
bool Options::checkArgument (std::vector< std::string > & rArgs, char const * arg, size_t len)
{
  bool result = ((arg != 0) && (len > 0));
  OSL_PRECOND(result, "idlc::Options::checkArgument(): invalid arguments");
  if (result)
  {
    switch(arg[0])
    {
    case '@':
      if ((result = (len > 1)) == true)
      {
        // "@<cmdfile>"
        result = Options::checkCommandFile (rArgs, &(arg[1]));
      }
      break;
    case '-':
      if ((result = (len > 1)) == true)
      {
        // "-<option>"
        switch (arg[1])
        {
        case 'O':
        case 'I':
        case 'D':
          {
            // "-<option>[<param>]
            std::string option(&(arg[0]), 2);
            rArgs.push_back(option);
            if (len > 2)
            {
              // "-<option><param>"
              std::string param(&(arg[2]), len - 2);
              rArgs.push_back(param);
            }
            break;
          }
        default:
          // "-<option>" ([long] option, w/o param)
          rArgs.push_back(std::string(arg, len));
          break;
        }
      }
      break;
    default:
      // "<param>"
      rArgs.push_back(std::string(arg, len));
      break;
    }
  }
  return (result);
}

// static
bool Options::checkCommandFile (std::vector< std::string > & rArgs, char const * filename)
{
    FILE * fp = fopen(filename, "r");
    if (fp == 0)
    {
        fprintf(stderr, "ERROR: can't open command file \"%s\"\n", filename);
        return (false);
    }

    std::string buffer;
    buffer.reserve(256);

    bool quoted = false;
    int c = EOF;
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
                  // append current argument.
                  if (!Options::checkArgument(rArgs, buffer.c_str(), buffer.size()))
                  {
                      (void) fclose(fp);
                      return (false);
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
    if (!buffer.empty())
    {
        // append unterminated argument.
        if (!Options::checkArgument(rArgs, buffer.c_str(), buffer.size()))
        {
            (void) fclose(fp);
            return (false);
        }
        buffer.clear();
    }
    return (fclose(fp) == 0);
}

bool Options::badOption(char const * reason, std::string const & rArg) throw(IllegalArgument)
{
  OStringBuffer message;
  if (reason != 0)
  {
    message.append(reason); message.append(" option '"); message.append(rArg.c_str()); message.append("'");
    throw IllegalArgument(message.makeStringAndClear());
  }
  return false;
}

bool Options::setOption(char const * option, std::string const & rArg)
{
  bool result = (0 == strcmp(option, rArg.c_str()));
  if (result)
    m_options[rArg.c_str()] = OString(rArg.c_str(), rArg.size());
  return (result);
}

bool Options::initOptions(std::vector< std::string > & rArgs) throw(IllegalArgument)
{
  std::vector< std::string >::const_iterator first = rArgs.begin(), last = rArgs.end();
  for (; first != last; ++first)
  {
    if ((*first)[0] != '-')
    {
      OString filename((*first).c_str(), (*first).size());
      OString tmp(filename.toAsciiLowerCase());
      if (tmp.lastIndexOf(".idl") != (tmp.getLength() - 4))
      {
        throw IllegalArgument("'" + filename + "' is not a valid input file, only '*.idl' files will be accepted");
      }
      m_inputFiles.push_back(filename);
      continue;
    }

    std::string const option(*first);
    switch((*first)[1])
    {
    case 'O':
      {
        if (!((++first != last) && ((*first)[0] != '-')))
        {
          return badOption("invalid", option);
        }
        OString param((*first).c_str(), (*first).size());
        m_options["-O"] = param;
        break;
      }
    case 'I':
      {
        if (!((++first != last) && ((*first)[0] != '-')))
        {
          return badOption("invalid", option);
        }
        OString param((*first).c_str(), (*first).size());
        {
          // quote param token(s).
          OStringBuffer buffer;
          sal_Int32 k = 0;
          do
          {
            OStringBuffer token; token.append("-I\""); token.append(param.getToken(0, ';', k)); token.append("\"");
            if (buffer.getLength() > 0)
              buffer.append(' ');
            buffer.append(token);
          } while (k != -1);
          param = buffer.makeStringAndClear();
        }
        if (m_options.count("-I") > 0)
        {
          // append param.
          OStringBuffer buffer(m_options["-I"]);
          buffer.append(' '); buffer.append(param);
          param = buffer.makeStringAndClear();
        }
        m_options["-I"] = param;
        break;
      }
    case 'D':
      {
        if (!((++first != last) && ((*first)[0] != '-')))
        {
          return badOption("invalid", option);
        }
        OString param("-D"); param += OString((*first).c_str(), (*first).size());
        if (m_options.count("-D") > 0)
        {
          OStringBuffer buffer(m_options["-D"]);
          buffer.append(' '); buffer.append(param);
          param = buffer.makeStringAndClear();
        }
        m_options["-D"] = param;
        break;
      }
    case 'C':
      {
        if (!setOption("-C", option))
        {
          return badOption("invalid", option);
        }
        break;
      }
    case 'c':
      {
        if (!setOption("-cid", option))
        {
          return badOption("invalid", option);
        }
        break;
      }
    case 'q':
      {
        if (!setOption("-quiet", option))
        {
          return badOption("invalid", option);
        }
        m_quiet = true;
        break;
      }
    case 'v':
      {
        if (!setOption("-verbose", option))
        {
          return badOption("invalid", option);
        }
        m_verbose = true;
        break;
      }
    case 'w':
      {
        if (!(setOption("-w", option) || setOption("-we", option)))
        {
          return badOption("invalid", option);
        }
        break;
      }
    case 'h':
    case '?':
      {
        if (!(setOption("-h", option) || setOption("-?", option)))
        {
          return badOption("invalid", option);
        }
        {
          (void) fprintf(stdout, "%s", prepareHelp().getStr());
          return (false);
        }
        // break; // Unreachable
      }
    case 's':
      {
        if (!setOption("-stdin", option))
        {
          return badOption("invalid", option);
        }
        m_stdin = true;
        break;
      }
    default:
      return badOption("unknown", option);
    }
  }
  return (true);
}

OString Options::prepareHelp()
{
    OString help("\nusing: ");
    help += m_program
        + " [-options] <file_1> ... <file_n> | @<filename> | -stdin\n";
    help += "    <file_n>    = file_n specifies one or more idl files.\n";
    help += "                  Only files with the extension '.idl' are valid.\n";
    help += "    @<filename> = filename specifies the name of a command file.\n";
    help += "    -stdin      = read idl file from standard input.\n";
    help += "  Options:\n";
    help += "    -O<path>    = path specifies the output directory.\n";
    help += "                  The generated output is a registry file with\n";
    help += "                  the same name as the idl input file (or 'stdin'\n";
    help += "                  for -stdin).\n";
    help += "    -I<path>    = path specifies a directory where include\n";
    help += "                  files will be searched by the preprocessor.\n";
    help += "                  Multiple directories can be combined with ';'.\n";
    help += "    -D<name>    = name defines a macro for the preprocessor.\n";
    help += "    -C          = generate complete type information, including\n";
    help += "                  documentation.\n";
    help += "    -cid        = check if identifiers fulfill the UNO naming\n";
    help += "                  requirements.\n";
    help += "    -w          = display warning messages.\n";
    help += "    -we         = treat warnings as errors.\n";
    help += "    -h|-?       = print this help message and exit.\n\n";
    help += prepareVersion();

    return help;
}

OString Options::prepareVersion()
{
    OString version(m_program);
    version += " Version 1.1\n\n";
    return version;
}

const OString& Options::getProgramName() const
{
    return m_program;
}

bool Options::isValid(const OString& option)
{
    return (m_options.count(option) > 0);
}

const OString& Options::getOption(const OString& option)
    throw( IllegalArgument )
{
    if (!isValid(option))
    {
        throw IllegalArgument("Option is not valid or currently not set.");
    }
    return m_options[option];
}
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
