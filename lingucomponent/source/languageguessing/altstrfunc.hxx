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



#ifndef _ALT_STRFUNC_HXX_
#define _ALT_STRFUNC_HXX_

#include <string>
#include <guess.hxx>

inline bool isSeparator(const char c){
    return c == GUESS_SEPARATOR_OPEN || c == GUESS_SEPARATOR_SEP || c == GUESS_SEPARATOR_CLOSE || c == '\0';
}

int             start(const std::string &s1, const std::string &s2);

#endif

