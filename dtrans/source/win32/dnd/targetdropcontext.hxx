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

#pragma once

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>

#include "target.hxx"

using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

class TargetDropContext: public WeakImplHelper<XDropTargetDropContext>
{
    // calls to the functions of XDropTargetDropContext are delegated
    // to non-interface functions of m_pDropTarget
    DropTarget* m_pDropTarget;

public:
    explicit TargetDropContext(DropTarget* pTarget);
    ~TargetDropContext() override;
    TargetDropContext( const TargetDropContext&) = delete;
    TargetDropContext &operator= ( const TargetDropContext&) = delete;

    // XDropTargetDragContext
    virtual void SAL_CALL acceptDrop( sal_Int8 dropOperation ) override;
    virtual void SAL_CALL rejectDrop(  ) override;

    // XDropTargetDropContext (inherits XDropTargetDragContext)
    virtual void SAL_CALL dropComplete( sal_Bool success ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
