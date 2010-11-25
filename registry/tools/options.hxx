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

#ifndef INCLUDED_REGISTRY_TOOLS_OPTIONS_HXX
#define INCLUDED_REGISTRY_TOOLS_OPTIONS_HXX

#include <string>
#include <vector>

namespace registry
{
namespace tools
{
class Options
{
    std::string m_program;

    Options (Options const &);
    Options & operator= (Options const &);

public:
    explicit Options (char const * program);
    virtual ~Options();

    static bool checkArgument (std::vector< std::string > & rArgs, char const * arg, size_t len);

    bool initOptions (std::vector< std::string > & rArgs);
    bool badOption (char const * reason, char const * option) const;

    std::string const & getProgramName() const { return m_program; }
    bool printUsage() const;

protected:
    static  bool checkCommandFile(std::vector< std::string > & rArgs, char const * filename);

    virtual bool initOptions_Impl(std::vector< std::string > & rArgs) = 0;
    virtual void printUsage_Impl() const = 0;
};

} // namespace tools
} // namespace registry

#endif /* INCLUDED_REGISTRY_TOOLS_OPTIONS_HXX */
