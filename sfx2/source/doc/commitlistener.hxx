/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commitlistener.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:37:06 $
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

#ifndef __COMMITLISTENER_HXX_
#define __COMMITLISTENER_HXX_

#ifndef _COM_SUN_STAR_EMBED_XTRANSACTIONLISTENER_HPP_
#include <com/sun/star/embed/XTransactionListener.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <sfxbasemodel.hxx>

class OChildCommitListen_Impl : public ::cppu::WeakImplHelper1 < ::com::sun::star::embed::XTransactionListener >
{
    ::osl::Mutex m_aMutex;
    SfxBaseModel* m_pModel;

public:
    OChildCommitListen_Impl( SfxBaseModel& aStorage );
    virtual ~OChildCommitListen_Impl();

    void OwnerIsDisposed();

    virtual void SAL_CALL preCommit( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL commited( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL preRevert( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reverted( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
};

#endif

