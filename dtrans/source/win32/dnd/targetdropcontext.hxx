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

#ifndef _TARGETDROPCONTEXT_HXX_
#define _TARGETDROPCONTEXT_HXX_

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>

#include "target.hxx"

using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

class TargetDropContext: public WeakImplHelper1<XDropTargetDropContext>
{
    // calls to the functions of XDropTargetDropContext are delegated
    // to non-interface functions of m_pDropTarget
    DropTarget* m_pDropTarget;

    TargetDropContext();
    TargetDropContext( const TargetDropContext&);
    TargetDropContext &operator= ( const TargetDropContext&);
public:
    TargetDropContext( DropTarget* pTarget);
    ~TargetDropContext();


    // XDropTargetDragContext
    virtual void SAL_CALL acceptDrop( sal_Int8 dropOperation )
        throw( RuntimeException);
    virtual void SAL_CALL rejectDrop(  )
        throw( RuntimeException);

    // XDropTargetDropContext (inherits XDropTargetDragContext)
    virtual void SAL_CALL dropComplete( sal_Bool success )
        throw(  RuntimeException);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
