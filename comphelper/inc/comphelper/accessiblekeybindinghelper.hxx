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



#ifndef COMPHELPER_ACCESSIBLE_KEYBINDING_HELPER_HXX
#define COMPHELPER_ACCESSIBLE_KEYBINDING_HELPER_HXX

#include <com/sun/star/accessibility/XAccessibleKeyBinding.hpp>
#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif
#include "comphelper/comphelperdllapi.h"

//..............................................................................
namespace comphelper
{
//..............................................................................

    //==============================================================================
    // OAccessibleKeyBindingHelper
    //==============================================================================

    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::accessibility::XAccessibleKeyBinding
                                    >   OAccessibleKeyBindingHelper_Base;

    /** a helper class for implementing an accessible keybinding
     */
    class COMPHELPER_DLLPUBLIC OAccessibleKeyBindingHelper : public OAccessibleKeyBindingHelper_Base
    {
    private:
        typedef ::std::vector< ::com::sun::star::uno::Sequence< ::com::sun::star::awt::KeyStroke > > KeyBindings;

        KeyBindings     m_aKeyBindings;

    protected:
        ::osl::Mutex    m_aMutex;

        virtual ~OAccessibleKeyBindingHelper();

    public:
        OAccessibleKeyBindingHelper();
        OAccessibleKeyBindingHelper( const OAccessibleKeyBindingHelper& rHelper );

        void AddKeyBinding( const ::com::sun::star::uno::Sequence< ::com::sun::star::awt::KeyStroke >& rKeyBinding ) throw (::com::sun::star::uno::RuntimeException);
        void AddKeyBinding( const ::com::sun::star::awt::KeyStroke& rKeyStroke ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleKeyBinding
        virtual sal_Int32 SAL_CALL getAccessibleKeyBindingCount() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::awt::KeyStroke > SAL_CALL getAccessibleKeyBinding( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    };

//..............................................................................
}   // namespace comphelper
//..............................................................................

#endif // COMPHELPER_ACCESSIBLE_KEYBINDING_HELPER_HXX
