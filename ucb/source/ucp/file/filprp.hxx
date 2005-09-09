/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filprp.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:26:45 $
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
#ifndef _FILPRP_HXX_
#define _FILPRP_HXX_

#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif


namespace fileaccess {

    class shell;

    class XPropertySetInfo_impl
        : public cppu::OWeakObject,
          public com::sun::star::lang::XTypeProvider,
          public com::sun::star::beans::XPropertySetInfo
    {
    public:
        XPropertySetInfo_impl( shell* pMyShell,const rtl::OUString& aUnqPath );
        XPropertySetInfo_impl( shell* pMyShell,const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& seq );

        virtual ~XPropertySetInfo_impl();

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();


        // XTypeProvider

        XTYPEPROVIDER_DECL()

        virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property > SAL_CALL
        getProperties(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::beans::Property SAL_CALL
        getPropertyByName(
            const rtl::OUString& aName )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        hasPropertyByName( const rtl::OUString& Name )
            throw( com::sun::star::uno::RuntimeException );

    private:
        shell*                                                                     m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >    m_xProvider;
        sal_Int32                                                                  m_count;
        com::sun::star::uno::Sequence< com::sun::star::beans::Property >           m_seq;
    };
}

#endif

