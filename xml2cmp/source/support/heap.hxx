/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: heap.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 11:56:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


