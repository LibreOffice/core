/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DragSourceContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 14:47:23 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"


#ifndef _COM_SUN_STAR_DATATRANSFER_DND_DNDCONSTANTS_HPP_
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#endif

#include "DragSourceContext.hxx"

#ifndef _RTL_UNLOAD_H_
#include <rtl/unload.h>
#endif


using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;
using namespace com::sun::star::uno;
using namespace cppu;


extern rtl_StandardModuleCount g_moduleCount;


DragSourceContext::DragSourceContext( DragSource* pSource) :
        WeakComponentImplHelper1<XDragSourceContext>(m_aMutex),
        m_pDragSource( pSource)
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
}

DragSourceContext::~DragSourceContext()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

sal_Int32 SAL_CALL DragSourceContext::getCurrentCursor(  )
    throw( RuntimeException)
{
    return 0;
}

void SAL_CALL DragSourceContext::setCursor( sal_Int32 /*cursorId*/ )
    throw( RuntimeException)
{
}

void SAL_CALL DragSourceContext::setImage( sal_Int32 /*imageId*/ )
    throw( RuntimeException)
{
}

void SAL_CALL DragSourceContext::transferablesFlavorsChanged(  )
    throw( RuntimeException)
{
}


