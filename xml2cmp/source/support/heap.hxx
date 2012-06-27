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
