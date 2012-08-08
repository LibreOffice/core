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

#ifndef INCLUDED_unotools_ITEMHOLDER1_HXX_
#define INCLUDED_unotools_ITEMHOLDER1_HXX_

#include <unotools/itemholderbase.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XEventListener.hpp>

//-----------------------------------------------
// namespaces

namespace css = ::com::sun::star;


class ItemHolder1 : private ItemHolderMutexBase
                  , public  ::cppu::WeakImplHelper1< css::lang::XEventListener >
{
    //...........................................
    // member
    private:

        TItems m_lItems;

    //...........................................
    // c++ interface
    public:

        ItemHolder1();
        virtual ~ItemHolder1();
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

#endif // INCLUDED_unotools_ITEMHOLDER1_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
