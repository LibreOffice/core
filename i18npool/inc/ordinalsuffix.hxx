/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ordinalsuffix.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-19 18:03:43 $
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

#include <com/sun/star/i18n/XOrdinalSuffix.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/Locale.hpp>


namespace com { namespace sun { namespace star { namespace i18n {

class OrdinalSuffix : public cppu::WeakImplHelper2
<
    XOrdinalSuffix,
    com::sun::star::lang::XServiceInfo
>
{
    public:
        OrdinalSuffix(const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF);
        virtual ~OrdinalSuffix();

        // XOrdinalSuffix
        virtual rtl::OUString SAL_CALL getOrdinalSuffix( sal_Int32 nNumber, const com::sun::star::lang::Locale &rLocale) throw(com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual rtl::OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw(com::sun::star::uno::RuntimeException);
        virtual com::sun::star::uno::Sequence < rtl::OUString > SAL_CALL getSupportedServiceNames() throw(com::sun::star::uno::RuntimeException);

    private:
        com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > _xServiceManager;
};
} } } }
