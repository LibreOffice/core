/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sourcecontext.hxx,v $
 * $Revision: 1.7 $
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
#ifndef _SOURCECONTEXT_HXX_
#define _SOURCECONTEXT_HXX_

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#include <cppuhelper/compbase1.hxx>


#include "source.hxx"

using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


// This class fires events to XDragSourceListener implementations.
// Of that interface only dragDropEnd and dropActionChanged are called.
// The functions dragEnter, dragExit and dragOver are not supported
// currently.
// An instance of SourceContext only lives as long as the drag and drop
// operation lasts.
class SourceContext: public MutexDummy,
                     public WeakComponentImplHelper1<XDragSourceContext>
{
    DragSource* m_pDragSource;
    Reference<XDragSource> m_dragSource;
    // the action ( copy, move etc)
    sal_Int8 m_currentAction;

    SourceContext();
    SourceContext( const SourceContext&);
    SourceContext &operator= (const SourceContext& );

public:
    SourceContext( DragSource* pSource, const Reference<XDragSourceListener>& listener);
    ~SourceContext();

    virtual void SAL_CALL addDragSourceListener( const Reference<XDragSourceListener >& dsl )
        throw( RuntimeException);
    virtual void SAL_CALL removeDragSourceListener( const Reference<XDragSourceListener >& dsl )
        throw( RuntimeException);
    virtual sal_Int32 SAL_CALL getCurrentCursor(  )
        throw( RuntimeException);
    virtual void SAL_CALL setCursor( sal_Int32 cursorId )
        throw( RuntimeException);
    virtual void SAL_CALL setImage( sal_Int32 imageId )
        throw( RuntimeException);
    virtual void SAL_CALL transferablesFlavorsChanged(  )
        throw( RuntimeException);



    // non - interface functions
    void fire_dragDropEnd( sal_Bool success, sal_Int8 byte);
    void fire_dropActionChanged( sal_Int8 dropAction, sal_Int8 userAction);

};



#endif
