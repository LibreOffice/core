/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DragSourceContext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 14:47:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _DRAGSOURCECONTEXT_HXX_
#define _DRAGSOURCECONTEXT_HXX_

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCECONTEXT_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif

#ifndef _CPPUHELPER_BASEMUTEX_HXX_
#include <cppuhelper/basemutex.hxx>
#endif

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
