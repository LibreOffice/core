/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nativenumberwrapper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:33:12 $
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
#ifndef INCLUDED_UNOTOOLSDLLAPI_H
#include "unotools/unotoolsdllapi.h"
#endif

#ifndef _UNOTOOLS_NATIVENUMBERWRAPPER_HXX
#define _UNOTOOLS_NATIVENUMBERWRAPPER_HXX

#ifndef _COM_SUN_STAR_I18N_XNATIVENUMBERSUPPLIER_HPP_
#include <com/sun/star/i18n/XNativeNumberSupplier.hpp>
#endif

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}


class UNOTOOLS_DLLPUBLIC NativeNumberWrapper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XNativeNumberSupplier >   xNNS;
                                // not implemented, prevent usage
                                NativeNumberWrapper( const NativeNumberWrapper& );
            NativeNumberWrapper&    operator=( const NativeNumberWrapper& );

public:
                                NativeNumberWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF
                                    );

                                ~NativeNumberWrapper();

    // Wrapper implementations of XNativeNumberSupplier

    ::rtl::OUString getNativeNumberString(
                        const ::rtl::OUString& rNumberString,
                        const ::com::sun::star::lang::Locale& rLocale,
                        sal_Int16 nNativeNumberMode ) const;

    sal_Bool        isValidNatNum(
                        const ::com::sun::star::lang::Locale& rLocale,
                        sal_Int16 nNativeNumberMode ) const;

    ::com::sun::star::i18n::NativeNumberXmlAttributes convertToXmlAttributes(
                        const ::com::sun::star::lang::Locale& rLocale,
                        sal_Int16 nNativeNumberMode ) const;

    sal_Int16       convertFromXmlAttributes(
                        const ::com::sun::star::i18n::NativeNumberXmlAttributes& rAttr ) const;

};

#endif // _UNOTOOLS_NATIVENUMBERWRAPPER_HXX
