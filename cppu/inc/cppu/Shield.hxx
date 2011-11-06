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



#ifndef INCLUDED_cppu_Shield_hxx
#define INCLUDED_cppu_Shield_hxx

#include <cppu/Map.hxx>


namespace cssu = com::sun::star::uno;

namespace cppu
{
    /** Helpers for mapping objects relative to the thread-safe and current environments.
        (http://wiki.services.openoffice.org/wiki/Uno/Cpp/Spec/Shield_Helpers)
    */


    /** Maps an object from the current to the thread-safe Environment, returns mapped object.

        @param  pT        the object to be mapped
        @return           the mapped object
        @since UDK 3.2.7
     */
    template<class T> inline T * shield(T * pT)
    {
        return mapOut(pT, cssu::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV)))));
    }

    /** Maps an object from the thread-safe Environment to the current one, returns mapped object.

        @param  pT        the object to be mapped
        @return           the mapped object
        @since UDK 3.2.7
     */
    template<class T> inline T * unshield(T * pT)
    {
        return mapIn(pT, cssu::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV)))));
    }


    /** Maps an any from the current to the thread-safe Environment, fills the passed any.

        @param  any  the any to be mapped
        @param       the target any
        @since UDK 3.2.7
     */
    inline void shieldAny(cssu::Any const & any, cssu::Any * res)
    {
        mapOutAny(any, res, cssu::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV)))));
    }


    /** Maps an any from the thread-safe Environment to the current one, fills the passed any.

        @param  any  the any to be mapped
        @param       the target any
        @since UDK 3.2.7
     */
    inline void unshieldAny(cssu::Any const & any, cssu::Any * res)
    {
        mapInAny(any, res, cssu::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV)))));
    }
}

#endif
