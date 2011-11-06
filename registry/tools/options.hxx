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
