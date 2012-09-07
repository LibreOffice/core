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

#ifndef SC_VBA_LISTCONTROLHELPER
#define SC_VBA_LISTCONTROLHELPER

#include <vbahelper/vbahelper.hxx>

class ListControlHelper
{
    css::uno::Reference< css::beans::XPropertySet > m_xProps;

public:
    ListControlHelper( const css::uno::Reference< css::beans::XPropertySet >& rxControl ) : m_xProps( rxControl ){}
    virtual ~ListControlHelper() {}
    virtual void SAL_CALL AddItem( const css::uno::Any& pvargItem, const css::uno::Any& pvargIndex ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeItem( const css::uno::Any& index ) throw (css::uno::RuntimeException);
        virtual void SAL_CALL setRowSource( const OUString& _rowsource ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getListCount() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL List( const css::uno::Any& pvargIndex, const css::uno::Any& pvarColumn ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Clear(  ) throw (css::uno::RuntimeException);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
