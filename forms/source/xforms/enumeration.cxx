/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: enumeration.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:03:59 $
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
#include "precompiled_forms.hxx"

#include "enumeration.hxx"

#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>

using com::sun::star::container::NoSuchElementException;
using com::sun::star::container::XIndexAccess;
using com::sun::star::lang::WrappedTargetException;
using com::sun::star::uno::Any;
using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;


Enumeration::Enumeration( const Reference<XIndexAccess>& xContainer )
    : mxContainer( xContainer ),
      mnIndex( 0 )
{
    OSL_ENSURE( mxContainer.is(), "no container?" );
}

Enumeration::Enumeration( XIndexAccess* pContainer )
    : mxContainer( pContainer ),
      mnIndex( 0 )
{
    OSL_ENSURE( mxContainer.is(), "no container?" );
}

sal_Bool Enumeration::hasMoreElements()
    throw( RuntimeException )
{
    if( ! mxContainer.is() )
        throw RuntimeException();

    return mnIndex < mxContainer->getCount();
}

Any Enumeration::nextElement()
    throw( NoSuchElementException,
           WrappedTargetException,
           RuntimeException )
{
    if( ! mxContainer.is() )
        throw RuntimeException();
    if( mnIndex >= mxContainer->getCount() )
        throw NoSuchElementException();

    return mxContainer->getByIndex( mnIndex++ );
}
