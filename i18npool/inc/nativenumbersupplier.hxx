/*************************************************************************
 *
 *  $RCSfile: nativenumbersupplier.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 15:43:43 $
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
#ifndef _I18N_NATIVENUMBERSUPPLIER_HXX_
#define _I18N_NATIVENUMBERSUPPLIER_HXX_

#include <drafts/com/sun/star/i18n/XNativeNumberSupplier.hpp>
#include <drafts/com/sun/star/i18n/NativeNumberMode.hpp>
#include <drafts/com/sun/star/i18n/NativeNumberXmlAttributes.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com { namespace sun { namespace star { namespace i18n {

//      ----------------------------------------------------
//      class NativeNumberSupplier
//      ----------------------------------------------------
class NativeNumberSupplier : public cppu::WeakImplHelper2
<
        drafts::com::sun::star::i18n::XNativeNumberSupplier,
        com::sun::star::lang::XServiceInfo
>
{
public:
        NativeNumberSupplier(sal_Bool _useOffset = sal_False) : useOffset(_useOffset) {}

        // Methods
        virtual ::rtl::OUString SAL_CALL getNativeNumberString( const ::rtl::OUString& aNumberString,
                const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nNativeNumberMode )
                throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL isValidNatNum( const ::com::sun::star::lang::Locale& aLocale,
                sal_Int16 nNativeNumberMode )
                throw (::com::sun::star::uno::RuntimeException);

        virtual ::drafts::com::sun::star::i18n::NativeNumberXmlAttributes SAL_CALL convertToXmlAttributes(
                const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nNativeNumberMode )
                throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Int16 SAL_CALL convertFromXmlAttributes(
                const ::drafts::com::sun::star::i18n::NativeNumberXmlAttributes& aAttr )
                throw (::com::sun::star::uno::RuntimeException);

        //XServiceInfo
        virtual rtl::OUString SAL_CALL getImplementationName()
                throw( com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
                throw( com::sun::star::uno::RuntimeException );
        virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
                throw( com::sun::star::uno::RuntimeException );

        // following methods are not for XNativeNumberSupplier, they are for calling from transliterations
        ::rtl::OUString SAL_CALL getNativeNumberString( const ::rtl::OUString& aNumberString,
                const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nNativeNumberMode,
                com::sun::star::uno::Sequence< sal_Int32 >& offset  )
                throw (::com::sun::star::uno::RuntimeException);
        sal_Unicode SAL_CALL getNativeNumberChar( const sal_Unicode inChar,
                const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nNativeNumberMode )
                throw(com::sun::star::uno::RuntimeException) ;
private:
        ::com::sun::star::lang::Locale aLocale;
        sal_Bool useOffset;
};

} } } }

#endif
