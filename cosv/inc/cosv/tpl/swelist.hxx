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

// IMPLEMENTATION

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
