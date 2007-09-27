/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RefreshListenerContainer.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:39:39 $
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
#include "precompiled_sw.hxx"



#ifndef _REFRESH_LISTENER_CONTAINER_HXX_
#include <RefreshListenerContainer.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_EVENTOBJECT_HPP_
#include <com/sun/star/lang/EventObject.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREFRESHLISTENER_HPP_
#include <com/sun/star/util/XRefreshListener.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

SwRefreshListenerContainer::SwRefreshListenerContainer( uno::XInterface* pxParent2 )
: SwEventListenerContainer ( pxParent2 )
{
}

void SwRefreshListenerContainer::Refreshed ()
{
    if(!pListenerArr)
        return;

    lang::EventObject aObj(pxParent);
    for(sal_uInt16 i = 0, nEnd = pListenerArr->Count(); i < nEnd ; i++)
    {
        Reference < XRefreshListener > xRefreshListener = Reference < XRefreshListener > ( *pListenerArr->GetObject(i), UNO_QUERY );
        xRefreshListener->refreshed(aObj);
    }
}
