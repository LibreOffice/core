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

#include <precomp.h>
#include <ary_i/ci_atag2.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary_i/disdocum.hxx>


namespace ary
{
namespace inf
{

void    DocuTag_Display::Display_TextToken(
                            const csi::dsapi::DT_TextToken & ) {}
void    DocuTag_Display::Display_White() {}
void    DocuTag_Display::Display_MupType(
                            const csi::dsapi::DT_MupType & ) {}
void    DocuTag_Display::Display_MupMember(
                            const csi::dsapi::DT_MupMember & ) {}
void    DocuTag_Display::Display_MupConst(
                            const csi::dsapi::DT_MupConst & ) {}
void    DocuTag_Display::Display_Style(
                            const csi::dsapi::DT_Style & ) {}
void    DocuTag_Display::Display_EOL() {}


}   // namespace inf
}   // namespace ary

