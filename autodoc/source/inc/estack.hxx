/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: estack.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:09:00 $
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

#ifndef ARY_ESTACK_HXX
#define ARY_ESTACK_HXX



// USED SERVICES
    // BASE CLASSES
#include <slist>
    // COMPONENTS
    // PARAMETERS



template <class ELEM>
class EStack : private std::slist<ELEM>
{
  private:
    typedef std::slist<ELEM>    base;
    const base &        Base() const            { return *this; }
    base &              Base()                  { return *this; }

  public:
    typedef ELEM                                    value_type;
    typedef typename std::slist<ELEM>::size_type    size_type;

    // LIFECYCLE
                        EStack()                {}
                        EStack(
                            const EStack &      i_rStack )
                                                :   base( (const base &)(i_rStack) ) {}
                        ~EStack()               {}
    // OPERATORS
    EStack &            operator=(
                            const EStack &      i_rStack )
                                                { base::operator=( i_rStack.Base() );
                                                  return *this; }
    bool                operator==(
                            const EStack<ELEM> &
                                                i_r2 ) const
                                                { return std::operator==( Base(), this->i_rStack.Base() ); }
    bool                operator<(
                            const EStack<ELEM> &
                                                i_r2 ) const
                                                { return std::operator<( Base(), this->i_rStack.Base() ); }
    // OPERATIONS
    void                push(
                            const value_type &  i_rElem )
                                                { base::push_front(i_rElem); }
    void                pop()                   { base::pop_front(); }
    void                erase_all()             { while (NOT empty()) pop(); }

    // INQUIRY
    const value_type &  top() const             { return base::front(); }
    size_type           size() const            { return base::size(); }
    bool                empty() const           { return base::empty(); }

    // ACCESS
    value_type &        top()                   { return base::front(); }
};



// IMPLEMENTATION


#endif

