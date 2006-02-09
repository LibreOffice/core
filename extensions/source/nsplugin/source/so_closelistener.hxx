/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: so_closelistener.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-02-09 13:53:18 $
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

#ifndef __SO_CLOSELISTENER_HXX_
#define __SO_CLOSELISTENER_HXX_

#ifndef _COM_SUN_STAR_UTIL_XCLOSELISTENER_HPP_
#include <com/sun/star/util/XCloseListener.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

class PluginDocumentClosePreventer : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XCloseListener >
{
    sal_Bool m_bPreventClose;

public:
    PluginDocumentClosePreventer()
    : m_bPreventClose( sal_True )
    {}

    void StopPreventClose() { m_bPreventClose = sal_False; }

    virtual void SAL_CALL queryClosing( const ::com::sun::star::lang::EventObject& aEvent, sal_Bool bDeliverOwnership )
        throw ( ::com::sun::star::uno::RuntimeException, ::com::sun::star::util::CloseVetoException );

    virtual void SAL_CALL notifyClosing( const ::com::sun::star::lang::EventObject& aEvent ) throw ( ::com::sun::star::uno::RuntimeException ) ;

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw ( ::com::sun::star::uno::RuntimeException ) ;
};

#endif

