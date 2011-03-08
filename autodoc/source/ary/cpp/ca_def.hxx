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

#ifndef ARY_CPP_CA_DEF_HXX
#define ARY_CPP_CA_DEF_HXX

// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/cp_def.hxx>
    // OTHER
#include "cs_def.hxx"



namespace ary
{
namespace cpp
{
    class Def_Storage;
    class RepositoryPartition;
}
}





namespace ary
{
namespace cpp
{




class DefAdmin : public DefPilot
{
  public:
    // LIFECYCLE
                        DefAdmin(
                            RepositoryPartition &
                                                io_myReposyPartition );
                        ~DefAdmin();

    // INQUIRY
    const Def_Storage & Storage() const;

    // INHERITED
    // Interface DefPilot:
    virtual Define &    Store_Define(
                            const InputContext& i_rContext,
                            const String  &     i_sName,
                            const StringVector &
                                                i_rDefinition );
    virtual Macro &     Store_Macro(
                            const InputContext& i_rContext,
                            const String  &     i_sName,
                            const StringVector &
                                                i_rParams,
                            const StringVector &
                                                i_rDefinition );
    virtual const DefineEntity &
                        Find_Def(
                            De_id               i_id ) const;
    virtual DefsResultList
                        AllDefines() const;
    virtual DefsResultList
                        AllMacros() const;

  private:
    // DATA
    Def_Storage     aStorage;
    RepositoryPartition *
                    pCppRepositoryPartition;
};




// IMPLEMENTATION
inline const Def_Storage &
DefAdmin::Storage() const
{
    return aStorage;
}



}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
