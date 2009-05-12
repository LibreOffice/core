/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DragSourceContext.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DRAGSOURCECONTEXT_HXX_
#define _DRAGSOURCECONTEXT_HXX_

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <boost/utility.hpp>

#include "DragSource.hxx"

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
  DragSourceContext(DragSource* pSource);
  ~DragSourceContext();

  virtual sal_Int32 SAL_CALL getCurrentCursor(  )
    throw( com::sun::star::uno::RuntimeException);

  virtual void SAL_CALL setCursor( sal_Int32 cursorId )
    throw( com::sun::star::uno::RuntimeException);

  virtual void SAL_CALL setImage( sal_Int32 imageId )
    throw( com::sun::star::uno::RuntimeException);

  virtual void SAL_CALL transferablesFlavorsChanged(  )
    throw( com::sun::star::uno::RuntimeException);

private:
  DragSource* m_pDragSource;
};



#endif
