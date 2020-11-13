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

#ifndef INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBALISTCONTROLHELPER_HXX
#define INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBALISTCONTROLHELPER_HXX

#include <vbahelper/vbahelper.hxx>

class ListControlHelper final
{
    css::uno::Reference< css::beans::XPropertySet > m_xProps;

public:
    explicit ListControlHelper( const css::uno::Reference< css::beans::XPropertySet >& rxControl ) : m_xProps( rxControl ){}
    /// @throws css::uno::RuntimeException
    void AddItem( const css::uno::Any& pvargItem, const css::uno::Any& pvargIndex );
    /// @throws css::uno::RuntimeException
    void removeItem( const css::uno::Any& index );
    /// @throws css::uno::RuntimeException
        void setRowSource( std::u16string_view _rowsource );
    /// @throws css::uno::RuntimeException
    ::sal_Int32 getListCount();
    /// @throws css::uno::RuntimeException
    css::uno::Any List( const css::uno::Any& pvargIndex, const css::uno::Any& pvarColumn );
    /// @throws css::uno::RuntimeException
    void Clear(  );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
