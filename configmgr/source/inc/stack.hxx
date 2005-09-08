/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stack.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:57:02 $
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

#ifndef CONFIGMGR_STACK_HXX_
#define CONFIGMGR_STACK_HXX_

#ifndef INCLUDED_STACK
#include <stack>
#define INCLUDED_STACK
#endif
#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

namespace configmgr
{

    // simple wrapper for a vector-based stack
    template <typename T_>
    struct Stack : public std::stack< T_, std::vector<T_> >
    {
        typedef typename std::vector<T_>::const_iterator bottomup_iterator;
        typedef typename std::vector<T_>::const_reverse_iterator topdown_iterator;
        bottomup_iterator begin_up() const { return this->c.begin(); }
        bottomup_iterator end_up() const { return this->c.end(); }
        topdown_iterator begin_down() const { return this->c.rbegin(); }
        topdown_iterator end_down() const { return this->c.rend(); }
    };
}

#endif // CONFIGMGR_STACK_HXX_


