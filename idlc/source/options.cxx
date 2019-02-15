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


#include <options.hxx>

#include <osl/diagnose.h>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>

#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#ifdef _WIN32
#   if !defined WIN32_LEAN_AND_MEAN
#      define WIN32_LEAN_AND_MEAN
#   endif
#   include <windows.h>
#endif

#include <stdio.h>
#include <string.h>


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
  bool result = ((arg != nullptr) && (len > 0));
  OSL_PRECOND(result, "idlc::Options::checkArgument(): invalid arguments");
  if (result)
  {
    switch(arg[0])
    {
    case '@':
      result = len > 1;
      if (result)
      {
        // "@<cmdfile>"
        result = Options::checkCommandFile (rArgs, &(arg[1]));
      }
      break;
    case '-':
      result = len > 1;
      if (result)
      {
        // "-<option>"
        switch (arg[1])
        {
        case 'O':
        case 'M':
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
          rArgs.emplace_back(arg, len);
          break;
        }
      }
      break;
    default:
      // "<param>"
      rArgs.emplace_back(arg, len);
      break;
    }
  }
  return result;
}

// static
bool Options::checkCommandFile (std::vector< std::string > & rArgs, char const * filename)
{
    FILE * fp = fopen(filename, "r");
    if (fp == nullptr)
    {
        fprintf(stderr, "ERROR: can't open command file \"%s\"\n", filename);
        return false;
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
                      return false;
                  }
                  buffer.clear();
              }
              break;
          }
          [[fallthrough]];
        default:
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
            return false;
        }
        buffer.clear();
    }
    return (fclose(fp) == 0);
}

bool Options::badOption(char const * reason, std::string const & rArg)
{
  OStringBuffer message;
  if (reason != nullptr)
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
  return result;
}

#ifdef _WIN32
/* Helper function to convert windows paths including spaces, brackets etc. into
   a windows short Url. The ucpp preprocessor has problems with such paths and returns
   with error.
*/
static OString convertIncPathtoShortWindowsPath(const OString& incPath) {
    OUString path = OStringToOUString(incPath, RTL_TEXTENCODING_UTF8);

    std::vector<sal_Unicode> vec(path.getLength() + 1);
    //GetShortPathNameW only works if the file can be found!
    const DWORD len = GetShortPathNameW(
        o3tl::toW(path.getStr()), o3tl::toW(&vec[0]), path.getLength() + 1);

    if (len > 0)
    {
        OUString ret(&vec[0], len);
        return OUStringToOString(ret, RTL_TEXTENCODING_UTF8);
    }

    return incPath;
}
#endif

bool Options::initOptions(std::vector< std::string > & rArgs)
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
        if ((++first == last) || ((*first)[0] == '-'))
        {
          return badOption("invalid", option);
        }
        OString param((*first).c_str(), (*first).size());
        m_options["-O"] = param;
        break;
      }
    case 'M':
      {
        if ((++first == last) || ((*first)[0] == '-'))
        {
          return badOption("invalid", option);
        }
        OString param((*first).c_str(), (*first).size());
        m_options["-M"] = param;
        break;
      }
    case 'I':
      {
        if ((++first == last) || ((*first)[0] == '-'))
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
            if (!buffer.isEmpty())
              buffer.append(' ');
//          buffer.append("-I\"");
#ifdef _WIN32
            OString incpath = convertIncPathtoShortWindowsPath(param.getToken(0, ';', k));
#else
            OString incpath = param.getToken(0, ';', k);
#endif
            buffer.append(incpath);
//          buffer.append("\"");
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
        if ((++first == last) || ((*first)[0] == '-'))
        {
          return badOption("invalid", option);
        }
        OString param("-D");
        param += OString((*first).c_str(), (*first).size());
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
          return false;
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
  return true;
}

OString Options::prepareHelp() const
{
    OString help("\nusing: ");
    help += m_program + " [-options] <file_1> ... <file_n> | @<filename> | -stdin\n";
    help += "    <file_n>    = file_n specifies one or more idl files.\n";
    help += "                  Only files with the extension '.idl' are valid.\n";
    help += "    @<filename> = filename specifies the name of a command file.\n";
    help += "    -stdin      = read idl file from standard input.\n";
    help += "  Options:\n";
    help += "    -O<path>    = path specifies the output directory.\n";
    help += "                  The generated output is a registry file with\n";
    help += "                  the same name as the idl input file (or 'stdin'\n";
    help += "                  for -stdin).\n";
    help += "    -M<path>    = path specifies the output directory for deps.\n";
    help += "                  Generate GNU make dependency files with the\n";
    help += "                  same name as the idl input file.\n";
    help += "    -I<path>    = path specifies a directory where include\n";
    help += "                  files will be searched by the preprocessor.\n";
    help += "                  Multiple directories can be combined with ';'.\n";
    help += "    -D<name>    = name defines a macro for the preprocessor.\n";
    help += "    -C          = generate complete type information, including\n";
    help += "                  documentation.\n";
    help += "    -cid        = check if identifiers fulfill the UNO naming\n";
    help += "                  requirements.\n";
    help += "    -quiet      = no output.\n";
    help += "    -verbose    = verbose output.\n";
    help += "    -w          = display warning messages.\n";
    help += "    -we         = treat warnings as errors.\n";
    help += "    -h|-?       = print this help message and exit.\n\n";
    help += prepareVersion();

    return help;
}

OString Options::prepareVersion() const
{
    OString version(m_program);
    version += " Version 1.1\n\n";
    return version;
}


bool Options::isValid(const OString& option) const
{
    return (m_options.count(option) > 0);
}

const OString& Options::getOption(const OString& option)
{
    if (!isValid(option))
    {
        throw IllegalArgument("Option is not valid or currently not set.");
    }
    return m_options[option];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
