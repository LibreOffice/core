/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftpcontentidentifier.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 13:50:07 $
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
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include "ftpcontentidentifier.hxx"
#include "ftpcontentprovider.hxx"

using namespace ftp;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::lang;


FTPContentIdentifier::FTPContentIdentifier(
    const rtl::OUString& ident
)
    : m_ident(ident)
{
}


FTPContentIdentifier::~FTPContentIdentifier()
{
}


Any SAL_CALL
FTPContentIdentifier::queryInterface(
    const Type& rType
)
    throw(
        RuntimeException
    )
{
    Any aRet =
        ::cppu::queryInterface(rType,
                               SAL_STATIC_CAST(XTypeProvider*,this),
                               SAL_STATIC_CAST(XContentIdentifier*,this));

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


void SAL_CALL FTPContentIdentifier::acquire( void ) throw() {
    OWeakObject::acquire();
}


void SAL_CALL FTPContentIdentifier::release( void ) throw() {
    OWeakObject::release();
}


Sequence<sal_Int8> SAL_CALL
FTPContentIdentifier::getImplementationId()
    throw(RuntimeException)
{
    static cppu::OImplementationId* pId = NULL;
    if(!pId)
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pId )
        {
            static cppu::OImplementationId id( sal_False );
            pId = &id;
        }
    }
    return (*pId).getImplementationId();
}


Sequence<Type> SAL_CALL
FTPContentIdentifier::getTypes(
    void )
    throw(RuntimeException)
{
    static cppu::OTypeCollection* pCollection = NULL;
    if ( !pCollection ) {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType(
                    static_cast<Reference<XTypeProvider>*>(0)),
                getCppuType(
                    static_cast<Reference<XContentIdentifier>*>(0)));
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}


rtl::OUString SAL_CALL
FTPContentIdentifier::getContentIdentifier(
)
    throw (
        com::sun::star::uno::RuntimeException
    )
{
    return m_ident;
}


rtl::OUString SAL_CALL
FTPContentIdentifier::getContentProviderScheme(
)
    throw (
        com::sun::star::uno::RuntimeException
    )
{
    return rtl::OUString::createFromAscii("ftp");
}






