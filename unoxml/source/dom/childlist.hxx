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

#ifndef DOM_CHILDLIST_HXX
#define DOM_CHILDLIST_HXX

#include <sal/types.h>
#include <rtl/ref.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>

#include <cppuhelper/implbase1.hxx>


using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;

namespace DOM
{
    class CNode;

    class CChildList
        : public cppu::WeakImplHelper1< XNodeList >
    {
    private:
        ::rtl::Reference<CNode> const m_pNode;
        ::osl::Mutex & m_rMutex;

    public:
        CChildList(::rtl::Reference<CNode> const& pBase,
                ::osl::Mutex & rMutex);

        /**
        The number of nodes in the list.
        */
        virtual sal_Int32 SAL_CALL getLength() throw (RuntimeException);
        /**
        Returns the indexth item in the collection.
        */
        virtual Reference< XNode > SAL_CALL item(sal_Int32 index)
            throw (RuntimeException);
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
