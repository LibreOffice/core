/*************************************************************************
 *
 *  $RCSfile: filid.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:53:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _FILID_HXX_
#define _FILID_HXX_

#ifndef _RTL_USTRING_
#include <rtl/ustring>
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
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        release(
            void )
            throw( com::sun::star::uno::RuntimeException );

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
