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



#ifndef GEN_INFO_HXX
#define GEN_INFO_HXX


#include "simstr.hxx"
#include <string.h>



class List_GenericInfo;


/** Holds generic informations in a simple hierarchical format.
*/
class GenericInfo
{
  public:
    // LIFECFYCLE
                        GenericInfo(
                            const Simstr &      i_sKey,
                            const Simstr &      i_sValue = Simstr::null_(),
                            const Simstr &      i_sComment = Simstr::null_() );
                        GenericInfo(
                            const GenericInfo & i_rInfo );
                        ~GenericInfo();

    // OPERATORS
    GenericInfo &       operator=(
                            const GenericInfo & i_rInfo );
    bool                operator<(
                            const GenericInfo & i_rInfo ) const
#ifdef UNX
                                                { return strcasecmp(sKey.str(),i_rInfo.sKey.str()) < 0; }
#else
                                                { return stricmp(sKey.str(),i_rInfo.sKey.str()) < 0; }
#endif
    // INFO
    const Simstr &      Key() const             { return sKey; }
    const Simstr &      Value() const           { return sValue; }
    const Simstr &      Comment() const         { return sComment; }
    bool                HasSubList() const      { return dpSubList != 0; }

    const List_GenericInfo &
                        SubList() const         { return HasSubList() ? *dpSubList : CreateMyList(); }
    // ACCESS
    List_GenericInfo &
                        SubList()               { return HasSubList() ? *dpSubList : CreateMyList(); }

  private:
    /// @precond dpSubList == 0 .
    List_GenericInfo &  CreateMyList() const;

    // DATA
    Simstr              sKey;
    Simstr              sValue;
    Simstr              sComment;
    List_GenericInfo *  dpSubList;      /// Owned by this object.
};


#endif

