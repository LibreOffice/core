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
    };
} // namespace dbaui
#endif //DBAUI_REFFUNCTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
