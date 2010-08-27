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

#ifndef DBAUI_REFFUNCTOR_HXX
#define DBAUI_REFFUNCTOR_HXX

#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef INCLUDED_FUNCTIONAL
#define INCLUDED_FUNCTIONAL
#include <functional>
#endif // INCLUDED_FUNCTIONAL

namespace dbaui
{
    template <class T> class OUnaryRefFunctor : public ::std::unary_function< ::vos::ORef<T> ,void>
    {
        ::std::mem_fun_t<bool,T> m_aFunction;
    public:
        OUnaryRefFunctor(const ::std::mem_fun_t<bool,T>& _aFunc) : m_aFunction(_aFunc)
        {}
        inline void operator()(const ::vos::ORef<T>& _aType) const
        {
            m_aFunction(_aType.getBodyPtr());
        }
//      inline void operator()(const ::vos::ORef<T>& _aType)
//      {
//          m_aFunction(_aType.getBodyPtr());
//      }
    };
// -----------------------------------------------------------------------------
} // namespace dbaui
// -----------------------------------------------------------------------------
#endif //DBAUI_REFFUNCTOR_HXX

