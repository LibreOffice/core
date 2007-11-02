/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stdconstiter.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:43:44 $
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
