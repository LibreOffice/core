/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

