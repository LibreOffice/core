/*************************************************************************
 *
 *  $RCSfile: numberformatcode.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bustamam $ $Date: 2001-09-16 15:22:59 $
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

#ifndef _I18N_NUMBERFORMATCODE_HXX_
#define _I18N_NUMBERFORMATCODE_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase1.hxx> // helper for implementations

#include <com/sun/star/i18n/XNumberFormatCode.hpp>
#include <com/sun/star/i18n/XLocaleData.hpp>

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif


class NumberFormatCodeMapper : public cppu::WeakImplHelper1
<
    ::com::sun::star::i18n::XNumberFormatCode
>
{
public:
    NumberFormatCodeMapper( const ::com::sun::star::uno::Reference <
                    ::com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    ~NumberFormatCodeMapper();

    virtual ::com::sun::star::i18n::NumberFormatCode SAL_CALL getDefault( sal_Int16 nFormatType, sal_Int16 nFormatUsage, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::i18n::NumberFormatCode SAL_CALL getFormatCode( sal_Int16 nFormatIndex, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > SAL_CALL getAllFormatCode( sal_Int16 nFormatUsage, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > SAL_CALL getAllFormatCodes( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::lang::Locale aLocale;
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > xMSF;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement > aFormatSeq;
    ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XLocaleData > xlocaleData;
    sal_Bool bFormatsValid;

    void setupLocale( const ::com::sun::star::lang::Locale& rLocale );
    void getFormats( const ::com::sun::star::lang::Locale& rLocale );
    ::rtl::OUString mapElementTypeShortToString(sal_Int16 formatType);
    sal_Int16 mapElementTypeStringToShort(const ::rtl::OUString& formatType);
    ::rtl::OUString mapElementUsageShortToString(sal_Int16 formatUsage);
    sal_Int16 mapElementUsageStringToShort(const ::rtl::OUString& formatUsage);
    void createLocaleDataObject();
};


#endif // _I18N_NUMBERFORMATCODE_HXX_
