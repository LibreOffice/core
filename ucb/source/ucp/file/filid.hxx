/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filid.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:24:15 $
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
#ifndef _FILID_HXX_
#define _FILID_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIER_HPP_
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#endif

namespace fileaccess {

    class shell;

    class FileContentIdentifier :
        public cppu::OWeakObject,
        public com::sun::star::lang::XTypeProvider,
        public com::sun::star::ucb::XContentIdentifier
    {

        // This implementation has to be reworked
    public:
        FileContentIdentifier( shell* pMyShell,
                               const rtl::OUString& aUnqPath,
                               sal_Bool IsNormalized = true );

        virtual ~FileContentIdentifier();

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();

        // XTypeProvider
        virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
        getTypes(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
        getImplementationId(
            void )
            throw( com::sun::star::uno::RuntimeException );

        // XContentIdentifier
        virtual rtl::OUString SAL_CALL
        getContentIdentifier(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual rtl::OUString SAL_CALL
        getContentProviderScheme(
            void )
            throw( com::sun::star::uno::RuntimeException );

    private:
        shell* m_pMyShell;
        rtl::OUString m_aContentId;              // The URL string
        rtl::OUString m_aNormalizedId;           // The somehow normalized string
        rtl::OUString m_aProviderScheme;
        sal_Bool      m_bNormalized;
    };

} // end namespace fileaccess


#endif
