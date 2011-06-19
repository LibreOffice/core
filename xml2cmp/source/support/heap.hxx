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

#ifndef X2C_HEAP_HXX
#define X2C_HEAP_HXX

#include "sistr.hxx"

class HeapItem;

class Heap
{
  public:
                        Heap(
                            unsigned            i_nWidth );
                        ~Heap();

    void                InsertValue(
                            const char *        i_sKey,
                            const char *        i_sValue );
    HeapItem *          ReleaseTop();           /// @return must be deleted by caller of method.

  private:
    typedef HeapItem * Column;

    void                IncColumn();
    Column &            ActiveColumn()          { return dpColumnsArray[nActiveColumn]; }

    Column *            dpColumnsArray;
    unsigned            nColumnsArraySize;
    unsigned            nActiveColumn;
};


class HeapItem
{
  public:
                        HeapItem(
                            const char *        i_sKey,
                            const char *        i_sValue );
                        ~HeapItem(  );

    bool                operator<(
                            const HeapItem &    i_rOther ) const;
    bool                operator<=(
                            const HeapItem &    i_rOther ) const
                                                { return ! (i_rOther < *this); }
    const Simstr &      Value() const;
    const Simstr &      Key() const;
    HeapItem *          Next() const;

    void                SetNext(
                            HeapItem *          i_pNext );
  private:
    Simstr              sValue;
    Simstr              sKey;
    HeapItem *          pNext;
};






#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
