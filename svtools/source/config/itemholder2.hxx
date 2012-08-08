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

#ifndef INCLUDED_SVTOOLS_ITEMHOLDER2_HXX_
#define INCLUDED_SVTOOLS_ITEMHOLDER2_HXX_

#include <unotools/itemholderbase.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XEventListener.hpp>

//-----------------------------------------------
// namespaces

namespace css = ::com::sun::star;

namespace svtools {
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

} // namespace svtools

#endif // INCLUDED_SVTOOLS_ITEMHOLDER2_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
