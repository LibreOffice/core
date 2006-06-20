/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mcnttfactory.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:59:09 $
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


#ifndef _MCNTTFACTORY_HXX_
#define _MCNTTFACTORY_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPEFACTORY_HPP_
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#endif

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

class CMimeContentTypeFactory : public
    cppu::WeakImplHelper2< com::sun::star::datatransfer::XMimeContentTypeFactory, \
    com::sun::star::lang::XServiceInfo >
{

public:
    CMimeContentTypeFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSrvMgr );

    //------------------------------------------------
    // XMimeContentTypeFactory
    //------------------------------------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XMimeContentType > SAL_CALL createMimeContentType( const ::rtl::OUString& aContentType )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XServiceInfo
    //------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw(::com::sun::star::uno::RuntimeException);

private:
    ::osl::Mutex                                                                           m_aMutex;
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_SrvMgr;
};


#endif

