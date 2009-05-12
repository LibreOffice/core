/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: so_closelistener.hxx,v $
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

#ifndef __SO_CLOSELISTENER_HXX_
#define __SO_CLOSELISTENER_HXX_

#include <com/sun/star/util/XCloseListener.hpp>
#include <cppuhelper/implbase1.hxx>

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

