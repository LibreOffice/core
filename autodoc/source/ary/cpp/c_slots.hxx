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

#ifndef ARY_CPP_C_SLOTS_HXX
#define ARY_CPP_C_SLOTS_HXX

// BASE CLASSES
#include <ary/ceslot.hxx>
// USED SERVICES
#include <ary/cpp/c_slntry.hxx>




namespace ary
{
namespace cpp
{


class Slot_SubNamespaces : public ary::Slot
{
  public:
                        Slot_SubNamespaces(
                            const Map_NamespacePtr &
                                                i_rData );
    virtual             ~Slot_SubNamespaces();

    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            ary::Display &      o_rDestination ) const;
    // DATA
    const Map_NamespacePtr *
                        pData;
};

class Slot_BaseClass : public ary::Slot
{
  public:
                        Slot_BaseClass(
                            const List_Bases &  i_rData );
    virtual             ~Slot_BaseClass();

    virtual uintt       Size() const;

  private:
    virtual void        StoreEntries(
                            ary::Display &      o_rDestination ) const;
    // DATA
    const List_Bases *  pData;
};




}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
