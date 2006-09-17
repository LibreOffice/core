/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: targetdragcontext.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 16:59:16 $
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

#ifndef _RTL_UNLOAD_H_
#include <rtl/unload.h>
#endif

#include "targetdragcontext.hxx"

extern rtl_StandardModuleCount g_moduleCount;
TargetDragContext::TargetDragContext( DropTarget* p)
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    m_pDropTarget= p;
    p->acquire();
}

TargetDragContext::~TargetDragContext()
{
    m_pDropTarget->release();
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

void SAL_CALL TargetDragContext::acceptDrag( sal_Int8 dragOperation )
    throw( RuntimeException)
{
    m_pDropTarget->_acceptDrag( dragOperation, static_cast<XDropTargetDragContext*>( this) );

}
void SAL_CALL TargetDragContext::rejectDrag( )
    throw( RuntimeException)
{
    m_pDropTarget->_rejectDrag( static_cast<XDropTargetDragContext*>( this) );
}

