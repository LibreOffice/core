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
#include <com/sun/star/datatransfer/dnd/XDragSourceListener.hpp>
#include <com/sun/star/datatransfer/dnd/DragSourceDropEvent.hpp>
#include <com/sun/star/datatransfer/dnd/DragSourceDragEvent.hpp>

using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

class DragSourceListener: public WeakImplHelper<XDragSourceListener>
{
    // this is a window where dropped data are shown as text (only text)
public:
    DragSourceListener( );
    ~DragSourceListener();

    virtual void SAL_CALL disposing( const EventObject& Source )
        throw(RuntimeException);

    virtual void SAL_CALL dragDropEnd( const DragSourceDropEvent& dsde )
        throw(RuntimeException);
    virtual void SAL_CALL dragEnter( const DragSourceDragEvent& dsde )
        throw(RuntimeException);
    virtual void SAL_CALL dragExit( const DragSourceEvent& dse )
        throw(RuntimeException);
    virtual void SAL_CALL dragOver( const DragSourceDragEvent& dsde )
        throw(RuntimeException);
    virtual void SAL_CALL dropActionChanged( const DragSourceDragEvent& dsde )
        throw(RuntimeException);

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
