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

#ifndef CSV_SWELIST_HXX
#define CSV_SWELIST_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <cosv/tpl/dyn.hxx>


namespace csv
{


template <class XX>
class SweListElement
{
  public:
    typedef SweListElement<XX> self;

                        SweListElement(
                            const XX &          in_aObj )
                                                :   aObj(in_aObj), pNext(0) {}

    const XX &          Obj() const             { return aObj; }
    XX &                Obj()                   { return aObj; }
    self *              Next() const            { return pNext; }

    void                SetNext(
                            self *              i_pNext )
                                                { pNext = i_pNext; }
  private:
    XX                  aObj;
    self *              pNext;
};



template <class XX>  class SweListIterator;
template <class XX>  class SweListCIterator;


template <class XX>
class SweList
{
  public:
    // TYPES
    typedef SweList<XX>             self;
    typedef XX                      value_type;
    typedef SweListIterator<XX>     iterator;
    typedef SweListCIterator<XX>    const_iterator;
  private:
    typedef SweListElement<XX>      elem;

  public:
    // LIFECYCLE
                        SweList()               : pTop(0), pTail(0) {}
                        ~SweList()              { erase_all(); }
    // OPERATIONS
    void                push_front(
                            const XX &          i_aObj );
    void                pop_front();
    void                push_back(
                            const XX &          i_aObj );
    void                erase_all();

    // INQUIRY
    const_iterator      begin() const           { return pTop; }
    iterator            begin()                 { return pTop; }
    const_iterator      end() const             { return (elem*)0; }
    iterator            end()                   { return (elem*)0; }
    const XX &          front() const           { return pTop->Obj(); }
    XX &                front()                 { return pTop->Obj(); }
    const XX &          back() const            { return pTail->Obj(); }
    XX &                back()                  { return pTail->Obj(); }

    bool                empty() const           { return pTop == 0; }
    uintt               size() const;


  private:
    // Forbiddden methods.
                        SweList(
                            const self &        i_rList );
    self &              operator=(
                            const self &        i_rList );

    // DATA
    DYN elem *          pTop;
    elem *              pTail;
};

template <class XX>
class SweList_dyn
{
  public:
    // TYPES
    typedef SweList_dyn<XX>         self;
    typedef SweListElement< XX* >   elem;
    typedef SweListIterator< XX* >  iterator;

    // LIFECYCLE
                        SweList_dyn()       : pTop(0), pTail(0) {}
                        ~SweList_dyn()      { erase_all(); }
    // OPERATIONS
    void                push_front(
                            XX *            i_pObj );
    void                push_back(
                            XX *            i_pObj );
    void                pop_front();
    void                erase_all();

    // INQUIRY
    iterator            begin() const       { return pTop; }
    iterator            end() const         { return (elem*)0; }
    XX *                front() const       { return pTop->Obj(); }
    XX *                back() const        { return pTail->Obj(); }

    bool                empty() const       { return pTop == 0; }
    uintt               size() const;

  private:
    // Forbiddden methods.
                        SweList_dyn(
                            const self &        i_rList );
    self &              operator=(
                            const self &        i_rList );

    DYN elem *          pTop;
    elem *              pTail;
};




template<class XX>
class SweListIterator
{
  public:
    typedef SweListIterator<XX> self;
    typedef SweListElement<XX>  elem;

                        SweListIterator(
                            elem *              i_pElem = 0)
                                                :   pElem(i_pElem) { }

    // OPERATORS
    XX &                operator*() const       { return pElem->Obj(); }
    self &              operator++()            { if (pElem != 0) pElem = pElem->Next();
                                                  return *this; }
    bool                operator==(
                            const self &        i_rIter ) const
                                                { return pElem == i_rIter.pElem; }
    bool                operator!=(
                            const self &        i_rIter ) const
                                                { return pElem != i_rIter.pElem; }
  private:
    friend class SweListCIterator<XX>;

    elem *              pElem;
};

template<class XX>
class SweListCIterator
{
  public:
    typedef SweListCIterator<XX> self;
    typedef SweListElement<XX>  elem;

                        SweListCIterator(
                            const elem *        i_pElem = 0)
                                                :   pElem(i_pElem) { }

    // OPERATORS
    self &              operator=(
                            const SweListIterator<XX> &
                                                i_rIter )
                                                { pElem = i_rIter.pElem; return *this; }

    const XX &          operator*() const       { return pElem->Obj(); }
    self &              operator++()            { if (pElem != 0) pElem = pElem->Next();
                                                  return *this; }
    bool                operator==(
                            const self &        i_rIter ) const
                                                { return pElem == i_rIter.pElem; }
    bool                operator!=(
                            const self &        i_rIter ) const
                                                { return pElem != i_rIter.pElem; }
  private:
    const elem *        pElem;
};

// Implementierung

template <class XX>
void
SweList<XX>::push_front( const XX & i_aObj )
{
    DYN elem * dpNew = new elem(i_aObj);
    dpNew->SetNext(pTop);
    pTop = dpNew;
    if (pTail == 0)
        pTail = pTop;
}

template <class XX>
void
SweList<XX>::push_back( const XX & i_aObj)
{
    if (pTail != 0)
    {
        pTail->SetNext(new elem(i_aObj));
        pTail = pTail->Next();
    }
    else
    {
        pTop = pTail = new elem(i_aObj);
    }
}

template <class XX>
void
SweList<XX>::pop_front()
{
    if (pTop != 0)
    {
        elem * pDel = pTop;
        pTop = pTop->Next();
        delete pDel;
        if (pTop == 0)
            pTail = 0;
    }
}

template <class XX>
uintt
SweList<XX>::size() const
{
    uintt ret = 0;
    for ( const_iterator iter = begin();
          iter != end();
          ++iter )
    {
        ++ret;
    }
    return ret;
}


template <class XX>
void
SweList<XX>::erase_all()
{
    for (pTail = pTop ; pTop != 0; pTail = pTop)
    {
        pTop = pTop->Next();
        delete pTail;
    }
    pTop = pTail = 0;
}


template <class XX>
void
SweList_dyn<XX>::push_front( XX * i_pObj )
{
    DYN elem * dpNew = new elem(i_pObj);
    dpNew->SetNext(pTop);
    pTop = dpNew;
    if (pTail == 0)
        pTail = pTop;
}

template <class XX>
void
SweList_dyn<XX>::push_back( XX * i_pObj )
{
    if (pTail != 0)
    {
        pTail->SetNext(new elem(i_pObj));
        pTail = pTail->Next();
    }
    else
    {
        pTop = pTail = new elem(i_pObj);
    }
}

template <class XX>
void
SweList_dyn<XX>::pop_front()
{
    if (pTop != 0)
    {
        elem * pDel = pTop;
        pTop = pTop->Next();
        if (pDel->Obj() != 0)
            Delete_dyn(pDel->Obj());
        delete pDel;
        if (pTop == 0)
            pTail = 0;
    }
}


template <class XX>
void
SweList_dyn<XX>::erase_all()
{
    for (pTail = pTop ; pTop != 0; pTail = pTop)
    {
        pTop = pTop->Next();
        if (pTail->Obj() != 0)
        {
            delete pTail->Obj();
        }
        delete pTail;
    }
    pTop = pTail = 0;
}

template <class XX>
uintt
SweList_dyn<XX>::size() const
{
    uintt ret = 0;
    for ( iterator iter = begin();
          iter != end();
          ++iter )
    {
        ++ret;
    }
    return ret;
}


}   // namespace csv


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
