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
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>
#include <com/sun/star/datatransfer/DataFlavor.hpp>

#include "target.hxx"
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

class TargetDragContext: public WeakImplHelper<XDropTargetDragContext>
{
    // some calls to the functions of XDropTargetDragContext are delegated
    // to non-interface functions of m_pDropTarget
    DropTarget* m_pDropTarget;

public:
    explicit TargetDragContext(DropTarget* pTarget);
    ~TargetDragContext() override;
    TargetDragContext( const TargetDragContext&) = delete;
    TargetDragContext &operator= ( const TargetDragContext&) = delete;

    virtual void SAL_CALL acceptDrag( sal_Int8 dragOperation ) override;
    virtual void SAL_CALL rejectDrag(  ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
