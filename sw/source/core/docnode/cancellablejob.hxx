/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cancellablejob.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 13:30:29 $
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
#ifndef _CANCELLABLEJOB_HXX
#define _CANCELLABLEJOB_HXX

#include "sal/config.h"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/util/XCancellable.hpp"

#include <rtl/ref.hxx>

class ObservableThread;

class CancellableJob : public ::cppu::WeakImplHelper1<com::sun::star::util::XCancellable>
{
public:
    explicit CancellableJob( const ::rtl::Reference< ObservableThread >& rThread );
    ~CancellableJob() {}

    // ::com::sun::star::util::XCancellable:
    virtual void SAL_CALL cancel() throw (com::sun::star::uno::RuntimeException);

private:
    CancellableJob( CancellableJob& ); // not defined
    void operator =( CancellableJob& ); // not defined

    ::rtl::Reference< ObservableThread > mrThread;
};
#endif
