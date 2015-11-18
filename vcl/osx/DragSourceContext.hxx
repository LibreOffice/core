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

#ifndef INCLUDED_VCL_OSX_DRAGSOURCECONTEXT_HXX
#define INCLUDED_VCL_OSX_DRAGSOURCECONTEXT_HXX

#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <boost/noncopyable.hpp>

// This class fires events to XDragSourceListener implementations.
// Of that interface only dragDropEnd and dropActionChanged are called.
// The functions dragEnter, dragExit and dragOver are not supported
// currently.
// An instance of SourceContext only lives as long as the drag and drop
// operation lasts.
class DragSourceContext: public cppu::BaseMutex,
                     public cppu::WeakComponentImplHelper<css::datatransfer::dnd::XDragSourceContext>,
                     private ::boost::noncopyable
{
public:
  DragSourceContext();
  virtual ~DragSourceContext();

  virtual sal_Int32 SAL_CALL getCurrentCursor(  )
    throw( css::uno::RuntimeException, std::exception) override;

  virtual void SAL_CALL setCursor( sal_Int32 cursorId )
    throw( css::uno::RuntimeException, std::exception) override;

  virtual void SAL_CALL setImage( sal_Int32 imageId )
    throw( css::uno::RuntimeException, std::exception) override;

  virtual void SAL_CALL transferablesFlavorsChanged(  )
    throw( css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_VCL_OSX_DRAGSOURCECONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
