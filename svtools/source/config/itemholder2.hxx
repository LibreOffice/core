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



#ifndef INCLUDED_SVTOOLS_ITEMHOLDER2_HXX_
#define INCLUDED_SVTOOLS_ITEMHOLDER2_HXX_

//-----------------------------------------------
// includes

#include <unotools/itemholderbase.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XEventListener.hpp>

//-----------------------------------------------
// namespaces

#ifdef css
#error "Can't use css as namespace alias."
#else
#define css ::com::sun::star
#endif

//-----------------------------------------------
// definitions

class ItemHolder2 : private ItemHolderMutexBase
                  , public  ::cppu::WeakImplHelper1< css::lang::XEventListener >
{
    //...........................................
    // member
    private:

        TItems m_lItems;

    //...........................................
    // c++ interface
    public:

        ItemHolder2();
        virtual ~ItemHolder2();
        static void holdConfigItem(EItem eItem);

    //...........................................
    // uno interface
    public:

        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

    //...........................................
    // helper
    private:

        void impl_addItem(EItem eItem);
        void impl_releaseAllItems();
        void impl_newItem(TItemInfo& rItem);
        void impl_deleteItem(TItemInfo& rItem);
};

//-----------------------------------------------
// namespaces

#undef css

#endif // INCLUDED_SVTOOLS_ITEMHOLDER2_HXX_
