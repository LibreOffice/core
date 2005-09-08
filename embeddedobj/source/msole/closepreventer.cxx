/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: closepreventer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:41:19 $
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

#include "closepreventer.hxx"

void SAL_CALL OClosePreventer::queryClosing( const ::com::sun::star::lang::EventObject& Source, sal_Bool GetsOwnership )
        throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException)
{
    throw ::com::sun::star::util::CloseVetoException();
}

void SAL_CALL OClosePreventer::notifyClosing( const com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException)
{
    // just a disaster
    OSL_ENSURE( sal_False, "The object can not be prevented from closing!\n" );
}

void SAL_CALL OClosePreventer::disposing( const com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException)
{
    // just a disaster
    OSL_ENSURE( sal_False, "The object can not be prevented from closing!\n" );
}

