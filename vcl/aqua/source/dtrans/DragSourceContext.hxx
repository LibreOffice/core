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

#ifndef _DRAGSOURCECONTEXT_HXX_
#define _DRAGSOURCECONTEXT_HXX_

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <boost/utility.hpp>

// This class fires events to XDragSourceListener implementations.
// Of that interface only dragDropEnd and dropActionChanged are called.
// The functions dragEnter, dragExit and dragOver are not supported
// currently.
// An instance of SourceContext only lives as long as the drag and drop
// operation lasts.
class DragSourceContext: public cppu::BaseMutex,
                     public cppu::WeakComponentImplHelper1<com::sun::star::datatransfer::dnd::XDragSourceContext>,
                     private ::boost::noncopyable
{
public:
  DragSourceContext();
  ~DragSourceContext();

  virtual sal_Int32 SAL_CALL getCurrentCursor(  )
    throw( com::sun::star::uno::RuntimeException);

  virtual void SAL_CALL setCursor( sal_Int32 cursorId )
    throw( com::sun::star::uno::RuntimeException);

  virtual void SAL_CALL setImage( sal_Int32 imageId )
    throw( com::sun::star::uno::RuntimeException);

  virtual void SAL_CALL transferablesFlavorsChanged(  )
    throw( com::sun::star::uno::RuntimeException);
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
