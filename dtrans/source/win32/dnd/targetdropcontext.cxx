/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: targetdropcontext.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 16:59:31 $
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

#include "targetdropcontext.hxx"

using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

extern rtl_StandardModuleCount g_moduleCount;
TargetDropContext::TargetDropContext( DropTarget* p)
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    m_pDropTarget= p;
    p->acquire();
}

TargetDropContext::~TargetDropContext()
{
    m_pDropTarget->release();
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

void SAL_CALL TargetDropContext::acceptDrop( sal_Int8 dropOperation )
        throw( RuntimeException)
{
    m_pDropTarget->_acceptDrop( dropOperation, static_cast<XDropTargetDropContext*>( this) );
}

void SAL_CALL TargetDropContext::rejectDrop( )
        throw( RuntimeException)
{
    m_pDropTarget->_rejectDrop(  static_cast<XDropTargetDropContext*>( this) );
}

void SAL_CALL TargetDropContext::dropComplete( sal_Bool success )
        throw( RuntimeException)
{
    m_pDropTarget->_dropComplete( success, static_cast<XDropTargetDropContext*>( this) );
}
