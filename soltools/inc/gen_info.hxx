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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
