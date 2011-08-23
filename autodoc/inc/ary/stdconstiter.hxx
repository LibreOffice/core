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

#ifndef ARY_STDCONSTITER_HXX
#define ARY_STDCONSTITER_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


namespace ary
{

template <class ELEM>

class StdConstIterator
{
  public:
    virtual             ~StdConstIterator() {}

    void                operator++()            { do_Advance(); }
    const ELEM &        operator*() const       { return *inq_CurElement(); }
                        operator bool() const   { return inq_CurElement() != 0; }

    /// Needed as replacement for operator bool() in gcc 2.95.
    bool                IsValid() const         { return operator bool(); }
    bool                IsSorted() const        { return inq_IsSorted(); }

  protected:
                        StdConstIterator() {}

  private:
    //Locals
    virtual void        do_Advance() = 0;
    virtual const ELEM *
                        inq_CurElement() const = 0;
    virtual bool        inq_IsSorted() const = 0;

    // Forbidden:
    StdConstIterator(const StdConstIterator<ELEM>&);
    StdConstIterator<ELEM> & operator=(const StdConstIterator<ELEM>&);
};


template <class ELEM>
class Dyn_StdConstIterator
{
  public:
    typedef StdConstIterator<ELEM> client_type;

                        Dyn_StdConstIterator(
                            DYN client_type *   pass_dpIterator = 0 )
                                                : pClient(pass_dpIterator) {}
    Dyn_StdConstIterator<ELEM> &
                        operator=(
                            DYN client_type *   pass_dpIterator )
                                                { pClient = pass_dpIterator;
                                                  return *this; }
    client_type &       operator*() const       { return *pClient.MutablePtr(); }

  private:
    Dyn<client_type>    pClient;
};





}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
