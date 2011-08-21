/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
