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



#ifndef INCLUDED_CODEMAKER_COMMONCPP_HXX
#define INCLUDED_CODEMAKER_COMMONCPP_HXX

#include "codemaker/codemaker.hxx"

namespace codemaker { namespace cpp {

rtl::OString typeToPrefix(TypeManager const & manager, rtl::OString const & type);

rtl::OString scopedCppName(rtl::OString const & type, bool bNoNameSpace=false,
                           bool shortname=false);

rtl::OString translateUnoToCppType(
    codemaker::UnoType::Sort sort, RTTypeClass typeClass,
    rtl::OString const & nucleus, bool shortname);

enum IdentifierTranslationMode {
    ITM_GLOBAL,
    ITM_NONGLOBAL,
    ITM_KEYWORDSONLY
};

rtl::OString translateUnoToCppIdentifier(
    rtl::OString const & identifier, rtl::OString const & prefix,
    IdentifierTranslationMode transmode = ITM_GLOBAL,
    rtl::OString const * forbidden = 0);

} }

#endif // INCLUDED_CODEMAKER_COMMONCPP_HXX
