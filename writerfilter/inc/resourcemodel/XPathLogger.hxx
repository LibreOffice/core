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


#ifndef INCLUDED_XPATH_LOGGER_HXX
#define INCLUDED_XPATH_LOGGER_HXX

#include <hash_map>
#include <stack>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <WriterFilterDllApi.hxx>

namespace writerfilter
{
using ::std::hash_map;
using ::std::stack;
using ::std::string;
using ::std::vector;

class WRITERFILTER_DLLPUBLIC XPathLogger
{
    typedef hash_map<string, unsigned int> TokenMap_t;
    typedef boost::shared_ptr<TokenMap_t> TokenMapPointer_t;

    TokenMapPointer_t mp_tokenMap;
    stack<TokenMapPointer_t> m_tokenMapStack;
    vector<string> m_path;
    string m_currentPath;

    void updateCurrentPath();

public:
    explicit XPathLogger();
    virtual ~XPathLogger();

    string getXPath() const;
    void startElement(string _token);
    void endElement();
};
}
#endif // INCLUDED_XPATH_LOGGER_HXX
