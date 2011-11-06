/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
