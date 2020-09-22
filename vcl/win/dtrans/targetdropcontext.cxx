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

#include "targetdropcontext.hxx"

using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

TargetDropContext::TargetDropContext( DropTarget* p)
{
    m_pDropTarget= p;
    p->acquire();
}

TargetDropContext::~TargetDropContext()
{
    m_pDropTarget->release();
}

void SAL_CALL TargetDropContext::acceptDrop( sal_Int8 dropOperation )
{
    m_pDropTarget->_acceptDrop( dropOperation, static_cast<XDropTargetDropContext*>( this) );
}

void SAL_CALL TargetDropContext::rejectDrop( )
{
    m_pDropTarget->_rejectDrop(  static_cast<XDropTargetDropContext*>( this) );
}

void SAL_CALL TargetDropContext::dropComplete( sal_Bool success )
{
    m_pDropTarget->_dropComplete( success, static_cast<XDropTargetDropContext*>( this) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
