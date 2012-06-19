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
