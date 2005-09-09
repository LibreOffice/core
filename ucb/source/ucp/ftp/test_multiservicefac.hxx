/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test_multiservicefac.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:42:11 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#ifndef _TEST_MULTISERVICEFAC_HXX_
#define _TEST_MULTISERVICEFAC_HXX_

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif


namespace test_ftp {

    class Test_MultiServiceFactory
        : public cppu::OWeakObject,
          public com::sun::star::lang::XMultiServiceFactory
    {
    public:

        // XInterface

        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface( const com::sun::star::uno::Type& rType )
            throw( com::sun::star::uno::RuntimeException );


        virtual void SAL_CALL acquire( void ) throw();

        virtual void SAL_CALL release( void ) throw();

        // XMultiServiceFactory

        virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > SAL_CALL
        createInstance(
            const ::rtl::OUString& aServiceSpecifier
        )
            throw (
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException
            );

        virtual
        ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > SAL_CALL
        createInstanceWithArguments(
            const ::rtl::OUString& ServiceSpecifier,
            const ::com::sun::star::uno::Sequence
            < ::com::sun::star::uno::Any >& Arguments
        )
            throw (
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException
            );

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getAvailableServiceNames(
        )
            throw (
                ::com::sun::star::uno::RuntimeException
            );
    };

}

#endif

