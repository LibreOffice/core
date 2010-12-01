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

#ifndef DBAUI_REFFUNCTOR_HXX
#define DBAUI_REFFUNCTOR_HXX

#include <rtl/ref.hxx>
#ifndef INCLUDED_FUNCTIONAL
#define INCLUDED_FUNCTIONAL
#include <functional>
#endif // INCLUDED_FUNCTIONAL

namespace dbaui
{
    template <class T> class OUnaryRefFunctor : public ::std::unary_function< ::rtl::Reference<T> ,void>
    {
        ::std::mem_fun_t<bool,T> m_aFunction;
    public:
        OUnaryRefFunctor(const ::std::mem_fun_t<bool,T>& _aFunc) : m_aFunction(_aFunc)
        {}
        inline void operator()(const ::rtl::Reference<T>& _aType) const
        {
            m_aFunction(_aType.get());
        }
//      inline void operator()(const ::rtl::Reference<T>& _aType)
//      {
//          m_aFunction(_aType.get());
//      }
    };
// -----------------------------------------------------------------------------
} // namespace dbaui
// -----------------------------------------------------------------------------
#endif //DBAUI_REFFUNCTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
