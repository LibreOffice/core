/*************************************************************************
 *
 *  $RCSfile: estack.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 15:22:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

