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


