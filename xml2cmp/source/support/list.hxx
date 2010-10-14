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

#ifndef __LISTEN_123456__
#define __LISTEN_123456__

#include <string.h>
#include <iostream>
#include <stdlib.h>

template <class XX>
class List
{
  public :
    typedef XX *        iterator;
    typedef const XX *  const_iterator;

    // LIFECYCLE
                        List();
    virtual             ~List() { delete [] inhalt; }

    // OPERATORS
    const XX &          operator[](
                            unsigned            n) const
                                                { return elem(n); }
    XX &                operator[](
                            unsigned            n)
                                                { return elem(n); }
    // OPERATIONS
    void                reserve(
                            unsigned            i_nSize )
                                                { alloc(i_nSize,true); }
    virtual void        insert(
                            unsigned            pos,
                            const XX &          elem );
    void                push_back(
                            const XX &          elem_)
                                                { insert(size(),elem_); }

    virtual void        remove(
                            unsigned            pos );
    void                pop_back()              { remove(size()-1); }
    void                erase_all()             { while (size()) remove(size()-1); }

    // INQUIRY
    const XX &          front() const           { return elem(0); }
    const XX &          back() const            { return elem(len-1); }

    unsigned            size() const            { return len; }
    unsigned            space() const           { return allocated; }
    // ACCESS
    XX &                front()                 { return elem(0); }
    XX &                back()                  { return elem(len-1); }

  protected:
    void                checkSize(
                            unsigned            newLength);
    void                alloc(
                            unsigned            newSpace,
                            bool                re = false );

    const XX &          elem(
                            unsigned            n ) const
                                                { return inhalt[n]; }
    XX &                elem(
                            unsigned            n )
                                                { return inhalt[n]; }
  // DATA
    XX *                inhalt;
    unsigned            len;
    unsigned            allocated;

  private:
    // forbidden functions
                        List(const List<XX> & L);
    List<XX> &          operator=(
                            const List<XX> &    L);

};

template <class XY>
class DynamicList : public List<XY*>
{
  public:
    virtual             ~DynamicList();

    virtual void        insert(
                            unsigned            pos,
                            XY * const &        elem );
    virtual void        remove(
                            unsigned            pos );
};



template <class XX>
List<XX>::List()
    :   inhalt(0),
        len(0),
        allocated(0)

{
    alloc(1);
}


template <class XX>
void
List<XX>::insert(unsigned pos, const XX & elem_)
{
    if ( pos > len )
      return;

    checkSize(len+2);
    for ( unsigned p = len; p > pos; --p)
    {
        inhalt[p] = inhalt[p-1];
    }
    inhalt[pos] = elem_;
    len++;
}


template <class XX>
void
List<XX>::remove(unsigned pos)
{
    if ( pos >= len )
      return;
    len--;
    for ( unsigned p = pos; p < len; ++p)
    {
        inhalt[p] = inhalt[p+1];
    }
}


// Protected:
template <class XX>
void
List<XX>::checkSize(unsigned newLength)
{
   // neuen Platzbedarf pruefen:
   unsigned newSpace = space();
   if (newLength > newSpace)
   {
      if (!newSpace)
         newSpace = 1;
      const unsigned nBorder = 65536 / 2;
      while(newLength > newSpace)
      {
        if (newSpace < nBorder)
            newSpace <<= 1;
        else
        {
            std::cerr << "List becomes too big" << std::endl;
            exit(1);
        }
      }
   }

   // Veraenderung ?:
   if (newSpace != space())
      alloc(newSpace,true);
}

template <class XX>
void
List<XX>::alloc( unsigned           newSpace,
                 bool               re )
{
    XX * pNew = new XX[newSpace];

    if (inhalt != 0)
    {
        if (re)
        {
            for (unsigned i = 0; i < len; ++i)
            {
                pNew[i] = inhalt[i];
            }  // end for
        }
        delete [] inhalt;
    }

    inhalt = pNew;
    allocated = newSpace;
}


template <class XY>
DynamicList<XY>::~DynamicList()
{
    this->erase_all();
}

template <class XY>
void
DynamicList<XY>::insert(unsigned pos, XY * const & elem_)
{
    if ( pos > this->len )
      return;

    checkSize(this->len+2);
    memmove(this->inhalt[pos+1], this->inhalt[pos], (this->len-pos) * sizeof(XY*) );
    this->inhalt[pos] = elem_;
    this->len++;
}

template <class XY>
void
DynamicList<XY>::remove( unsigned           pos )
{
    if (pos >= this->len)
        return;
    this->len--;
    delete this->inhalt[pos];
    memmove(this->inhalt[pos], this->inhalt[pos+1], (this->len-pos) * sizeof(XY*) );
}



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
