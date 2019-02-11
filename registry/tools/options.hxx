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
    std::string const m_program;

    Options (Options const &) = delete;
    Options & operator= (Options const &) = delete;

public:
    explicit Options (char const * program);
    virtual ~Options();

    static bool checkArgument (std::vector< std::string > & rArgs, char const * arg, size_t len);

    bool initOptions (std::vector< std::string > & rArgs);
    bool badOption (char const * reason, char const * option) const;

    bool printUsage() const;

protected:
    static  bool checkCommandFile(std::vector< std::string > & rArgs, char const * filename);

    virtual bool initOptions_Impl(std::vector< std::string > & rArgs) = 0;
    virtual void printUsage_Impl() const = 0;
};

} // namespace tools
} // namespace registry

#endif /* INCLUDED_REGISTRY_TOOLS_OPTIONS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
