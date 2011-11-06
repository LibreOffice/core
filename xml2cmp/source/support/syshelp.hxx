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



#ifndef X2C_SYSHELP_HXX
#define X2C_SYSHELP_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <iosfwd>

class Simstr;
template <class XY> class List;


#if defined(WNT) || defined(OS2)
const char   C_sSLASH[] = "\\";
const char   C_cSLASH = '\\';
#elif defined(UNX)
const char   C_sSLASH[] = "/";
const char   C_cSLASH = '/';
#else
#error Must run under unix or windows, please define UNX or WNT.
#endif

enum E_LinkType
{
    lt_nolink = 0,
    lt_idl,
    lt_html
};


void                WriteName(
                        std::ostream &      o_rFile,
                        const Simstr &      i_rIdlDocuBaseDir,
                        const Simstr &      i_rName,
                        E_LinkType          i_eLinkType );


void                WriteStr(
                        std::ostream &      o_rFile,
                        const char *        i_sStr );
void                WriteStr(
                        std::ostream &      o_rFile,
                        const Simstr &      i_sStr );

void                GatherFileNames(
                        List<Simstr> &      o_sFiles,
                        const char *        i_sSrcDirectory );
void                GatherSubDirectories(
                        List<Simstr> &      o_sSubDirectories,
                        const char *        i_sParentdDirectory );



#endif

