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



#ifndef _SW_REWRITER_HXX
#define _SW_REWRITER_HXX

#include <vector>
#include <tools/string.hxx>


String const UNDO_ARG1("$1", RTL_TEXTENCODING_ASCII_US);
String const UNDO_ARG2("$2", RTL_TEXTENCODING_ASCII_US);
String const UNDO_ARG3("$3", RTL_TEXTENCODING_ASCII_US);


typedef std::pair<String, String> SwRewriteRule;

class SwRewriter
{
    std::vector<SwRewriteRule> mRules;

public:
    SwRewriter();
    SwRewriter(const SwRewriter & rSrc);
    ~SwRewriter();

    void AddRule(const String & rWhat, const String & rWith);

    String Apply(const String & rStr) const;
};

#endif // _SW_REWRITER_HXX
