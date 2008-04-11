/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: targetdropcontext.hxx,v $
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


/*  virtual Sequence< DataFlavor > SAL_CALL getCurrentDataFlavors(  )
        throw( RuntimeException);
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const DataFlavor& df )
        throw( RuntimeException);
*/

    // XDropTargetDropContext (inherits XDropTargetDragContext)
    virtual void SAL_CALL dropComplete( sal_Bool success )
        throw(  RuntimeException);
};
#endif
